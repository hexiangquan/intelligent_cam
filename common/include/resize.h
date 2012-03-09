/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : resize.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/2/24
  Last Modified :
  Description   : resize.c header file
  Function List :
  History       :
  1.Date        : 2012/2/24
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __RESIZE_H__
#define __RESIZE_H__
	
#include "common.h"

/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/
#define RSZ_MAX_OUT_CHAN	2

/*----------------------------------------------*
 * external routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/
 
/* Capture output attributes */
typedef struct _RszOutAttrs {
	Bool			enbale; /* Enable or disable this output, for chan1, this must always be enabled */
	Uint16			width;	/* Output width, should be aligned to 16 */
	Uint16			height; /* Output height, should be aligned to 16 */
	ChromaFormat	pixFmt; /* Output color space format */
}RszOutAttrs;

typedef struct _RszAttrs {
	Bool			isChained;
	Uint16			inWidth;
	Uint16			inHeight;
	ChromaFormat	inFmt;
	RszOutAttrs 	outAttrs[RSZ_MAX_OUT_CHAN];
}RszAttrs;

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

/*****************************************************************************
 Prototype    : resizer_init
 Description  : Init resizer
 Input        : Bool onTheFly            
                RszOutAttrs *outParams0  
                RszOutAttrs *outParams1  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/24
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern int resizer_init(Bool onTheFly, RszOutAttrs *outParams0, RszOutAttrs *outParams1);

extern int resize_single_shot_init();

extern Int32 resize_config(int fd, RszAttrs *attrs);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __RESIZE_H__ */
