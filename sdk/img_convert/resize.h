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
#include "img_convert.h"

/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/


/*----------------------------------------------*
 * external routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/

/* Attributes for single shot resize */
typedef struct _RszAttrs {
	Bool			isChained;
	Uint16			inWidth;
	Uint16			inHeight;
    Uint16 			inPitch;
    Uint16 			inHStart;
    Uint16 			inVStart;
	ChromaFormat	inFmt;
	ConvOutAttrs 	outAttrs[CONV_MAX_OUT_CHAN];
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
extern int resize_init(const char *name, Bool onTheFly, ConvOutAttrs outParams[]);


/*****************************************************************************
 Prototype    : resize_ss_config
 Description  : Config for single shot mode
 Input        : int fd           
                RszAttrs *attrs  
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 resize_ss_config(int fd, RszAttrs *attrs);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __RESIZE_H__ */
