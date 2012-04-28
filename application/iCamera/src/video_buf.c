/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : video_buf.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/4/24
  Last Modified :
  Description   : using circular buf to mange video read and write
  Function List :
              video_buf_init
  History       :
  1.Date        : 2012/4/24
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "common.h"
#include "log.h"
#include "video_buf.h"
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

struct VidBufObj {
	Int8			*buf;
	Int32			bufSize;
	Int32			wrPos;
	Int32			rdPos;
	pthread_mutex_t	mutex;
};

VidBufHandle vid_buf_create(Uint32 bufSize)
{
	VidBufHandle hVidBuf;

	/* validate input params */
	if(!bufSize)
		return NULL;

	hVidBuf = calloc(1, sizeof(struct VidBufObj));
	if(!hVidBuf) {
		ERR("alloc obj mem failed");
		return NULL;
	}

	/* alloc buffer */
	hVidBuf->buf = calloc(1, bufSize);
	if(!hVidBuf) {
		ERR("alloc circular buf failed");
		goto exit;
	}

	return hVidBuf;

exit:

	if(hVidBuf->buf)
		free(hVidBuf->buf);

	if(hVidBuf)
		free(hVidBuf);
	
	return NULL;
	
}

