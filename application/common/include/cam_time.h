/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : time.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/5
  Last Modified :
  Description   : time.c header file
  Function List :
  History       :
  1.Date        : 2012/3/5
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __CAM_TIME_H__
#define __CAM_TIME_H__

#include "common.h"

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

/* 
  * Time for sync
  */
typedef DateTime CamDateTime;


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*****************************************************************************
 Prototype    : cam_time_sync
 Description  : sync rtc time with system time
 Input        : None
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/9/11
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 cam_time_sync();


/*****************************************************************************
 Prototype    : cam_convert_time
 Description  : Convert from timeval to CamDateTime
 Input        : struct timeval *tv     
                CamDateTime *dateTime  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/5
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 cam_convert_time(IN const struct timeval *tv, OUT CamDateTime *dateTime);

/*****************************************************************************
 Prototype    : cam_get_time
 Description  : get current time
 Input        : CamDateTime *dateTime  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/5
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 cam_get_time(OUT CamDateTime *dateTime);

/*****************************************************************************
 Prototype    : cam_set_time
 Description  : Set time to system
 Input        : CamDateTime *dateTime  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/5
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 cam_set_time(IN const CamDateTime *dateTime);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __TIME_H__ */
