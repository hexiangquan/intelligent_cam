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

#define CAP_DEVICE					"/dev/video0"
#define CAP_BUF_NUM					3
#define CONV_BUF_NUM				2

#define CAP_TRIG_TEST

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/* thread running environment */
struct DataCapObj{
	ParamsMngHandle 	hParamsMng;
	CapHandle			hCap;
	CapInputInfo		inputInfo;
	ConverterHandle		hConverter;
	Bool				encImg;
	Bool				exit;
	Int32				status;
	MsgHandle			hMsg;
	ImgDimension		capDim;
	pthread_t			pid;
	const char 			*dstName[2];
};

/*****************************************************************************
 Prototype    : cap_module_init
 Description  : create capture module
 Input        : ParamsMngHandle hParamsMng  
                CamWorkMode *workMode       
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/19
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static CapHandle cap_module_init(ParamsMngHandle hParamsMng)
{
	/* get current work mode  & img enhance params */
	CapAttrs		capAttrs;
	CapInputInfo	inputInfo;
	CapHandle		hCap;
	CamWorkMode 	workMode;
	Int32			ret;

	/* fill attrs for capture create */
	capAttrs.devName = CAP_DEVICE;
	capAttrs.inputType = CAP_INPUT_CAMERA;
	capAttrs.userAlloc = TRUE;
	capAttrs.bufNum = CAP_BUF_NUM;
	capAttrs.defRefCnt = 1;

	ret = params_mng_control(hParamsMng, PMCMD_G_WORKMODE, &workMode, sizeof(workMode));
	assert(ret == E_NO);

	capAttrs.std = 
		(workMode.resType == CAM_RES_FULL_FRAME) ? CAP_STD_FULL_FRAME : CAP_STD_HIGH_SPEED;
	capAttrs.mode = 
		(workMode.capMode == CAM_CAP_MODE_CONTINUE) ? CAP_MODE_CONT : CAP_MODE_TRIG;
	
	/* create capture object */
	hCap = capture_create(&capAttrs);
	if(!hCap) {
		ERR("create capture handle failed...");
		return NULL;
	}

	capture_get_input_info(hCap, &inputInfo);

	/* set capture info to params manager */
	ret = params_mng_control(hParamsMng, PMCMD_S_CAPINFO, &inputInfo, sizeof(CapInputInfo));
	assert(ret == E_NO);

	return hCap;
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
	CamWorkMode			workMode;
	Int32				ret;
	Int32				bufRefCnt = 1;
	
	ret = params_mng_control(hDataCap->hParamsMng, PMCMD_G_WORKMODE, &workMode, sizeof(workMode));
	assert(ret == E_NO);

	/* alloc encode for continously stream, test */
	if(workMode.capMode == CAM_CAP_MODE_CONTINUE)
		hDataCap->encImg = FALSE;
	
	hDataCap->dstName[1] = NULL;
	hDataCap->status |= CAPTHR_STAT_CONV_EN;
	
	if(workMode.format == CAM_FMT_JPEG) {
		hDataCap->dstName[0] = MSG_IMG_ENC;
		DBG("snd stream to jpg only");
	} else if(workMode.format == CAM_FMT_H264) {
		hDataCap->dstName[0] = MSG_VID_ENC;
		DBG("snd stream to h.264 only");
	} else if(workMode.format == CAM_FMT_JPEG_H264) {
		hDataCap->dstName[0] = MSG_VID_ENC;
		hDataCap->dstName[1] = MSG_IMG_ENC;
		bufRefCnt = 1; //convert only in one format
		DBG("snd stream to h.264 and jpeg");
	} else {
		hDataCap->dstName[0] = MSG_IMG_TX;	// send directly
		hDataCap->status &= ~CAPTHR_STAT_CONV_EN;
		DBG("snd stream to tx only");
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
	
	std = (workMode.resType == CAM_RES_HIGH_SPEED) ? CAP_STD_HIGH_SPEED : CAP_STD_FULL_FRAME;
	mode = (workMode.capMode == CAM_CAP_MODE_CONTINUE) ? CAP_MODE_CONT : CAP_MODE_TRIG;

	/* Change attrs */
	ret = capture_config(hDataCap->hCap, std, mode);
	ret |= capture_set_def_frame_ref_cnt(hDataCap->hCap, bufRefCnt);
	if(ret) {
		ERR("config capture failed....");
	}
	
	/* Init msg for new image */
	CapInputInfo inputInfo;
	
	ret = capture_get_input_info(hDataCap->hCap, &inputInfo);
	assert(ret == E_NO);

	/* set capture info to params manager */
	ret = params_mng_control(hDataCap->hParamsMng, PMCMD_S_CAPINFO, &inputInfo, sizeof(CapInputInfo));
	assert(ret == E_NO);
	
	hDataCap->capDim = inputInfo;
	
	DBG("capture out size: %u X %u", hDataCap->capDim.width, hDataCap->capDim.height);

	/* Restart capture */
	if(hDataCap->status & CAPTHR_STAT_CAP_STARTED)
		capture_start(hDataCap->hCap);
	
	hDataCap->exit = FALSE;
	return E_NO;
}

static Int32 data_cap_ctrl(DataCapHandle hDataCap, CamCapCtrl ctrl)
{
	Int32 ret = E_NO;

	/* stop capture */
	if(ctrl == CAM_CAP_STOP || ctrl == CAM_CAP_RESTART) {
		if(hDataCap->status & CAPTHR_STAT_CAP_STARTED) {
			ret = capture_stop(hDataCap->hCap);
			if(!ret)
				hDataCap->status &= ~CAPTHR_STAT_CAP_STARTED;
		}
	}

	/* start capture */
	if(ctrl == CAM_CAP_START || ctrl == CAM_CAP_RESTART) {
		if(!(hDataCap->status & CAPTHR_STAT_CAP_STARTED))
			ret = capture_start(hDataCap->hCap);
		if(!ret)
			hDataCap->status |= CAPTHR_STAT_CAP_STARTED;
	}

	/* trigger capture */
	if((ctrl == CAM_CAP_TRIG || ctrl == CAM_CAP_SPEC_TRIG) && 
		(hDataCap->status & CAPTHR_STAT_CAP_STARTED)) {
		hDataCap->encImg = TRUE; // capture next frame
	}

	return ret;
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
	Int32 	streamId;
	if(hDataCap->dstName[1] && hDataCap->encImg) {
		streamId = 1;
		dstName = hDataCap->dstName[1];
		hDataCap->encImg = FALSE;
		/* add capture info here */
	} else {
		streamId = 0;
	}

	/* do convert */
	//DBG("<%d> conv one frame", capFrame->index);
	imgMsg.dimension = hDataCap->capDim;
	cam_convert_time(&capFrame.timeStamp, &imgMsg.timeStamp);
	err = converter_run(hDataCap->hConverter, &capFrame, streamId, &imgMsg);

	/* free cap buffer */
	capture_free_frame(hDataCap->hCap, &capFrame);

	if(err) {
		/* convert failed, just return */
		return err;
	}
	
	/* send msg */
	err = msg_send(hDataCap->hMsg, dstName, (MsgHeader *)&imgMsg, 0);
	//err = 1;
	if(err) {
		DBG("msg send err");
		/* err happens, free buf after convert */
		buf_pool_free(imgMsg.hBuf);
	} else
		DBG("<%d> cap run ok...", capFrame.index);

#ifdef CAP_TRIG_TEST
	if((capFrame.index % 4) == 0)
		hDataCap->encImg = TRUE;
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
	case APPCMD_FREE_RAW:
		//err = free_raw_buf(hDataCap, (RawMsg *)msgBuf);
		err = E_NO;
		break;
	case APPCMD_SET_WORK_MODE:
		err = data_capture_config(hDataCap);
		break;
	case APPCMD_SET_IMG_CONV:
		err = converter_params_update(hDataCap->hConverter);
		break;
	case APPCMD_CAP_EN:
		err = data_cap_ctrl(hDataCap, msgHdr->param[0]);
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
	Int32			fdCap, fdMsg, fdMax;
	fd_set			rdSet;
	CommonMsg 		msgBuf;

	assert(hDataCap);

	/* get fd for select */
	fdCap = capture_get_fd(hDataCap->hCap);
	fdMsg = msg_get_fd(hDataCap->hMsg);
	fdMax = MAX(fdMsg, fdCap) + 1;
	
	/* start capture */
	ret = 0;//capture_start(hDataCap->hCap);
	if(ret) {
		ERR("Start capture failed.");
		goto exit;
	}
	hDataCap->status |= CAPTHR_STAT_CAP_STARTED;
	
	while(!hDataCap->exit) {
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
			capture_new_img(hDataCap);
		}

		if(FD_ISSET(fdMsg, &rdSet)) {
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
DataCapHandle data_capture_create(DataCapAttrs *attrs)
{
	assert(attrs && attrs->hParamsMng);

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
	
	hDataCap->hParamsMng = attrs->hParamsMng;
	hDataCap->hCap = cap_module_init(attrs->hParamsMng);

	/* create converter */
	ConverterAttrs convAttrs;
	convAttrs.hParamsMng = attrs->hParamsMng;
	convAttrs.maxImgWidth = attrs->maxOutWidth;
	convAttrs.maxImgHeight = attrs->maxOutHeight;
	convAttrs.bufNum = CONV_BUF_NUM;
	convAttrs.flags = 0;

	hDataCap->hConverter = converter_create(&convAttrs);
	if(!hDataCap->hConverter) {
		ERR("creater converter failed.");
		goto exit;
	}
	

	/* init our running evironment */
	ret = data_capture_config(hDataCap);
	if(ret) {
		goto exit;	
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
	
	if(hDataCap->pid > 0) {
		/* send exit cmd */
		data_capture_send_cmd(hDataCap, APPCMD_EXIT, hCurMsg);
		
		/* set flag to exit */
		hDataCap->exit = TRUE;
		
		pthread_join(hDataCap->pid, NULL);
	}

	/* delete our own msg module */
	if(hDataCap->hMsg)
		msg_delete(hDataCap->hMsg);

	/* delete capture handle */
	if(hDataCap->hCap)
		capture_delete(hDataCap->hCap);

	if(hDataCap->hConverter)
		converter_delete(hDataCap->hConverter);

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
Int32 data_capture_set_work_mode(DataCapHandle hDataCap, MsgHandle hCurMsg)
{
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
Int32 data_capture_conv_params(DataCapHandle hDataCap, MsgHandle hCurMsg)
{
	return data_capture_send_cmd(hDataCap, APPCMD_SET_IMG_CONV, hCurMsg);
}


