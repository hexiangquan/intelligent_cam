/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : rtp_upload.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/12
  Last Modified :
  Description   : image upload using RTP/RTSP protocol
  Function List :
           
  History       :
  1.Date        : 2012/3/12
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "rtp_upload.h"
#include "media_server.h"
#include "log.h"

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

#define TRANS_TIMEOUT	0 //seconds

#define STREAM_DESP		"Video/Audio from IPNC" 
#define VIDEO_BITRATE	5000000

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/
typedef struct {
	MediaSrvHandle 			hSrv;
	MediaSessionHandle		hSession;
	MediaSubSessionHandle	hSubSession;
}RtpTransObj;

/*****************************************************************************
 Prototype    : rtp_upload_fmt_convert
 Description  : convert fmt to media type
 Input        : ChromaFormat fmt  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 rtp_upload_fmt_convert(ChromaFormat fmt)
{
	switch(fmt) {
	case FMT_H264:
		return MEDIA_TYPE_H264;
	default:
		return E_UNSUPT;
	}
}

/*****************************************************************************
 Prototype    : rtp_upload_create
 Description  : create and run media server
 Input        : void *params  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static void *rtp_upload_create(void *params)
{
	RtpTransObj *rtpTrans;
	RtpUploadParams *uploadParams = (RtpUploadParams *)params;
	Int32 type;
	
	if(!params || uploadParams->size != sizeof(RtpUploadParams)) {
		ERR("invalid params size");
		return NULL;
	}

	type = rtp_upload_fmt_convert(uploadParams->fmt);
	if(type < 0) {
		ERR("invalid fmt.");
		return NULL;
	}

	rtpTrans = calloc(1, sizeof(RtpTransObj));
	if(!rtpTrans) {
		ERR("alloc mem for rtp trans failed");
		return NULL;
	}

	/* create media server */
	rtpTrans->hSrv = media_srv_create(uploadParams->rtspPort);
	if(!rtpTrans->hSrv) {
		ERR("create media server failed...");
		goto exit;
	}

	/* create session */
	rtpTrans->hSession = 
		media_srv_create_session(rtpTrans->hSrv, 
			uploadParams->streamName, STREAM_DESP, STREAMING_UNICAST);
	if(!rtpTrans->hSession) {
		ERR("create new session failed...");
		goto exit;
	}
 
	/* create  sub session */
	rtpTrans->hSubSession = 
		media_srv_add_sub_session(rtpTrans->hSession, type, 0, 0, VIDEO_BITRATE);

	if(!rtpTrans->hSubSession) {
		ERR("create sub session failed...");
		goto exit;
	}

	/* start running server */
	if(media_srv_run(rtpTrans->hSrv) < 0) {
		ERR("running media server err.");
		goto exit;
	}
	
	return rtpTrans;

exit:

	if(rtpTrans->hSrv)
		media_srv_delete(rtpTrans->hSrv);
	
	if(rtpTrans)
		free(rtpTrans);

	return NULL;
}

/*****************************************************************************
 Prototype    : rtp_upload_send
 Description  : send videa frames
 Input        : void *handle       
                const ImgMsg *img  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 rtp_upload_send(void *handle, const ImgMsg *img)
{
	MediaFrame frame;
	RtpTransObj *rtpTrans = handle;

	frame.index = img->index;
	frame.frameType = img->frameType;
	frame.timestamp = img->timeCode;
	frame.data = buffer_get_user_addr(img->hBuf);
	frame.dataLen = img->dimension.size;

	Int32 err;

	/* send to media server */
	err = media_stream_in(rtpTrans->hSubSession, &frame, TRUE);

	return err;
}

/*****************************************************************************
 Prototype    : rtp_upload_set_params
 Description  : set params at running time
 Input        : void *handle        
                const void *params  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 rtp_upload_set_params(void *handle, const void *params)
{
	return E_UNSUPT;
}

/*****************************************************************************
 Prototype    : rtp_upload_delete
 Description  : delete rtp upload obj
 Input        : void *handle  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 rtp_upload_delete(void *handle)
{
	RtpTransObj *rtpTrans = handle;

	if(!rtpTrans)
		return E_INVAL;

	/* delete server */
	media_srv_delete(rtpTrans->hSrv);
	free(rtpTrans);
	
	return E_NO;
}


/* fxns for tcp upload */
const UploadFxns RTP_UPLOAD_FXNS = {
	.create = rtp_upload_create,
	.delete = rtp_upload_delete,
	.connect = NULL,
	.disconnect = NULL,
	.sendFrame = rtp_upload_send,
	.sendHeartBeat = NULL,
	.setParams = rtp_upload_set_params,
	.saveFrame = NULL,
};

