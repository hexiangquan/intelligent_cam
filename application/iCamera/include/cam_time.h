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
typedef struct {
	Uint16	year;			//year, >= 2011
	Uint8	month;			//month, 1~12
	Uint8	day;			//day, 1~31
	Uint8	weekDay;		//weekday, 1~7
	Uint8	hour;			//hour, 0~23
	Uint8	minute;			//minute, 0~59
	Uint8	second;			//second, 0~59
	Uint16	ms;				//mili second
	Uint16	us;				//us
} CamDateTime;


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

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
extern Int32 cam_convert_time(IN struct timeval *tv, OUT CamDateTime *dateTime);

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
extern Int32 cam_set_time(IN CamDateTime *dateTime);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __TIME_H__ */
