/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : video_buf.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/4/24
  Last Modified :
  Description   : video_buf.c header file
  Function List :
  History       :
  1.Date        : 2012/4/24
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __VIDEO_BUF_H__
#define __VIDEO_BUF_H__
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

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/
typedef struct VidBufObj *VidBufHandle;

enum VideoType {
	VID_TYPE_H264 = 0,
	VID_TYPE_MPEG4,
	VID_TYPE_MJPEG,
};

enum FrameType {
	VID_I_FRAME = 0,
	VID_P_FRAME,
	VID_B_FRAME,
};

typedef struct {
	Int32			index;
	struct timeval 	tv;
	Uint16			videoType;
	Uint16			frameType;
	Uint32			len;
}VidBufHdr;


typedef enum  {
	VID_STARTED = 0,
	VID_STOPPED,
}VidStatus;

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern VidBufHandle vid_buf_create(Uint32 bufSize);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __VIDEO_BUF_H__ */
