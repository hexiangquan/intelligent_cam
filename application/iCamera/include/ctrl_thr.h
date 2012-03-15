/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : ctrl_thr.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/15
  Last Modified :
  Description   : ctrl_thr.c header file
  Function List :
  History       :
  1.Date        : 2012/3/15
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __CTRL_THR_H__
#define __CTRL_THR_H__

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

/* init argument for this thread */
typedef struct {
	ParamsMngHandle hParamsMng;
	FrameDispHandle hDispatch;
}CtrlThrArg;

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern void *ctrl_thr(void *arg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __CTRL_THR_H__ */
