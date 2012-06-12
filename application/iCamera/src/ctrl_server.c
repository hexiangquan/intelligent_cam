/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : ctrl_thr.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/15
  Last Modified :
  Description   : ctrl module for params manage 
  Function List :
              ctrl_thr
              ctrl_thr_init
  History       :
  1.Date        : 2012/3/15
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "ctrl_server.h"
#include "log.h"
#include "icam_ctrl.h"
#include "cam_time.h"
#include <pthread.h>
#include "app_msg.h"
#include "data_capture.h"
#include "encoder.h"
#include "jpg_encoder.h"
#include "h264_encoder.h"
#include "params_mng.h"
#include "cap_init.h"
#include "local_upload.h"
#include "ext_io.h"
#include <sys/ioctl.h>

/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/

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
#define CTRL_MSG_BUF_LEN		(2 * 1024 * 1024)
#define EXTIO_DEV				"/dev/extio"

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/* ctrl msg */
typedef struct {
	MsgHeader 	hdr;
	Int8		buf[CTRL_MSG_BUF_LEN];
}CtrlMsg;

/* Private data for this object */
struct CtrlSrvObj{
	ParamsMngHandle 	hParamsMng;		//params manage handle
	CapHandle			hCapture;		//image capture handle
	DataCapHandle		hDataCap;		//data capture handle
	EncoderHandle		hJpgEncoder;	//jpeg encoder object
	EncoderHandle		hH264Encoder;	//H.264 encoder object	
	LocalUploadHandle	hLocalUpload;	//local file upload handle
	MsgHandle			hMsg;			//msg handle for IPC
	CtrlMsg				msgBuf;			//buf for recv msg
	pthread_t			pid;			//pid of thread
	pthread_mutex_t 	encMutex;		//mutex for encoders
	Bool				exit;			//flag for exit
	const char			*msgName;		//our msg name for IPC
};

typedef enum {
	ENCODER_IMG = 0,
	ENCODER_VID,
}EncoderType;

/*****************************************************************************
 Prototype    : cap_info_update
 Description  : update capture input info
 Input        : CtrlSrvHandle hCtrlSrv  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 cap_info_update(CtrlSrvHandle hCtrlSrv)
{
	Int32 ret;
	
	/* get input info and set to param manager */
	CapInputInfo inputInfo;
	ret = capture_get_input_info(hCtrlSrv->hCapture, &inputInfo);
	if(ret)
		return ret;
	
	ret = params_mng_control(hCtrlSrv->hParamsMng, PMCMD_S_CAPINFO, 
			&inputInfo, sizeof(inputInfo));

	return ret;
}

/*****************************************************************************
 Prototype    : conv_params_update
 Description  : update converter params
 Input        : CtrlSrvHandle hCtrlSrv  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 conv_params_update(CtrlSrvHandle hCtrlSrv)
{
	Int32 ret;
	
	/* update params in sub module */
	ConverterParams params;
	ret = params_mng_control(hCtrlSrv->hParamsMng, PMCMD_G_CONVTERPARAMS, 
			&params, sizeof(params));

	if(ret)
		return ret;

	ret = data_capture_set_conv_params(hCtrlSrv->hDataCap, hCtrlSrv->hMsg, &params);

	return ret;
}

/*****************************************************************************
 Prototype    : encoder_params_update
 Description  : update encoder params
 Input        : CtrlSrvHandle hCtrlSrv  
                Bool isImgEnc           
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/13
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 encoder_params_update(CtrlSrvHandle hCtrlSrv, EncoderType type)
{
	Int32				ret;
	EncoderParams 		encParams;
	EncoderHandle		hEncoder;
	ParamsMngCtrlCmd	cmd;

	if(type == ENCODER_IMG) {
		cmd = PMCMD_G_IMGENCODERPARAMS;
		hEncoder = hCtrlSrv->hJpgEncoder;
	} else {
		cmd = PMCMD_G_VIDENCODERPARAMS;
		hEncoder = hCtrlSrv->hH264Encoder;
	}

	/* update params in encoder */
	ret = params_mng_control(hCtrlSrv->hParamsMng, cmd, &encParams, sizeof(encParams));
	assert(ret == E_NO);
	
	ret = encoder_set_enc_params(hEncoder, hCtrlSrv->hMsg, &encParams);

	return ret;
}

/*****************************************************************************
 Prototype    : encoder_upload_update
 Description  : update encoder upload
 Input        : CtrlSrvHandle hCtrlSrv  
                Bool isImgEnc           
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/13
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 encoder_upload_update(CtrlSrvHandle hCtrlSrv, EncoderType type)
{
	Int32				ret;
	UploadParams 		uploadParams;
	EncoderHandle		hEncoder;
	ParamsMngCtrlCmd	cmd;

	if(type == ENCODER_IMG) {
		cmd = PMCMD_G_IMGUPLOADPARAMS;
		hEncoder = hCtrlSrv->hJpgEncoder;
	} else {
		cmd = PMCMD_G_VIDUPLOADPARAMS;
		hEncoder = hCtrlSrv->hH264Encoder;
	}

	/* update params in encoder */
	params_mng_control(hCtrlSrv->hParamsMng, cmd, &uploadParams, sizeof(uploadParams));
	ret = encoder_set_upload(hEncoder, hCtrlSrv->hMsg, &uploadParams);

	return ret;
}

/*****************************************************************************
 Prototype    : local_upload_update
 Description  : update local file upload params
 Input        : CtrlSrvHandle hCtrlSrv  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/5/7
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 local_upload_update(CtrlSrvHandle hCtrlSrv)
{
	Int32				ret;
	UploadParams 		uploadParams;
	CamImgUploadCfg		uploadCfg;

	/* update params in local upload */
	params_mng_control(hCtrlSrv->hParamsMng, PMCMD_G_IMGUPLOADPARAMS, 
		&uploadParams, sizeof(uploadParams));

	params_mng_control(hCtrlSrv->hParamsMng, PMCMD_G_IMGTRANSPROTO, 
		&uploadCfg, sizeof(uploadCfg));
	
	ret = local_upload_cfg(hCtrlSrv->hLocalUpload, &uploadParams, uploadCfg.flags, 
				hCtrlSrv->hMsg);

	return ret;
}


/*****************************************************************************
 Prototype    : ctrl_server_thread
 Description  : thread for recv and process request
 Input        : void *arg  
 Output       : None
 Return Value : void
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/15
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static void *ctrl_server_thread(void *arg)
{
	CtrlSrvHandle	hCtrlSrv = (CtrlSrvHandle)arg;
	Int32			ret;

	if(!hCtrlSrv)
		goto exit;

	MsgHeader 	*msgHdr = &hCtrlSrv->msgBuf.hdr;
	Int8		*data = hCtrlSrv->msgBuf.buf;
	Int32		respLen;
	Bool		needResp = TRUE;
	ParamsMngHandle	hParamsMng = hCtrlSrv->hParamsMng;
	Int32		fdExtIO = -1;

	DBG("ctrl thread start...");

	fdExtIO = open(EXTIO_DEV, O_RDWR);
	if(fdExtIO < 0) {
		ERRSTR("open ext io dev failed");
	}

	/* start main loop */
	while(!hCtrlSrv->exit) {		
		/* recv msg */
		ret = msg_recv(hCtrlSrv->hMsg, (MsgHeader *)&hCtrlSrv->msgBuf, sizeof(CtrlMsg), 0);
		if(ret < 0) {
			ERR("ctrl thr recv msg err: %s", str_err(ret));
			continue;
		}

		//DBG("got msg: 0x%X", msgHdr->cmd);
		/* process msg */
		needResp = TRUE;
		respLen = 0;
#if 1
		switch(msgHdr->cmd) {
		case APPCMD_EXIT:
			hCtrlSrv->exit = TRUE;
			needResp = FALSE;
			break;
		case APPCMD_SET_CAP_INPUT:
			ret = cap_info_update(hCtrlSrv);
			needResp = FALSE;
			break;
		case ICAMCMD_G_VERSION:
			ret = params_mng_control(hParamsMng, PMCMD_G_VERSION, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamVersionInfo);
			break;
		case ICAMCMD_G_WORKSTATUS:
			ret = params_mng_control(hParamsMng, PMCMD_G_WORKSTATUS, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamWorkStatus); 
			break;
		case ICAMCMD_G_TEMP:
			ret = ioctl(fdExtIO, EXTIO_G_TMP, data);
			if(ret < 0)
				ret = E_IO;
			respLen = sizeof(Int32); 
			break;
		case ICAMCMD_G_INPUTINFO:
			ret = params_mng_control(hParamsMng, PMCMD_G_CAPINFO, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamInputInfo); 
			break;
		case ICAMCMD_S_DATETIME:
			if(msgHdr->dataLen != sizeof(CamDateTime))
				ret = E_INVAL;
			else
				ret = cam_set_time((CamDateTime *)data);
			break;
		case ICAMCMD_G_DATETIME:
			ret = cam_get_time((CamDateTime *)data);
			respLen = sizeof(CamDateTime); 
			break;
		case ICAMCMD_S_NETWORKINFO:
			ret = params_mng_control(hParamsMng, PMCMD_S_NETWORKINFO, data, msgHdr->dataLen);
			break;
		case ICAMCMD_G_NETWORKINFO:
			ret = params_mng_control(hParamsMng, PMCMD_G_NETWORKINFO, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamNetworkInfo); 
			break;
		case ICAMCMD_S_DEVINFO:
			ret = params_mng_control(hParamsMng, PMCMD_S_DEVINFO, data, msgHdr->dataLen);
			break;
		case ICAMCMD_G_DEVINFO:
			ret = params_mng_control(hParamsMng, PMCMD_G_DEVINFO, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamDeviceInfo);
			break;
		case ICAMCMD_S_OSDPARAMS:
			ret = params_mng_control(hParamsMng, PMCMD_S_OSDPARAMS, data, msgHdr->dataLen);
			if(!ret) {
				/* update encoder params */
				ret = encoder_params_update(hCtrlSrv, ENCODER_IMG);
				ret = encoder_params_update(hCtrlSrv, ENCODER_VID);
			}
			break;	
		case ICAMCMD_G_OSDPARAMS:
			ret = params_mng_control(hParamsMng, PMCMD_G_OSDPARAMS, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamOsdParams);
			break;
		case ICAMCMD_S_ROADINFO:
			ret = params_mng_control(hParamsMng, PMCMD_S_ROADINFO, data, msgHdr->dataLen);
			break;
		case ICAMCMD_G_ROADINFO:
			ret = params_mng_control(hParamsMng, PMCMD_G_ROADINFO, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamRoadInfo);
			break;
		case ICAMCMD_S_RTPPARAMS:
			ret = params_mng_control(hParamsMng, PMCMD_S_RTPPARAMS, data, msgHdr->dataLen);
			if(!ret) {
				ret = encoder_upload_update(hCtrlSrv, ENCODER_VID);
			}
			break;
		case ICAMCMD_G_RTPPARAMS:
			ret = params_mng_control(hParamsMng, PMCMD_G_RTPPARAMS, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamRtpParams);
			break;
		case ICAMCMD_S_UPLOADPROTO:
			ret = params_mng_control(hParamsMng, PMCMD_S_IMGTRANSPROTO, data, msgHdr->dataLen);
			if(!ret) {
				/* update img encoder */
				ret = encoder_upload_update(hCtrlSrv, ENCODER_IMG);
				/* update local file upload params */
				ret = local_upload_update(hCtrlSrv);
			}
			break;
		case ICAMCMD_G_UPLOADPROTO:
			ret = params_mng_control(hParamsMng, PMCMD_G_IMGTRANSPROTO, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamImgUploadCfg);
			break;
		case ICAMCMD_S_IMGSRVINFO:
			ret = params_mng_control(hParamsMng, PMCMD_S_TCPSRVINFO, data, msgHdr->dataLen);
			if(!ret) {
				/* update img encoder */
				ret = encoder_upload_update(hCtrlSrv, ENCODER_IMG);
				/* update local file upload params */
				ret = local_upload_update(hCtrlSrv);
			}
			break;
		case ICAMCMD_G_IMGSRVINFO:
			ret = params_mng_control(hParamsMng, PMCMD_G_TCPSRVINFO, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamTcpImageServerInfo);
			break;
		case ICAMCMD_S_FTPSRVINFO:
			ret = params_mng_control(hParamsMng, PMCMD_S_FTPSRVINFO, data, msgHdr->dataLen);
			if(!ret) {
				/* update img encoder */
				ret = encoder_upload_update(hCtrlSrv, ENCODER_IMG);
				/* update local file upload params */
				ret = local_upload_update(hCtrlSrv);
			}
			break;
		case ICAMCMD_G_FTPSRVINFO:
			ret = params_mng_control(hParamsMng, PMCMD_G_FTPSRVINFO, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamFtpImageServerInfo);
			break;
		case ICAMCMD_S_NTPSRVINFO:
			ret = params_mng_control(hParamsMng, PMCMD_S_NTPSRVINFO, data, msgHdr->dataLen);
			break;
		case ICAMCMD_G_NTPSRVINFO:
			ret = params_mng_control(hParamsMng, PMCMD_G_NTPSRVINFO, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamNtpServerInfo);
			break;
		case ICAMCMD_S_EXPPARAMS:
			ret = params_mng_control(hParamsMng, PMCMD_S_EXPPARAMS, data, msgHdr->dataLen);
			break;
		case ICAMCMD_G_EXPPARAMS:
			ret = params_mng_control(hParamsMng, PMCMD_G_EXPPARAMS, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamExprosureParam);
			break;
		case ICAMCMD_S_RGBGAINS:
			ret = params_mng_control(hParamsMng, PMCMD_S_RGBGAINS, data, msgHdr->dataLen);
			break;
		case ICAMCMD_G_RGBGAINS:
			ret = params_mng_control(hParamsMng, PMCMD_G_RGBGAINS, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamRGBGains);
			break;
		case ICAMCMD_S_TRAFLIGHTREG:
			ret = params_mng_control(hParamsMng, PMCMD_S_LIGHTCORRECT, data, msgHdr->dataLen);
			break;
		case ICAMCMD_G_TRAFLIGHTREG:
			ret = params_mng_control(hParamsMng, PMCMD_G_LIGHTCORRECT, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamTrafficLightRegions);
			break;
		case ICAMCMD_S_IMGENHANCE:
			ret = params_mng_control(hParamsMng, PMCMD_S_IMGADJPARAMS, data, msgHdr->dataLen);
			if(!ret) {
				ret = conv_params_update(hCtrlSrv);
			}
			break;
		case ICAMCMD_G_IMGENHANCE:
			ret = params_mng_control(hParamsMng, PMCMD_G_IMGADJPARAMS, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamImgEnhanceParams);
			break;
		case ICAMCMD_S_H264PARAMS:
			ret = params_mng_control(hParamsMng, PMCMD_S_H264ENCPARAMS, data, msgHdr->dataLen);
			if(!ret) {
				/* update img encoder */
				ret = encoder_params_update(hCtrlSrv, ENCODER_VID);
				/* capture update convert params */
				if(!ret)
					ret = conv_params_update(hCtrlSrv);
			}
			break;
		case ICAMCMD_G_H264PARAMS:
			ret = params_mng_control(hParamsMng, PMCMD_G_H264ENCPARAMS, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamH264Params);
			break;
		case ICAMCMD_S_AVPARAMS:
			ret = params_mng_control(hParamsMng, PMCMD_S_AVPARAMS, data, msgHdr->dataLen);
			break;
		case ICAMCMD_G_AVPARAMS:
			ret = params_mng_control(hParamsMng, PMCMD_G_AVPARAMS, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamAVParam);
			break;
		case ICAMCMD_S_WORKMODE:
			ret = params_mng_control(hParamsMng, PMCMD_S_WORKMODE, data, msgHdr->dataLen);
			if(!ret) {
				/* tell other tasks to update params */
				ret = data_capture_set_work_mode(hCtrlSrv->hDataCap, hCtrlSrv->hMsg, (CamWorkMode *)data);
				usleep(100000);
				ret = encoder_params_update(hCtrlSrv, ENCODER_IMG);
				ret = encoder_params_update(hCtrlSrv, ENCODER_VID);
			}
			break;
		case ICAMCMD_G_WORKMODE:
			ret = params_mng_control(hParamsMng, PMCMD_G_WORKMODE, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamWorkMode);
			break;
		case ICAMCMD_S_IMGENCPARAMS:
			ret = params_mng_control(hParamsMng, PMCMD_S_IMGENCPARAMS, data, msgHdr->dataLen);
			if(!ret) {
				/* tell other tasks to update params */
				ret = encoder_params_update(hCtrlSrv, ENCODER_IMG);
				/* tell capture to update convert params */
				if(!ret)
					ret = conv_params_update(hCtrlSrv);
			}
			break;
		case ICAMCMD_G_IMGENCPARAMS:
			ret = params_mng_control(hParamsMng, PMCMD_G_IMGENCPARAMS, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamImgEncParams);
			break;
		case ICAMCMD_S_SPECCAPPARAMS:
			ret = params_mng_control(hParamsMng, PMCMD_S_SPECCAPPARAMS, data, msgHdr->dataLen);
			break;
		case ICAMCMD_G_SPECCAPPARAMS:
			ret = params_mng_control(hParamsMng, PMCMD_G_SPECCAPPARAMS, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamSpecCapParams);
			break;
		case ICAMCMD_S_IOCFG:
			ret = params_mng_control(hParamsMng, PMCMD_S_IOCFG, data, msgHdr->dataLen);
			break;
		case ICAMCMD_G_IOCFG:
			ret = params_mng_control(hParamsMng, PMCMD_G_IOCFG, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamIoCfg);
			break;
		case ICAMCMD_S_STROBEPARAMS:
			ret = params_mng_control(hParamsMng, PMCMD_S_STROBEPARAMS, data, msgHdr->dataLen);
			break;
		case ICAMCMD_G_STROBEPARAMS:
			ret = params_mng_control(hParamsMng, PMCMD_G_STROBEPARAMS, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamStrobeCtrlParam);
			break;
		case ICAMCMD_S_DETECTORPARAMS:
			ret = params_mng_control(hParamsMng, PMCMD_S_DETECTORPARAMS, data, msgHdr->dataLen);
			if(!ret) {
				/* tell other tasks to update params */
				ret = data_capture_set_detector_params(hCtrlSrv->hDataCap, hCtrlSrv->hMsg, (CamDetectorParam *)data);
			}
			break;
		case ICAMCMD_G_DETECTORPARAMS:
			ret = params_mng_control(hParamsMng, PMCMD_G_DETECTORPARAMS, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamDetectorParam);
			break;
		case ICAMCMD_S_AEPARAMS:
			ret = params_mng_control(hParamsMng, PMCMD_S_AEPARAMS, data, msgHdr->dataLen);
			break;
		case ICAMCMD_G_AEPARAMS:
			ret = params_mng_control(hParamsMng, PMCMD_G_AEPARAMS, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamAEParam);
			break;
		case ICAMCMD_S_AWBPARAMS:
			ret = params_mng_control(hParamsMng, PMCMD_S_AWBPARAMS, data, msgHdr->dataLen);
			break;
		case ICAMCMD_G_AWBPARAMS:
			ret = params_mng_control(hParamsMng, PMCMD_G_AWBPARAMS, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamAWBParam);
			break;
		case ICAMCMD_S_DAYNIGHTMODE:
			ret = params_mng_control(hParamsMng, PMCMD_S_DAYNIGHTCFG, data, msgHdr->dataLen);
			break;
		case ICAMCMD_G_DAYNIGHTMODE:
			ret = params_mng_control(hParamsMng, PMCMD_G_DAYNIGHTCFG, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamDayNightModeCfg);
			break;
		case ICAMCMD_S_CAPCTRL:
			/* tell other tasks to update params */
			ret = data_capture_ctrl(hCtrlSrv->hDataCap, hCtrlSrv->hMsg, *(Int32 *)data);
			break;
		case ICAMCMD_G_SDROOTPATH:
			strcpy(data, FILE_SAVE_PATH);
			respLen = strlen(FILE_SAVE_PATH) + 1;
			ret = E_NO;
			break;
		case ICAMCMD_G_SDDIRINFO:
			/* TBD */
			ret = E_NO;
			break;
		case ICAMCMD_S_SNDDIR:
			/* send all files in a dir */
			ret = local_upload_send(hCtrlSrv->hLocalUpload, TRUE, data, hCtrlSrv->hMsg);
			break;
		case ICAMCMD_S_SNDFILE:
			/* send single file */
			ret = local_upload_send(hCtrlSrv->hLocalUpload, FALSE, data, hCtrlSrv->hMsg);
			break;
		case ICAMCMD_S_RESTORECFG:
			ret = params_mng_control(hParamsMng, PMCMD_S_RESTOREDEFAULT, data, msgHdr->dataLen);
			/* we should reboot after restore params */
			if(!ret)
				ret = app_hdr_msg_send(hCtrlSrv->hMsg, MSG_MAIN, APPCMD_REBOOT, 0, 0);	
			break;
		default:
			ERR("unkown cmd: 0x%X", (unsigned int)msgHdr->cmd);
			ret = E_UNSUPT;
			if(msgHdr->cmd < ICAM_CMD_BASE)
				needResp = FALSE;
			break;
		}
#endif

		/* reponse to request */
		if(needResp) {
			msgHdr->param[0] = ret;		//put operation result as param0
			if(ret < 0)
				msgHdr->dataLen = 0; 	//an error happens, so we need not reply any data
			else
				msgHdr->dataLen = respLen;

			//DBG("reply cmd len: %d, ret: %d", msgHdr->dataLen, (int)msgHdr->param[0]);
			msgHdr->type = MSG_TYPE_RESP;
			/* send back response */
			ret = msg_send(hCtrlSrv->hMsg, NULL, (MsgHeader *)&hCtrlSrv->msgBuf, 0);
		}
	}

exit:

	INFO("ctrl thread exit...");
	pthread_exit(0);
	
}


/*****************************************************************************
 Prototype    : ctrl_server_create
 Description  : create ctrl server
 Input        : CtrlSrvAttrs *attrs  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/22
    Author       : Sun
    Modification : Created function

*****************************************************************************/
CtrlSrvHandle ctrl_server_create(CtrlSrvAttrs *attrs)
{
	CtrlSrvHandle 	hCtrlSrv;
	Int32			ret;
	
	assert(attrs && attrs->cfgFileName);

	hCtrlSrv = calloc(1, sizeof(struct CtrlSrvObj));
	if(!hCtrlSrv) {
		ERR("alloc mem failed...");
		return NULL;
	}

	/* read params */
	hCtrlSrv->hParamsMng = params_mng_create(attrs->cfgFileName);
	if(!hCtrlSrv->hParamsMng) {
		ERR("create params failed.");
		goto exit;
	}

	/* copy attrs */
	hCtrlSrv->msgName = attrs->msgName;

	/* create msg handle  */
	hCtrlSrv->hMsg = msg_create(attrs->msgName, MSG_MAIN, 0);
	if(!hCtrlSrv->hMsg) {
		ERR("create msg handle failed");
		goto exit;
	}

	hCtrlSrv->exit = FALSE;

	/* create data capture module (App module) */
	DataCapAttrs 		dataCapAttrs;

	dataCapAttrs.maxOutWidth = IMG_MAX_WIDTH;
	dataCapAttrs.maxOutHeight = IMG_MAX_HEIGHT;

	ret = params_mng_control(hCtrlSrv->hParamsMng, PMCMD_G_WORKMODE, 
			&dataCapAttrs.workMode, sizeof(dataCapAttrs.workMode));

	/* create image capture module */
	hCtrlSrv->hCapture = cap_module_init(&dataCapAttrs.workMode, NULL);
	if(!hCtrlSrv->hCapture) {
		ERR("create image capture failed");
		goto exit;
	}

	/* set input info */
	ret = cap_info_update(hCtrlSrv);
	assert(ret == E_NO);

	/* create data capture module */
	dataCapAttrs.hCapture = hCtrlSrv->hCapture;
	ret = params_mng_control(hCtrlSrv->hParamsMng, PMCMD_G_DETECTORPARAMS, 
			&dataCapAttrs.detectorParams, sizeof(dataCapAttrs.detectorParams));
	ret |= params_mng_control(hCtrlSrv->hParamsMng, PMCMD_G_CONVTERPARAMS, 
			&dataCapAttrs.convParams, sizeof(dataCapAttrs.convParams));
	assert(ret == E_NO);

	DBG("creating data capture...");
	hCtrlSrv->hDataCap = data_capture_create(&dataCapAttrs);
	if(!hCtrlSrv->hDataCap) {
		ERR("create data capture failed");
		goto exit;
	}

	/* init lock */
	pthread_mutex_init(&hCtrlSrv->encMutex, NULL);
	
	EncoderParams 	encParams;
	UploadParams	uploadParams;

	
	/* get img enc & upload params */
	ret = params_mng_control(hCtrlSrv->hParamsMng, PMCMD_G_IMGENCODERPARAMS, 
			&encParams, sizeof(encParams));
	ret |= params_mng_control(hCtrlSrv->hParamsMng, PMCMD_G_IMGUPLOADPARAMS, 
			&uploadParams, sizeof(uploadParams));
	assert(ret == E_NO);

	/* create jpg encoder */
	DBG("creating img encoder...");
	hCtrlSrv->hJpgEncoder = jpg_encoder_create(&encParams, &uploadParams, &hCtrlSrv->encMutex);
	if(!hCtrlSrv->hJpgEncoder) {
		goto exit;
	}

	/* get upload protol */
	CamImgUploadCfg uploadCfg;
	ret = params_mng_control(hCtrlSrv->hParamsMng, PMCMD_G_IMGTRANSPROTO, 
			&uploadCfg, sizeof(uploadCfg));
	assert(ret == E_NO);

	/* create local upload handle */
	LocalUploadAttrs localAttrs;
	localAttrs.filePath = SD_MNT_PATH;
	localAttrs.flags = uploadCfg.flags;
	localAttrs.maxFileSize = IMG_MAX_WIDTH * IMG_MAX_HEIGHT * 8 / 10;
	localAttrs.msgName = MSG_LOCAL;

	/* using image upload params */
	DBG("creating local upload...");
	hCtrlSrv->hLocalUpload = local_upload_create(&localAttrs, &uploadParams);
	if(!hCtrlSrv->hLocalUpload) {
		goto exit;
	}

	/* get vid enc & upload params */
	ret = params_mng_control(hCtrlSrv->hParamsMng, PMCMD_G_VIDENCODERPARAMS, 
			&encParams, sizeof(encParams));
	ret |= params_mng_control(hCtrlSrv->hParamsMng, PMCMD_G_VIDUPLOADPARAMS, 
			&uploadParams, sizeof(uploadParams));
	assert(ret == E_NO);

	/* create video encoder */
	DBG("creating video encoder...");
	hCtrlSrv->hH264Encoder = h264_encoder_create(&encParams, &uploadParams, &hCtrlSrv->encMutex);
	if(!hCtrlSrv->hH264Encoder) {
		goto exit;
	}	
	
	return hCtrlSrv;

exit:
	ctrl_server_delete(hCtrlSrv, NULL);

	return NULL;

}

/*****************************************************************************
 Prototype    : ctrl_server_run
 Description  : start ctrl server running
 Input        : CtrlSrvHandle hCtrlSrv  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/22
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 ctrl_server_run(CtrlSrvHandle hCtrlSrv)
{
	Int32 err = E_NO;

	if(!hCtrlSrv)
		return E_INVAL;
	
	/* create thread and run data collecting */
	err = pthread_create(&hCtrlSrv->pid, NULL, ctrl_server_thread, hCtrlSrv);
	if(err < 0) {
		ERRSTR("create thread failed...");
		hCtrlSrv->pid = 0;
		return E_NOMEM;
	}

	/* start data capture */
	err = data_capture_run(hCtrlSrv->hDataCap);
	if(err ) {
		ERR("data capture run failed...");
		return err;
	}

#if 1
	/* run encode thread */
	DBG("start running encoders");

	err = encoder_run(hCtrlSrv->hJpgEncoder);
	if(err) {
		ERR("jpg encoder run failed...");
		return err;
	}

	err = encoder_run(hCtrlSrv->hH264Encoder);
	if(err) {
		ERR("h264 encoder run failed...");
		return err;
	}
#endif

	/* run local upload */
	if(hCtrlSrv->hLocalUpload) {
		err = local_upload_run(hCtrlSrv->hLocalUpload);
		if(err) {
			ERR("local upload running failed...");
			return err;
		}
	}

	/* set current status to working */
	CamWorkStatus workStatus = WORK_STATUS_RUNNING;
	params_mng_control(hCtrlSrv->hParamsMng, PMCMD_S_WORKSTATUS, 
		&workStatus, sizeof(workStatus));

	return E_NO;
}


/*****************************************************************************
 Prototype    : ctrl_server_delete
 Description  : delete this module
 Input        : CtrlSrvHandle hCtrlSrv  
                MsgHandle hCurMsg       
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/22
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 ctrl_server_delete(CtrlSrvHandle hCtrlSrv, MsgHandle hCurMsg)
{
	if(!hCtrlSrv)
		return E_INVAL;

	if(hCtrlSrv->pid) {
		/* exit thread */
		if(hCurMsg)
			app_hdr_msg_send(hCurMsg, hCtrlSrv->msgName, APPCMD_EXIT, 0, 0);
		
		/* set exit flag */
		hCtrlSrv->exit = TRUE;
		
		/* wait thread exit */
		pthread_join(hCtrlSrv->pid, NULL);
	}

	/* free msg handle */
	if(hCtrlSrv->hMsg)
		msg_delete(hCtrlSrv->hMsg);

	/* local upload exit */
	if(hCtrlSrv->hLocalUpload)
		local_upload_delete(hCtrlSrv->hLocalUpload, hCurMsg);
	
	/* call encoders exit */
	if(hCtrlSrv->hJpgEncoder)
		encoder_delete(hCtrlSrv->hJpgEncoder, hCurMsg);

	if(hCtrlSrv->hH264Encoder) {
		DBG("deleting h264 encoder");
		encoder_delete(hCtrlSrv->hH264Encoder, hCurMsg);
	}

	/* exit data capture */
	if(hCtrlSrv->hDataCap)
		data_capture_delete(hCtrlSrv->hDataCap, hCurMsg);

	/* delete image capture */
	if(hCtrlSrv->hCapture)
		capture_delete(hCtrlSrv->hCapture);

	pthread_mutex_destroy(&hCtrlSrv->encMutex);

	/* save and free params */
	if(hCtrlSrv->hParamsMng) {
		params_mng_write_back(hCtrlSrv->hParamsMng);
		params_mng_delete(hCtrlSrv->hParamsMng);
	}

	free(hCtrlSrv);

	return E_NO;
}


