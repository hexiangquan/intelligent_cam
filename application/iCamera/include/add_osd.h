/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : add_osd.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/8
  Last Modified :
  Description   : add_osd.c header file
  Function List :
  History       :
  1.Date        : 2012/3/8
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __ADD_OSD_H__
#define __ADD_OSD_H__

#include "osd.h"
#include "app_msg.h"
#include "cam_osd.h"

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

extern Int32 add_osd(OsdHandle hOsd, ImgMsg *imgMsg, CamOsdInfo *osdInfo);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __ADD_OSD_H__ */
