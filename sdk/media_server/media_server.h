/******************************************************************************

  Copyright (C), 2012-2022, DCN Co., Ltd.

 ******************************************************************************
  File Name     : media_server.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/4/26
  Last Modified :
  Description   : media_server.cpp header file
  Function List :
  History       :
  1.Date        : 2012/4/26
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __MEDIA_SERVER_H__
#define __MEDIA_SERVER_H__

#include "common.h"

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
#define MEDIA_SRV_MAX_SESSION_NUM		2
#define MEDIA_SRV_MAX_SUB_SESSION_NUM	2

#define AUDIO_SAMPLE_FREQUENCY			16000
#define AUDIO_OUT_BITRATE				12800

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/* Handle for media server */
typedef struct MediaSrvObj *MediaSrvHandle;

/* Handle for media session */
typedef struct MediaSessionObj *MediaSessionHandle;

/* Handle for sub-media session */
typedef struct MediaSubSessionObj *MediaSubSessionHandle;

/* rtsp server streaming mode */
typedef enum {
	STREAMING_UNICAST,
	STREAMING_MULTICAST_ASM,
	STREAMING_MULTICAST_SSM
}StreamingMode;

/* type of media */
typedef enum {
	MEDIA_TYPE_H264 = 0,
	MEDIA_TYPE_MPEG4,
	MEDIA_TYPE_MJPEG,
	MEDIA_TYPE_AUDIO,
}MediaType;

/* type of frame */
enum {
	FRAME_TYPE_I = 0,
	FRAME_TYPE_P,	
	FRAME_TYPE_B,
};

/* one frame for a video or audio media */
typedef struct {
	Uint16			index;			//index of total frame serials
	Uint16			frameType;		//type of the frame, such as I frame, B frame
	struct timeval	timestamp;		//timestamp for frame capture
	void			*data;			//addr of data
	Int32			dataLen;		//data len
	DateTime		dateTime;		//capture time
	Int32			reserved;		//reserved
}MediaFrame;

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*****************************************************************************
 Prototype    : media_srv_create
 Description  : create media server
 Input        : Uint16 rtspSrvPort  
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/26
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern MediaSrvHandle media_srv_create(Uint16 rtspSrvPort);

/*****************************************************************************
 Prototype    : media_srv_create_session
 Description  : create a rtsp session
 Input        : MediaSrvHandle hSrv  
                const char *name     
                const char *desp     
                StreamingMode mode   
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/26
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern MediaSessionHandle media_srv_create_session(MediaSrvHandle hSrv, const char *name, const char *desp, StreamingMode mode);

/*****************************************************************************
 Prototype    : media_srv_add_sub_session
 Description  : add a sub session
 Input        : MediaSessionHandle hSession  
                MediaType type               
                Uint16 videoPort             
                Uint16 audioPort             
                Uint32 bitRate               
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/26
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern MediaSubSessionHandle media_srv_add_sub_session(MediaSessionHandle hSession, MediaType type, Uint16 videoPort, Uint16 audioPort, Uint32 bitRate);

/*****************************************************************************
 Prototype    : media_srv_run
 Description  : running server
 Input        : MediaSrvHandle hSrv  
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/26
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 media_srv_run(MediaSrvHandle hSrv);

/*****************************************************************************
 Prototype    : media_srv_delete
 Description  : delete server
 Input        : MediaSrvHandle hSrv  
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/26
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 media_srv_delete(MediaSrvHandle hSrv);

/*****************************************************************************
 Prototype    : media_stream_in
 Description  : input new frame for a sub session
 Input        : MediaSubSessionHandle hSubSession  
                MediaFrame *frame                  
                Bool isVideo                       
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 media_stream_in(MediaSubSessionHandle hSubSession, MediaFrame *frame, Bool isVideo);

/*****************************************************************************
 Prototype    : media_get_link_status
 Description  : Check link status
 Input        : MediaSubSessionHandle hSubSession  
 Output       : None
 Return Value : TRUE -- data link is established, FALSE -- not linked
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Bool media_get_link_status(MediaSubSessionHandle hSubSession);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __MEDIA_SERVER_H__ */
