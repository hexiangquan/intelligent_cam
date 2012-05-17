/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : params_mng.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/6
  Last Modified :
  Description   : params manage module
  Function List :
              params_mng_create
              params_mng_delete
  History       :
  1.Date        : 2012/3/6
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "params_mng.h"
#include "log.h"
#include "crc16.h"
#include "converter.h"
#include "capture.h"
#include "jpg_enc.h"
#include "h264_enc.h"
#include "osd.h"
#include "tcp_upload.h"
#include "ftp_upload.h"
#include "rtp_upload.h"
#include "version.h"
#include "linux/types.h"
#include "encoder.h"
#include "ext_io.h"
#include "img_ctrl.h"
#include <sys/ioctl.h>

/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/

/* Default params */
extern const AppParams c_appParamsDef;

/*----------------------------------------------*
 * external routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * project-wide global variables                *
 *----------------------------------------------*/

/*----------------------------------------------*
 * module-wide global variables                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * constants                                    *
 *----------------------------------------------*/

/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/
#define PM_FLAG_CAPINFO_SET		(1 << 0)
#define PM_CMD_MASK				(0xFFFF0000)

#define IMG_CTRL_DEV			"/dev/imgctrl"
#define IMG_EXTIO_DEV			"/dev/extio"


typedef Int32 (*PmCtrlFxn)(ParamsMngHandle hParamsMng, void *data, Int32 size);

typedef struct {
	ParamsMngCtrlCmd 	cmd;
	PmCtrlFxn			fxn;
}PmCtrlInfo;

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/* Our object */
struct ParamsMngObj {
	AppParams		appParams;		//Params
	CapInputInfo	capInputInfo;	//capture input info, set at run time
	Int32			flags;			//status flags 
	CamWorkStatus	workStatus;		//current work status
	pthread_mutex_t	mutex;			//mutex for lock
	const char		*cfgFile;		//name of cfg file
	FILE			*fp;			//file for read/write params
	int				fdImgCtrl;		//fd for img ctrl dev
	int				fdExtIo;		//fd for ext io dev
};


/*****************************************************************************
 Prototype    : params_mng_create
 Description  : Create module, read params from file
 Input        : const char *cfgFile  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/6
    Author       : Sun
    Modification : Created function

*****************************************************************************/
ParamsMngHandle params_mng_create(const char *cfgFile)
{
	if(!cfgFile) {
		return NULL;
	}

	/* alloc memory */
	ParamsMngHandle hParamsMng = NULL;

	hParamsMng = calloc(1, sizeof(struct ParamsMngObj));
	if(!hParamsMng) {
		ERR("alloc mem failed.");
		return NULL;
	}

	Bool useDefault = TRUE;
	
	/* read file */
	FILE *fp = fopen(cfgFile, "rb");
	if(!fp) {
		ERRSTR("open %s failed:", cfgFile);
	} else {
		Int32 len = fread(&hParamsMng->appParams, sizeof(AppParams), 1, fp);
		if(len != 1) {
			ERR("Read %d, needed: %d", len, sizeof(AppParams));
		} else {
			/* validate data */
			if(hParamsMng->appParams.magicNum != APP_PARAMS_MAGIC) {
				ERR("invalid magic num");
			} else {
				/* do crc */
				Uint8 *buf = (Uint8 *)(&hParamsMng->appParams) + sizeof(Uint32) * 3;
				Uint32 checkSum = crc16(buf, hParamsMng->appParams.dataLen);
				if(checkSum != hParamsMng->appParams.crc) {
					ERR("do check sum failed...");
				} else {
					/* everything is good, use this param */
					useDefault = FALSE;
					//DBG("reading cfg file success...");
				}
			}
			
		}
	}

	if(useDefault) {
		DBG("using default params");
		hParamsMng->appParams = c_appParamsDef;
	}

	/* init mutex */
	if(pthread_mutex_init(&hParamsMng->mutex, NULL)) {
		ERRSTR("init mutex failed");
		goto exit;
	}

	/* open img ctrl dev */
	hParamsMng->fdImgCtrl = open(IMG_CTRL_DEV, O_RDWR);
	if(hParamsMng->fdImgCtrl <= 0)
		ERRSTR("can't open %s", IMG_CTRL_DEV);

	/* open ext io dev */
	hParamsMng->fdExtIo = open(IMG_EXTIO_DEV, O_RDWR);
	if(hParamsMng->fdExtIo <= 0)
		ERRSTR("can't open %s", IMG_EXTIO_DEV);
	
	
	hParamsMng->fp = fp;
	hParamsMng->cfgFile = cfgFile;

	return hParamsMng;

exit:

	if(fp)
		fclose(fp);

	if(hParamsMng)
		free(hParamsMng);

	return NULL;
	
}

/*****************************************************************************
 Prototype    : params_mng_delete
 Description  : Delete module
 Input        : ParamsMngHandle hParamsMng  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/6
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 params_mng_delete(ParamsMngHandle hParamsMng)
{
	if(!hParamsMng)
		return E_INVAL;
	
	if(hParamsMng->fp)
		fclose(hParamsMng->fp);
	
	pthread_mutex_destroy(&hParamsMng->mutex);

	
	/* close img ctrl dev */
	if(hParamsMng->fdImgCtrl > 0)
		close(hParamsMng->fdImgCtrl);

	if(hParamsMng->fdExtIo > 0)
		close(hParamsMng->fdExtIo);
	
	free(hParamsMng);

	return E_NO;
}

/*****************************************************************************
 Prototype    : params_mng_write_back
 Description  : write back current params
 Input        : ParamsMngHandle hParamsMng  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 params_mng_write_back(ParamsMngHandle hParamsMng)
{
	if(!hParamsMng)
		return E_INVAL;

	Int32 ret = E_NO;

	pthread_mutex_lock(&hParamsMng->mutex);
	
	/* write back current params */
	Uint8 *buf = (Uint8 *)(&hParamsMng->appParams) + sizeof(Uint32) * 3;

	hParamsMng->appParams.dataLen = sizeof(AppParams) - 3 * sizeof(Uint32);
	hParamsMng->appParams.crc = crc16(buf, hParamsMng->appParams.dataLen);

	if(!hParamsMng->fp) {
		hParamsMng->fp = fopen(hParamsMng->cfgFile, "wb+");
		if(!hParamsMng->fp) {
			ERRSTR("open cfg file failed");
			ret = E_IO;
			goto exit;
		}	
	}

	//DBG("writing cfg to %s...", hParamsMng->cfgFile);
	ret = fwrite(&hParamsMng->appParams, sizeof(AppParams), 1, hParamsMng->fp);
	if(ret < 0) {
		ERRSTR("write cfg file err");
		ret = E_IO;
	}

exit:
	pthread_mutex_unlock(&hParamsMng->mutex);
	return ret;
	
}

/*****************************************************************************
 Prototype    : set_work_mode
 Description  : Set work mode data
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/6
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 set_work_mode(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size != sizeof(CamWorkMode)) 
		return E_INVAL;

	/* Validate data */
	CamWorkMode *workMode = (CamWorkMode *)data;
	if( workMode->format >= CAM_FMT_MAX || 
		workMode->resType >= CAM_RES_MAX ||
		workMode->capMode >= CAM_CAP_MODE_MAX) {
		ERR("invalid work mode data");
		return E_INVAL;
	}

	hParamsMng->appParams.workMode = *workMode;
	return E_NO;
}

/*****************************************************************************
 Prototype    : get_work_mode
 Description  : Get work mode
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/6
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline Int32 get_work_mode(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(CamWorkMode)) 
		return E_INVAL;

	/* Copy data */
	*(CamWorkMode *)data = hParamsMng->appParams.workMode;
	return E_NO;
}

/*****************************************************************************
 Prototype    : set_osd_params
 Description  : Set cam osd params 
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/8
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 set_osd_params(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size != sizeof(CamOsdParams)) 
		return E_INVAL;

	/* Validate data */
	CamOsdParams *osdParams = (CamOsdParams *)data;
	CamOsdInfo	 *osdInfo = &osdParams->imgOsd;
	if( osdInfo->color >= CAM_OSD_COLOR_MAX ||
		osdInfo->postion >= CAM_OSD_TEXT_POS_MAX ||
		osdInfo->size >= CAM_OSD_SIZE_MAX) {
		ERR("invalid img osd params");
		return E_INVAL;
	}

	Int32 offset = sizeof(osdInfo->osdString);
	osdInfo->osdString[offset - 1] = 0; 
	
	osdInfo = &osdParams->vidOsd;
	if( osdInfo->color >= CAM_OSD_COLOR_MAX ||
		osdInfo->postion >= CAM_OSD_TEXT_POS_MAX ||
		osdInfo->size >= CAM_OSD_SIZE_MAX) {
		ERR("invalid video osd params");
		return E_INVAL;
	}
	osdInfo->osdString[offset - 1] = 0; 
	/* clear flags unsupported by video */
	osdInfo->flags &= CAM_VID_OSD_FLAG_MASK;
	
	hParamsMng->appParams.osdParams = *osdParams;
	return E_NO;
}

/*****************************************************************************
 Prototype    : get_osd_params
 Description  : Get osd params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/8
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_osd_params(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(CamOsdParams)) 
		return E_INVAL;

	/* Copy data */
	*(CamOsdParams *)data = hParamsMng->appParams.osdParams;
	return E_NO;
}

/*****************************************************************************
 Prototype    : set_img_enc_params
 Description  : set img encode params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/15
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 set_img_enc_params(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size != sizeof(CamImgEncParams)) 
		return E_INVAL;

	/* Validate data */
	CamImgEncParams *params = (CamImgEncParams *)data;

	if( params->encQuality > 97 || params->encQuality < 10 || 
		(params->rotation != 0 && params->rotation != 90 && 
		params->rotation != 180 && params->rotation != 270) ||
		!ALIGNED(params->width, 16) || !ALIGNED(params->height, 8)) {
		ERR("invalid img enc params, rotation must be 0, 90 or 270, qaulity must between 10 and 98, width must multi of 16");
		return E_INVAL;
	}

	hParamsMng->appParams.imgEncParams = *params;
	return E_NO;
}

/*****************************************************************************
 Prototype    : get_img_enc_params
 Description  : get img encode params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/15
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_img_enc_params(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(CamImgEncParams)) 
		return E_INVAL;

	/* Copy data */
	*(CamImgEncParams *)data = hParamsMng->appParams.imgEncParams;
	return E_NO;
}

/*****************************************************************************
 Prototype    : get_img_upload_params
 Description  : get image upload params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_img_upload_params(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data) 
		return E_INVAL;

	AppParams 		*appCfg = &hParamsMng->appParams;
	Int32			err = E_NO;
	UploadParams	*params = (UploadParams *)data;
	
	if(size < sizeof(UploadParams)) {
		return E_NOMEM;
	}

	/* set upload protol */
	params->protol = appCfg->uploadCfg.protocol;

	/* set params according to proto */
	switch(appCfg->uploadCfg.protocol) {
		case CAM_UPLOAD_PROTO_FTP: {
			FtpUploadParams *ftpParams = (FtpUploadParams *)params->paramsBuf;
			ftpParams->srvInfo = appCfg->ftpSrvInfo;
			ftpParams->roadInfo = appCfg->roadInfo;
			ftpParams->size = sizeof(FtpUploadParams);
			break;
		}
		case CAM_UPLOAD_PROTO_TCP: {
			ImgTcpUploadParams *tcpParams = (ImgTcpUploadParams *)params->paramsBuf;
			tcpParams->srvInfo = appCfg->tcpImgSrvInfo;
			tcpParams->devInfo = appCfg->devInfo;
			tcpParams->size = sizeof(ImgTcpUploadParams);
			break;
		}
		case CAM_UPLOAD_PROTO_NONE:
		default:
			break;
	}

	return err;
}

/*****************************************************************************
 Prototype    : set_img_adj_params
 Description  : Set params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/6
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 set_img_adj_params(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size != sizeof(CamImgEnhanceParams)) 
		return E_INVAL;

	/* Validate data */
	CamImgEnhanceParams *params = (CamImgEnhanceParams *)data;
	if( params->brightness > 255 || 
		params->sharpness > 255 ||
		params->saturation > 255 ) {
		ERR("invalid img adj data");
		return E_INVAL;
	}

	hParamsMng->appParams.imgAdjParams = *params;

	if(hParamsMng->fdImgCtrl > 0) {
		/* sync with hardware */
		struct hdcam_img_enhance_cfg cfg;
		cfg.flags = 0;
		if(params->flags & CAM_IMG_CONTRAST_EN)
			cfg.flags |= HDCAM_ENH_FLAG_CONTRAST_EN;
		if(params->flags & CAM_IMG_SHARP_EN)
			cfg.flags |= HDCAM_ENH_FLAG_SHARP_EN;
		if(params->flags & CAM_IMG_GAMMA_EN)
			cfg.flags |= HDCAM_ENH_FLAG_GAMMA_EN;
		if(params->flags & CAM_IMG_SAT_EN)
			cfg.flags |= HDCAM_ENH_FLAG_SAT_EN;
		if(params->flags & CAM_IMG_MED_FILTER_EN)
			cfg.flags |= HDCAM_ENH_FLAG_NF_EN;
		if(params->flags & CAM_IMG_DRC_EN)
			cfg.flags |= HDCAM_ENH_FLAG_DRC_EN;
		if(params->flags & CAM_IMG_BRIGHTNESS_EN)
			cfg.flags |= HDCAM_ENH_FLAG_BRIGHT_EN;

		cfg.brightness = params->brightness;
		cfg.contrast = params->contrast;
		cfg.drcStrength = params->drcStrength;
		cfg.saturation = params->saturation;
		cfg.sharpness = params->sharpness;
		cfg.reserved[0] = cfg.reserved[1] = 0;

		if(ioctl(hParamsMng->fdImgCtrl, IMGCTRL_S_ENHCFG, &cfg) < 0) {
			return E_IO;
		}
	}
	
	return E_NO;
}

/*****************************************************************************
 Prototype    : get_img_adj_params
 Description  : get params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/6
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline Int32 get_img_adj_params(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(CamImgEnhanceParams)) 
		return E_INVAL;

	/* Copy data */
	*(CamImgEnhanceParams *)data = hParamsMng->appParams.imgAdjParams;
	return E_NO;
}

/*****************************************************************************
 Prototype    : set_h264_params
 Description  : set h.264 params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/15
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 set_h264_params(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size != sizeof(CamH264Params)) 
		return E_INVAL;

	/* Validate data */
	CamH264Params *params = (CamH264Params *)data;
	if( params->resolution >= H264_RES_MAX || 
		params->frameRate > 60 ||
		params->rateControl >= CAM_H264_RC_MAX ||
		params->QPMin > params->QPMax ||
		params->QPMin > 51 || params->QPMax > 51) {
		ERR("invalid h264 params");
		return E_INVAL;
	}

	if(params->bitRate > CAM_H264_MAX_BIT_RATE) {
		WARN("bit rate too big, set to max value: %d", CAM_H264_MAX_BIT_RATE);
		params->bitRate= CAM_H264_MAX_BIT_RATE;
	}

	hParamsMng->appParams.h264EncParams = *params;
	return E_NO;
}

/*****************************************************************************
 Prototype    : get_h264_params
 Description  : get h.264 params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/15
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_h264_params(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(CamH264Params)) 
		return E_INVAL;

	/* Copy data */
	*(CamH264Params *)data = hParamsMng->appParams.h264EncParams;
	return E_NO;
}

/*****************************************************************************
 Prototype    : get_video_out_attrs
 Description  : Get video out attrs for img convert
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/6
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_video_out_attrs(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(ConvOutAttrs)) 
		return E_INVAL;

	ConvOutAttrs *outAttrs = (ConvOutAttrs *)data;
	AppParams *appCfg = &hParamsMng->appParams;

	switch(appCfg->h264EncParams.resolution) {
	case H264_RES_1920X1080:
		outAttrs->width = 1920;
		outAttrs->height = 1080;
		break;
	case H264_RES_1280X720:
		outAttrs->width = 1280;
		outAttrs->height = 720;
		break;
	case H264_RES_1600X1200:
		outAttrs->width = 1600;
		outAttrs->height = 1200;
		break;
	case H264_RES_720X480:
	default:
		outAttrs->width = 720;
		outAttrs->height = 480;
		break;
	}

	outAttrs->enbale = TRUE;
	outAttrs->pixFmt = FMT_YUV_420SP;
	outAttrs->hFlip = 0;
	outAttrs->vFlip = 0;
	outAttrs->lineLength = 0;

	return E_NO;
}

/*****************************************************************************
 Prototype    : wait_cap_info_set
 Description  : Wait other threads set cap info
 Input        : ParamsMngHandle hParamsMng  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/8
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 wait_cap_info_set(ParamsMngHandle hParamsMng)
{
	Int32 cnt = 0;

	if(hParamsMng->flags & PM_FLAG_CAPINFO_SET)
		return E_NO;

	/* unlock, because we must have got the lock  */
	pthread_mutex_unlock(&hParamsMng->mutex);

	/* wait other threads set this flag */
	while(!(hParamsMng->flags & PM_FLAG_CAPINFO_SET)) {
		usleep(10000);
		if(cnt++ > 10)
			break;
	}

	/* lock again */
	pthread_mutex_lock(&hParamsMng->mutex);
	
	if(cnt > 10)
		return E_BUSY;

	return E_NO;
	
}

/*****************************************************************************
 Prototype    : get_img_conv_dyn
 Description  : Get image convert dynamic params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/6
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_img_conv_dyn(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(ImgConvDynParams)) 
		return E_INVAL;

	#if 0
	Int32 err = wait_cap_info_set(hParamsMng);
	if(err) {
		ERR("capture input info has not set.");
		return err;
	}
	#endif

	ImgConvDynParams *params = (ImgConvDynParams *)data;
	AppParams *appCfg = &hParamsMng->appParams;

	/* Clear data first */
	memset(params, 0, sizeof(ImgConvDynParams));
	params->size = sizeof(ImgConvDynParams);
	params->inputFmt = FMT_BAYER_RGBG;
	
	params->digiGain = appCfg->imgAdjParams.digiGain;
	params->brigtness = appCfg->imgAdjParams.brightness;
	params->contrast = appCfg->imgAdjParams.contrast;
	params->inputFmt = hParamsMng->capInputInfo.colorSpace;
	params->inputWidth = hParamsMng->capInputInfo.width;
	params->inputHeight = hParamsMng->capInputInfo.height;
	
	/* Set control flags */
	if(appCfg->imgAdjParams.flags & CAM_IMG_GAMMA_EN)
		params->ctrlFlags |= CONV_FLAG_GAMMA_EN;
	if(appCfg->imgAdjParams.flags & CAM_IMG_SHARP_EN)
		params->ctrlFlags |= CONV_FLAG_EE_EN;
	if(appCfg->imgAdjParams.flags & CAM_IMG_NF_EN)
		params->ctrlFlags |= CONV_FLAG_NF_EN;
	if(appCfg->imgAdjParams.flags & CAM_IMG_MED_FILTER_EN)
		params->ctrlFlags |= CONV_FLAG_AVG_EN;

	/* Set output params */
	params->outAttrs[0].enbale = TRUE;
	params->outAttrs[1].enbale = FALSE;
	params->outAttrs[0].pixFmt = FMT_YUV_420SP;
	if( appCfg->workMode.format == CAM_FMT_H264 || 
		appCfg->workMode.format == CAM_FMT_JPEG_H264) {
		/* 1st stream is h.264 */
		get_video_out_attrs(hParamsMng, &params->outAttrs[0], sizeof(params->outAttrs[0]));
	}else if(appCfg->workMode.format == CAM_FMT_JPEG){
		/* Out0 is jpeg */
		params->outAttrs[0].width = appCfg->imgEncParams.width;
		params->outAttrs[0].height = appCfg->imgEncParams.height;
		if( params->outAttrs[0].width == 0 || 
			params->outAttrs[0].height == 0) {
			/* use input width */
			params->outAttrs[0].width = params->inputWidth;
			params->outAttrs[0].height = params->inputHeight;
		}
	} else {
		/* raw output, disable convert */
		params->outAttrs[0].enbale = FALSE;
	}

	return E_NO;
} 

/*****************************************************************************
 Prototype    : get_stream2_out_attrs
 Description  : Get stream2 out attrs
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/7
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_stream2_out_attrs(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	Int32 err;
	
	if(!data || size < sizeof(ConvOutAttrs)) 
		return E_INVAL;

	ConvOutAttrs *outAttrs = (ConvOutAttrs *)data;
	AppParams *appCfg = &hParamsMng->appParams;

	bzero(outAttrs, sizeof(*outAttrs));

	if(appCfg->workMode.format == CAM_FMT_JPEG_H264) {
		outAttrs->enbale = TRUE;
		outAttrs->width = appCfg->imgEncParams.width;
		outAttrs->height = appCfg->imgEncParams.height;
		if(!outAttrs->width || !outAttrs->height) {
			err = wait_cap_info_set(hParamsMng);
			if(err)
				return err;
			outAttrs->width = hParamsMng->capInputInfo.width;
			outAttrs->height = hParamsMng->capInputInfo.height;
		}
		outAttrs->pixFmt = FMT_YUV_420SP;
	} else 
		outAttrs->enbale = FALSE;

	return E_NO;
}

/*****************************************************************************
 Prototype    : get_converter_params
 Description  : get converter params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_converter_params(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	Int32 err;
	
	if(!data || size < sizeof(ConverterParams)) 
		return E_INVAL;
	
	ConverterParams *params = (ConverterParams *)data;

	err = get_img_conv_dyn(hParamsMng, &params->convDyn[0], sizeof(params->convDyn[0]));
	if(err)
		return err;

	/* set same params */
	params->convDyn[1] = params->convDyn[0];
	err = get_stream2_out_attrs(hParamsMng, &params->convDyn[1].outAttrs[0], sizeof(ConvOutAttrs));

	return err;
}

/*****************************************************************************
 Prototype    : get_jpg_enc_dyn
 Description  : Get jpeg enc dyn params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/8
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_jpg_enc_dyn(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(JpgEncDynParams)) 
		return E_INVAL;
	
	JpgEncDynParams *dynParams = (JpgEncDynParams *)data;
	AppParams *appCfg = &hParamsMng->appParams;

	dynParams->size = sizeof(JpgEncDynParams);
	dynParams->width = appCfg->imgEncParams.width;
	dynParams->height = appCfg->imgEncParams.height;
	dynParams->quality = appCfg->imgEncParams.encQuality;
	dynParams->rotation = appCfg->imgEncParams.rotation;
	dynParams->inputFormat = FMT_YUV_420SP;

	if(!dynParams->width || !dynParams->height) {
		Int32 err = wait_cap_info_set(hParamsMng);
		if(err) {
			ERR("you should wait cap input info set");
			return err;
		}
		
		/* use input resolution */
		dynParams->width = hParamsMng->capInputInfo.width;
		dynParams->height = hParamsMng->capInputInfo.height;
	}
	
	return E_NO;
}

/*****************************************************************************
 Prototype    : get_h264_enc_dyn
 Description  : get h.264 encode dyn params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_h264_enc_dyn(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(H264EncDynParams)) 
		return E_INVAL;
	
	H264EncDynParams *dynParams = (H264EncDynParams *)data;
	AppParams *appCfg = &hParamsMng->appParams;
	ConvOutAttrs vidOutAttrs;

	/* get video resolution */
	get_video_out_attrs(hParamsMng, &vidOutAttrs, sizeof(vidOutAttrs));

	/* set to default first */
	*dynParams = H264ENC_DYN_DEFAULT;

	/* set according to cfg */
	dynParams->width = vidOutAttrs.width;
	dynParams->height = vidOutAttrs.height;
	dynParams->frameRate = appCfg->h264EncParams.frameRate;
	dynParams->targetBitRate = appCfg->h264EncParams.bitRate * 1000;
	dynParams->intraFrameInterval = appCfg->h264EncParams.IPRatio;
	dynParams->maxQP = appCfg->h264EncParams.QPMax;
	dynParams->minQP = appCfg->h264EncParams.QPMin;
	dynParams->initQP = appCfg->h264EncParams.QPInit;
	dynParams->rateCtrlMode = appCfg->h264EncParams.rateControl;
	if(appCfg->h264EncParams.forceIFrame)
		dynParams->forceFrame = VID_I_FRAME;
	dynParams->maxBitrateCVBR = appCfg->h264EncParams.bitRate * 2000;
	if(dynParams->maxBitrateCVBR > CAM_H264_MAX_BIT_RATE * 1000) 
		dynParams->maxBitrateCVBR = CAM_H264_MAX_BIT_RATE * 1000;

	DBG("bit rate: %d, max %d", dynParams->targetBitRate, dynParams->maxBitrateCVBR);
	
	return E_NO;
}


/*****************************************************************************
 Prototype    : get_img_osd_dyn
 Description  : get osd dyn params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/8
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_img_osd_dyn(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(OsdDynParams)) 
		return E_INVAL;
	
	OsdDynParams *dynParams = (OsdDynParams *)data;
	AppParams *appCfg = &hParamsMng->appParams;

	dynParams->size = sizeof(OsdDynParams);
	dynParams->width = appCfg->imgEncParams.width;
	dynParams->height = appCfg->imgEncParams.height;
	dynParams->imgFormat = FMT_YUV_420SP;
	dynParams->color = appCfg->osdParams.imgOsd.color;
	if(appCfg->imgEncParams.rotation == 90)
		dynParams->mode = OSD_MODE_32x32_ROTATE_R;
	else if(appCfg->imgEncParams.rotation == 270)
		dynParams->mode = OSD_MODE_32x32_ROTATE_L;
	else {
		if(appCfg->osdParams.imgOsd.size == CAM_OSD_SIZE_32x16)
			dynParams->mode = OSD_MODE_32x16;
		else
			dynParams->mode = OSD_MODE_32x32; 
	}
	
	if(!dynParams->width || !dynParams->height) {
		Int32 err = wait_cap_info_set(hParamsMng);
		if(err) {
			ERR("you should wait cap input info set");
			return err;
		}
		
		/* use input resolution */
		dynParams->width = hParamsMng->capInputInfo.width;
		dynParams->height = hParamsMng->capInputInfo.height;
	}
	
	return E_NO;
}

/*****************************************************************************
 Prototype    : get_vid_osd_dyn
 Description  : get osd params for video
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_vid_osd_dyn(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(OsdDynParams)) 
		return E_INVAL;
	
	OsdDynParams *dynParams = (OsdDynParams *)data;
	AppParams *appCfg = &hParamsMng->appParams;
	ConvOutAttrs vidOutAttrs;

	/* get video out attrs */
	get_video_out_attrs(hParamsMng, &vidOutAttrs, sizeof(vidOutAttrs));

	dynParams->size = sizeof(OsdDynParams);
	dynParams->width = vidOutAttrs.width;
	dynParams->height = vidOutAttrs.height;
	dynParams->imgFormat = FMT_YUV_420SP;
	dynParams->color = appCfg->osdParams.vidOsd.color;
	if(appCfg->osdParams.vidOsd.size == CAM_OSD_SIZE_32x16)
		dynParams->mode = OSD_MODE_32x16;
	else
		dynParams->mode = OSD_MODE_32x32;
	
	return E_NO;
}

/*****************************************************************************
 Prototype    : set_cap_input_info
 Description  : Set capture info
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/8
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 set_cap_input_info(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size != sizeof(CapInputInfo)) 
		return E_INVAL;

	hParamsMng->capInputInfo = *(CapInputInfo *)data;
	hParamsMng->flags |= PM_FLAG_CAPINFO_SET;

	assert(hParamsMng->capInputInfo.width && hParamsMng->capInputInfo.height);
	
	return E_NO;
}

/*****************************************************************************
 Prototype    : get_cap_input_info
 Description  : get capture input info
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_cap_input_info(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(CamInputInfo)) 
		return E_INVAL;

	/* if capture info is not set, tell app to call later */
	if(!(hParamsMng->flags & PM_FLAG_CAPINFO_SET))
		return E_AGAIN;
	
	*(CapInputInfo *)data = hParamsMng->capInputInfo;
	
	return E_NO;
}


/*****************************************************************************
 Prototype    : get_version_info
 Description  : get software version
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_version_info(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(CamVersionInfo)) 
		return E_INVAL;

	/* Copy data */
	CamVersionInfo *version = (CamVersionInfo *)data;
	version->armVer = ICAM_VERSION;
	version->dspVer = ICAM_VERSION;

	if(hParamsMng->fdImgCtrl > 0) {
		/* read from driver */
		if(ioctl(hParamsMng->fdImgCtrl, IMGCTRL_G_VER, &version->fpgaVer) < 0) {
			return E_IO;
		}
	}
	
	return E_NO;
}

/*****************************************************************************
 Prototype    : get_work_status
 Description  : get  work status
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_work_status(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(CamWorkStatus)) 
		return E_INVAL;

	/* Copy data */
	*(CamWorkStatus *)data = hParamsMng->workStatus;
	
	return E_NO;
}

/*****************************************************************************
 Prototype    : set_work_status
 Description  : set work status
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 set_work_status(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size != sizeof(CamWorkStatus)) 
		return E_INVAL;

	if(*(CamWorkStatus *)data >= WORK_STATUS_MAX)
		return E_INVAL;

	/* Copy data */
	hParamsMng->workStatus = *(CamWorkStatus *)data;
	
	return E_NO;
}

/*****************************************************************************
 Prototype    : set_network_info
 Description  : set network info
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 set_network_info(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size != sizeof(CamNetworkInfo)) 
		return E_INVAL;

	CamNetworkInfo *info = (CamNetworkInfo *)data;
	AppParams *appCfg = &hParamsMng->appParams;
	
	/* Copy data */
	appCfg->networkInfo = *info;

	return E_NO;
}

/*****************************************************************************
 Prototype    : get_network_info
 Description  : get network info
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_network_info(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(CamNetworkInfo)) 
		return E_INVAL;

	AppParams *appCfg = &hParamsMng->appParams;
	
	/* Copy data */
	*(CamNetworkInfo *)data = appCfg->networkInfo;

	return E_NO;
}

/*****************************************************************************
 Prototype    : set_dev_info
 Description  : set device info
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 set_dev_info(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size != sizeof(CamDeviceInfo)) 
		return E_INVAL;

	CamDeviceInfo *info = (CamDeviceInfo *)data;
	AppParams *appCfg = &hParamsMng->appParams;
	
	/* Copy data */
	appCfg->devInfo = *info;

	return E_NO;
}

/*****************************************************************************
 Prototype    : get_dev_info
 Description  : get device info
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_dev_info(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(CamDeviceInfo)) 
		return E_INVAL;

	AppParams *appCfg = &hParamsMng->appParams;
	
	/* Copy data */
	*(CamDeviceInfo *)data = appCfg->devInfo;

	return E_NO;
}

/*****************************************************************************
 Prototype    : set_road_info
 Description  : set road info
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 set_road_info(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size != sizeof(CamRoadInfo)) 
		return E_INVAL;

	CamRoadInfo *info = (CamRoadInfo *)data;
	AppParams *appCfg = &hParamsMng->appParams;

	/* nothing to validate */
	
	/* Copy data */
	appCfg->roadInfo = *info;

	return E_NO;
}

/*****************************************************************************
 Prototype    : get_road_info
 Description  : get road info
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_road_info(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(CamRoadInfo)) 
		return E_INVAL;

	AppParams *appCfg = &hParamsMng->appParams;
	
	/* Copy data */
	*(CamRoadInfo *)data = appCfg->roadInfo;

	return E_NO;
}

/*****************************************************************************
 Prototype    : set_rtp_params
 Description  : set rtp params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 set_rtp_params(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size != sizeof(CamRtpParams)) 
		return E_INVAL;

	CamRtpParams *params = (CamRtpParams *)data;
	AppParams *appCfg = &hParamsMng->appParams;

	/* validate data */
	if(!params->rtspSrvPort) {
		ERR("invalid rtp params");
		return E_INVAL;
	}
	
	/* Copy data */
	appCfg->rtpParams = *params;
	appCfg->rtpParams.streamName[15] = '\0'; //just in case

	return E_NO;
}

/*****************************************************************************
 Prototype    : get_rtp_params
 Description  : get rtp params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_rtp_params(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(CamRtpParams)) 
		return E_INVAL;

	AppParams *appCfg = &hParamsMng->appParams;
	
	/* Copy data */
	*(CamRtpParams *)data = appCfg->rtpParams;

	return E_NO;
}

/*****************************************************************************
 Prototype    : set_tcp_srv_info
 Description  : set tcp server info
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 set_tcp_srv_info(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size != sizeof(CamTcpImageServerInfo)) 
		return E_INVAL;

	CamTcpImageServerInfo *params = (CamTcpImageServerInfo *)data;
	AppParams *appCfg = &hParamsMng->appParams;

	/* validate data */
	if(!params->serverPort) {
		ERR("invalid rtp params");
		return E_INVAL;
	}
	
	/* Copy data */
	appCfg->tcpImgSrvInfo = *params;

	return E_NO;
}

/*****************************************************************************
 Prototype    : get_tcp_srv_info
 Description  : get tcp srv info
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_tcp_srv_info(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(CamTcpImageServerInfo)) 
		return E_INVAL;

	AppParams *appCfg = &hParamsMng->appParams;
	
	/* Copy data */
	*(CamTcpImageServerInfo *)data = appCfg->tcpImgSrvInfo;

	return E_NO;
}

/*****************************************************************************
 Prototype    : set_ftp_srv_info
 Description  : set ftp server info
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 set_ftp_srv_info(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size != sizeof(CamFtpImageServerInfo)) 
		return E_INVAL;

	CamFtpImageServerInfo *params = (CamFtpImageServerInfo *)data;
	AppParams *appCfg = &hParamsMng->appParams;

	/* validate data */
	if(!params->serverPort) {
		ERR("invalid ftp params");
		return E_INVAL;
	}
	
	/* Copy data */
	appCfg->ftpSrvInfo = *params;

	return E_NO;
}

/*****************************************************************************
 Prototype    : get_ftp_srv_info
 Description  : get ftp server info
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_ftp_srv_info(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(CamFtpImageServerInfo)) 
		return E_INVAL;

	AppParams *appCfg = &hParamsMng->appParams;
	
	/* Copy data */
	*(CamFtpImageServerInfo *)data = appCfg->ftpSrvInfo;

	return E_NO;
}

/*****************************************************************************
 Prototype    : set_ntp_srv_info
 Description  : set ntp server info
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 set_ntp_srv_info(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size != sizeof(CamNtpServerInfo)) 
		return E_INVAL;

	CamNtpServerInfo *params = (CamNtpServerInfo *)data;
	AppParams *appCfg = &hParamsMng->appParams;

	/* validate data */
	if(!params->serverPort) {
		ERR("invalid ntp params");
		return E_INVAL;
	}
	
	/* Copy data */
	appCfg->ntpSrvInfo = *params;

	return E_NO;
}

/*****************************************************************************
 Prototype    : get_ntp_srv_info
 Description  : get ntp server info
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_ntp_srv_info(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(CamNtpServerInfo)) 
		return E_INVAL;

	AppParams *appCfg = &hParamsMng->appParams;
	
	/* Copy data */
	*(CamNtpServerInfo *)data = appCfg->ntpSrvInfo;

	return E_NO;
}

/*****************************************************************************
 Prototype    : set_exposure_params
 Description  : set exposure params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 set_exposure_params(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size != sizeof(CamExprosureParam)) 
		return E_INVAL;

	CamExprosureParam *params = (CamExprosureParam *)data;
	AppParams *appCfg = &hParamsMng->appParams;

	/* validate data */
	if(params->globalGain > 1023) {
		ERR("invalid exposure params");
		return E_INVAL;
	}
	
	/* Copy data */
	appCfg->exposureParams = *params;

	if(hParamsMng->fdImgCtrl > 0) {
		/* sync with hardware */
		struct hdcam_lum_info info;

		info.exposureTime = params->shutterTime;
		info.globalGain = params->globalGain;
		info.apture = 0;
		info.reserved = 0;
		
		if(ioctl(hParamsMng->fdImgCtrl, IMGCTRL_S_LUM, &info) < 0) {
			return E_IO;
		}
	}

	return E_NO;
}

/*****************************************************************************
 Prototype    : get_exposure_params
 Description  : get exposure params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_exposure_params(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(CamExprosureParam)) 
		return E_INVAL;

	AppParams *appCfg = &hParamsMng->appParams;

	if(hParamsMng->fdImgCtrl > 0) {	
		/* read from hardware */
		struct hdcam_lum_info info;
		CamExprosureParam *params = (CamExprosureParam *)data;
		
		if(ioctl(hParamsMng->fdImgCtrl, IMGCTRL_G_LUM, &info) < 0) {
			return E_IO;
		}
		
		params->shutterTime = info.exposureTime;
		params->globalGain = info.globalGain;
		params->reserved = 0;
	} else {
	
		/* Copy data */
		*(CamExprosureParam *)data = appCfg->exposureParams;
	}

	return E_NO;
}

/*****************************************************************************
 Prototype    : set_rgb_gains
 Description  : set rgb gains
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 set_rgb_gains(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size != sizeof(CamRGBGains)) 
		return E_INVAL;

	CamRGBGains *params = (CamRGBGains *)data;
	AppParams *appCfg = &hParamsMng->appParams;

	/* validate data */
	if(params->redGain > 512 || params->blueGain > 512) {
		ERR("invalid exposure params");
		return E_INVAL;
	}
	
	/* Copy data */
	appCfg->rgbGains = *params;

	if(hParamsMng->fdImgCtrl > 0) {
		/* sync with hardware */
		struct hdcam_chroma_info info;

		info.redGain = params->redGain;
		info.greenGain = params->greenGain[0];
		info.blueGain = params->blueGain;
		info.reserved = 0;
		
		if(ioctl(hParamsMng->fdImgCtrl, IMGCTRL_S_CHROMA, &info) < 0) {
			return E_IO;
		}
	}

	return E_NO;
}

/*****************************************************************************
 Prototype    : get_rgb_gains
 Description  : get rgb gains
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_rgb_gains(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(CamRGBGains)) 
		return E_INVAL;

	AppParams *appCfg = &hParamsMng->appParams;

	if(hParamsMng->fdImgCtrl > 0) {	
		/* read from driver */
		struct hdcam_chroma_info info;
		CamRGBGains *params = (CamRGBGains *)data;
		
		if(ioctl(hParamsMng->fdImgCtrl, IMGCTRL_G_CHROMA, &info) < 0) {
			return E_IO;
		}
		
		params->redGain = info.redGain;
		params->greenGain[0] = info.greenGain;
		params->greenGain[1] = info.greenGain;
		params->blueGain = info.blueGain;
	} else {
		/* Copy data */
		*(CamRGBGains *)data = appCfg->rgbGains;
	}
	
	return E_NO;
}

/*****************************************************************************
 Prototype    : validate_region
 Description  : validate region
 Input        : ParamsMngHandle hParamsMng  
                CamRect *rect               
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 validate_region(ParamsMngHandle hParamsMng, CamRect *rect)
{
	if( rect->startX > rect->endX || 
		rect->startY > rect->endY ||
		rect->startX > hParamsMng->capInputInfo.width || 
		rect->startY > hParamsMng->capInputInfo.height) {
		ERR("invalid region");
			return E_INVAL;
	}

	return E_NO;
}

/*****************************************************************************
 Prototype    : set_light_regions
 Description  : set traffic light regions
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 set_light_regions(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size != sizeof(CamTrafficLightRegions)) 
		return E_INVAL;

	CamTrafficLightRegions *params = (CamTrafficLightRegions *)data;
	AppParams *appCfg = &hParamsMng->appParams;

	/* validate data */
	Int32  i;
	for(i = 0; i < 3; i++) {
		CamRect *rect = &params->region[i];
		if(validate_region(hParamsMng, rect))
			return E_INVAL;
	}
	
	/* Copy data */
	appCfg->correctRegs = *params;

	if(hParamsMng->fdImgCtrl > 0) {
		/* reserved */
	}

	return E_NO;
}

/*****************************************************************************
 Prototype    : get_light_regions
 Description  : get traffic light regions
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_light_regions(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(CamTrafficLightRegions)) 
		return E_INVAL;

	AppParams *appCfg = &hParamsMng->appParams;
	
	/* Copy data */
	*(CamTrafficLightRegions *)data = appCfg->correctRegs;

	return E_NO;
}

/*****************************************************************************
 Prototype    : set_io_cfg
 Description  : set IO config params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/15
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 set_io_cfg(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size != sizeof(CamIoCfg)) 
		return E_INVAL;

	CamIoCfg *params = (CamIoCfg *)data;
	AppParams *appCfg = &hParamsMng->appParams;

	/* validate data */
	
	/* Copy data */
	appCfg->ioCfg = *params;

	if(hParamsMng->fdExtIo > 0) {
		/* sync with hardware */
		struct hdcam_io_info info;

		info.direction = params->direction;
		info.status = params->status;
		info.pos_intr = 0;
		info.neg_intr = 0;
		
		if(ioctl(hParamsMng->fdExtIo, EXTIO_S_GPIO, &info) < 0) {
			return E_IO;
		}
	}

	return E_NO;
}

/*****************************************************************************
 Prototype    : get_io_cfg
 Description  : get io config params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/15
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_io_cfg(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(CamIoCfg)) 
		return E_INVAL;

	AppParams *appCfg = &hParamsMng->appParams;

	if(hParamsMng->fdExtIo > 0) {
		/* read from hardware */
		struct hdcam_io_info info;
		CamIoCfg *cfg = (CamIoCfg *)data;

		if(ioctl(hParamsMng->fdExtIo, EXTIO_G_GPIO, &info) < 0) {
			return E_IO;
		}

		cfg->direction = info.direction;
		cfg->status = info.status;
	} else {
		/* Copy data */
		*(CamIoCfg *)data = appCfg->ioCfg;
	}

	return E_NO;
}

/*****************************************************************************
 Prototype    : set_strobe_params
 Description  : set strobe params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/15
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 set_strobe_params(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size != sizeof(CamStrobeCtrlParam)) 
		return E_INVAL;

	CamStrobeCtrlParam *params = (CamStrobeCtrlParam *)data;
	AppParams *appCfg = &hParamsMng->appParams;

	/* validate data */
	if( params->switchMode >= CAM_STROBE_SWITCH_MAX || 
		params->enStartHour > 23 || 
		params->enStartMin > 59 ||
		params->enEndHour > 23 || 
		params->enEndMin > 59) {
		ERR("invalid strobe params, time must be valid, hour[0-23], min[0-59]");
		return E_INVAL;
	}
	
	/* Copy data */
	appCfg->strobeParams = *params;

	if(hParamsMng->fdExtIo > 0) {
		/* sync with hardware */
		struct hdcam_strobe_info info;

		info.status = params->ctrlFlags;
		info.offset = params->offset;
		
		if(ioctl(hParamsMng->fdExtIo, EXTIO_S_STROBE, &info) < 0) {
			return E_IO;
		}
	}

	return E_NO;
}

/*****************************************************************************
 Prototype    : get_strobe_params
 Description  : get strobe params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/15
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_strobe_params(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(CamStrobeCtrlParam)) 
		return E_INVAL;

	AppParams *appCfg = &hParamsMng->appParams;

	/* Copy data */
	*(CamStrobeCtrlParam *)data = appCfg->strobeParams;
	
	if(hParamsMng->fdExtIo > 0) {
		/* read from hardware */
		struct hdcam_strobe_info info;
		CamStrobeCtrlParam *params = (CamStrobeCtrlParam *)data;

		if(ioctl(hParamsMng->fdExtIo, EXTIO_G_STROBE, &info) < 0) {
			return E_IO;
		}

		params->status = info.status;
		params->offset = info.offset;
	}

	return E_NO;
}

/*****************************************************************************
 Prototype    : set_detector_params
 Description  : set detector params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/15
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 set_detector_params(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size != sizeof(CamDetectorParam)) 
		return E_INVAL;

	CamDetectorParam *params = (CamDetectorParam *)data;
	AppParams *appCfg = &hParamsMng->appParams;

	/* validate data */
	if( params->detecotorId >= DETECTOR_MAX ) {
		ERR("invalid detector id");
		return E_INVAL;
	}
	
	/* Copy data */
	appCfg->detectorParams = *params;

	return E_NO;
}

/*****************************************************************************
 Prototype    : get_detector_params
 Description  : get detector params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/15
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_detector_params(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(CamDetectorParam)) 
		return E_INVAL;

	AppParams *appCfg = &hParamsMng->appParams;
	
	/* Copy data */
	*(CamDetectorParam *)data = appCfg->detectorParams;

	return E_NO;
}

/*****************************************************************************
 Prototype    : set_upload_proto
 Description  : set upload protocol
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 set_upload_cfg(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size != sizeof(CamImgUploadCfg)) 
		return E_INVAL;

	CamImgUploadCfg *cfg = (CamImgUploadCfg *)data;
	AppParams *appCfg = &hParamsMng->appParams;

	/* validate data */
	if(cfg->protocol >= CAM_UPLOAD_PROTO_MAX) {
		ERR("invalid upload proto type");
		return E_INVAL;
	}
	
	/* Copy data */
	appCfg->uploadCfg = *cfg;

	return E_NO;
}

/*****************************************************************************
 Prototype    : get_upload_proto
 Description  : get upload protocol
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_upload_cfg(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(CamImgUploadCfg)) 
		return E_INVAL;

	AppParams *appCfg = &hParamsMng->appParams;
	
	/* Copy data */
	*(CamImgUploadCfg *)data = appCfg->uploadCfg;

	return E_NO;
}

/*****************************************************************************
 Prototype    : set_ae_params
 Description  : set auto exposure params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/15
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 set_ae_params(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size != sizeof(CamAEParam)) 
		return E_INVAL;

	CamAEParam *params = (CamAEParam *)data;
	AppParams *appCfg = &hParamsMng->appParams;

	/* validate data */
	if( params->targetValue >= 255 || 
		params->minShutterTime > params->maxShutterTime ||
		params->minGainValue > params->maxGainValue || 
		params->minAperture > params->maxAperture) {
		ERR("invalid ae params");
		return E_INVAL;
	}

	/* validate region */
	Int32 i;
	for(i = 0; i < CAM_AE_MAX_ROI_NUM; i++) {
		if(validate_region(hParamsMng, &params->roi[i]))
			return E_INVAL;
	}
	
	/* Copy data */
	appCfg->aeParams = *params;

	
	if(hParamsMng->fdImgCtrl > 0) {
		/* sync with hardware */
		struct hdcam_ab_cfg cfg;

		cfg.flags = params->flags;
		cfg.targetValue = params->targetValue;
		cfg.minShutterTime = params->minShutterTime;
		cfg.maxShutterTime = params->maxShutterTime;
		cfg.minAperture = params->minAperture;
		cfg.maxAperture = params->maxAperture;
		for(i = 0; i < HDCAM_AB_MAX_ROI; i++) {
			memcpy(&cfg.roi[i], &params->roi[i], sizeof(cfg.roi[i]));
		}
		
		if(ioctl(hParamsMng->fdImgCtrl, IMGCTRL_S_ABCFG, &cfg) < 0) {
			return E_IO;
		}
	}

	return E_NO;
}

/*****************************************************************************
 Prototype    : get_ae_params
 Description  : get auto exposure params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/15
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_ae_params(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(CamAEParam)) 
		return E_INVAL;

	AppParams *appCfg = &hParamsMng->appParams;
	
	/* Copy data */
	*(CamAEParam *)data = appCfg->aeParams;

	return E_NO;
}

/*****************************************************************************
 Prototype    : set_awb_params
 Description  : set awb params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/15
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 set_awb_params(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size != sizeof(CamAWBParam)) 
		return E_INVAL;

	CamAWBParam *params = (CamAWBParam *)data;
	AppParams *appCfg = &hParamsMng->appParams;

	/* validate data */
	if( params->minValueR >= params->maxValueR || 
		params->minValueG >= params->maxValueG ||
		params->minValueB >= params->maxValueB ) {
		ERR("invalid awb params");
		return E_INVAL;
	}

	/* Copy data */
	appCfg->awbParams = *params;

	if(hParamsMng->fdImgCtrl > 0) {
		/* sync with hardware */
		struct hdcam_awb_cfg cfg;

		cfg.flags = params->flags;
		cfg.minRedGain = params->minValueR;
		cfg.maxRedGain = params->maxValueR;
		cfg.maxGreenGain = params->maxValueG;
		cfg.minGreenGain = params->minValueG;
		cfg.maxBlueGain = params->maxValueB;
		cfg.minBlueGain = params->minValueB;
		cfg.redModifyRatio = params->redModifyRatio;
		cfg.greenModifyRatio = params->greenModifyRatio;
		cfg.blueModifyRatio = params->blueModifyRatio;
		cfg.initRedGain[0] = params->initValueR[0];
		cfg.initRedGain[1] = params->initValueR[1];
		cfg.initGreenGain[0] = params->initValueG[0];
		cfg.initGreenGain[1] = params->initValueG[1];
		cfg.initBlueGain[0] = params->initValueB[0];
		cfg.initBlueGain[1] = params->initValueB[1];
		
		if(ioctl(hParamsMng->fdImgCtrl, IMGCTRL_S_AWBCFG, &cfg) < 0) {
			return E_IO;
		}
	}

	return E_NO;
}

/*****************************************************************************
 Prototype    : get_awb_params
 Description  : get awb params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/15
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_awb_params(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(CamAWBParam)) 
		return E_INVAL;

	AppParams *appCfg = &hParamsMng->appParams;
	
	/* Copy data */
	*(CamAWBParam *)data = appCfg->awbParams;

	return E_NO;
}

/*****************************************************************************
 Prototype    : set_day_night_params
 Description  : set day night mode params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/15
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 set_day_night_params(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size != sizeof(CamDayNightModeCfg)) 
		return E_INVAL;

	CamDayNightModeCfg *params = (CamDayNightModeCfg *)data;
	AppParams *appCfg = &hParamsMng->appParams;

	/* validate data */
	if( params->switchMethod >= CAM_DN_SWT_MAX || 
		params->dayModeStartHour > 23 ||
		params->dayModeStartMin > 59 ||
		params->nightModeStartHour > 23 || 
		params->nightModeStartMin > 59 ) {
		ERR("invalid day night params");
		return E_INVAL;
	}

	/* Copy data */
	appCfg->dayNightCfg = *params;

	return E_NO;
}

/*****************************************************************************
 Prototype    : set_day_night_params
 Description  : get day night mode params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/15
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_day_night_params(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(CamAWBParam)) 
		return E_INVAL;

	AppParams *appCfg = &hParamsMng->appParams;
	
	/* Copy data */
	*(CamDayNightModeCfg *)data = appCfg->dayNightCfg;

	return E_NO;
}

/*****************************************************************************
 Prototype    : set_av_params
 Description  : set av params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/15
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 set_av_params(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size != sizeof(CamAVParam)) 
		return E_INVAL;

	CamAVParam *params = (CamAVParam *)data;
	AppParams *appCfg = &hParamsMng->appParams;

	/* validate data */
	if( params->avType >= AV_TYPE_MAX ) {
		ERR("invalid av params");
		return E_INVAL;
	}

	/* Copy data */
	appCfg->avParams = *params;

	return E_NO;
}

/*****************************************************************************
 Prototype    : get_av_params
 Description  : get av params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/15
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_av_params(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(CamAVParam)) 
		return E_INVAL;

	AppParams *appCfg = &hParamsMng->appParams;
	
	/* Copy data */
	*(CamAVParam *)data = appCfg->avParams;

	return E_NO;
}

/*****************************************************************************
 Prototype    : set_spec_cap_params
 Description  : special capture params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/15
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 set_spec_cap_params(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size != sizeof(CamSpecCapParams)) 
		return E_INVAL;

	CamSpecCapParams *params = (CamSpecCapParams *)data;
	AppParams *appCfg = &hParamsMng->appParams;

	/* validate data */
	if( params->globalGain >= 1023 ) {
		ERR("invalid spec cap params");
		return E_INVAL;
	}

	/* Copy data */
	appCfg->specCapParams = *params;

	if(hParamsMng->fdImgCtrl > 0) {
		/* sync with hardware */
		struct hdcam_spec_cap_cfg cfg;

		cfg = *(struct hdcam_spec_cap_cfg *)data;
		
		if(ioctl(hParamsMng->fdImgCtrl, IMGCTRL_S_SEPCCAP, &cfg) < 0) {
			return E_IO;
		}
	}

	return E_NO;
}

/*****************************************************************************
 Prototype    : get_spec_cap_params
 Description  : special capture params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/15
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_spec_cap_params(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(CamSpecCapParams)) 
		return E_INVAL;

	AppParams *appCfg = &hParamsMng->appParams;
	
	/* Copy data */
	*(CamSpecCapParams *)data = appCfg->specCapParams;

	return E_NO;
}

/*****************************************************************************
 Prototype    : get_img_osd_info
 Description  : get osd info for image
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/20
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_img_osd_info(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(CamOsdInfo)) 
		return E_INVAL;

	AppParams *appCfg = &hParamsMng->appParams;
	
	/* Copy data */
	*(CamOsdInfo *)data = appCfg->osdParams.imgOsd;

	return E_NO;
}

/*****************************************************************************
 Prototype    : get_vid_osd_info
 Description  : get osd info for video
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/20
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_vid_osd_info(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(CamOsdInfo)) 
		return E_INVAL;

	AppParams *appCfg = &hParamsMng->appParams;
	
	/* Copy data */
	*(CamOsdInfo *)data = appCfg->osdParams.vidOsd;

	return E_NO;
}

/*****************************************************************************
 Prototype    : get_img_encoder_params
 Description  : get img encoder params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/13
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_img_encoder_params(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(EncoderParams)) 
		return E_INVAL;

	EncoderParams	*params = (EncoderParams *)data;
	Int32			err;

	/* get osd info for img */
	err = get_img_osd_info(hParamsMng, &params->osdInfo, sizeof(params->osdInfo));
	if(err)
		return err;

	/* get osd dyn params for img */
	err = get_img_osd_dyn(hParamsMng, &params->osdDyn, sizeof(params->osdDyn));
	if(err)
		return err;

	/* get img encode dyn params */
	err = get_jpg_enc_dyn(hParamsMng, params->encDynBuf, sizeof(params->encDynBuf));
	
	return err;
}


/*****************************************************************************
 Prototype    : get_vid_encoder_params
 Description  : get h.264 encoder params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/13
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_vid_encoder_params(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(EncoderParams)) 
		return E_INVAL;

	EncoderParams	*params = (EncoderParams *)data;
	Int32			err;

	/* get osd info for img */
	err = get_vid_osd_info(hParamsMng, &params->osdInfo, sizeof(params->osdInfo));
	if(err)
		return err;

	/* get osd dyn params for img */
	err = get_vid_osd_dyn(hParamsMng, &params->osdDyn, sizeof(params->osdDyn));
	if(err)
		return err;

	/* get img encode dyn params */
	err = get_h264_enc_dyn(hParamsMng, params->encDynBuf, sizeof(params->encDynBuf));
	
	return err;
}

/*****************************************************************************
 Prototype    : get_vid_upload_proto
 Description  : get video upload protocol
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/21
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 get_vid_upload_params(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	if(!data || size < sizeof(UploadParams)) 
		return E_INVAL;

	AppParams *appCfg = &hParamsMng->appParams;
	UploadParams *params = (UploadParams *)data;
	RtpUploadParams *rtpParams = (RtpUploadParams *)params->paramsBuf;
	
	if(appCfg->rtpParams.flags & CAM_RTP_FLAG_EN) {
		/* Copy data */
		params->protol = CAM_UPLOAD_PROTO_RTP;
		rtpParams->streamName = appCfg->rtpParams.streamName;
		rtpParams->rtspPort = appCfg->rtpParams.rtspSrvPort;
		rtpParams->fmt = FMT_H264;
		rtpParams->size = sizeof(RtpUploadParams);
	} else {
		/* not send */
		params->protol = CAM_UPLOAD_PROTO_NONE;
	}

	return E_NO;
}

/*****************************************************************************
 Prototype    : restore_default
 Description  : restore to default params
 Input        : ParamsMngHandle hParamsMng  
                void *data                  
                Int32 size                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 restore_default(ParamsMngHandle hParamsMng, void *data, Int32 size)
{
	AppParams *appCfg = &hParamsMng->appParams;
	
	/* Restore to default params */
	INFO("restore to default params!!!");
	*appCfg = c_appParamsDef;

	return E_NO;
}


/* tables for control fxns */
static const PmCtrlInfo g_paramsConfig[] = {
	{.cmd = PMCMD_S_NETWORKINFO, .fxn = set_network_info,},
	{.cmd = PMCMD_G_NETWORKINFO, .fxn = get_network_info,},
	{.cmd = PMCMD_S_DEVINFO, .fxn = set_dev_info,},
	{.cmd = PMCMD_G_DEVINFO, .fxn = get_dev_info,},
	{.cmd = PMCMD_S_OSDPARAMS, .fxn = set_osd_params,},
	{.cmd = PMCMD_G_OSDPARAMS, .fxn = get_osd_params,},
	{.cmd = PMCMD_S_ROADINFO, .fxn = set_road_info,},
	{.cmd = PMCMD_G_ROADINFO, .fxn = get_road_info,},
	{.cmd = PMCMD_S_RTPPARAMS, .fxn = set_rtp_params,},
	{.cmd = PMCMD_G_RTPPARAMS, .fxn = get_rtp_params,},
	{.cmd = PMCMD_S_IMGTRANSPROTO, .fxn = set_upload_cfg,},
	{.cmd = PMCMD_G_IMGTRANSPROTO, .fxn = get_upload_cfg,},
	{.cmd = PMCMD_S_TCPSRVINFO, .fxn = set_tcp_srv_info,},
	{.cmd = PMCMD_G_TCPSRVINFO, .fxn = get_tcp_srv_info,},
	{.cmd = PMCMD_S_FTPSRVINFO, .fxn = set_ftp_srv_info,},
	{.cmd = PMCMD_G_FTPSRVINFO, .fxn = get_ftp_srv_info,},
	{.cmd = PMCMD_S_NTPSRVINFO, .fxn = set_ntp_srv_info,},
	{.cmd = PMCMD_G_NTPSRVINFO, .fxn = get_ntp_srv_info,},
	{.cmd = PMCMD_S_EXPPARAMS, .fxn = set_exposure_params,},
	{.cmd = PMCMD_G_EXPPARAMS, .fxn = get_exposure_params,},
	{.cmd = PMCMD_S_RGBGAINS, .fxn = set_rgb_gains,},
	{.cmd = PMCMD_G_RGBGAINS, .fxn = get_rgb_gains,},
	{.cmd = PMCMD_S_LIGHTCORRECT, .fxn = set_light_regions,},
	{.cmd = PMCMD_G_LIGHTCORRECT, .fxn = get_light_regions,},
	{.cmd = PMCMD_S_IMGADJPARAMS, .fxn = set_img_adj_params,},
	{.cmd = PMCMD_G_IMGADJPARAMS, .fxn = get_img_adj_params,},
	{.cmd = PMCMD_S_H264ENCPARAMS, .fxn = set_h264_params,},
	{.cmd = PMCMD_G_H264ENCPARAMS, .fxn = get_h264_params,},
	{.cmd = PMCMD_S_WORKMODE, .fxn = set_work_mode,},
	{.cmd = PMCMD_G_WORKMODE, .fxn = get_work_mode,},
	{.cmd = PMCMD_S_IMGENCPARAMS, .fxn = set_img_enc_params,},
	{.cmd = PMCMD_G_IMGENCPARAMS, .fxn = get_img_enc_params,},
	{.cmd = PMCMD_S_IOCFG, .fxn = set_io_cfg,},
	{.cmd = PMCMD_G_IOCFG, .fxn = get_io_cfg,},
	{.cmd = PMCMD_S_STROBEPARAMS, .fxn = set_strobe_params,},
	{.cmd = PMCMD_G_STROBEPARAMS, .fxn = get_strobe_params,},
	{.cmd = PMCMD_S_DETECTORPARAMS, .fxn = set_detector_params,},
	{.cmd = PMCMD_G_DETECTORPARAMS, .fxn = get_detector_params,},
	{.cmd = PMCMD_S_AEPARAMS, .fxn = set_ae_params,},
	{.cmd = PMCMD_G_AEPARAMS, .fxn = get_ae_params,},
	{.cmd = PMCMD_S_AWBPARAMS, .fxn = set_awb_params,},
	{.cmd = PMCMD_G_AWBPARAMS, .fxn = get_awb_params,},
	{.cmd = PMCMD_S_DAYNIGHTCFG, .fxn = set_day_night_params,},
	{.cmd = PMCMD_G_DAYNIGHTCFG, .fxn = get_day_night_params,},
	{.cmd = PMCMD_G_VERSION, .fxn = get_version_info,},
	{.cmd = PMCMD_S_WORKSTATUS, .fxn = set_work_status,},
	{.cmd = PMCMD_G_WORKSTATUS, .fxn = get_work_status,},
	{.cmd = PMCMD_S_AVPARAMS, .fxn = set_av_params,},
	{.cmd = PMCMD_G_AVPARAMS, .fxn = get_av_params,},
	{.cmd = PMCMD_S_SPECCAPPARAMS, .fxn = set_spec_cap_params,},
	{.cmd = PMCMD_G_SPECCAPPARAMS, .fxn = get_spec_cap_params,},
	{.cmd = PMCMD_S_RESTOREDEFAULT, .fxn = restore_default,},
	{.cmd = PMCMD_MAX0, .fxn = NULL,},
};

/* tables for params convert fxns */
static const PmCtrlInfo g_paramsConvert[] = {
	{.cmd = PMCMD_G_IMGCONVDYN, .fxn = get_img_conv_dyn,},
	{.cmd = PMCMD_G_2NDSTREAMATTRS, .fxn = get_stream2_out_attrs, },
	{.cmd = PMCMD_G_JPGENCDYN, .fxn = get_jpg_enc_dyn, },
	{.cmd = PMCMD_S_CAPINFO, .fxn = set_cap_input_info, },
	{.cmd = PMCMD_G_CAPINFO, .fxn = get_cap_input_info, },
	{.cmd = PMCMD_G_IMGOSDDYN, .fxn = get_img_osd_dyn, },
	{.cmd = PMCMD_G_H264ENCDYN, .fxn = get_h264_enc_dyn, },
	{.cmd = PMCMD_G_VIDOSDDYN, .fxn = get_vid_osd_dyn, },
	{.cmd = PMCMD_G_IMGUPLOADPARAMS, .fxn = get_img_upload_params, },
	{.cmd = PMCMD_G_IMGOSDINFO, .fxn = get_img_osd_info, },
	{.cmd = PMCMD_G_VIDOSDINFO, .fxn = get_vid_osd_info, },
	{.cmd = PMCMD_G_VIDUPLOADPARAMS, .fxn = get_vid_upload_params,},
	{.cmd = PMCMD_G_CONVTERPARAMS, .fxn = get_converter_params, },
	{.cmd = PMCMD_G_IMGENCODERPARAMS, .fxn = get_img_encoder_params, },
	{.cmd = PMCMD_G_VIDENCODERPARAMS, .fxn = get_vid_encoder_params, },
	{.cmd = PMCMD_MAX1, .fxn = NULL,},
};


/*****************************************************************************
 Prototype    : params_mng_control
 Description  : do params control
 Input        : ParamsMngHandle hParamsMng  
                ParamsMngCtrlCmd cmd        
                void *arg                   
                Int32 size                  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/6
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 params_mng_control(ParamsMngHandle hParamsMng, ParamsMngCtrlCmd cmd, void *arg, Int32 size)
{
	if(!hParamsMng)
		return E_INVAL;

	Int32 		ret;
	Int32 		index;
	PmCtrlFxn	ctrlFxn = NULL;

	if((cmd & PM_CMD_MASK) == PMCMD_BASE0) {
		index = cmd - PMCMD_BASE0;
		if(cmd > PMCMD_MAX0 || g_paramsConfig[index].cmd != cmd) {
			ERR("can't find fxn for cmd: 0x%X", (__u32)cmd);
			return E_UNSUPT;
		}
		ctrlFxn = g_paramsConfig[index].fxn;
	} else if((cmd & PM_CMD_MASK) == PMCMD_BASE1) {
		index = cmd - PMCMD_BASE1;
		if(cmd > PMCMD_MAX1 || g_paramsConvert[index].cmd != cmd) {
			ERR("can't find fxn for cmd: 0x%X", (__u32)cmd);
			return E_UNSUPT;
		}
		ctrlFxn = g_paramsConvert[index].fxn;
	} else {
		ERR("unsupported cmd: 0x%X", (__u32)cmd);
		return E_UNSUPT;
	}

	assert(ctrlFxn);
	pthread_mutex_lock(&hParamsMng->mutex);
	ret = ctrlFxn(hParamsMng, arg, size);
	pthread_mutex_unlock(&hParamsMng->mutex);

	return ret;
}


