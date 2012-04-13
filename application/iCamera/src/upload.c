/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : upload.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/10
  Last Modified :
  Description   : abstrac level for frame upload
  Function List :
              upload_connect
              upload_create
              upload_delete
              upload_disconnect
              upload_get_connect_status
              upload_send_frame
              upload_send_heartbeat
              upload_set_params
  History       :
  1.Date        : 2012/3/10
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "upload.h"
#include "log.h"
#include "tcp_upload.h"
#include "ftp_upload.h"
#include <pthread.h>
#include "jpg_encoder.h"

/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/

/*----------------------------------------------*
 * external routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/
static Int32 upload_send_frame(UploadHandle hUpload, const ImgMsg *data);

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

#define RECONNECT_TIMEOUT	5 	//second
#define MSG_RECV_TIMEOUT	25	//second
#define HEAT_BEAT_INTERVAL	60	//second

//#define TEST_SEND_TIME
#define PRINT_FPS			1

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/
/* structure for this module */
struct UploadObj {
	const UploadFxns		*fxns;
	Bool					isConnected;
	void					*handle;			//low level handle
	const char				*msgName;
	Uint32					connectTimeout;
	Int32					flags;
	pthread_t 				pid;
	pthread_mutex_t			mutex;
	Bool					exit;
	CamImgUploadProto		protol;
	const char				*savePath;
	Int32					frameCnt;
	time_t 					lastSnd;
};

/* fxns for none upload */
const UploadFxns NONE_UPLOAD_FXNS = {
	.create = NULL,
	.delete = NULL,
	.connect = NULL,
	.disconnect = NULL,
	.sendFrame = NULL,
	.sendHeartBeat = NULL,
	.setParams = NULL,
	.saveFrame = NULL,
};

/*****************************************************************************
 Prototype    : upload_thread
 Description  : running this thread
 Input        : void *arg  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/21
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static void *upload_thread(void *arg)
{
	assert(arg);

	UploadHandle	hUpload = (UploadHandle)arg;
	Int32			ret;
	ImgMsg			msgBuf;
	time_t			lastTime = time(NULL);
	MsgHandle		hMsg;

	/* init for thread env */
	hMsg = msg_create(hUpload->msgName, MSG_MAIN, 0);
	if(!hMsg)
		goto exit;

	ret = msg_set_recv_timeout(hMsg, MSG_RECV_TIMEOUT);

	DBG("upload thread start...");

	while(!hUpload->exit) {
		/* check if we need connect server */
		if(!hUpload->isConnected) {
			ret = upload_connect(hUpload, RECONNECT_TIMEOUT);
		} else {
			/* keep alive with server */
			if(time(NULL) - lastTime > HEAT_BEAT_INTERVAL) {
				upload_send_heartbeat(hUpload);
				lastTime = time(NULL);
			}	
		}
				
		/* recv msg */
		ret = msg_recv(hMsg, (MsgHeader *)&msgBuf, sizeof(msgBuf), 0);
		if(ret < 0) {
			//ERR("recv msg err: %s", str_err(ret));
			continue;
		}
	
		/* process msg */
		MsgHeader *msgHdr = &msgBuf.header;
		switch(msgHdr->cmd) {
		case APPCMD_NEW_DATA:
			ret = upload_send_frame(hUpload, &msgBuf);
			break;
		case APPCMD_EXIT:
			hUpload->exit = TRUE;
			break;
		default:
			ERR("unkown cmd: 0x%X", (unsigned int)msgHdr->cmd);
			ret = E_UNSUPT;
			break;
		}
	}

exit:

	if(hMsg)
		msg_delete(hMsg);

	INFO("upload thread exit...");
	pthread_exit(0);
}


/*****************************************************************************
 Prototype    : upload_create
 Description  : create upload module
 Input        : const UploadFxns *fxns  
                void *params            
                Uint32 size             
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
UploadHandle upload_create(UploadAttrs *attrs, UploadParams *params)
{
	UploadHandle hUpload;

	if(!attrs)
		return NULL;

	hUpload = calloc(1, sizeof(struct UploadObj));
	if(!hUpload) {
		ERR("alloc memory failed.");
		return NULL;
	}

	hUpload->isConnected = FALSE;
	hUpload->connectTimeout = attrs->reConTimeout;
	hUpload->msgName = attrs->msgName;
	hUpload->savePath = attrs->savePath;
	hUpload->flags = attrs->flags;

	/* select low level module */			
	switch(params->protol) {
	case CAM_UPLOAD_PROTO_TCP:
		hUpload->fxns = &TCP_UPLOAD_FXNS;
		hUpload->flags |= UPLOAD_FLAG_ANSYNC;
		hUpload->protol = CAM_UPLOAD_PROTO_TCP;
		break;
	case CAM_UPLOAD_PROTO_FTP:
		hUpload->fxns = &FTP_UPLOAD_FXNS;
		hUpload->flags |= UPLOAD_FLAG_ANSYNC;
		hUpload->protol = CAM_UPLOAD_PROTO_FTP;
		break;
	case CAM_UPLOAD_PROTO_RTP:
		//hUpload->fxns = NULL; //not supported now
		//break;
	case CAM_UPLOAD_PROTO_NONE:
	default:
		hUpload->flags &= ~UPLOAD_FLAG_ANSYNC;
		hUpload->protol = CAM_UPLOAD_PROTO_NONE;
		hUpload->fxns = &NONE_UPLOAD_FXNS;
		break;
	}

	assert(hUpload->fxns);

	/* create new handle */
	if(hUpload->fxns->create) {
		hUpload->handle = hUpload->fxns->create(params->paramsBuf);
		if(!hUpload->handle) {
			ERR("create sub object failed.");
			goto err_quit;
		}
	}
	
	/* init lock */
	pthread_mutex_init(&hUpload->mutex, NULL);
	
	if(hUpload->flags & UPLOAD_FLAG_ANSYNC) {
		/* create upload thread */
		if(pthread_create(&hUpload->pid, NULL, upload_thread, hUpload) < 0)	{
			ERRSTR("create upload thread err");
			goto err_quit;
		}
	}

	hUpload->lastSnd = time(NULL);

	return hUpload;

err_quit:
	
	free(hUpload);
	return NULL;
	
}

/*****************************************************************************
 Prototype    : upload_delete
 Description  : delete upload module
 Input        : UploadHandle hUpload  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 upload_delete(UploadHandle hUpload, MsgHandle hCurMsg)
{
	if(hUpload == NULL)
		return E_INVAL;

	if(hUpload->pid) {
		hUpload->exit = TRUE;
		if(hCurMsg) {
			/* send msg to our thread to exit */
			app_hdr_msg_send(hCurMsg, hUpload->msgName, APPCMD_EXIT, 0, 0);
		}

		pthread_join(hUpload->pid, NULL);	
	}
	
	if(hUpload->isConnected)
		upload_disconnect(hUpload);

	if(hUpload->fxns->delete)
		hUpload->fxns->delete(hUpload->handle);

	pthread_mutex_destroy(&hUpload->mutex);
	
	free(hUpload);

	return E_NO;
}

/*****************************************************************************
 Prototype    : upload_connect
 Description  : connect server
 Input        : UploadHandle hUpload  
                Uint32 timeoutMs      
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 upload_connect(UploadHandle hUpload, Uint32 timeoutMs)
{
	Int32 err;
	
	if(!hUpload)
		return E_INVAL;

	if(!hUpload->fxns->connect) {
		hUpload->isConnected = TRUE;
		return E_NO;
	}

	if(hUpload->isConnected)
		upload_disconnect(hUpload);

	pthread_mutex_lock(&hUpload->mutex);
	err = hUpload->fxns->connect(hUpload->handle, timeoutMs);
	pthread_mutex_unlock(&hUpload->mutex);
	if(err)
		return err;

	hUpload->isConnected = TRUE;

	return E_NO;
}

/*****************************************************************************
 Prototype    : upload_disconnect
 Description  : disconnect server
 Input        : UploadHandle hUpload  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 upload_disconnect(UploadHandle hUpload)
{
	Int32 err;
	
	if(!hUpload)
		return E_INVAL;

	if(!hUpload->fxns->disconnect) {
		hUpload->isConnected = FALSE;
		return E_NO;
	}

	if(hUpload->isConnected) {
		hUpload->isConnected = FALSE;
		pthread_mutex_lock(&hUpload->mutex);
		err = hUpload->fxns->disconnect(hUpload->handle);
		pthread_mutex_unlock(&hUpload->mutex);
		return err;
	}
	
	return E_NO;
}

/*****************************************************************************
 Prototype    : upload_get_connect_status
 Description  : get if the server is connected
 Input        : UploadHandle hUpload  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Bool upload_get_connect_status(UploadHandle hUpload)
{
	if(!hUpload)
		return FALSE;
	return hUpload->isConnected;
}

/*****************************************************************************
 Prototype    : upload_save_frame
 Description  : save frame
 Input        : UploadHandle hUpload  
                const ImgMsg *data    
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/21
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 upload_save_frame(UploadHandle hUpload, const ImgMsg *data)
{
	Int32 err = E_NO;

	if(hUpload->fxns->saveFrame)
		err = hUpload->fxns->saveFrame(hUpload->handle, data);
	else {
		/* try using default save function  */
		//DBG("<%d> save frame to jpeg ", data->index);
		err = jpg_encoder_save_frame(data, hUpload->savePath);
	}

	/* free buf */
	if(hUpload->flags & UPLOAD_FLAG_FREE_BUF)
		err = buf_pool_free(data->hBuf);
	
	return err;
}

/*****************************************************************************
 Prototype    : upload_send_frame
 Description  : send one frame
 Input        : UploadHandle hUpload  
                Ptr pFrameBuf         
                Uint32 unLen          
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 upload_send_frame(UploadHandle hUpload, const ImgMsg *data)
{
	Int32 err = E_NO;
	
	if(!hUpload->fxns->sendFrame) {
		/* save directly */
		err = E_TRANS;
		goto save_frame;
	}

	/* if the server is not connect, try connect first */
	if(!hUpload->isConnected) {
		err = upload_connect(hUpload, hUpload->connectTimeout);
		if(err) {
			ERR("not connected server");
			goto save_frame;
		}
	}

#ifdef	TEST_SEND_TIME
	struct timeval tmStart,tmEnd; 
	float   timeUse;

	gettimeofday(&tmStart,NULL);
#endif

	/* try send frame */
	pthread_mutex_lock(&hUpload->mutex);
	err = hUpload->fxns->sendFrame(hUpload->handle, data);
	pthread_mutex_unlock(&hUpload->mutex);
	if(err) {
		/* send err */
		ERR("<%d> send err", data->index);
		upload_disconnect(hUpload);
		goto save_frame;
	}

	if(hUpload->flags & UPLOAD_FLAG_FREE_BUF)
		buf_pool_free(data->hBuf); //send success, free buf

#ifdef TEST_SEND_TIME
	gettimeofday(&tmEnd,NULL); 
	timeUse = 1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec;
	DBG("<%d> send frame ok, size: %u KB, cost: %.2f ms", data->index, data->dimension.size>>10, timeUse/1000);
#endif

#ifdef PRINT_FPS
	hUpload->frameCnt++;
	if(time(NULL) - hUpload->lastSnd > 8) {
		INFO("upload frame fps: %d", hUpload->frameCnt >> 3);
		hUpload->frameCnt = 0;
		hUpload->lastSnd = time(NULL);
	}
#endif

	return E_NO;
	
save_frame:
	/* send err, save to local */
	err = upload_save_frame(hUpload, data);

	return err;
}

/*****************************************************************************
 Prototype    : upload_send_heartbeat
 Description  : keep connect with server
 Input        : UploadHandle hUpload  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 upload_send_heartbeat(UploadHandle hUpload)
{
	if(!hUpload)
		return E_INVAL;

	if(!hUpload->fxns->sendHeartBeat)
		return E_NO;

	Int32 err;

	/* try to connect first */
	if(!hUpload->isConnected) {
		err = upload_connect(hUpload, hUpload->connectTimeout);
		if(err)
			return err;
		hUpload->isConnected = TRUE;
	}

	/* send heartbeat  */
	pthread_mutex_lock(&hUpload->mutex);
	err = hUpload->fxns->sendHeartBeat(hUpload->handle);
	pthread_mutex_unlock(&hUpload->mutex);
	if(err) {
		/* send err, reconnect server */
		err = upload_connect(hUpload, hUpload->connectTimeout);
	}
	
	return err;
}

/*****************************************************************************
 Prototype    : upload_update_params
 Description  : update params
 Input        : UploadHandle hUpload  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/21
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 upload_update_params(UploadHandle hUpload, UploadParams *params)
{
	if(!hUpload || !params)
		return E_INVAL;

	if(hUpload->protol != params->protol) {
		ERR("can not change protol at run time");
		return E_MODE;
	}

	if(!hUpload->fxns->setParams)
		return E_NO;
	
	if(hUpload->isConnected)
		upload_disconnect(hUpload);
	
	Int32 	err;

	/* set params */
	pthread_mutex_lock(&hUpload->mutex);
	err = hUpload->fxns->setParams(hUpload->handle, params->paramsBuf);
	pthread_mutex_unlock(&hUpload->mutex);
	
	return err;
}

/*****************************************************************************
 Prototype    : upload_run
 Description  : run upload 
 Input        : UploadHandle hUpload  
                MsgHandle hCurMsg     
                const ImgMsg *data    
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/21
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 upload_run(UploadHandle hUpload, MsgHandle hCurMsg, const ImgMsg *data)
{
	if(hUpload->pid) {
		Int32 err = E_CONNECT;
		/* send to upload thread */
		if(hUpload->isConnected)
			err = msg_send(hCurMsg, hUpload->msgName, (MsgHeader *)data, 0);
		if(err) {
			/* save if server is not connected or send msg err */
			err = upload_save_frame(hUpload, data);
		}
		return err;
	} else {
		/* directly send */
		return upload_send_frame(hUpload, data);
	}
}

