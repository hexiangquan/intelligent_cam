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
#include <linux/types.h>
#include "ext_io.h"
#include <sys/ioctl.h>

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
Int32 cam_time_sync()
{
	int fd = open(EXTIO_DEV_NAME, O_RDWR);

	if(fd < 0) {
		ERRSTR("open %s failed.", EXTIO_DEV_NAME);
		return E_IO;
	}

	struct hdcam_rtc_time rtcTime;
	int err = ioctl(fd, EXTIO_G_RTC, &rtcTime);
	
	close(fd);
	if(err < 0) {
		ERRSTR("get rtc time failed");
		return E_IO;
	}

	/* convert time */
	time_t			newTime;
	struct tm		tm;
	struct timeval 	tv;
	tm.tm_sec = rtcTime.second;
	tm.tm_min = rtcTime.minute;
	tm.tm_hour = rtcTime.hour;
	tm.tm_mday = rtcTime.day;
	tm.tm_mon = rtcTime.month - 1;
	tm.tm_year = rtcTime.year - 1900;

	newTime = mktime(&tm);

	tv.tv_sec = newTime;
	tv.tv_usec = rtcTime.millisec << 10;

	/* sync with system time */
	err = settimeofday(&tv, NULL);
	if(err < 0) {
		ERRSTR("set time failed");
		return E_REFUSED;
	}
	
	return E_NO;
}
 
/*****************************************************************************
 Prototype    : cam_hw_clock_set
 Description  : Sync time to hardware rtc 
 Input        : const CamDateTime *dateTime  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/9/11
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 cam_hw_clock_set(const CamDateTime *dateTime)
{
	int fd = open(EXTIO_DEV_NAME, O_RDWR);

	if(fd < 0) {
		ERRSTR("open %s failed.", EXTIO_DEV_NAME);
		return E_IO;
	}

	struct hdcam_rtc_time rtcTime;
	rtcTime.year = dateTime->year;
	rtcTime.month = dateTime->month;
	rtcTime.day = dateTime->day;
	rtcTime.hour = dateTime->hour;
	rtcTime.minute = dateTime->minute;
	rtcTime.second = dateTime->second;
	rtcTime.millisec = dateTime->ms;
	rtcTime.weekday = dateTime->weekDay;
	rtcTime.flags = 0;

	int err = ioctl(fd, EXTIO_S_RTC, &rtcTime);
	close(fd);
	
	if(err < 0) {
		ERRSTR("set rtc time failed");
		return E_INVAL;
	}
	
	return E_NO;
}


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
Int32 cam_set_time(const CamDateTime *dateTime)
{
	Int32			result;
	time_t			newTime;
	struct tm		tm;
	struct timeval 	tv;

	if(!dateTime)
		return E_INVAL;

	/* sync time with hw clock */
	result = cam_hw_clock_set(dateTime);
	if(result != E_NO)
		return result;

	tm.tm_sec = dateTime->second;
	tm.tm_min = dateTime->minute;
	tm.tm_hour = dateTime->hour;
	tm.tm_mday = dateTime->day;
	tm.tm_mon = dateTime->month - 1;
	tm.tm_year = dateTime->year - 1900;

	newTime = mktime(&tm);

	tv.tv_sec = newTime;
	tv.tv_usec = (dateTime->ms << 10) + dateTime->us;

	result = settimeofday(&tv, NULL);

	if(result < 0) {
		ERRSTR("set time failed");
		return E_REFUSED;
	}

	DBG("cur time: %04u.%02u.%02u %02u:%02u:%02u",
		(__u32)dateTime->year, (__u32)dateTime->month, (__u32)dateTime->day,
		(__u32)dateTime->hour, (__u32)dateTime->minute, (__u32)dateTime->second);
	//system("date");

	return result;
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
	/* For FPGA debug only */
	int fd = open(EXTIO_DEV_NAME, O_RDWR);

	if(fd > 0) {
		struct hdcam_rtc_time rtcTime;
		ioctl(fd, EXTIO_G_RTC, &rtcTime);
		close(fd);
	}

	
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
Int32 cam_convert_time(const struct timeval *tv, CamDateTime *dateTime)
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

