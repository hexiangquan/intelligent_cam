/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : previewer.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/2/24
  Last Modified :
  Description   : previewer.c header file
  Function List :
  History       :
  1.Date        : 2012/2/24
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __PREVIEWER_H__
#define __PREVIEWER_H__
	
#include "capture.h"
#include <media/davinci/dm365_ipipe.h>


/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/

/*----------------------------------------------*
 * external routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/
typedef struct _PreviewAttrs{
	Bool				onTheFly;
	Bool				setRgbColorPallet;
	Uint16				inputWidth;
	Uint16				inputHeight;
	enum ipipe_pix_formats 	inputFmt;
}PreviewAttrs;
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

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern int previewer_init(PreviewAttrs *attrs);
extern Int32 previewer_update(int fdPrev, CapAttrs *attrs);

extern Int32 previewer_convert(int fd,void * inBuf,Int32 inBufSize,void * outBuf,Int32 outBufSize);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __PREVIEWER_H__ */
