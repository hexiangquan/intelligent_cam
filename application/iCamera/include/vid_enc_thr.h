/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : vid_enc_thr.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/10
  Last Modified :
  Description   : vid_enc_thr.c header file
  Function List :
  History       :
  1.Date        : 2012/3/10
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __VID_ENC_THR_H__
#define __VID_ENC_THR_H__

#include "params_mng.h"
#include "frame_dispatch.h"

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
	ParamsMngHandle hParamsMng;
	FrameDispHandle	hDispatch;
}VidEncThrArg;


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern void *vid_enc_thr(void *arg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __VID_ENC_THR_H__ */
