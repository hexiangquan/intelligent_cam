/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : cap_init.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/4/12
  Last Modified :
  Description   : cap_init.c header file
  Function List :
  History       :
  1.Date        : 2012/4/12
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __CAP_INIT_H__
#define __CAP_INIT_H__

#include "cam_status.h"
#include "capture.h"

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
#define CAP_DEVICE					"/dev/video0"
#define CAP_BUF_NUM					3

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern CapHandle cap_module_init(IN const CamWorkMode *workMode, OUT ImgDimension *inputInfo);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __CAP_INIT_H__ */
