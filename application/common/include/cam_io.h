#ifndef __CAM_IO_H__
#define __CAM_IO_H__

#include "common.h"

/*
  * General IO config
  */
#define CAM_IO_DIR_INPUT		0
#define CAM_IO_DIR_OUTPUT		1
#define CAM_IO_STAT_HIGH		1
#define CAM_IO_STAT_LOW			0

typedef struct {
	Uint32	direction;	//bit[0:7] -- IO[0:7], 1--output, 0--input
	Uint32  status;		//bit[0:7] -- IO[0:7], 1--high, 0--low
}CamIoCfg;

/*
  * Strobe control params
  */

#define CAM_MAX_STROBE_NUM			3

#define CAM_STROBE_STAT_EN0			(1 << 0)	//enable strobe0
#define CAM_STROBE_STAT_EN1			(1 << 1)	//enable strobe1
#define CAM_STROBE_STAT_EN2			(1 << 2)	//enable strobe2

#define CAM_STROBE_FLAG_AUTO0		(1 << 0)	//enable strobe0 auto switch
#define CAM_STROBE_FLAG_AUTO1		(1 << 1)	//enable strobe1 auto switch
#define CAM_STROBE_FLAG_AUTO2		(1 << 2)	//enable strobe2 auto switch
#define CAM_STROBE_FLAG_EN_BY_WAY	(1 << 4)	// enable according to trigger way num
#define CAM_STROBE_FLAG_EN_IN_TURN	(1 << 5)	// enable each strobe in turn

#define CAM_STROBE_SIG_HIGH			(1)	// edge of sync enable
#define CAM_STROBE_SIG_LOW			(0)

#define CAM_STROBE_MODE_TRIG		(0)	// trigger output
#define CAM_STROBE_MODE_FREQ		(1)	// frequent output

#define CAM_STROBE_AC_SYNC_EN		(1)	//sync with AC signals

typedef enum {
	CAM_STROBE_SWITCH_OFF = 0,	//not switch 
	CAM_STROBE_SWITCH_AUTO, 	//switch automatically by env lum
	CAM_STROBE_SWITCH_TIME, 	//switch according to system time
	CAM_STROBE_SWITCH_MAX,
}CamStrobeSwitchMode;

typedef struct {
	Uint16	status;			//current status, bit[0:2]--strobe[0:2]
	Uint16	switchMode;		//switch mode, see CamStrobeSwitchMode
	Uint16	ctrlFlags;		//see macros for CAM_STROBE_FLAG_XXX above
	Uint16	thresholdOn;	//lum threshold for turn on strobe in auto switch mode
	Uint16	thresholdOff;	//lum threshold for turn off strobe in auto switch mode
	Uint16	offset;			//offset for pre-enable strobe, Uinit: us
	Uint8	enStartHour;	//start hour, 0~23
	Uint8	enStartMin;		//start minute, 0~59
	Uint8	enEndHour;		//end hour
	Uint8	enEndMin;		//end minute, 0~59
	Uint8	sigVal;			//output signals value, bit[0:2]-stobe[0:2], 
	Uint8	outMode;		//trig or freq output, bit[0:2]-strobe[0:2]
	Uint8	acSyncEn;		//sync with AC signals or not
	Uint8	specCapEn;		//used for special trigger, bit[0:2] - strobe[0:2], 1-enable when special trigger
	Uint8	reserved[4];		
} CamStrobeCtrlParam;

#endif

