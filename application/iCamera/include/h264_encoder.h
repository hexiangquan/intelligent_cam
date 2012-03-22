/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : h264_encoder.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/20
  Last Modified :
  Description   : h264_encoder.c header file
  Function List :
  History       :
  1.Date        : 2012/3/20
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __H264_ENCODER_H__
#define __H264_ENCODER_H__

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

extern EncoderHandle h264_encoder_create(IN ParamsMngHandle hParamsMng, IN pthread_mutex_t *mutex);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __H264_ENCODER_H__ */
