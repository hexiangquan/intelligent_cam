/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : time.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/5
  Last Modified :
  Description   : Time api
  Function List :
              cam_convert_time
              cam_get_time
              cam_set_time
  History       :
  1.Date        : 2012/3/5
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "cam_time.h"
#include "log.h"

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
Int32 cam_set_time(CamDateTime *dateTime)
{
	Int32			result;
	time_t			newTime;
	struct tm		tm;
	struct timeval 	tv;

	if(!dateTime)
		return E_INVAL;

	tm.tm_sec = dateTime->second;
	tm.tm_min = dateTime->minute;
	tm.tm_hour = dateTime->hour;
	tm.tm_mday = dateTime->day;
	tm.tm_mon = dateTime->month - 1;
	tm.tm_year = dateTime->year - 1900;

	newTime = mktime(&tm);

	tv.tv_sec = newTime;
	tv.tv_usec = 0;

	result = settimeofday(&tv, NULL);

	if(result < 0) {
		ERRSTR("set time failed");
		return E_REFUSED;
	}

	return E_NO;
}

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
Int32 cam_get_time(CamDateTime *dateTime)
{
	struct timeval 	tv;

    gettimeofday(&tv, NULL);

	return cam_convert_time(&tv, dateTime);
}

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
Int32 cam_convert_time(struct timeval *tv, CamDateTime *dateTime)
{
	struct tm 		tm;
	
	if(!tv || !dateTime)
		return E_INVAL;
	
	localtime_r(&tv->tv_sec, &tm);

	dateTime->year = tm.tm_year + 1900;
	dateTime->month = tm.tm_mon + 1;
	dateTime->day = tm.tm_mday;
	dateTime->weekDay = tm.tm_wday;
	dateTime->hour = tm.tm_hour;
	dateTime->minute = tm.tm_min;
	dateTime->second = tm.tm_sec;
	dateTime->ms = tv->tv_usec >> 10; //just convert to a close value
	dateTime->us = 0;

	return E_NO;
}

