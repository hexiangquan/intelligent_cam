/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : img_enc_thr.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/13
  Last Modified :
  Description   : send image data thread
  Function List :
              img_enc_params_update
              img_enc_thr
              img_enc_thr_init
              img_enc_thr_run
              msg_process
  History       :
  1.Date        : 2012/3/13
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "encoder.h"
#include "alg.h"
#include "add_osd.h"
#include "log.h"
#include "crc16.h"
#include "upload.h"
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
static Int32 upload_update(EncoderHandle hEnc);

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

#define ENC_POOL_BUF_NUM		3
#define BUF_ALLOC_TIMEOUT		1000		//ms
#define UPLOAD_RECON_TIMEOUT	5		//second

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/* module environment */
struct EncoderObj {
	const char			*name;			//our module name
	const char			*saveRootPath;	//root path for save
	const EncoderOps	*ops;
	ParamsMngHandle 	hParamsMng;
	UploadHandle		hUpload;
	AlgHandle			hEncode;
	MsgHandle			hMsg;
	BufPoolHandle		hPoolIn;
	OsdHandle			hOsd;
	CamOsdInfo			osdInfo;
	BufHandle			hBufEnc;		//buffer for encode out
	BufPoolHandle		hPoolEnc;
	ParamsMngCtrlCmd	cmdGetEncDyn;
	ParamsMngCtrlCmd	cmdGetOsdDyn;
	ParamsMngCtrlCmd	cmdGetOsdInfo;
	ParamsMngCtrlCmd	cmdGetUploadProto;
	CamImgUploadProto	uploadProto;
	pthread_t			pid;
	pthread_mutex_t		*mutex;
	Bool				exit;
};

/*****************************************************************************
 Prototype    : img_enc_thr_init
 Description  : Init thread
 Input        : ImgEncThrArg *arg   
                EncoderHandle hEnc  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/8
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 encoder_init(EncoderHandle hEnc, EncoderAttrs *attrs)
{
	Int32	err;
	Int8	dynParamsBuf[1024];
	
	assert(attrs && attrs->hParamsMng && attrs->encFxns && attrs->encInitParams);

	hEnc->name = attrs->name;

	/* set params input */
	hEnc->hParamsMng = attrs->hParamsMng;
	hEnc->cmdGetEncDyn = attrs->cmdGetEncDyn;
	hEnc->cmdGetOsdDyn = attrs->cmdGetOsdDyn;
	hEnc->cmdGetOsdInfo = attrs->cmdGetOsdInfo;
	hEnc->cmdGetUploadProto = attrs->cmdGetUploadProto;
	hEnc->saveRootPath = attrs->saveRootPath;
	hEnc->ops = attrs->encOps;
	hEnc->mutex = attrs->mutex;

	assert(hEnc->ops && hEnc->mutex);
	
	/* get dyn params, we may need wait capture ready */
	err = params_mng_control(hEnc->hParamsMng, hEnc->cmdGetEncDyn, 
			dynParamsBuf, sizeof(dynParamsBuf));
	if(err) {
		ERR("<%s> get enc dyn params failed.", hEnc->name);
		return err;
	}

	/* create enc handle */
	hEnc->hEncode = alg_create(attrs->encFxns, attrs->encInitParams, dynParamsBuf);
	if(!hEnc->hEncode) {
		ERR("<%s> create enc handle failed", hEnc->name);
		return E_INVAL;
	}

	/* create osd handle */
	OsdInitParams	osdInitParams;
	OsdDynParams	osdDynParams;

	osdInitParams.size = sizeof(osdInitParams);
	osdInitParams.asc16Tab = NULL;
	osdInitParams.hzk16Tab = NULL;

	err = params_mng_control(hEnc->hParamsMng, hEnc->cmdGetOsdDyn, 
			&osdDynParams, sizeof(osdDynParams));
	if(err) {
		ERR("<%s> get enc dyn params failed.", hEnc->name);
		return err;
	}	
	
	hEnc->hOsd = osd_create(&osdInitParams, &osdDynParams);
	if(!hEnc->hOsd) {
		ERR("<%s> create osd handle failed", hEnc->name);
		return E_INVAL;
	}

	/* get current osd config */
	err = params_mng_control(hEnc->hParamsMng, hEnc->cmdGetOsdInfo, 
				&hEnc->osdInfo, sizeof(CamOsdInfo));
	assert(err == E_NO);

	/* create msg handle  */
	hEnc->hMsg = msg_create(attrs->msgName, attrs->dstName, 0);
	if(!hEnc->hMsg) {
		ERR("<%s> create msg handle failed", hEnc->name);
		return E_NOMEM;
	}

	/* create buffer pool for encoded image */
	Uint32 bufSize = attrs->encBufSize;
	bufSize = ROUND_UP(bufSize, 256);
	
	BufAllocAttrs	allocAttrs;
	allocAttrs.align = 256;
	allocAttrs.flags = 0;
	allocAttrs.type = BUF_TYPE_POOL;
	
	if(attrs->poolBufNum > 0) {
		hEnc->hPoolEnc = buf_pool_create(bufSize, attrs->poolBufNum, &allocAttrs);
		if(!hEnc->hPoolEnc) {
			ERR("<%s> create enc pool failed", hEnc->name);
			return E_NOMEM;
		}
	}

	/* alloc one buffer for save to local */
	hEnc->hBufEnc = buffer_alloc(bufSize, &allocAttrs);
	if(!hEnc->hBufEnc) {
		ERR("<%s> alloc buf for enc failed", hEnc->name);
		return E_NOMEM;
	}

	/* update upload params */
	err = upload_update(hEnc);
	if(err)
		return err;

	hEnc->exit = FALSE;

	return E_NO;
}

/*****************************************************************************
 Prototype    : img_enc_thr_run
 Description  : add osd and do jpeg encode
 Input        : EncoderHandle hEnc  
                ImgMsg *msg         
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/9
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 encode_frame(EncoderHandle hEnc, ImgMsg *msg)
{
	Int32 	err;

	/* Add osd first & ignore error */
	err = add_osd(hEnc->hOsd, msg, &hEnc->osdInfo);

	/* Init buffer for encode   */
	AlgBuf		inBuf, outBuf;
	BufHandle	hBufIn, hBufOut;
	Bool		saveToLocal = FALSE;
	
	hBufIn = msg->hBuf;
	inBuf.buf = buffer_get_user_addr(hBufIn);
	inBuf.bufSize = buffer_get_bytes_used(hBufIn);
	hEnc->hPoolIn = buffer_get_pool(hBufIn);
	
	if(hEnc->hPoolEnc) {
		/* Alloc buffer for encoded data */
		//hBufOut = buf_pool_alloc_wait(hEnc->hPoolEnc, BUF_ALLOC_TIMEOUT);
		hBufOut = buf_pool_alloc(hEnc->hPoolEnc);
		if(!hBufOut) {
			/* alloc buffer failed, use buf for save */
			DBG("<%s> save file to local file system", hEnc->name);
			hBufOut = hEnc->hBufEnc;
			saveToLocal = TRUE;
		}
	} else {
		/* no pool */
		hBufOut = hEnc->hBufEnc;
	}
	
	outBuf.buf = buffer_get_user_addr(hBufOut);
	outBuf.bufSize = buffer_get_size(hBufOut);
	if(!outBuf.buf || !outBuf.bufSize) {
		ERR("<%s> got invalid encode buf handle", hEnc->name);
		err = E_NOMEM;
		goto err_quit;
	}

	/* do encode process */
	//pthread_mutex_lock(hEnc->mutex);
	err = hEnc->ops->encProcess(hEnc->hEncode, &inBuf, &outBuf, msg);
	//pthread_mutex_unlock(hEnc->mutex);
	if(err) {
		ERR("<%s> enc err: %s", hEnc->name, str_err(err));
		goto err_quit;
	}

	/* free input buffer */
	buf_pool_free(hBufIn);
	hBufIn = NULL;

	/* modify msg for send */
	msg->hBuf = hBufOut;

	if(!saveToLocal) {
		/* upload to server */
		err = upload_run(hEnc->hUpload, hEnc->hMsg, msg);
	}
	
	/* save to local filesys */
	if((saveToLocal || err) && hEnc->ops->saveFrame) {
		err = hEnc->ops->saveFrame(msg, hEnc->saveRootPath);
		goto err_quit;
	}

	//DBG("<%d> %s encode ok", msg->index, hEnc->name);

	return E_NO;

err_quit:

	/* free buffers */
	if(hBufIn)
		buf_pool_free(hBufIn);
	
	if(!saveToLocal && hEnc->hPoolEnc)
		buf_pool_free(hBufOut);

	return err;
}

/*****************************************************************************
 Prototype    : enc_params_update
 Description  : update encode thread params
 Input        : EncoderHandle hEnc  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/9
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 enc_params_update(EncoderHandle hEnc)
{
	OsdDynParams	osdDynParams;
	Int8			encDynParamsBuf[512];
	Int32 			err;

	/* update osd params */
	err = params_mng_control(hEnc->hParamsMng, hEnc->cmdGetOsdDyn, 
			&osdDynParams, sizeof(osdDynParams));
	if(!err) {
		err = osd_control(hEnc->hOsd, OSD_CMD_SET_DYN_PARAMS, &osdDynParams);
		if(err)
			ERR("set osd params err.");
	}

	err = params_mng_control(hEnc->hParamsMng, hEnc->cmdGetOsdInfo, 
				&hEnc->osdInfo, sizeof(CamOsdInfo));

	/* update  enc params */
	err = params_mng_control(hEnc->hParamsMng, hEnc->cmdGetEncDyn, 
			encDynParamsBuf, sizeof(encDynParamsBuf));
	err = alg_control(hEnc->hEncode, ALG_CMD_SET_DYN_PARAMS, encDynParamsBuf);
	if(err)
		ERR("<%s> set enc dyn params err.", hEnc->name);

	return err;
}

/*****************************************************************************
 Prototype    : upload_update
 Description  : update upload params
 Input        : EncoderHandle hEnc  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/21
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 upload_update(EncoderHandle hEnc)
{
	/* get upload protol */
	Int32 				ret;
	CamImgUploadProto	proto;

	ret = params_mng_control(hEnc->hParamsMng, hEnc->cmdGetUploadProto, 
			&proto, sizeof(proto));
	if(ret) {
		ERR("get upload proto err.");
		return ret;
	}

	if(!hEnc->hUpload || proto != hEnc->uploadProto) {
		/* create upload handle */
		UploadAttrs uploadAttrs;

		/* delete old handle */
		if(hEnc->hUpload)
			upload_delete(hEnc->hUpload, hEnc->hMsg);

		uploadAttrs.flags = 0;
		if(hEnc->hPoolEnc)
			uploadAttrs.flags |= UPLOAD_FLAG_FREE_BUF; //free buffer after send when using pool
		uploadAttrs.hParamsMng = hEnc->hParamsMng;
		uploadAttrs.msgName = MSG_IMG_TX;
		uploadAttrs.protol = proto;
		uploadAttrs.reConTimeout = UPLOAD_RECON_TIMEOUT;
		uploadAttrs.savePath = hEnc->saveRootPath;

		hEnc->hUpload = upload_create(&uploadAttrs);
		if(!hEnc->hUpload) {
			ERR("<%s> create upload failed", hEnc->name);
			return E_INVAL;
		}
	}else {
		/* update params */
		ret = upload_update_params(hEnc->hUpload);
	}

	return ret;
}

/*****************************************************************************
 Prototype    : msg_process
 Description  : process msg
 Input        : EncoderHandle hEnc  
                CommonMsg *msgBuf   
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/8
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 msg_process(EncoderHandle hEnc, CommonMsg *msgBuf)
{
	Int32 ret;

	/* recv msg */
	ret = msg_recv(hEnc->hMsg, (MsgHeader *)msgBuf, sizeof(CommonMsg), 0);
	if(ret < 0) {
		ERR("img enc thr recv msg err: %s", str_err(ret));
		return ret;
	}

	/* process msg */
	MsgHeader *msgHdr = &msgBuf->header;
	switch(msgHdr->cmd) {
	case APPCMD_NEW_DATA:
		ret = encode_frame(hEnc, (ImgMsg *)msgBuf);
		break;
	case APPCMD_SET_ENC_PARAMS:
		ret = enc_params_update(hEnc);
		break;
	case APPCMD_SET_UPLOAD_PARAMS:
		ret = upload_update(hEnc);
		break;
	case APPCMD_EXIT:
		hEnc->exit = TRUE;
		break;
	default:
		ERR("unkown cmd: 0x%X", (unsigned int)msgHdr->cmd);
		ret = E_UNSUPT;
		break;
	}

	return ret;
}

/*****************************************************************************
 Prototype    : encoder_thread
 Description  : encode thread
 Input        : void *arg  
 Output       : None
 Return Value : void
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/8
    Author       : Sun
    Modification : Created function

*****************************************************************************/
void *encoder_thread(void *arg)
{
	EncoderHandle 	hEnc = (EncoderHandle)arg;
	CommonMsg		msgBuf;
	Int32			ret;
	Int32			fdMsg, fdMax;
	fd_set			rdSet;

	assert(arg);

	fdMsg = msg_get_fd(hEnc->hMsg);
	fdMax = fdMsg + 1;

	/* start main loop */
	while(!hEnc->exit) {
		/* wait data ready */
		FD_ZERO(&rdSet);
		FD_SET(fdMsg, &rdSet);
		
		ret = select(fdMax, &rdSet, NULL, NULL, NULL);
		if(ret < 0 && errno != EINTR) {
			ERRSTR("select err");
			break;
		}

		/* no data ready */
		if(!ret)
			continue;

		if(FD_ISSET(fdMsg, &rdSet)) {
			/* process msg */
			msg_process(hEnc, &msgBuf);
		}
	}
	
	INFO("<%s> encode thread exit...", hEnc->name);
	pthread_exit(0);
	
}

/*****************************************************************************
 Prototype    : encoder_create
 Description  : create encoder module
 Input        : EncoderAttrs *attrs  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/17
    Author       : Sun
    Modification : Created function

*****************************************************************************/
EncoderHandle encoder_create(EncoderAttrs *attrs)
{
	EncoderHandle hEnc;

	hEnc = calloc(1, sizeof(struct EncoderObj));
	if(!hEnc) {
		ERR("alloc mem failed");
		return NULL;
	}

	Int32 err;

	err = encoder_init(hEnc, attrs);
	if(err) {
		encoder_delete(hEnc, NULL);
		return NULL;
	}

	return hEnc;
}

/*****************************************************************************
 Prototype    : encoder_delete
 Description  : delete encoder module
 Input        : EncoderHandle hEnc  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/17
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 encoder_delete(EncoderHandle hEnc, MsgHandle hCurMsg)
{
	if(!hEnc)
		return E_INVAL;

	/* ask thread to exit */
	if(hEnc->pid > 0) {
		if(hCurMsg) {
			MsgHeader msg;

			/* send msg to our thread to exit */
			msg.cmd = APPCMD_EXIT;
			msg.index = 0;
			msg.dataLen = 0;
			msg.type = MSG_TYPE_REQU;
			msg_send(hCurMsg, msg_get_name(hEnc->hMsg), &msg, 0);
		}

		/* set flag to exit */
		hEnc->exit = TRUE;
		
		pthread_join(hEnc->pid, NULL);
	}

	/* delete modules used */
	if(hEnc->hEncode)
		alg_delete(hEnc->hEncode);

	if(hEnc->hBufEnc)
		buffer_free(hEnc->hBufEnc);

	if(hEnc->hOsd)
		osd_delete(hEnc->hOsd);

	if(hEnc->hPoolEnc)
		buf_pool_delete(hEnc->hPoolEnc);

	if(hEnc->hMsg)
		msg_delete(hEnc->hMsg);

	if(hEnc->hPoolIn)
		buf_pool_free_all(hEnc->hPoolIn);

	free(hEnc);

	return E_NO;
}

/*****************************************************************************
 Prototype    : encoder_run
 Description  : run this module
 Input        : EncoderHandle hEnc  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/20
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 encoder_run(EncoderHandle hEnc)
{
	Int32 err;
	
	/* create thread and run our thread */
	err = pthread_create(&hEnc->pid, NULL, encoder_thread, hEnc);
	if(err < 0) {
		ERRSTR("create data capture thread failed...");
		return E_NOMEM;
	}

	return E_NO;
}

/*****************************************************************************
 Prototype    : encoder_set_enc_params
 Description  : update params
 Input        : EncoderHandle hEnc  
                MsgHandle hCurMsg   
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/20
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 encoder_set_enc_params(EncoderHandle hEnc, MsgHandle hCurMsg)
{
	if(!hEnc)
		return E_INVAL;

	Int32 ret = E_NO;
	
	if(hEnc->pid > 0 && hCurMsg) {
		MsgHeader msg;

		/* send msg to our thread to exit */
		msg.cmd = APPCMD_SET_ENC_PARAMS;
		msg.index = 0;
		msg.dataLen = 0;
		msg.type = MSG_TYPE_REQU;
		ret = msg_send(hCurMsg, msg_get_name(hEnc->hMsg), &msg, 0);
	}

	return ret;
}

