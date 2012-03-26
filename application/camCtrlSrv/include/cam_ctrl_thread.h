/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : cam_ctrl_thread.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/23
  Last Modified :
  Description   : cam_ctrl_thread.c header file
  Function List :
  History       :
  1.Date        : 2012/3/23
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __CAM_CTRL_THREAD_H__
#define __CAM_CTRL_THREAD_H__

#include "icam_ctrl.h"

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

/* params for thread */
typedef struct {
	ICamCtrlHandle 		hCamCtrl;	//handle for camera ctrl
	pthread_mutex_t		*mutex;		//mutex
	int					*conCntPtr;	//connection count, should be decrease when thread exit
	int					sock;		//connect socket
	void				*dataBuf;	//data buf 
	Int32				bufLen;		//len of buf
}CamCtrlThrParams;


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern void *cam_ctrl_thread(void *arg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __CAM_CTRL_THREAD_H__ */
