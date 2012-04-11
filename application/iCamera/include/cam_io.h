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

#define CAM_MAX_STROBE_NUM		2

#define CAM_STROBE_FLAG_EN0		(1 << 0)	//enable strobe0
#define CAM_STROBE_FLAG_EN1		(1 << 1)	//enable strobe1
#define CAM_STROBE_FLAG_AUTO0	(1 << 8)	//enable strobe0 auto switch
#define CAM_STROBE_FLAG_AUTO1	(1 << 9)	//enable strobe1 auto switch

typedef enum {
	CAM_STROBE_SWITCH_AUTO = 0, //switch automatically
	CAM_STROBE_SWITCH_TIME, 	//switch according to system time
	CAM_STROBE_SWITCH_MAX,
}CamStrobeSwitchMode;

typedef struct {
	Uint16	status;			//current status, bit[0:1]--strobe[0:1]
	Uint16	switchMode;		//switch mode, see CamStrobeSwitchMode
	Uint16	ctrlFlags;		//bit[0:1]--strobe[0:1] enable
	Uint16	threshold;		//threshold for auto switch
	Int32	offset;			//offset for pre-enable strobe, Uinit: us
	Uint8	enStartHour;	//start hour, 0~23
	Uint8	enStartMin;		//start minute, 0~59
	Uint8	enEndHour;		//end hour
	Uint8	enEndMin;		//end minute, 0~59
} CamStrobeCtrlParam;

#endif

