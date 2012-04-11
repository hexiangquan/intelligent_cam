/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : converter.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/19
  Last Modified :
  Description   : converter.c header file
  Function List :
  History       :
  1.Date        : 2012/3/19
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __CONVERTER_H__
#define __CONVERTER_H__

#include "img_convert.h"
#include "capture.h"
#include "app_msg.h"

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
typedef struct ConverterObj *ConverterHandle;

typedef struct {
	Uint16			maxImgWidth;	//max width of out image
	Uint16			maxImgHeight;	//max height of out image
	Int32			flags;			//flags for ctrl
	Uint32			bufNum;			//num of buffer in pool to alloc
}ConverterAttrs;

typedef struct {
	ImgConvDynParams	convDyn[2];
}ConverterParams;

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*****************************************************************************
 Prototype    : converter_create
 Description  : create converter module
 Input        : ConverterAttrs *attrs  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/19
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern ConverterHandle converter_create(ConverterAttrs *attrs, const ConverterParams *params);

/*****************************************************************************
 Prototype    : converter_delete
 Description  : delete converter module
 Input        : ConverterHandle hConverter  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/19
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 converter_delete(ConverterHandle hConverter);

/*****************************************************************************
 Prototype    : converter_run
 Description  : do convert
 Input        : ConverterHandle hConverter  
                Int32 streamId              
                ImgMsg *sndMsg              
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/19
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 converter_run(ConverterHandle hConverter, FrameBuf *rawBuf, Int32 streamId, ImgMsg *imgMsg);

/*****************************************************************************
 Prototype    : converter_params_update
 Description  : update params
 Input        : ConverterHandle hConverter  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/19
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 converter_params_update(ConverterHandle hConverter, ConverterParams *params);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __CONVERTER_H__ */
