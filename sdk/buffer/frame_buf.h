/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : frame_buf.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/2/25
  Last Modified :
  Description   : frame_buf.c header file, this is an extended class of buffer
  Function List :
  History       :
  1.Date        : 2012/2/25
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __FRAME_BUF_H__
#define __FRAME_BUF_H__

#include "common.h"
#include "buffer.h"

/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/

/*----------------------------------------------*
 * external routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/

/* Frame buffer for video */
typedef struct _FrameBuf {
	BufHandle 		hBuf;				//Buf handle, so we can use all buffer APIs
	Int32			index;				//Index of this frame
	ChromaFormat 	colorSpace;			//Color space for pixel format
	Uint16			width;				//Width of this frame
	Uint16			height;				//Height of this frame
	Uint16			bytesPerLine;		//Bytes of a line
	Uint16			globalGain;			//Global gain
	Uint16			redGain;			//Red gain
	Uint16			greenGain;			//green gain
	Uint16			blueGain;			//blue gain
	struct timeval	timeStamp;			//time stamp for capture
	Int8			private[448];		//private data
}FrameBuf;
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
 * routines' implementations                    *
 *----------------------------------------------*/




#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern FrameBufHandle frame_buf_alloc(Uint32 size, BufAllocAttrs *attrs);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __FRAME_BUF_H__ */
