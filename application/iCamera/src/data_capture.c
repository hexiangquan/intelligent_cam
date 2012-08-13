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
#include "data_capture.h"
#include "log.h"
#include "capture.h"
#include "app_msg.h"
#include <sys/select.h>
#include "crc16.h"
#include <pthread.h>
#include "converter.h"
#include "cam_time.h"
#include "detector.h"
#include "img_ctrl.h"
#include <sys/ioctl.h>
#include "rtp_upload.h"

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
#define CAPTHR_STAT_CONV_EN			(1 << 0)	//enable convert
#define CAPTHR_STAT_CAP_STARTED		(1 << 1)	//capture started
#define CAPTHR_STAT_TRIG			(1 << 2)	//tirgger mode

#define CONV_BUF_NUM				2

//#define CAP_TRIG_TEST

/* index for trigger capture */
#define CAP_INDEX_NEXT_FRAME		(-1)

#define IMG_CTRL_DEV				"/dev/imgctrl"

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/* thread running environment */
struct DataCapObj{
	CamWorkMode			workMode;
	CamDetectorParam	detectorParams;
	ConverterParams		convParams;
	CapHandle			hCap;
	CapInputInfo		inputInfo;
	ConverterHandle		hConverter;
	DetectorHandle		hDetector;
	CaptureInfo			capInfo, defCapInfo;
	Int32				capIndex;
	Bool				encImg;
	Bool				exit;
	Int32				status;
	Int32				fdMsg, fdCap, fdDetect, fdMax;
	Int32				fdImgCtrl;
	MsgHandle			hMsg;
	ImgDimension		capDim;
	CamRoadInfo			roadInfo;
	pthread_t			pid;
	const char 			*dstName[2];
};


/*****************************************************************************
 Prototype    : data_capture_get_fds
 Description  : get fds for select
 Input        : DataCapHandle hDataCap  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/1
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 data_capture_get_fds(DataCapHandle hDataCap)
{
	Int32 ret;
	
	/* get fd for select */
	hDataCap->fdCap = capture_get_fd(hDataCap->hCap);
	hDataCap->fdMsg = msg_get_fd(hDataCap->hMsg);
	hDataCap->fdMax = MAX(hDataCap->fdMsg, hDataCap->fdCap);
	/* get detector fd */
	ret = detector_control(hDataCap->hDetector, DETECTOR_CMD_GET_FD, &hDataCap->fdDetect, sizeof(Int32));
	if(ret == E_NO && hDataCap->fdDetect > 0)
		hDataCap->fdMax = MAX(hDataCap->fdMax, hDataCap->fdDetect);

	hDataCap->fdMax += 1;
	return ret;
}

/*****************************************************************************
 Prototype    : data_collector_init
 Description  : init module env
 Input        : DataCapHandle hDataCap  
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
static Int32 data_capture_config(DataCapHandle hDataCap)
{
	CamWorkMode			*workMode = &hDataCap->workMode;
	Int32				ret;
	Int32				bufRefCnt = 1;

	DBG("set work mode, fmt: %d, res: %d, capMode: %d", 
		workMode->format, workMode->resType, workMode->capMode);
	
	/* alloc encode for continously stream, test */
	if(workMode->capMode == CAM_CAP_MODE_CONTINUE) {
		hDataCap->encImg = FALSE;
		hDataCap->status &= ~CAPTHR_STAT_TRIG;
	} else {
		hDataCap->status |= CAPTHR_STAT_TRIG;
	}
	
	hDataCap->dstName[1] = NULL;
	hDataCap->status |= CAPTHR_STAT_CONV_EN;
	
	if(workMode->format == CAM_FMT_JPEG) {
		hDataCap->dstName[0] = MSG_IMG_ENC;
		DBG("send stream to jpg only");
	} else if(workMode->format == CAM_FMT_H264) {
		hDataCap->dstName[0] = MSG_VID_ENC;
		DBG("send stream to h.264 only");
	} else if(workMode->format == CAM_FMT_JPEG_H264) {
		hDataCap->dstName[0] = MSG_VID_ENC;
		hDataCap->dstName[1] = MSG_IMG_ENC;
		bufRefCnt = 1; //convert only in one format
		DBG("send stream to h.264 and jpeg");
	} else {
		hDataCap->dstName[0] = MSG_IMG_TX;	// send directly
		hDataCap->status &= ~CAPTHR_STAT_CONV_EN;
		DBG("send stream to tx only");
	}

	
	/* change default dest */
	ret = msg_set_default_dst(hDataCap->hMsg, hDataCap->dstName[0]);
	assert(ret == E_NO);

	/* set capture mode */
	if(hDataCap->status & CAPTHR_STAT_CAP_STARTED) {
		ret = capture_stop(hDataCap->hCap);
		if(!ret)
			usleep(100000); //wait for process complete
	}
	
	/* Change capture OsdAlgHandle */
	CaptureStd	std;
	CaptureMode mode;
	
	std = (workMode->resType == CAM_RES_HIGH_SPEED) ? CAP_STD_HIGH_SPEED : CAP_STD_FULL_FRAME;
	mode = (workMode->capMode == CAM_CAP_MODE_CONTINUE) ? CAP_MODE_CONT : CAP_MODE_TRIG;

	/* Change attrs */
	ret = capture_config(hDataCap->hCap, std, mode);
	ret |= capture_set_def_frame_ref_cnt(hDataCap->hCap, bufRefCnt);
	if(ret) {
		ERR("config capture failed....");
	}
	
	/* Init msg for new image */

	ret = capture_get_input_info(hDataCap->hCap, &hDataCap->inputInfo);
	assert(ret == E_NO);

	/* set capture info to params manager */
	if(app_hdr_msg_send(hDataCap->hMsg, MSG_CTRL, APPCMD_SET_CAP_INPUT, 0, 0)) {
		ERR("send msg to update input info failed");
	}
	
	hDataCap->capDim = hDataCap->inputInfo;
	
	DBG("capture out size: %u X %u", hDataCap->capDim.width, hDataCap->capDim.height);

	/* Restart capture */
	if(hDataCap->status & CAPTHR_STAT_CAP_STARTED)
		capture_start(hDataCap->hCap);

	/* Set default capture info */
	CaptureInfo *capInfo = &hDataCap->defCapInfo;
	bzero(capInfo, sizeof(*capInfo));
	capInfo->capCnt = 1;
	
	hDataCap->exit = FALSE;
	return E_NO;
}

/*****************************************************************************
 Prototype    : detector_config
 Description  : config detector
 Input        : DataCapHandle hDataCap  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/1
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 detector_config(DataCapHandle hDataCap)
{
	CamDetectorParam 	*detectorParams = &hDataCap->detectorParams;
	Int32				ret;

	/* reset fd of detector */
	hDataCap->fdDetect = -1;
	
	if(!hDataCap->hDetector) {
		/* create */
		hDataCap->hDetector = detector_create(detectorParams);
		if(!hDataCap->hDetector) {
			ERR("create new detector failed");
			ret = E_IO;
		}
		/* init default capture info */
		CaptureInfo *capInfo = &hDataCap->defCapInfo;

		memset(capInfo, 0, sizeof(CaptureInfo));
		capInfo->capCnt = 1;
		capInfo->triggerInfo[0].frameId = FRAME_CONTINUE;
		
	} else {
		/* cfg detector */
		ret = detector_control(hDataCap->hDetector, DETECTOR_CMD_SET_PARAMS, 
				detectorParams, sizeof(CamDetectorParam));
		if(ret) {
			ERR("set detector params failed");
		}
	}
	
	data_capture_get_fds(hDataCap);

	return ret;
}

/*****************************************************************************
 Prototype    : data_cap_ctrl
 Description  : control capture
 Input        : DataCapHandle hDataCap  
                CamCapCtrl ctrl         
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/1
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 data_cap_ctrl(DataCapHandle hDataCap, CamCapCtrl ctrl)
{
	Int32 ret = E_NO;

	//DBG("capture ctrl: %d", ctrl);

	/* stop capture */
	if(ctrl == CAM_CAP_STOP || ctrl == CAM_CAP_RESTART) {
		if(hDataCap->status & CAPTHR_STAT_CAP_STARTED) {
			ret = capture_stop(hDataCap->hCap);
			if(!ret) {
				hDataCap->status &= ~CAPTHR_STAT_CAP_STARTED;
				/* stop detector */
				ret = detector_control( hDataCap->hDetector, 
										DETECTOR_CMD_STOP, 
										NULL, 0 );
			}
		}
	}

	/* start capture */
	if(ctrl == CAM_CAP_START || ctrl == CAM_CAP_RESTART) {
		if(!(hDataCap->status & CAPTHR_STAT_CAP_STARTED))
			ret = capture_start(hDataCap->hCap);
		if(!ret) {
			hDataCap->status |= CAPTHR_STAT_CAP_STARTED;
			/* start detector */
			ret = detector_control( hDataCap->hDetector, 
									DETECTOR_CMD_START, 
									NULL, 0 );
			ret |= data_capture_get_fds(hDataCap);
		}
	}

	/* trigger capture */
	if((ctrl == CAM_CAP_TRIG || ctrl == CAM_CAP_SPEC_TRIG) && 
		(hDataCap->status & CAPTHR_STAT_CAP_STARTED)) {
		hDataCap->encImg = TRUE; // capture next frame
		/* set capture info for mannual trigger */
		hDataCap->capInfo.capCnt = 1;
		hDataCap->capInfo.flags = 0;
		hDataCap->capInfo.triggerInfo[0].frameId = FRAME_MANUALTRIG;
		hDataCap->capIndex = -1;

		int cmd = ((ctrl == CAM_CAP_TRIG) ? IMGCTRL_TRIGCAP : IMGCTRL_SPECTRIG);
		ret = ioctl(hDataCap->fdImgCtrl, cmd, &hDataCap->capIndex);
		if(ret < 0) {
			ERRSTR("trig failed");
			ret = E_IO;
		} else {
			//DBG("trig %d success", ctrl);
		}
	}

	return ret;
}

/*****************************************************************************
 Prototype    : send_video_clip
 Description  : send msg to enable video clip sending
 Input        : DataCapHandle hDataCap  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/7/9
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline Int32 send_video_clip(DataCapHandle hDataCap)
{
	return app_hdr_msg_send(hDataCap->hMsg, MSG_VID_ENC, APPCMD_UPLOAD_CTRL, RTP_CMD_SND_VID_CLIP, 0);
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
static Int32 capture_new_img(DataCapHandle hDataCap)
{
	FrameBuf	capFrame;
	Int32		err;
	ImgMsg		imgMsg;
	const char	*dstName = NULL;

	/* get frame from driver */
	err = capture_get_frame(hDataCap->hCap, &capFrame);
	if(err) {
		ERR("get new frame failed.");
		return err;
	}

	/* choose strem channel Id */
	Int32 		streamId = 0;
	CaptureInfo	*capInfo = &hDataCap->defCapInfo;

	/* parse raw capture info */
	cap_info_parse((Uint8 *)capFrame.dataBuf, &hDataCap->capDim, &imgMsg.rawInfo);
	/*
	DBG("capInfo: index: %d, cap mode: %d, avg Y: %u", 
		imgMsg.rawInfo.index, imgMsg.rawInfo.capMode, imgMsg.rawInfo.avgLum);
	DBG("  strobe: 0x%X, exposure: %u, global gain: %u", 
		imgMsg.rawInfo.strobeStat, imgMsg.rawInfo.exposure, imgMsg.rawInfo.globalGain);
	*/
	if(hDataCap->encImg) {
		/* check frame index from img data */
		Uint16 frameIndex = imgMsg.rawInfo.index;
		
		if(hDataCap->capIndex < 0 || hDataCap->capIndex <= frameIndex) {
			if(hDataCap->dstName[1])
				streamId = 1;
			dstName = hDataCap->dstName[1];
			hDataCap->encImg = FALSE;
			capInfo = &hDataCap->capInfo;

			/* enable video send */
			send_video_clip(hDataCap);
		}
	}

	/* do convert */
	//DBG("<%d> conv one frame", capFrame->index);
	imgMsg.dimension = hDataCap->capDim;
	imgMsg.timeCode = capFrame.timeStamp;
	imgMsg.roadInfo = hDataCap->roadInfo;
	cam_convert_time(&capFrame.timeStamp, &imgMsg.timeStamp);
	err = converter_run(hDataCap->hConverter, &capFrame, streamId, &imgMsg);

	/* free cap buffer */
	capture_free_frame(hDataCap->hCap, &capFrame);

	if(err) {
		/* convert failed, just return */
		return err;
	}
	
	/* send msg */
	imgMsg.capInfo = *capInfo;
	err = msg_send(hDataCap->hMsg, dstName, (MsgHeader *)&imgMsg, 0);
	//err = 1;
	if(err) {
		DBG("msg send err");
		/* err happens, free buf after convert */
		buf_pool_free(imgMsg.hBuf);
	} //else
		//DBG("<%d> cap run ok...", capFrame.index);

#ifdef CAP_TRIG_TEST
	if((capFrame.index % 150) == 0) {
		hDataCap->encImg = TRUE;
		hDataCap->capInfo = hDataCap->defCapInfo;
	}
#endif

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
static Int32 msg_process(DataCapHandle hDataCap, CommonMsg *msgBuf)
{
	Int32 		err;
	MsgHeader 	*msgHdr = &msgBuf->header;
	
	err = msg_recv(hDataCap->hMsg, msgHdr, sizeof(CommonMsg), 0);
	if(err < 0)
		return err;

	switch(msgHdr->cmd) {
	case APPCMD_CAP_EN:
		err = data_cap_ctrl(hDataCap, msgHdr->param[0]);
		break;
	case APPCMD_SET_WORK_MODE:
		err = data_capture_config(hDataCap);
		break;
	case APPCMD_SET_IMG_CONV:
		err = converter_params_update(hDataCap->hConverter, &hDataCap->convParams);
		break;
	case APPCMD_SET_TRIG_PARAMS:
		err = detector_config(hDataCap);
		break;
	case APPCMD_SET_ROAD_INFO:
		hDataCap->roadInfo = *(CamRoadInfo *)msgBuf->buf;
		err = E_NO;
		break;
	case APPCMD_EXIT:
		hDataCap->exit = TRUE;
		break;
	default:
		INFO("cap thr, unsupport cmd: 0x%X", (unsigned int)msgHdr->cmd);
		err = E_UNSUPT;
		break;
	}

	return err;
}

/*****************************************************************************
 Prototype    : detector_trigger
 Description  : trigger capture
 Input        : DataCapHandle hDataCap  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/1
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 detector_trigger(DataCapHandle hDataCap)
{
	Int32 		ret;
	CaptureInfo	*capInfo = &hDataCap->capInfo;

	/* run detector to get trigger info */
	ret = detector_run(hDataCap->hDetector, capInfo);
	if(!ret && capInfo->capCnt && (hDataCap->status & CAPTHR_STAT_CAP_STARTED)) {
		/* need trigger */
		hDataCap->encImg = TRUE;
		int cmd;
		if(CAPTHR_STAT_TRIG & hDataCap->status) {
			/* set trigger cmd */
			hDataCap->capIndex = CAP_INDEX_NEXT_FRAME;
			cmd = IMGCTRL_TRIGCAP;
		} else {
			/* we may set special trigger and use index of that frame */
			hDataCap->capIndex = CAP_INDEX_NEXT_FRAME;
			cmd = IMGCTRL_SPECTRIG;
		}

		/* send trig cmd */
		ret = ioctl(hDataCap->fdImgCtrl, cmd, NULL);
		if(ret < 0) {
			ERRSTR("trig capture failed");
			ret = E_IO;
		}
	}

	return ret;
}


/*****************************************************************************
 Prototype    : data_capture_thread
 Description  : data capture thread function
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
static void *data_capture_thread(void *arg)
{
	DataCapHandle	hDataCap = (DataCapHandle)arg;
	Int32			ret;
	fd_set			rdSet;
	CommonMsg 		msgBuf;

	assert(hDataCap);

	/* get fds for detect */
	data_capture_get_fds(hDataCap);
	
	/* start capture */
	ret = capture_start(hDataCap->hCap);
	if(ret) {
		ERR("Start capture failed.");
		goto exit;
	}
	hDataCap->status |= CAPTHR_STAT_CAP_STARTED;

	DBG("data capture thread start...");
	
	while(!hDataCap->exit) {
		/* wait data ready */
		FD_ZERO(&rdSet);
		FD_SET(hDataCap->fdCap, &rdSet);
		FD_SET(hDataCap->fdMsg, &rdSet);
		if(hDataCap->fdDetect > 0)
			FD_SET(hDataCap->fdDetect, &rdSet);
		
		ret = select(hDataCap->fdMax, &rdSet, NULL, NULL, NULL);
		if(ret < 0 && errno != EINTR) {
			ERRSTR("select err");
			usleep(1000);
			continue;
		}

		/* check which is ready */
		if( hDataCap->fdDetect > 0 &&
			FD_ISSET(hDataCap->fdDetect, &rdSet) ){
			/* trigger capture */
			detector_trigger(hDataCap);
		}
		
		if(FD_ISSET(hDataCap->fdCap, &rdSet)) {
			/* read new frame */
			capture_new_img(hDataCap);
		}

		if(FD_ISSET(hDataCap->fdMsg, &rdSet)) {
			/* process msg */
			msg_process(hDataCap, &msgBuf);
		}
	}


exit:

	/* stop capture first */
	if(hDataCap->status & CAPTHR_STAT_CAP_STARTED)
		capture_stop(hDataCap->hCap);

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
DataCapHandle data_capture_create(const DataCapAttrs *attrs)
{
	assert(attrs);

	DataCapHandle 	hDataCap;
	Int32 			ret;

	hDataCap = calloc(1, sizeof(struct DataCapObj));
	if(!hDataCap) {
		ERR("alloc mem failed.");
		return NULL;
	}

	hDataCap->hMsg = msg_create(MSG_CAP, hDataCap->dstName[0], 0);
	if(!hDataCap->hMsg) {
		ERR("create msg handle failed...");
		goto exit;
	}

	hDataCap->roadInfo = attrs->roadInfo;
	hDataCap->workMode = attrs->workMode;
	hDataCap->hCap = attrs->hCapture;
	if(!hDataCap->hCap) {
		ERR("create cap module failed...");
		goto exit;
	}

	capture_get_input_info(hDataCap->hCap, &hDataCap->inputInfo);

	/* create converter */
	ConverterAttrs 	convAttrs;
	hDataCap->convParams = attrs->convParams;

	convAttrs.maxImgWidth = attrs->maxOutWidth;
	convAttrs.maxImgHeight = attrs->maxOutHeight;
	convAttrs.bufNum = CONV_BUF_NUM;
	convAttrs.flags = 0;

	/* set input to current info */
	hDataCap->hConverter = converter_create(&convAttrs, &hDataCap->convParams);
	if(!hDataCap->hConverter) {
		ERR("creater converter failed.");
		goto exit;
	}

	/* record params */
	hDataCap->detectorParams = attrs->detectorParams;

	/* init our running evironment */
	ret = data_capture_config(hDataCap);
	if(ret) {
		goto exit;	
	}

	/* init detector */
	ret = detector_config(hDataCap);
	//if(ret)
		//goto exit;

	/* open img ctrl dev for trigger */
	hDataCap->fdImgCtrl = open(IMG_CTRL_DEV, O_RDWR);
	if(hDataCap->fdImgCtrl < 0) {
		ERRSTR("open %s failed", IMG_CTRL_DEV);
	}

	return hDataCap;

exit:
	
	data_capture_delete(hDataCap, NULL);
	return NULL;
}

/*****************************************************************************
 Prototype    : data_capture_send_cmd
 Description  : send cmd to our pthread
 Input        : DataCapHandle hDataCap  
                Uint16 cmd              
                MsgHandle hCurMsg       
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/21
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 data_capture_send_cmd(DataCapHandle hDataCap, Uint16 cmd, MsgHandle hCurMsg)
{
	if(!hDataCap || !hCurMsg)
		return E_INVAL;

	/* make sure we have start running */
	if(!hDataCap->pid)
		return E_MODE;

	/* send cmd to our thread */
	Int32 		err;
	MsgHeader 	msg;

	msg.cmd = cmd;
	msg.dataLen = 0;
	msg.index = 0;
	msg.type = MSG_TYPE_REQU;

	err = msg_send(hCurMsg, msg_get_name(hDataCap->hMsg), &msg, 0);
	return err;
}

/*****************************************************************************
 Prototype    : data_capture_delete
 Description  : delete data capture handle
 Input        : DataCapHandle hDataCap  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/16
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 data_capture_delete(DataCapHandle hDataCap, MsgHandle hCurMsg)
{
	if(!hDataCap)
		return E_INVAL;
	
	if(hDataCap->pid) {
		/* send exit cmd */
		data_capture_send_cmd(hDataCap, APPCMD_EXIT, hCurMsg);
		
		/* set flag to exit */
		hDataCap->exit = TRUE;
		
		pthread_join(hDataCap->pid, NULL);
	}

	/* delete our own msg module */
	if(hDataCap->hMsg)
		msg_delete(hDataCap->hMsg);

	if(hDataCap->hConverter)
		converter_delete(hDataCap->hConverter);

	if(hDataCap->hDetector)
		detector_delete(hDataCap->hDetector);

	if(hDataCap->fdImgCtrl > 0)
		close(hDataCap->fdImgCtrl);

	free(hDataCap);

	return E_NO;
}

/*****************************************************************************
 Prototype    : data_capture_run
 Description  : run data collect
 Input        : DataCapHandle hDataCap  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/16
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 data_capture_run(DataCapHandle hDataCap)
{
	Int32 err;

	if(!hDataCap)
		return E_INVAL;
	
	/* create thread and run data collecting */
	err = pthread_create(&hDataCap->pid, NULL, data_capture_thread, hDataCap);
	if(err < 0) {
		ERRSTR("create data capture thread failed...");
		return E_NOMEM;
	}

	return E_NO;
}

/*****************************************************************************
 Prototype    : data_capture_set_work_mode
 Description  : set work mode
 Input        : DataCapHandle hDataCap  
                MsgHandle hCurMsg       
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/21
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 data_capture_set_work_mode(DataCapHandle hDataCap, MsgHandle hCurMsg, const CamWorkMode *workMode)
{
	if(!workMode)
		return E_INVAL;

	hDataCap->workMode = *workMode;

	return data_capture_send_cmd(hDataCap, APPCMD_SET_WORK_MODE, hCurMsg);
}

/*****************************************************************************
 Prototype    : data_capture_conv_params
 Description  : set convert params
 Input        : DataCapHandle hDataCap  
                MsgHandle hCurMsg       
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/21
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 data_capture_set_conv_params(DataCapHandle hDataCap, MsgHandle hCurMsg, const ConverterParams *params)
{	
	if(!params)
		return E_INVAL;
	
	hDataCap->convParams = *params;

	return data_capture_send_cmd(hDataCap, APPCMD_SET_IMG_CONV, hCurMsg);
}

/*****************************************************************************
 Prototype    : data_capture_set_detector_params
 Description  : set detector params
 Input        : DataCapHandle hDataCap          
                MsgHandle hCurMsg               
                const CamDetectorParam *params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 data_capture_set_detector_params(DataCapHandle hDataCap, MsgHandle hCurMsg, const CamDetectorParam *params)
{	
	if(!params)
		return E_INVAL;
	
	hDataCap->detectorParams = *params;

	return data_capture_send_cmd(hDataCap, APPCMD_SET_TRIG_PARAMS, hCurMsg);
}


/*****************************************************************************
 Prototype    : data_capture_get_input_info
 Description  : get input info
 Input        : DataCapHandle hDataCap   
                CamInputInfo *inputInfo  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 data_capture_get_input_info(DataCapHandle hDataCap, ImgDimension *inputInfo)
{
	if(!hDataCap || !inputInfo) {
		return E_INVAL;
	}

	*inputInfo = hDataCap->inputInfo;

	return E_NO;
}

/*****************************************************************************
 Prototype    : data_capture_ctrl
 Description  : change capture status
 Input        : DataCapHandle hDataCap  
                MsgHandle hCurMsg       
                CamCapCtrl ctrl         
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 data_capture_ctrl(DataCapHandle hDataCap, MsgHandle hCurMsg, Int32 ctrl)
{
	if(!hDataCap)
		return E_INVAL;
	
	return app_hdr_msg_send(hCurMsg, msg_get_name(hDataCap->hMsg), APPCMD_CAP_EN, ctrl, 0);
}

/*****************************************************************************
 Prototype    : data_capture_set_road_info
 Description  : update road info
 Input        : DataCapHandle hDataCap  
                MsgHandle hCurMsg       
                CamRoadInfo *info       
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/1
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 data_capture_set_road_info(DataCapHandle hDataCap, MsgHandle hCurMsg, CamRoadInfo *info)
{
	if(!hDataCap || !info || !hCurMsg) {
		return E_INVAL;
	}

	struct {
		MsgHeader		hdr;
		CamRoadInfo		info;
	} msg;

	/* fill header fileds */
	msg.hdr.type = MSG_TYPE_REQU;
	msg.hdr.index = 0;
	msg.hdr.dataLen = sizeof(CamRoadInfo);
	msg.hdr.cmd = APPCMD_SET_ROAD_INFO;
	msg.info = *info;

	/* send msg */
	Int32 err = msg_send(hCurMsg, msg_get_name(hDataCap->hMsg), (MsgHeader *)&msg, 0);
	
	return err;
}


