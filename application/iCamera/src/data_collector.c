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
#include "data_collector.h"
#include "log.h"
#include "capture.h"
#include "app_msg.h"
#include <sys/select.h>
#include "crc16.h"
#include <pthread.h>

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
#define CAPTHR_STAT_CONV_EN			(1 << 0)
#define CAPTHR_STAT_CAP_STARTED		(1 << 1)


/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/* thread running environment */
struct CollectorObj{
	ParamsMngHandle 	hParamsMng;
	CapHandle			hCap;
	CapInputInfo		inputInfo;
	Bool				encImg;
	Bool				exit;
	Int32				status;
	MsgHandle			hMsg;
	RawMsg				imgMsg;
	pthread_t			pid;
	const char 			*dstName[2];
};


/*****************************************************************************
 Prototype    : data_collector_init
 Description  : init module env
 Input        : CollectorHandle hCollector  
                CollectorAttrs *attrs     
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/16
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 data_collector_config(CollectorHandle hCollector)
{
	CamWorkMode			workMode;
	Int32				ret;
	Int32				bufRefCnt = 1;
	
	ret = params_mng_control(hCollector->hParamsMng, PMCMD_G_WORKMODE, &workMode, sizeof(workMode));
	assert(ret == E_NO);

	/* alloc encode for continously stream, test */
	if(workMode.capMode == CAM_CAP_MODE_CONTINUE)
		hCollector->encImg = TRUE;
	
	hCollector->dstName[1] = NULL;	
	
	if(workMode.format == CAM_FMT_JPEG) {
		hCollector->dstName[0] = MSG_IMG_ENC;
		DBG("snd stream to jpg only");
	} else if(workMode.format == CAM_FMT_H264) {
		hCollector->dstName[0] = MSG_VID_ENC;
		DBG("snd stream to h.264 only");
	} else if(workMode.format == CAM_FMT_JPEG_H264) {
		hCollector->dstName[0] = MSG_VID_ENC;
		hCollector->dstName[1] = MSG_IMG_ENC;
		bufRefCnt = 2;
		DBG("snd stream to h.264 and jpeg");
	} else {
		hCollector->dstName[0] = MSG_IMG_TX;	//direct send
		DBG("snd stream to tx only");
	}

	
	/* change default dest */
	ret = msg_set_default_dst(hCollector->hMsg, hCollector->dstName[0]);
	assert(ret == E_NO);

	/* set capture mode */
	if(hCollector->status & CAPTHR_STAT_CAP_STARTED) {
		ret = capture_stop(hCollector->hCap);
		if(!ret)
			usleep(100000); //wait for process complete
	}
	
	/* Change capture OsdAlgHandle */
	CaptureStd	std;
	CaptureMode mode;
	
	std = (workMode.resType == CAM_RES_HIGH_SPEED) ? CAP_STD_HIGH_SPEED : CAP_STD_FULL_FRAME;
	mode = (workMode.capMode == CAM_CAP_MODE_CONTINUE) ? CAP_MODE_CONT : CAP_MODE_TRIG;

	/* Change attrs */
	ret = capture_config(hCollector->hCap, std, mode);
	ret |= capture_set_def_frame_ref_cnt(hCollector->hCap, bufRefCnt);
	if(ret) {
		ERR("config capture failed....");
	}
	
	/* Init msg for new image */
	CapInputInfo inputInfo;
	
	ret = capture_get_input_info(hCollector->hCap, &inputInfo);
	assert(ret == E_NO);

	/* set capture info to params manager */
	ret = params_mng_control(hCollector->hParamsMng, PMCMD_S_CAPINFO, &inputInfo, sizeof(CapInputInfo));
	assert(ret == E_NO);
	
	memset(&hCollector->imgMsg, 0, sizeof(RawMsg));
	hCollector->imgMsg.header.cmd = APPCMD_RAW_DATA;
	hCollector->imgMsg.header.magicNum = MSG_MAGIC_SEND;
	hCollector->imgMsg.header.dataLen = sizeof(RawMsg) - sizeof(MsgHeader);
	hCollector->imgMsg.dimension = inputInfo;
	
	DBG("capture out size: %u X %u", hCollector->imgMsg.dimension.width, hCollector->imgMsg.dimension.height);

	/* Restart capture */
	if(hCollector->status & CAPTHR_STAT_CAP_STARTED)
		capture_start(hCollector->hCap);
	
	hCollector->exit = FALSE;
	return E_NO;
}


/*****************************************************************************
 Prototype    : capture_new_img
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
static Int32 capture_new_img(CollectorHandle hCollector)
{
	FrameBuf	*capFrame;
	Int32		err;
	RawMsg 		*imgMsg = &hCollector->imgMsg;

	/* get frame from driver */
	capFrame = &imgMsg->capBuf;
	err = capture_get_frame(hCollector->hCap, capFrame);
	if(err) {
		ERR("get new frame failed.");
		return err;
	}

	/* send to next task */
	//err = msg_send(hCollector->hMsg, NULL, imgMsg, sizeof(RawMsg));
	err = 1;
	if(err) {
		//ERR("cap run, send msg failed");
		capture_free_frame(hCollector->hCap, capFrame);
	}

	if(hCollector->dstName[1]) {
		/* send to 2nd stream */
		//err = msg_send(hCollector->hMsg, hCollector->dstName[1], imgMsg, sizeof(RawMsg));
		err = 1;
		if(err) {
			//ERR("cap run, send msg failed");
			capture_free_frame(hCollector->hCap, capFrame);
		}
	}

	DBG("<%d> cap run ok...", capFrame->index);

	return err;
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
static Int32 msg_process(CollectorHandle hCollector, CommonMsg *msgBuf)
{
	Int32 		err;
	MsgHeader 	*msgHdr = &msgBuf->header;
	
	err = msg_recv(hCollector->hMsg, msgBuf, sizeof(CommonMsg));
	if(err < 0)
		return err;

	switch(msgHdr->cmd) {
	case APPCMD_EXIT:
		hCollector->exit = TRUE;
		break;
	case APPCMD_SET_WORK_MODE:
		err = data_collector_config(hCollector);
		break;
	default:
		INFO("cap thr, unsupport cmd: 0x%X", (unsigned int)msgHdr->cmd);
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
static void *collector_thread(void *arg)
{
	CollectorHandle	hCollector = (CollectorHandle)arg;
	Int32			ret;
	Int32			fdCap, fdMsg, fdMax;
	fd_set			rdSet;
	CommonMsg 		msgBuf;

	assert(hCollector);

	/* get fd for select */
	fdCap = capture_get_fd(hCollector->hCap);
	fdMsg = msg_get_fd(hCollector->hMsg);
	fdMax = MAX(fdMsg, fdCap) + 1;
	
	/* start capture */
	ret = capture_start(hCollector->hCap);
	if(ret) {
		ERR("Start capture failed.");
		goto exit;
	}
	hCollector->status |= CAPTHR_STAT_CAP_STARTED;
	
	while(!hCollector->exit) {
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
			capture_new_img(hCollector);
		}

		if(FD_ISSET(fdMsg, &rdSet)) {
			/* process msg */
			msg_process(hCollector, &msgBuf);
		}
	}


exit:

	/* stop capture first */
	if(hCollector->status & CAPTHR_STAT_CAP_STARTED)
		capture_stop(hCollector->hCap);

	DBG("capture thread exit...");
	pthread_exit(0);
}


/*****************************************************************************
 Prototype    : data_capture_create
 Description  : create this module
 Input        : DataCapAttrs *attrs  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/16
    Author       : Sun
    Modification : Created function

*****************************************************************************/
CollectorHandle data_collector_create(CollectorAttrs *attrs)
{
	assert(attrs && attrs->hParamsMng && attrs->hCap);

	CollectorHandle hCollector;
	Int32 			ret;

	hCollector = calloc(1, sizeof(struct CollectorObj));
	if(!hCollector) {
		ERR("alloc mem failed.");
		return NULL;
	}

	hCollector->hMsg = msg_create(MSG_CAP, hCollector->dstName[0], 0);
	if(!hCollector->hMsg) {
		ERR("create msg handle failed...");
		goto exit;
	}
	
	hCollector->hParamsMng = attrs->hParamsMng;
	hCollector->hCap = attrs->hCap;

	/* init our running evironment */
	ret = data_collector_config(hCollector);
	if(ret) {
		goto exit;	
	}

	return hCollector;

exit:
	if(hCollector->hMsg)
		msg_delete(hCollector->hMsg);
	
	if(hCollector)
		free(hCollector);

	return NULL;
}

/*****************************************************************************
 Prototype    : data_collector_delete
 Description  : delete collector handle
 Input        : CollectorHandle hCollector  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/16
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 data_collector_delete(CollectorHandle hCollector, MsgHandle hCurMsg)
{
	if(hCollector->pid > 0) {
		if(hCurMsg) {
			MsgHeader msg;

			/* send msg to our thread to exit */
			msg.cmd = APPCMD_EXIT;
			msg.index = 0;
			msg.dataLen = 0;
			msg.magicNum = MSG_MAGIC_SEND;
			msg_send(hCurMsg, MSG_CAP, &msg, sizeof(msg));
		}

		/* set flag to exit */
		hCollector->exit = TRUE;
		
		pthread_join(hCollector->pid, NULL);
	}

	/*delete our own msg module */
	if(hCollector->hMsg)
		msg_delete(hCollector->hMsg);

	free(hCollector);

	return E_NO;
}

/*****************************************************************************
 Prototype    : data_collector_run
 Description  : run data collect
 Input        : CollectorHandle hCollector  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/16
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 data_collector_run(CollectorHandle hCollector)
{
	Int32 err;
	
	/* create thread and run data collecting */
	err = pthread_create(&hCollector->pid, NULL, collector_thread, hCollector);
	if(err < 0) {
		ERRSTR("create data collector thread failed...");
		return E_NOMEM;
	}

	return E_NO;
}


