/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : jpg_encoder.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/20
  Last Modified :
  Description   : jpg_encoder.c header file
  Function List :
  History       :
  1.Date        : 2012/3/20
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __JPG_ENCODER_H__
#define __JPG_ENCODER_H__

#include "encoder.h"
#include "params_mng.h"

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




#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*****************************************************************************
 Prototype    : jpg_encoder_create
 Description  : create jpeg encoder obj
 Input        : ParamsMngHandle hParamsMng  
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/20
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern EncoderHandle jpg_encoder_create(IN ParamsMngHandle hParamsMng, IN pthread_mutex_t *mutex);

/*****************************************************************************
 Prototype    : jpg_encoder_save_frame
 Description  : save one frame to local file system
 Input        : IN ImgMsg *msg       
                IN const char *path  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/20
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 jpg_encoder_save_frame(IN const ImgMsg *msg, IN const char *path);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __JPG_ENCODER_H__ */
