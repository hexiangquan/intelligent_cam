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
#include <pthread.h>
#include "common.h"
#include "h264_enc.h"

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

#define TRANS_TIMEOUT		0 //seconds

#define STREAM_DESP			"Video/Audio from IPNC" 
#define VIDEO_BITRATE		5000000

#define SYNC_CODE_IFRAME	0xC0DE2640
#define SYNC_CODE_PFRAME	0xC0DE264E
#define SYNC_CODE_SKIP		0xC0DEDEAD

#define BUF_MIN_RW_DST		4

#define BUF_SIZE			2500000

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/
typedef struct {
	MediaSrvHandle 			hSrv;
	MediaSessionHandle		hSession;
	MediaSubSessionHandle	hSubSession;
	Int8					*cacheBuf;
	Int32					bufSize;
	Int32					rdPos;
	Int32					wrPos;
	Uint32					cacheTime;
	Uint32					keepTime;
	time_t					vidEndTime;
	pthread_mutex_t			mutex;
	Bool					disableCache;
	Int32					maxFrameNum;		//max num of caching frames
	Int32					numFrameCached;		//current num of caching frames
}RtpTransObj;

typedef struct {
	Uint32 syncCode;
	Uint32 len;
}SyncHdr;

static Int32 rtp_upload_set_params(void *handle, const void *params);

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
		media_srv_add_sub_session(rtpTrans->hSession, type, 0, 0,
			uploadParams->bitRate);

	if(!rtpTrans->hSubSession) {
		ERR("create sub session failed...");
		goto exit;
	}

	/* start running server */
	if(media_srv_run(rtpTrans->hSrv) < 0) {
		ERR("running media server err.");
		goto exit;
	}

	pthread_mutex_init(&rtpTrans->mutex, NULL);

	/* set params */
	Int32 err = rtp_upload_set_params(rtpTrans, params);
	if(err) {
		ERR("init rtp upload params failed");
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
 Prototype    : img_msg_to_media_frame
 Description  : convert img msg to media frame
 Input        : const ImgMsg *img  
                MediaFrame *frame  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/7/3
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline void img_msg_to_media_frame(const ImgMsg *img, MediaFrame *frame)
{
	frame->index = img->index;
	frame->frameType = img->frameType;
	frame->timestamp = img->timeCode;
	frame->data = buffer_get_user_addr(img->hBuf);
	frame->dataLen = img->dimension.size;
}

/*****************************************************************************
 Prototype    : free_size_for_write
 Description  : get free size for write
 Input        : const RtpTransObj *rtpTrans  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/7/3
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline Uint32 free_size_for_write(const RtpTransObj *rtpTrans) 
{
	return ((rtpTrans->wrPos >= rtpTrans->rdPos) ? 
		(rtpTrans->bufSize - rtpTrans->wrPos - BUF_MIN_RW_DST) : 
		(rtpTrans->rdPos - rtpTrans->wrPos - BUF_MIN_RW_DST));
}

/*****************************************************************************
 Prototype    : free_size_for_read
 Description  : free size for read
 Input        : RtpTransObj *rtpTrans  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/7/3
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline Uint32 free_size_for_read(RtpTransObj *rtpTrans) 
{
	return ((rtpTrans->wrPos >= rtpTrans->rdPos) ? 
		(rtpTrans->wrPos - rtpTrans->rdPos) : 
		(rtpTrans->bufSize + rtpTrans->wrPos - rtpTrans->rdPos));

}

/*****************************************************************************
 Prototype    : write_buf
 Description  : write to buffer
 Input        : RtpTransObj *rtpTrans  
                const Uint8 *data      
                Uint32 len             
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/7/3
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static void write_frame(RtpTransObj *rtpTrans, const SyncHdr *hdr, const MediaFrame *frame) 
{
	//Int32 sizeToEnd = rtpTrans->bufSize - rtpTrans->wrPos;

	//DBG("rtp buf write, len: %u, wrpos: %u, sizeToEnd: %d", len, rtpTrans->wrPos, sizeToEnd);
	assert(rtpTrans->cacheBuf);
	memcpy(rtpTrans->cacheBuf + rtpTrans->wrPos, hdr, sizeof(*hdr));
	rtpTrans->wrPos += sizeof(*hdr);
	memcpy(rtpTrans->cacheBuf + rtpTrans->wrPos, frame, sizeof(*frame));
	rtpTrans->wrPos += sizeof(*frame);
	memcpy(rtpTrans->cacheBuf + rtpTrans->wrPos, frame->data, ROUND_UP(frame->dataLen, 4));
	rtpTrans->wrPos += ROUND_UP(frame->dataLen, 4);
}

/*****************************************************************************
 Prototype    : read_buf
 Description  : read circular buf
 Input        : RtpTransObj *rtpTrans  
                void *buf              
                Uint32 len             
                Bool mvRdPos           
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/7/4
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 read_buf(RtpTransObj *rtpTrans, void *buf, Uint32 len, Bool mvRdPos)
{
	/* copy data */
	Int32 sizeToEnd = rtpTrans->bufSize - rtpTrans->rdPos;
	
	if(len <= sizeToEnd) {
		memcpy(buf, rtpTrans->cacheBuf + rtpTrans->rdPos, len);
		if(mvRdPos) {
			rtpTrans->rdPos += len;
			if(rtpTrans->rdPos >= rtpTrans->bufSize)
				rtpTrans->rdPos = 0;
		}
	} else {
		/* copy to the end */
		memcpy(buf, rtpTrans->cacheBuf + rtpTrans->rdPos, sizeToEnd);
		/* copy left from the beginning */
		memcpy((Int8 *)buf + sizeToEnd, rtpTrans->cacheBuf, len - sizeToEnd);
		if(mvRdPos)
			rtpTrans->rdPos = len - sizeToEnd;
	}

	//DBG("rd pos: %d", hCirBuf->rdPos);

	return E_NO;
}


/*****************************************************************************
 Prototype    : rd_pos_inc
 Description  : increase read position
 Input        : RtpTransObj *rtpTrans  
                Uint32 len             
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/7/3
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline void rd_pos_inc(RtpTransObj *rtpTrans, Uint32 len)
{
	rtpTrans->rdPos += len;
	if(rtpTrans->rdPos >= rtpTrans->bufSize)
		rtpTrans->rdPos = 0;
}

/*****************************************************************************
 Prototype    : clear_to_next_iframe
 Description  : clear until next i frame
 Input        : RtpTransObj *rtpTrans  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/7/3
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static void clear_to_next_iframe(RtpTransObj *rtpTrans, Uint32 minSize)
{
	SyncHdr	*hdr;
	Bool	firstFrame = TRUE;
	Int32   step;
	Bool	backToBegin = FALSE;

	if(rtpTrans->wrPos >= rtpTrans->rdPos && minSize) {
		/* this means size left to the end is not enough, mark it to skip */
		*(Uint32 *)(rtpTrans->cacheBuf + rtpTrans->wrPos) = SYNC_CODE_SKIP;
		//rtpTrans->wrPos = 0;
		backToBegin = TRUE;
	}
	
	while(rtpTrans->rdPos != rtpTrans->wrPos) {
		//read_buf(rtpTrans, &hdr, sizeof(hdr), FALSE);
		hdr = (SyncHdr *)(rtpTrans->cacheBuf + rtpTrans->rdPos);
		if(hdr->syncCode == SYNC_CODE_IFRAME) {
			if(!firstFrame) {
				//DBG("rtp clear, got next i frame");
				break;
			} else
				firstFrame = FALSE;
		}

		if(hdr->syncCode == SYNC_CODE_PFRAME || hdr->syncCode == SYNC_CODE_IFRAME) {
			step = sizeof(*hdr) + hdr->len; //add sync code & wrLen
			rd_pos_inc(rtpTrans, step);
			rtpTrans->numFrameCached--;
			assert(rtpTrans->numFrameCached >= 0);
		} else if(hdr->syncCode == SYNC_CODE_SKIP) {
			/* skip code, back to the beginning */
			rtpTrans->rdPos = 0;
		} else {
			step = sizeof(Uint32); //invalid sync code
			//DBG("inv sync code");
			rd_pos_inc(rtpTrans, step);
		}
	}

	if(backToBegin)
		rtpTrans->wrPos = 0;
}

/*****************************************************************************
 Prototype    : rtp_upload_cache_frame
 Description  : cache frame to local buffer
 Input        : RtpTransObj *rtpTrans    
                const MediaFrame *frame  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/7/3
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 rtp_upload_cache_frame(RtpTransObj *rtpTrans, const MediaFrame *frame)
{
	if(!rtpTrans->cacheBuf || !rtpTrans->bufSize)
		return E_NOMEM;

	SyncHdr hdr;
	hdr.len = sizeof(MediaFrame) + ROUND_UP(frame->dataLen, 4);

	if(hdr.len + sizeof(hdr) > rtpTrans->bufSize - BUF_MIN_RW_DST) {
		DBG("rtp upload, wrLen: %d > buf size: %d", hdr.len, rtpTrans->bufSize);
		return E_NOSPC;
	}

	pthread_mutex_lock(&rtpTrans->mutex);

	while(free_size_for_write(rtpTrans) < hdr.len + sizeof(hdr)) {
		clear_to_next_iframe(rtpTrans, hdr.len + sizeof(hdr));
		//if(rtpTrans->rdPos == rtpTrans->wrPos) {
			//DBG("rtp clear, buf is empty");
			//if(free_size_for_write(rtpTrans) < hdr.len + sizeof(hdr))
				//goto exit;
			//break;
		//}
	}

	/* write to cache buffer */
	if(frame->frameType == VID_I_FRAME || frame->frameType == VID_IDR_FRAME)
		hdr.syncCode = SYNC_CODE_IFRAME;
	else
		hdr.syncCode = SYNC_CODE_PFRAME;

	write_frame(rtpTrans, &hdr, frame);

	if( rtpTrans->maxFrameNum > 0 && 
		++rtpTrans->numFrameCached > rtpTrans->maxFrameNum ) {
		//DBG("max num frames cached, clear oldest ones.");
		clear_to_next_iframe(rtpTrans, 0);
	}

//exit:
	pthread_mutex_unlock(&rtpTrans->mutex);
		
	return E_NO;
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
	RtpTransObj *rtpTrans = handle;	
	Int32 err;
	MediaFrame frame;
	
	img_msg_to_media_frame(img, &frame);

	if(!rtpTrans->cacheBuf || rtpTrans->disableCache) {
		/* send to media server */
		err = media_stream_in(rtpTrans->hSubSession, &frame, TRUE);
		if(rtpTrans->disableCache && (time(NULL) > rtpTrans->vidEndTime)) {
			rtpTrans->disableCache = FALSE;
			//DBG("Restart cache, %u/%u", time(NULL), rtpTrans->vidEndTime);
		}
		
	} else {
		err = rtp_upload_cache_frame(rtpTrans, &frame);
	}

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
	RtpTransObj *rtpTrans = handle;
	const RtpUploadParams *rtpParams = params;

	if(!rtpTrans || !rtpParams || rtpParams->size != sizeof(RtpUploadParams))
		return E_INVAL;

	if( rtpParams->cacheTime != rtpTrans->cacheTime ) {
		pthread_mutex_lock(&rtpTrans->mutex);
		
		if(rtpTrans->cacheBuf) {
			DBG("rtp upload, free cache buf");
			free(rtpTrans->cacheBuf);
			rtpTrans->cacheBuf = NULL;
			rtpTrans->bufSize = 0;
		}

		if( rtpParams->cacheTime ) {
			DBG("rtp cache time: %d second", rtpParams->cacheTime);
			rtpTrans->cacheTime = MIN(rtpParams->cacheTime, RTP_MAX_VID_LEN);
			rtpTrans->bufSize = rtpTrans->cacheTime * rtpParams->bitRate / 4;
			rtpTrans->bufSize = ROUND_UP(rtpTrans->bufSize, 4);
			rtpTrans->cacheBuf = malloc(rtpTrans->bufSize);
			if(!rtpTrans->cacheBuf)
				ERR("rtp upload, alloc buf for video cache failed");
			rtpTrans->rdPos = rtpTrans->wrPos = 0;
		}

		rtpTrans->keepTime = rtpParams->keepTime;
		rtpTrans->maxFrameNum = rtpParams->frameRate * rtpParams->cacheTime;
		pthread_mutex_unlock(&rtpTrans->mutex);	
	}
	
	return E_NO;
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

	if(rtpTrans->cacheBuf) {
		free(rtpTrans->cacheBuf);
		rtpTrans->cacheBuf = NULL;
	}

	pthread_mutex_destroy(&rtpTrans->mutex);
	
	free(rtpTrans);
	
	return E_NO;
}

/*****************************************************************************
 Prototype    : rtp_upload_flush_cache
 Description  : flush cache
 Input        : RtpTransObj *rtpTrans  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/7/5
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 rtp_upload_flush_cache(RtpTransObj *rtpTrans)
{
	if(!rtpTrans->cacheBuf || !rtpTrans->cacheTime)
		return E_NO; /* no cache */
	
	pthread_mutex_lock(&rtpTrans->mutex);
	
	SyncHdr 	*hdr;
	MediaFrame 	*frame;

	while(rtpTrans->rdPos != rtpTrans->wrPos) {
		hdr = (SyncHdr *)(rtpTrans->cacheBuf + rtpTrans->rdPos);
		
		if(hdr->syncCode == SYNC_CODE_PFRAME || hdr->syncCode == SYNC_CODE_IFRAME) {
			rtpTrans->rdPos += sizeof(*hdr);
			frame = (MediaFrame *)(rtpTrans->cacheBuf + rtpTrans->rdPos);
			/* set ptr to following data */
			frame->data = (void *)(rtpTrans->cacheBuf + rtpTrans->rdPos + sizeof(*frame)); 
			/* send to media server for upload */
			media_stream_in(rtpTrans->hSubSession, frame, TRUE);
			rd_pos_inc(rtpTrans, hdr->len);
		} else if(hdr->syncCode == SYNC_CODE_SKIP) {
			/* skip code, back to the beginning */
			rtpTrans->rdPos = 0;
		} else {
			//DBG("rtp cache flush, invalid sync code");
			rd_pos_inc(rtpTrans, sizeof(Uint32));
		}
	}

	pthread_mutex_unlock(&rtpTrans->mutex);

	/* not caching for sending a while after the event */
	rtpTrans->vidEndTime = time(NULL) + rtpTrans->keepTime;
	rtpTrans->disableCache = TRUE;
	rtpTrans->numFrameCached = 0;
	//DBG("flush cached frames....");
	
	return E_NO;
}


/*****************************************************************************
 Prototype    : rtp_upload_ctrl
 Description  : ctrl fxns 
 Input        : void *handle  
                Int32 cmd     
                void *arg     
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/7/4
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 rtp_upload_ctrl(void *handle, Int32 cmd, void *arg)
{
	if(!handle)
		return E_INVAL;

	Int32 err = E_NO;

	switch(cmd) {
	case RTP_CMD_SND_VID_CLIP:
		err = rtp_upload_flush_cache((RtpTransObj *)handle);
		break;
	default:
		err = E_UNSUPT;
		break;
	}

	return err;
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
	.ctrl = rtp_upload_ctrl,
};


Int32 rtp_upload_test()
{
	RtpTransObj *rtpTrans;
	RtpUploadParams params;

	params.size = sizeof(params);
	params.rtspPort = 554; //using default
	params.bitRate = 4000000;
	params.frameRate = 15;
	params.keepTime = 3;
	params.cacheTime = 0;
	params.streamName = "h264";
	params.fmt = FMT_H264;

	rtpTrans = rtp_upload_create(&params);
	assert(rtpTrans);
	DBG("rtp create ok.");

	Int32 err;
	ImgMsg img;
	
	memset(&img, 0, sizeof(img));
	img.hBuf = buffer_alloc(100 * 1024, NULL);
	assert(img.hBuf);
	img.dimension.size = buffer_get_size(img.hBuf);
	img.frameType = VID_IDR_FRAME;
	gettimeofday(&img.timeCode, NULL);

	//struct timeval 

	DBG("start sending frames.");
	while(img.index++ < 6000) {
		err = rtp_upload_send(rtpTrans, &img);
		assert(err == E_NO || err == E_MODE);
		if((img.index % 20) == 0)
			img.frameType = VID_IDR_FRAME;
		else
			img.frameType = VID_P_FRAME;
		gettimeofday(&img.timeCode, NULL);
		img.dimension.size = RAND(10000, buffer_get_size(img.hBuf));
		usleep(33000);
		if((img.index % 300) == 0) {
			err = rtp_upload_ctrl(rtpTrans, RTP_CMD_SND_VID_CLIP, NULL);
			assert(err == 0);
			DBG("enable send");
		}
	}

	buffer_free(img.hBuf);

	err = rtp_upload_delete(rtpTrans);
	assert(rtpTrans);

	return E_NO;
}

