/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : capture_thr.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/6
  Last Modified :
  Description   : capture image and do convert
  Function List :
              capture_thr
  History       :
  1.Date        : 2012/3/6
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "capture_thr.h"
#include "log.h"
#include "capture.h"
#include "img_convert.h"
#include "app_msg.h"
#include <sys/select.h>
#include "cam_time.h"
#include "crc16.h"

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
#define CAP_DEVICE			"/dev/video0"
#define CAP_BUF_NUM			3
#define POOL_BUF_NUM		4

#define CAPTHR_STAT_CONV_EN			(1 << 0)
#define CAPTHR_STAT_CAP_STARTED		(1 << 1)


/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/* thread running environment */
typedef struct {
	ParamsMngHandle 	hParamsMng;
	FrameDispHandle		hDispatch;
	CapHandle			hCap;
	AlgHandle			hImgConv;
	ImgConvDynParams	convDynParams;
	ConvOutAttrs		stream2OutAttrs;
	CapInputInfo		inputInfo;
	Bool				encImg;
	Bool				exit;
	Int32				status;
	MsgHandle			hMsg;
	ImgMsg				imgMsg;
	BufPoolHandle		hBufPool;
}CapThrEnv;

/*****************************************************************************
 Prototype    : capture_thr_init
 Description  : Init thread 
 Input        : CapThrArg *arg   
                CapThrEnv *envp  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/7
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 capture_thr_init(CapThrArg *arg, CapThrEnv *envp)
{
	CapAttrs 			capAttrs;
	CamWorkMode			workMode;
	ImgConvInitParams	convInitParams;
	ImgConvDynParams	*pConvDynParams;
	Int32				ret;
	Uint32				bufSize;
	BufAllocAttrs		bufAllocAttrs;

	assert(arg && arg->hParamsMng && arg->hDispatch);
	
	memset(envp, 0, sizeof(CapThrEnv));
	envp->hParamsMng = arg->hParamsMng;
	envp->hDispatch = arg->hDispatch;

	/* we do not need arg any more*/
	free(arg);

	/* get current work mode  & img enhance params */
	ret = params_mng_control(envp->hParamsMng, PMCMD_G_WORKMODE, &workMode, sizeof(workMode));
	if(ret)
		return ret;

	/* fill attrs for capture create */
	capAttrs.devName = CAP_DEVICE;
	capAttrs.inputType = CAP_INPUT_CAMERA;
	capAttrs.std = 
		(workMode.resType == CAM_RES_HIGH_SPEED) ? CAP_STD_HIGH_SPEED : CAP_STD_FULL_FRAME;
	capAttrs.mode = 
		(workMode.capMode == CAM_CAP_MODE_CONTINUE) ? CAP_MODE_CONT : CAP_MODE_TRIG;
	capAttrs.userAlloc = TRUE;
	capAttrs.bufNum = CAP_BUF_NUM;

	/* create capture object */
	envp->hCap = capture_create(&capAttrs);
	if(!envp->hCap) {
		ERR("create capture handle failed...");
		return E_IO;
	}

	capture_get_input_info(envp->hCap, &envp->inputInfo);

	/* set capture info to params manager */
	ret = params_mng_control(envp->hParamsMng, PMCMD_S_CAPINFO, &envp->inputInfo, sizeof(CapInputInfo));
	assert(ret == E_NO);

	/* register capture handle for frame dispatch */
	ret = frame_disp_register_capture(envp->hDispatch, envp->hCap);
	assert(ret == E_NO);

	envp->status |= CAPTHR_STAT_CONV_EN;
	frame_disp_set_encode_mode(envp->hDispatch, FRAME_ENC_ON);
	if(workMode.capMode == CAM_CAP_MODE_CONTINUE)
		envp->encImg = TRUE;
	
	/* Create msg handle */
	const char *dstMsgName = MSG_VID_ENC;
	if(workMode.format == CAM_FMT_JPEG) {
		dstMsgName = MSG_IMG_ENC;
	} else if(workMode.format == CAM_FMT_RAW || workMode.format == CAM_FMT_YUV) {
		envp->status &= ~CAPTHR_STAT_CONV_EN;
		dstMsgName = MSG_IMG_TX;	//direct send
		frame_disp_set_encode_mode(envp->hDispatch, FRAME_ENC_OFF);
	}

	envp->hMsg = msg_create(MSG_CAP, dstMsgName, 0);
	if(!envp->hMsg) {
		ERR("create msg handle failed...");
		return E_IO;
	}

	/* set img convert init params */
	convInitParams.prevDevName = NULL;
	convInitParams.rszDevName = NULL;
	convInitParams.size = sizeof(convInitParams);

	/* get img convert dynamic params */
	pConvDynParams = &envp->convDynParams;
	ret = params_mng_control(envp->hParamsMng, PMCMD_G_IMGCONVDYN, pConvDynParams, sizeof(ImgConvDynParams));
	if(ret)
		return ret;

	/* create image convert object */
	envp->hImgConv = img_conv_create(&convInitParams, pConvDynParams);
	if(!envp->hImgConv) {
		ERR("create img convert handle failed...");
		return E_IO;
	}
	
	/* Init msg for new image */
	memset(&envp->imgMsg, 0, sizeof(ImgMsg));
	envp->imgMsg.header.cmd = APPCMD_NEW_DATA;
	envp->imgMsg.header.magicNum = MSG_MAGIC_SEND;
	envp->imgMsg.header.dataLen = sizeof(ImgMsg) - sizeof(MsgHeader);

	if(envp->status & CAPTHR_STAT_CONV_EN) {
		envp->imgMsg.dimension.width = pConvDynParams->outAttrs[0].width;
		envp->imgMsg.dimension.height = pConvDynParams->outAttrs[0].height;
		envp->imgMsg.dimension.colorSpace = pConvDynParams->outAttrs[0].pixFmt;
		envp->imgMsg.dimension.bytesPerLine = envp->imgMsg.dimension.width;
	} else {
		/* need not covert, use input params */
		envp->imgMsg.dimension = envp->inputInfo;
	}

	DBG("cap out size: %u X %u", envp->imgMsg.dimension.width, envp->imgMsg.dimension.height);

	/* check if we need 2nd stream */
	params_mng_control(envp->hParamsMng, PMCMD_G_2NDSTREAMATTRS, 
		&envp->stream2OutAttrs, sizeof(ConvOutAttrs));

	DBG("stream2 enable: %d", envp->stream2OutAttrs.enbale);
	DBG("stream2 res: %u X %u", envp->stream2OutAttrs.width, envp->stream2OutAttrs.height);
	
	/* create buffer pool */
	bufSize = pConvDynParams->outAttrs[0].width * pConvDynParams->outAttrs[0].height * 3/2;
	bufSize = ROUND_UP(bufSize, 32);
	bufAllocAttrs.align = 256;
	bufAllocAttrs.type = BUF_TYPE_POOL;
	bufAllocAttrs.flags = 0;
	
	envp->hBufPool = buf_pool_create(bufSize, POOL_BUF_NUM, &bufAllocAttrs);
	if(!envp->hBufPool) {
		ERR("create buffer pool failed");
		return E_NOMEM;
	}

	envp->exit = FALSE;
	return E_NO;
}


/*****************************************************************************
 Prototype    : thread_run
 Description  : get new frame from driver and convert to yuv format
 Input        : CapThrEnv *envp  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/7
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 cap_thr_run(CapThrEnv *envp)
{
	FrameBuf	capFrame;
	Int32		err;
	AlgBuf 		inBuf, outBuf;

	/* get frame from driver */
	err = capture_get_frame(envp->hCap, &capFrame);
	if(err) {
		ERR("get new frame failed.");
		return err;
	}

	/* get buffer from pool */
	static int cnt = 0;
	DBG("cap alloc buf: %d", ++cnt);
	BufHandle hBuf = buf_pool_alloc(envp->hBufPool);
	if(!hBuf) {
		ERR("cap run alloc buffer failed");
		err = E_NOMEM;
		goto free_buf;
	}

	ImgMsg *imgMsg = &envp->imgMsg;
	
	if(envp->status & CAPTHR_STAT_CONV_EN) {
		/* do image convert */
		inBuf.buf = capFrame.dataBuf;
		inBuf.bufSize = capFrame.bufSize;
		outBuf.buf = buffer_get_user_addr(hBuf);
		outBuf.bufSize = buffer_get_size(hBuf);
		
		err = img_conv_process(envp->hImgConv, &inBuf, NULL, &outBuf, NULL);
		if(err) {
			ERR("imgConv err.");
			goto free_buf;
		}
		
#ifdef CRC_EN
		imgMsg->header.param[0] = crc16(outBuf.buf, imgMsg->dimension.size);
#endif
	} else {
		err = buffer_copy(buffer_get_user_addr(hBuf), capFrame.dataBuf, capFrame.bytesUsed);
		if(err) {
			ERR("buf cpy err.");
			goto free_buf;
		}
	}

	/* send to next task */
	imgMsg->hBuf = hBuf;
	imgMsg->index = capFrame.index;
	
	cam_convert_time(&capFrame.timeStamp, &imgMsg->timeStamp);

	err = frame_disp_run(envp->hDispatch, envp->hMsg, imgMsg, NULL, FD_FLAG_POOL_FRAME);
	if(err) {
		ERR("cap run, send msg failed");
		buf_pool_free(hBuf);
	}

	/* check if 2nd stream convert needed */
	if((envp->status & CAPTHR_STAT_CONV_EN) && 
		envp->stream2OutAttrs.enbale && envp->encImg) {

		DBG("cap alloc buf: %d", ++cnt);
		/* alloc again for img enc */
		hBuf = buf_pool_alloc(envp->hBufPool);
		if(!hBuf) {
			ERR("cap run alloc buffer failed");
			err = E_NOMEM;
			goto free_buf;
		}

		/* convert for img enc */
		err = img_conv_control( envp->hImgConv, 
								CONV_CMD_SET_OUT0_ATTRS, 
								&envp->stream2OutAttrs);
		if(err) {
			ERR("imgConv config for img enc err.");
			buf_pool_free(hBuf);
			goto free_buf;
		}

		/* conv again */
		outBuf.buf = buffer_get_user_addr(hBuf);
		outBuf.bufSize = buffer_get_size(hBuf);
		err = img_conv_process(envp->hImgConv, &inBuf, NULL, &outBuf, NULL);
		if(err) {
			ERR("imgConv for img enc err.");
			buf_pool_free(hBuf);
			goto free_buf;
		}

		imgMsg->hBuf = hBuf;
		imgMsg->dimension.width = envp->stream2OutAttrs.width;
		imgMsg->dimension.height = envp->stream2OutAttrs.height;
		imgMsg->dimension.colorSpace = envp->stream2OutAttrs.pixFmt;

		err = frame_disp_run(envp->hDispatch, envp->hMsg, imgMsg, MSG_IMG_ENC, FD_FLAG_POOL_FRAME);
		if(err) {
			ERR("cap run, send msg failed");
			buf_pool_free(hBuf);
		}

		/* set back for next capture */
		err = img_conv_control( envp->hImgConv, 
								CONV_CMD_SET_DYN_PARAMS, 
								&envp->convDynParams);
		if(err) {
			ERR("imgConv set back config err.");
			buf_pool_free(hBuf);
		}
		imgMsg->dimension.width = envp->convDynParams.outAttrs[0].width;
		imgMsg->dimension.height = envp->convDynParams.outAttrs[0].height;
		imgMsg->dimension.colorSpace = envp->convDynParams.outAttrs[0].pixFmt;
	}

	//DBG("<%d> cap run ok...", imgMsg->index);

free_buf:

	if(hBuf)
		buf_pool_free(hBuf);

	capture_free_frame(envp->hCap, &capFrame);
	return err;
}

/*****************************************************************************
 Prototype    : img_conv_update
 Description  : update img conv alg
 Input        : CapThrEnv *envp  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/7
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 img_conv_update(CapThrEnv *envp)
{
	Int32 				err;
	ImgConvDynParams 	*pConvDynParams;
	
	pConvDynParams = &envp->convDynParams;
	err = params_mng_control(envp->hParamsMng, PMCMD_G_IMGCONVDYN, 
				pConvDynParams, sizeof(ImgConvDynParams));
	if(err)
		return err;

	/* config alg */
	err = img_conv_control( envp->hImgConv, 
							ALG_CMD_SET_DYN_PARAMS, 
							pConvDynParams );
	if(err) {
		ERR(" config for img enc err.");
		return err;
	}

	return E_NO;
}

/*****************************************************************************
 Prototype    : cap_attrs_update
 Description  : update capture params
 Input        : CapThrEnv *envp  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/7
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 cap_attrs_update(CapThrEnv *envp)
{
	Int32 ret;
	CamWorkMode workMode;
	
	ret = params_mng_control(envp->hParamsMng, PMCMD_G_WORKMODE, &workMode, sizeof(workMode));
	if(ret)
		return ret;

	if(envp->status & CAPTHR_STAT_CAP_STARTED) 
		capture_stop(envp->hCap);
	
	/* Change capture OsdAlgHandle */
	CaptureStd 	std;
	CaptureMode mode;
	
	std = 
		(workMode.resType == CAM_RES_HIGH_SPEED) ? CAP_STD_HIGH_SPEED : CAP_STD_FULL_FRAME;
	mode = 
		(workMode.capMode == CAM_CAP_MODE_CONTINUE) ? CAP_MODE_CONT : CAP_MODE_TRIG;

	/* Change attrs */
	ret = capture_config(envp->hCap, std, mode);

	/* Restart capture */
	if(envp->status & CAPTHR_STAT_CAP_STARTED)
		capture_start(envp->hCap);

	if(workMode.format <= CAM_FMT_YUV) {
		msg_set_default_dst(envp->hMsg, MSG_IMG_TX);
		envp->status &= ~CAPTHR_STAT_CONV_EN;
		frame_disp_set_encode_mode(envp->hDispatch, FRAME_ENC_OFF);

		/* need not covert, use input params */
		envp->imgMsg.dimension = envp->inputInfo;
	} else {
		if(workMode.format == CAM_FMT_JPEG)
			msg_set_default_dst(envp->hMsg, MSG_IMG_ENC);
		else
			msg_set_default_dst(envp->hMsg, MSG_VID_ENC);
		envp->status |= CAPTHR_STAT_CONV_EN;
		
		/* update 1st stream params */
		params_mng_control(envp->hParamsMng, PMCMD_G_IMGCONVDYN, 
			&envp->convDynParams, sizeof(ImgConvDynParams));

		/* update 2nd stream params */
		params_mng_control(envp->hParamsMng, PMCMD_G_2NDSTREAMATTRS, 
				&envp->stream2OutAttrs, sizeof(ConvOutAttrs));

		envp->imgMsg.dimension.width = envp->convDynParams.outAttrs[0].width;
		envp->imgMsg.dimension.height = envp->convDynParams.outAttrs[0].height;
		envp->imgMsg.dimension.colorSpace = envp->convDynParams.outAttrs[0].pixFmt;
		envp->imgMsg.dimension.bytesPerLine = envp->imgMsg.dimension.bytesPerLine;

		frame_disp_set_encode_mode(envp->hDispatch, FRAME_ENC_ON);
	}
	
	return ret;
}

/*****************************************************************************
 Prototype    : msg_process
 Description  : process msg recieved
 Input        : CapThrEnv *envp  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/7
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 msg_process(CapThrEnv *envp, CommonMsg *msgBuf)
{
	Int32 		err;
	MsgHeader 	*msgHdr = &msgBuf->header;
	
	err = msg_recv(envp->hMsg, msgBuf, sizeof(CommonMsg));
	if(err < 0)
		return err;

	switch(msgHdr->cmd) {
	case APPCMD_EXIT:
		envp->exit = TRUE;
		break;
	case APPCMD_SET_IMG_CONV:
		err = img_conv_update(envp);
		break;
	case APPCMD_SET_STREAM2:
		err = params_mng_control(envp->hParamsMng, PMCMD_G_2NDSTREAMATTRS, 
				&envp->stream2OutAttrs, sizeof(ConvOutAttrs));
		break;
	case APPCMD_SET_WORK_MODE:
		err = cap_attrs_update(envp);
		break;
	default:
		INFO("cap thr, unkown cmd: 0x%X", (unsigned int)msgHdr->cmd);
		err = E_UNSUPT;
		break;
	}

	return err;
}

/*****************************************************************************
 Prototype    : capture_thr
 Description  : Capture thread function
 Input        : void *arg  
 Output       : None
 Return Value : void
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/7
    Author       : Sun
    Modification : Created function

*****************************************************************************/
void *capture_thr(void *arg)
{
	CapThrEnv 	env;
	Int32		ret;
	Int32		fdCap, fdMsg, fdMax;
	fd_set		rdSet;
	CommonMsg 	msgBuf;

	/* init */
	ret = capture_thr_init((CapThrArg *)arg, &env);
	assert(ret == E_NO);

	/* get fd for select */
	fdCap = capture_get_fd(env.hCap);
	fdMsg = msg_get_fd(env.hMsg);
	fdMax = MAX(fdMsg, fdCap) + 1;
	
	/* start capture */
	ret = capture_start(env.hCap);
	if(ret) {
		ERR("Start capture failed.");
		goto exit;
	}
	env.status |= CAPTHR_STAT_CAP_STARTED;
	
	while(!env.exit) {
		/* wait data ready */
		FD_ZERO(&rdSet);
		FD_SET(fdCap, &rdSet);
		FD_SET(fdMsg, &rdSet);
		
		ret = select(fdMax, &rdSet, NULL, NULL, NULL);
		if(ret < 0 && errno != EINTR) {
			ERRSTR("select err");
			break;
		}

		/* no data ready */
		if(!ret)
			continue;

		/* check which is ready */
		if(FD_ISSET(fdCap, &rdSet)) {
			/* read new frame */
			cap_thr_run(&env);
		}

		if(FD_ISSET(fdMsg, &rdSet)) {
			/* process msg */
			msg_process(&env, &msgBuf);
		}
	}


exit:
	if(env.hMsg)
		msg_delete(env.hMsg);

	if(env.hCap)
		capture_delete(env.hCap);

	if(env.hImgConv)
		img_conv_delete(env.hImgConv);

	DBG("capture thread exit...");
	pthread_exit(0);
}

