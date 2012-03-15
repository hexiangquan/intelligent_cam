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
#include "ctrl_thr.h"
#include "log.h"
#include "icam_ctrl.h"
#include "cam_time.h"

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
#define CTRL_MSG_BUF_LEN		(1 * 1024 * 1024)

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/* ctrl msg */
typedef struct {
	MsgHeader 	hdr;
	Int8		buf[CTRL_MSG_BUF_LEN];
}CtrlMsg;

/* thread environment */
typedef struct {
	ParamsMngHandle hParamsMng;
	MsgHandle		hMsg;
	FrameDispHandle	hDispatch;
	CtrlMsg			*msgBuf;
	Bool			exit;
}CtrlThrEnv;

/*****************************************************************************
 Prototype    : ctrl_thr_init
 Description  : init ctrl thread
 Input        : CtrlThrArg *arg   
                CtrlThrEnv *envp  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/15
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 ctrl_thr_init(CtrlThrArg *arg, CtrlThrEnv *envp)
{
	//Int32 err;
	
	assert(arg && arg->hParamsMng && arg->hDispatch);

	/* clear */
	memset(envp, 0, sizeof(CtrlThrEnv));
	envp->hParamsMng = arg->hParamsMng;
	envp->hDispatch = arg->hDispatch;

	/* no longer need input arg */
	free(arg);

	/* create msg handle  */
	envp->hMsg = msg_create(MSG_CTRL, MSG_MAIN, 0);
	if(!envp->hMsg) {
		ERR("create msg handle failed");
		return E_NOMEM;
	}

	/* alloc msg buffer */
	envp->msgBuf = calloc(1, sizeof(CtrlMsg));
	assert(envp->msgBuf);

	envp->exit = FALSE;

	DBG("ctrl thread init ok.");
	return E_NO;
}

/*****************************************************************************
 Prototype    : ctrl_thr
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
void *ctrl_thr(void *arg)
{
	CtrlThrEnv 		env;
	Int32			ret;

	ret = ctrl_thr_init((CtrlThrArg *)arg, (CtrlThrEnv *)&env);
	assert(ret == E_NO);
	if(ret)
		goto exit;

	MsgHeader 	*msgHdr = &env.msgBuf->hdr;
	Int8		*data = env.msgBuf->buf;
	Int32		respLen;
	Bool		needResp = TRUE;
	MsgHeader	msgSnd;

	msgSnd.dataLen = 0;
	msgSnd.index = 0;
	msgSnd.magicNum = MSG_MAGIC_SEND;

	/* start main loop */
	while(!env.exit) {		
		/* recv msg */
		ret = msg_recv(env.hMsg, env.msgBuf, sizeof(CtrlMsg));
		if(ret < 0) {
			ERR("ctrl thr recv msg err: %s", str_err(ret));
			continue;
		}
	
		/* process msg */
		needResp = TRUE;
		respLen = 0;
		
		switch(msgHdr->cmd) {
		case APPCMD_EXIT:
			env.exit = TRUE;
			needResp = FALSE;
			break;
		case ICAMCMD_G_VERSION:
			ret = params_mng_control(env.hParamsMng, PMCMD_G_VERSION, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamVersionInfo);
			break;
		case ICAMCMD_G_WORKSTATUS:
			ret = params_mng_control(env.hParamsMng, PMCMD_G_WORKSTATUS, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamWorkStatus); 
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
			ret = params_mng_control(env.hParamsMng, PMCMD_S_NETWORKINFO, data, msgHdr->dataLen);
			break;
		case ICAMCMD_G_NETWORKINFO:
			ret = params_mng_control(env.hParamsMng, PMCMD_G_NETWORKINFO, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamNetworkInfo); 
			break;
		case ICAMCMD_S_DEVINFO:
			ret = params_mng_control(env.hParamsMng, PMCMD_S_DEVINFO, data, msgHdr->dataLen);
			break;
		case ICAMCMD_G_DEVINFO:
			ret = params_mng_control(env.hParamsMng, PMCMD_S_DEVINFO, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamDeviceInfo);
			break;
		case ICAMCMD_S_OSDPARAMS:
			ret = params_mng_control(env.hParamsMng, PMCMD_S_OSDPARAMS, data, msgHdr->dataLen);
			if(!ret) {
				/* tell other tasks to update params */
				msgSnd.cmd = APPCMD_SET_ENC_PARAMS;
				msg_send(env.hMsg, MSG_VID_ENC, &msgSnd, sizeof(msgSnd));
				ret = msg_send(env.hMsg, MSG_IMG_ENC, &msgSnd, sizeof(msgSnd));
			}
			break;	
		case ICAMCMD_G_OSDPARAMS:
			ret = params_mng_control(env.hParamsMng, PMCMD_G_OSDPARAMS, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamOsdParams);
			break;
		case ICAMCMD_S_RODAINFO:
			ret = params_mng_control(env.hParamsMng, PMCMD_S_ROADINFO, data, msgHdr->dataLen);
			break;
		case ICAMCMD_G_RODAINFO:
			ret = params_mng_control(env.hParamsMng, PMCMD_G_ROADINFO, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamRoadInfo);
			break;
		case ICAMCMD_S_RTPPARAMS:
			ret = params_mng_control(env.hParamsMng, PMCMD_S_RTPPARAMS, data, msgHdr->dataLen);
			if(!ret) {
				msgSnd.cmd = APPCMD_SET_UPLOAD_PARAMS;
				ret = msg_send(env.hMsg, MSG_VID_ENC, &msgSnd, sizeof(msgSnd));
			}
			break;
		case ICAMCMD_G_RTPPARAMS:
			ret = params_mng_control(env.hParamsMng, PMCMD_G_RTPPARAMS, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamRtpParams);
			break;
		case ICAMCMD_S_UPLOADPROTO:
			ret = params_mng_control(env.hParamsMng, PMCMD_S_IMGTRANSPROTO, data, msgHdr->dataLen);
			if(!ret) {
				/* tell other tasks to update params */
				msgSnd.cmd = APPCMD_SET_UPLOAD_PARAMS;
				ret = msg_send(env.hMsg, MSG_IMG_ENC, &msgSnd, sizeof(msgSnd));
			}
			break;
		case ICAMCMD_G_UPLOADPROTO:
			ret = params_mng_control(env.hParamsMng, PMCMD_G_IMGTRANSPROTO, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamImageUploadProtocol);
			break;
		case ICAMCMD_S_IMGSRVINFO:
			ret = params_mng_control(env.hParamsMng, PMCMD_S_TCPSRVINFO, data, msgHdr->dataLen);
			if(!ret) {
				/* tell other tasks to update params */
				msgSnd.cmd = APPCMD_SET_UPLOAD_PARAMS;
				ret = msg_send(env.hMsg, MSG_IMG_ENC, &msgSnd, sizeof(msgSnd));
			}
			break;
		case ICAMCMD_G_IMGSRVINFO:
			ret = params_mng_control(env.hParamsMng, PMCMD_G_TCPSRVINFO, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamTcpImageServerInfo);
			break;
		case ICAMCMD_S_FTPSRVINFO:
			ret = params_mng_control(env.hParamsMng, PMCMD_S_FTPSRVINFO, data, msgHdr->dataLen);
			if(!ret) {
				/* tell other tasks to update params */
				msgSnd.cmd = APPCMD_SET_UPLOAD_PARAMS;
				ret = msg_send(env.hMsg, MSG_IMG_ENC, &msgSnd, sizeof(msgSnd));
			}
			break;
		case ICAMCMD_G_FTPSRVINFO:
			ret = params_mng_control(env.hParamsMng, PMCMD_G_FTPSRVINFO, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamFtpImageServerInfo);
			break;
		case ICAMCMD_S_NTPSRVINFO:
			ret = params_mng_control(env.hParamsMng, PMCMD_S_NTPSRVINFO, data, msgHdr->dataLen);
			break;
		case ICAMCMD_G_NTPSRVINFO:
			ret = params_mng_control(env.hParamsMng, PMCMD_G_NTPSRVINFO, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamNtpServerInfo);
			break;
		case ICAMCMD_S_EXPPARAMS:
			ret = params_mng_control(env.hParamsMng, PMCMD_S_EXPPARAMS, data, msgHdr->dataLen);
			break;
		case ICAMCMD_G_EXPPARAMS:
			ret = params_mng_control(env.hParamsMng, PMCMD_G_EXPPARAMS, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamExprosureParam);
			break;
		case ICAMCMD_S_RGBGAINS:
			ret = params_mng_control(env.hParamsMng, PMCMD_S_RGBGAINS, data, msgHdr->dataLen);
			break;
		case ICAMCMD_G_RGBGAINS:
			ret = params_mng_control(env.hParamsMng, PMCMD_G_RGBGAINS, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamRGBGains);
			break;
		case ICAMCMD_S_DRCPARAMS:
			ret = params_mng_control(env.hParamsMng, PMCMD_S_DRCPARAMS, data, msgHdr->dataLen);
			break;
		case ICAMCMD_G_DRCPARAMS:
			ret = params_mng_control(env.hParamsMng, PMCMD_G_DRCPARAMS, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamDRCParam);
			break;
		case ICAMCMD_S_TRAFLIGHTREG:
			ret = params_mng_control(env.hParamsMng, PMCMD_S_LIGHTCORRECT, data, msgHdr->dataLen);
			break;
		case ICAMCMD_G_TRAFLIGHTREG:
			ret = params_mng_control(env.hParamsMng, PMCMD_G_LIGHTCORRECT, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamTrafficLightRegions);
			break;
		case ICAMCMD_S_IMGENHANCE:
			ret = params_mng_control(env.hParamsMng, PMCMD_S_IMGADJPARAMS, data, msgHdr->dataLen);
			if(!ret) {
				/* tell other tasks to update params */
				msgSnd.cmd = APPCMD_SET_IMG_CONV;
				ret = msg_send(env.hMsg, MSG_CAP, &msgSnd, sizeof(msgSnd));
			}
			break;
		case ICAMCMD_G_IMGENHANCE:
			ret = params_mng_control(env.hParamsMng, PMCMD_G_IMGADJPARAMS, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamImgEnhanceParams);
			break;
		case ICAMCMD_S_H264PARAMS:
			ret = params_mng_control(env.hParamsMng, PMCMD_S_H264ENCPARAMS, data, msgHdr->dataLen);
			if(!ret) {
				/* tell other tasks to update params */
				msgSnd.cmd = APPCMD_SET_ENC_PARAMS;
				ret = msg_send(env.hMsg, MSG_VID_ENC, &msgSnd, sizeof(msgSnd));
			}
			break;
		case ICAMCMD_G_H264PARAMS:
			ret = params_mng_control(env.hParamsMng, PMCMD_G_H264ENCPARAMS, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamH264Params);
			break;
		case ICAMCMD_S_AVPARAMS:
			ret = params_mng_control(env.hParamsMng, PMCMD_S_AVPARAMS, data, msgHdr->dataLen);
			break;
		case ICAMCMD_G_AVPARAMS:
			ret = params_mng_control(env.hParamsMng, PMCMD_G_AVPARAMS, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamAVParam);
			break;
		case ICAMCMD_S_WORKMODE:
			ret = params_mng_control(env.hParamsMng, PMCMD_S_WORKMODE, data, msgHdr->dataLen);
			if(!ret) {
				/* tell other tasks to update params */
				msgSnd.cmd = APPCMD_SET_WORK_MODE;
				ret = msg_send(env.hMsg, MSG_CAP, &msgSnd, sizeof(msgSnd));
			}
			break;
		case ICAMCMD_G_WORKMODE:
			ret = params_mng_control(env.hParamsMng, PMCMD_G_WORKMODE, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamWorkMode);
			break;
		case ICAMCMD_S_IMGENCPARAMS:
			ret = params_mng_control(env.hParamsMng, PMCMD_S_IMGENCPARAMS, data, msgHdr->dataLen);
			if(!ret) {
				/* tell other tasks to update params */
				msgSnd.cmd = APPCMD_SET_ENC_PARAMS;
				ret = msg_send(env.hMsg, MSG_IMG_ENC, &msgSnd, sizeof(msgSnd));
			}
			break;
		case ICAMCMD_G_IMGENCPARAMS:
			ret = params_mng_control(env.hParamsMng, PMCMD_G_IMGENCPARAMS, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamImgEncParams);
			break;
		case ICAMCMD_S_SPECCAPPARAMS:
			ret = params_mng_control(env.hParamsMng, PMCMD_S_SPECCAPPARAMS, data, msgHdr->dataLen);
			break;
		case ICAMCMD_G_SPECCAPPARAMS:
			ret = params_mng_control(env.hParamsMng, PMCMD_G_SPECCAPPARAMS, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamSpecCapParams);
			break;
		case ICAMCMD_S_IOCFG:
			ret = params_mng_control(env.hParamsMng, PMCMD_S_IOCFG, data, msgHdr->dataLen);
			break;
		case ICAMCMD_G_IOCFG:
			ret = params_mng_control(env.hParamsMng, PMCMD_G_IOCFG, data,CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamIoCfg);
			break;
		case ICAMCMD_S_STROBEPARAMS:
			ret = params_mng_control(env.hParamsMng, PMCMD_S_STROBEPARAMS, data, msgHdr->dataLen);
			break;
		case ICAMCMD_G_STROBEPARAMS:
			ret = params_mng_control(env.hParamsMng, PMCMD_G_STROBEPARAMS, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamStrobeCtrlParam);
			break;
		case ICAMCMD_S_DETECTORPARAMS:
			ret = params_mng_control(env.hParamsMng, PMCMD_S_DETECTORPARAMS, data, msgHdr->dataLen);
			if(!ret) {
				/* tell other tasks to update params */
				msgSnd.cmd = APPCMD_SET_TRIG_PARAMS;
				ret = msg_send(env.hMsg, MSG_CAP, &msgSnd, sizeof(msgSnd));
			}
			break;
		case ICAMCMD_G_DETECTORPARAMS:
			ret = params_mng_control(env.hParamsMng, PMCMD_G_DETECTORPARAMS, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamDetectorParam);
			break;
		case ICAMCMD_S_AEPARAMS:
			ret = params_mng_control(env.hParamsMng, PMCMD_S_AEPARAMS, data, msgHdr->dataLen);
			if(!ret) {
				/* tell other tasks to update params */
				msgSnd.cmd = APPCMD_SET_AE_PARAMS;
				ret = msg_send(env.hMsg, MSG_CAP, &msgSnd, sizeof(msgSnd));
			}
			break;
		case ICAMCMD_G_AEPARAMS:
			ret = params_mng_control(env.hParamsMng, PMCMD_G_AEPARAMS, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamAEParam);
			break;
		case ICAMCMD_S_AWBPARAMS:
			ret = params_mng_control(env.hParamsMng, PMCMD_S_AWBPARAMS, data, msgHdr->dataLen);
			if(!ret) {
				/* tell other tasks to update params */
				msgSnd.cmd = APPCMD_SET_AWB_PARAMS;
				ret = msg_send(env.hMsg, MSG_CAP, &msgSnd, sizeof(msgSnd));
			}
			break;
		case ICAMCMD_G_AWBPARAMS:
			ret = params_mng_control(env.hParamsMng, PMCMD_G_AWBPARAMS, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamAWBParam);
			break;
		case ICAMCMD_S_DAYNIGHTMODE:
			ret = params_mng_control(env.hParamsMng, PMCMD_S_DAYNIGHTCFG, data, msgHdr->dataLen);
			break;
		case ICAMCMD_G_DAYNIGHTMODE:
			ret = params_mng_control(env.hParamsMng, PMCMD_G_DAYNIGHTCFG, data, CTRL_MSG_BUF_LEN);
			respLen = sizeof(CamDayNightModeCfg);
			break;
		case ICAMCMD_S_CAPEN:
			/* tell other tasks to update params */
			msgSnd.cmd = APPCMD_SET_AWB_PARAMS;
			msgSnd.param[0] = *(Int32 *)data;
			ret = msg_send(env.hMsg, MSG_CAP, &msgSnd, sizeof(msgSnd));
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
			/* TBD */
			ret = E_NO;
			break;
		default:
			ERR("unkown cmd: 0x%X", (unsigned int)msgHdr->cmd);
			ret = E_UNSUPT;
			if(msgHdr->cmd < ICAM_CMD_BASE)
				needResp = FALSE;
			break;
		}

		/* reponse to request */
		if(needResp) {
			msgHdr->param[0] = ret;		//put operation result as param0
			if(ret < 0)
				msgHdr->dataLen = 0; 	//an error happens, so we need not reply any data
			else
				msgHdr->dataLen = respLen;
			msgHdr->magicNum = MSG_MAGIC_RESP;
			/* send back response */
			msg_send(env.hMsg, NULL, env.msgBuf, respLen + sizeof(msgHdr));
		}
	}

exit:
	if(env.msgBuf)
		free(env.msgBuf);

	if(env.hMsg)
		msg_delete(env.hMsg);

	INFO("ctrl thread exit...");
	pthread_exit(0);
	
}

