#ifndef __CAM_OSD_H__
#define __CAM_OSD_H__

#include "common.h"

/* 
  * Cam OSD params
  */
typedef struct {
	Int8	osdString[64];	//osd string, ended with '/0', support GB2312 chinese code
	Uint16	flags;			//flag for ctrl
	Uint16	color;			//color of osd
	Uint16	postion;		//position of this osd
	Uint16	size;			//size of osd
} CamOsdInfo;

typedef struct {
	CamOsdInfo	imgOsd; 	//image osd params
	CamOsdInfo	vidOsd; 	//video osd params
} CamOsdParams;
	
#define CAM_OSD_FLAG_EN						(1 << 0) 	//Enable display
#define CAM_OSD_FLAG_TIME_EN 				(1 << 1)	//Display time
#define CAM_OSD_FLAG_TEXT_EN				(1 << 2)	//Display osd string
#define CAM_OSD_FLAG_WAY_NUM_EN				(1 << 3)	//Display way number, only for image
#define CAM_OSD_FLAG_SPEED_EN				(1 << 4)	//Display speed, only for image
#define CAM_OSD_FLAG_RED_LIGHT_TIME_EN		(1 << 5)	//Display red light time, only for image
#define CAM_OSD_FLAG_OVERSPEED_INFO_EN		(1 << 6)	//Display overspeed info, only for image
#define CAM_OSD_FLAG_FRAME_NUM_EN			(1 << 7)	//Display frame number, only for image
#define CAM_OSD_FLAG_GROUP_ID_EN			(1 << 8)	//Display group id, only for image
#define CAM_OSD_FLAG_RETROGRADE_EN			(1 << 9) 	//Dsiplay retrograde info, only for image
#define CAM_OSD_FLAG_MILISEC_EN				(1 << 10)	//Display milisecond

#define CAM_VID_OSD_FLAG_MASK				(0x0007)
	
/* OSD color define */
typedef enum _CamOsdColor {
	CAM_OSD_COLOR_YELLOW = 0,
	CAM_OSD_COLOR_RED,
	CAM_OSD_COLOR_GREEN,
	CAM_OSD_COLOR_BLUE,
	CAM_OSD_COLOR_GRAY,
	CAM_OSD_COLOR_MAX
}CamOsdColor; 

/* OSD position define */
typedef enum _CamosdPos {
	CAM_OSD_POS_UP_LEFT = 0,
	CAM_OSD_POS_DOWN_LEFT = 1,
	CAM_OSD_TEXT_POS_MAX,
}CamOsdPos;

/* OSD size */
typedef enum _CamOsdSize {
	CAM_OSD_SIZE_32x16 = 0,
	CAM_OSD_SIZE_32x32,
	CAM_OSD_SIZE_MAX
}CamOsdSize;

#endif 

