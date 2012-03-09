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
#include "img_convert.h"
#include "capture.h"
#include "jpg_enc.h"
#include "osd.h"

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

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/* Our object */
struct ParamsMngObj {
	AppParams		appParams;		//Params
	CapInputInfo	capInputInfo;	//capture input info, set at run time
	Int32			flags;			//status flags 
	pthread_mutex_t	mutex;			//mutex for lock
	FILE			*fp;			//file for read/write params
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
	FILE *fp = fopen(cfgFile, "wb+");
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
	
	hParamsMng->fp = fp;

	return hParamsMng;

exit:

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
	free(hParamsMng);

	return E_NO;
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

	Int32 err = wait_cap_info_set(hParamsMng);
	if(err) {
		ERR("capture input info has not set.");
		return err;
	}

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
	}else {
		/* Out0 is jpeg */
		params->outAttrs[0].width = appCfg->imgEncParams.width;
		params->outAttrs[0].height = appCfg->imgEncParams.height;
		if( params->outAttrs[0].width == 0 || 
			params->outAttrs[0].height == 0) {
			/* use input width */
			params->outAttrs[0].width = params->inputWidth;
			params->outAttrs[0].height = params->inputHeight;
		}
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
	} else 
		outAttrs->enbale = FALSE;

	return E_NO;
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
		if(appCfg->osdParams.imgOsd.size = CAM_OSD_SIZE_32x16)
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

	Int32 ret = E_NO;
	
	pthread_mutex_lock(&hParamsMng->mutex);

	switch(cmd) {
	case PMCMD_S_AEPARAMS:
		break;
	case PMCMD_G_AEPARAMS:
		break;
	case PMCMD_S_OSDPARAMS:
		ret = set_osd_params(hParamsMng, arg, size);
		break;
	case PMCMD_G_OSDPARAMS:
		ret = get_osd_params(hParamsMng, arg, size);
		break;
	case PMCMD_S_WORKMODE:
		ret = set_work_mode(hParamsMng, arg, size);
		break;
	case PMCMD_G_WORKMODE:
		ret = get_work_mode(hParamsMng, arg, size);
		break;
	case PMCMD_S_CAPINFO:
		ret = set_cap_input_info(hParamsMng, arg, size);
		break;
	case PMCMD_S_IMGADJPARAMS:
		ret = set_img_adj_params(hParamsMng, arg, size);
		break;
	case PMCMD_G_IMGADJPARAMS:
		ret = get_img_adj_params(hParamsMng, arg, size);
		break;
	case PMCMD_G_IMGCONVDYN:
		ret = get_img_conv_dyn(hParamsMng, arg, size);
		break;
	case PMCMD_G_2NDSTREAMATTRS:
		ret = get_stream2_out_attrs(hParamsMng, arg, size);
		break;
	case PMCMD_G_JPGENCDYN:
		ret = get_jpg_enc_dyn(hParamsMng, arg, size);
		break;
	case PMCMD_G_IMGOSDDYN:
		ret = get_img_osd_dyn(hParamsMng, arg, size);
		break;
	default:
		ret = E_UNSUPT;
		ERR("unkown cmd: 0x%X", (unsigned int)cmd);
		break;
	}

	pthread_mutex_unlock(&hParamsMng->mutex);

	return ret;
}


