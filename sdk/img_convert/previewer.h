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
	
#include "img_convert.h"
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
	Bool				setRgbColorPallet;
	Uint16				inputWidth;
	Uint16				inputHeight;
    Uint16   			inputPitch;
    Uint16   			inputHStart;
    Uint16  			inputVStart;
	Uint32				ctrlFlags;
	ChromaFormat		inputFmt;
	Uint16				digiGain;
	Uint16				gamma;		/* 100 times of actual gamma value, e.g., set 220 for 2.2 gamma */
	Uint8				brightness;
	Uint8				contrast;
	Uint8				sharpness;
	Uint8				saturation;	/* saturation, 0~255 */
	Uint8				reserved[4];
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

/*****************************************************************************
 Prototype    : previewer_init
 Description  : init preview device
 Input        : const char *name     
                PreviewAttrs *attrs  
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern int previewer_init(const char *name, Bool onTheFly, PreviewAttrs *params);

/*****************************************************************************
 Prototype    : previewer_update
 Description  : config params
 Input        : int fdPrev       
                CapAttrs *attrs  
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 previewer_ss_config(int fdPrev, PreviewAttrs *params);

/*****************************************************************************
 Prototype    : previewer_convert
 Description  : Run convert
 Input        : int fd            
                void * inBuf      
                Int32 inBufSize   
                void * outBuf     
                Int32 outBufSize  
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 previewer_convert(int fd, AlgBuf *inBuf, AlgBuf *outBuf, Bool enChanB);

/*****************************************************************************
 Prototype    : ipipe_update
 Description  : Update ipipe params
 Input        : CapHandle hCap   
                CapAttrs *attrs  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/24
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 previewer_cap_update(int fdPrev, PreviewAttrs *attrs);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __PREVIEWER_H__ */
