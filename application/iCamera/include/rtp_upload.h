/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : rtp_upload.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/4/28
  Last Modified :
  Description   : rtp_upload.c header file
  Function List :
  History       :
  1.Date        : 2012/4/28
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __RTP_UPLOAD_H__
#define __RTP_UPLOAD_H__

#include "upload.h"

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

typedef struct {
	Int32			size;			//size of this struct
	Uint16			rtspPort;		//rtsp serer listen port
	const char		*streamName;	//name of this stream
	ChromaFormat	fmt;			//type of this media, only support H.264 currently
}RtpUploadParams;


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/* fxns for this module */
extern const UploadFxns RTP_UPLOAD_FXNS ;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __RTP_UPLOAD_H__ */
