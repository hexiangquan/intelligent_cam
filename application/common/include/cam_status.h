#ifndef __CAM_STATUS_H__
#define __CAM_STATUS_H__

#include "common.h"

/* 
  * Current camera status
  */
typedef enum 
{
	WORK_STATUS_IDLE = 0,		//Idle, do nothing
	WORK_STATUS_RUNNING,		//Capturing and encoding
	WORK_STATUS_UPDATING,		//Updating program
	WORK_STATUS_MAX,
}CamWorkStatus;

/* 
  * Work mode 
  */
typedef struct 
{
	Uint16	format;		//Output Format
	Uint16	resType;	//Resolution type
	Uint16	capMode;	//Capture mode
	Uint16	flags;		//Flag for control
	Uint16	reserved[2];
} CamWorkMode;

typedef enum
{
	CAM_FMT_RAW = 0,		// Only raw
	CAM_FMT_YUV = 1,		// Only yuv
	CAM_FMT_JPEG = 2,		// Only jpeg
	CAM_FMT_H264 = 3,		// Only h264
	CAM_FMT_JPEG_H264 = 4,	// jpeg & h264, double stream
	CAM_FMT_MAX,
}CamFormat;

typedef enum
{
	CAM_RES_FULL_FRAME = 0,		//Full frame
	CAM_RES_HIGH_SPEED = 1,		//High speed frame
	CAM_RES_MAX,
}CamResType;

typedef enum _CamCapMode
{
	CAM_CAP_MODE_CONTINUE = 0,	// Continouly capture
	CAM_CAP_MODE_TRIGGER = 1,	// Trigger by detector or mannual
	CAM_CAP_MODE_MAX,
}CamCapMode;


/*
 *  Day night mode cfg
 */
typedef enum {
	CAM_DN_SWT_OFF = 0,		//don't switch
	CAM_DN_SWT_AUTO = 1,	//switch according to environment light
	CAM_DN_SWT_TIME = 2,	//switch according to time  
	CAM_DN_SWT_MAX,
}CamDayNightSwitchMethod;

typedef enum {
	CAM_DAY_MODE = 0,
	CAM_NIGHT_MODE,
	CAM_DN_MODE_MAX,
}CamDayNightMode;

typedef struct {
	Uint8	mode;				// Current mode,  See enum CamDayNightMode
	Uint8	switchMethod;		// See enum CamDayNightSwitchMethod
	Uint16	threshold;			// Avg Lum threshold for auto switch, 0~1023
	Uint8	dayModeStartHour;	//Day  mode start hour, 0~23
	Uint8	dayModeStartMin;	//Day mode end hour, 0~59
	Uint8	nightModeStartHour;	//Night mode start hour, 0~23
	Uint8	nightModeStartMin;	//Night mode start minute, 0~59
} CamDayNightModeCfg;

/* enum for capture control */
typedef enum {
	CAM_CAP_STOP = 0,
	CAM_CAP_START = 1,
	CAM_CAP_RESTART = 2,
	CAM_CAP_TRIG = 3,
	CAM_CAP_SPEC_TRIG = 4,
	CAM_CAP_MAX,
}CamCapCtrl;

#endif

