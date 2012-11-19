#ifndef __PLATE_RECOG_H__
#define __PLATE_RECOG_H__

#include "common.h"

#define CAM_MAX_PLATE_RECOG_NUM		4	

#define PR_MAX_LOCATE_THRESHOLD		9
#define PR_MAX_RECOG_THRESHOLD		9

/* Bits define for flags */
#define PR_FLAG_MOD_EN				(1 << 0)	//overall enable flag for plate recog
#define PR_FLAG_INDIVIDUAL_EN		(1 << 1)	//enable individual plate recog
#define PR_FLAG_TORROW_YELLOW_EN	(1 << 2)	//enable torrow yellow plate recog
#define PR_FLAG_ARM_POLICE_EN		(1 << 3)	//enable arm police plate recog
#define PR_FLAG_TORROW_ARMY_EN		(1 << 4)	//enable torrow army plate recog
#define PR_FLAG_TRACTOR_EN			(1 << 5)	//enable tractor plate recog

/* Config params for license plate recognization */
typedef struct _CamPlateRecogCfg {	
	Uint32 		flags;					//flags for enable special functions
	Uint16 		minPlateWidth;			//min license plate width, in pixels
	Uint16 		maxPlateWidth;			//max license plate width, in pixels
	Rectangle 	recogRegion;			//region for recognize plate
	Int8 		defaultProvinces[24];	//default provinces in Chinese characters
	Uint16 		locateThreshold;		//threshold for locate plate, 0~9	
	Uint16 		recogThreshold;			//threshold for recognize plate, 0~9
	Int8 		reserved[16];
}CamPlateRecogCfg;

/* type of license plate */
typedef enum {
	LT_UNKNOWN = 0, //unkown license plate
	LT_BLUE,   		//normal blue plate
	LT_BLACK,  		//normal black plate
	LT_YELLOW,  	//single layer yellow plate
	LT_YELLOW2,		//double layer yellow plate, truck/tractor
	LT_POLICE,		//police plate
	LT_ARMPOL,		//arm police plate
	LT_INDIVI,  	//invdividual plate
	LT_ARMY,   		//signle layer army plate
	LT_ARMY2,  		//double layer army plate
	LT_EMBASSY,  	//embassy plate
	LT_HONGKONG,  	//Hong Kong plate
	LT_TRACTOR,  	//agriculture tractor plate
}LicensePlateType;


/* Plate recognization result define */
typedef struct _SinglePlateInfo {
	Int8 		license[16];		//licence info
	Rectangle   location;    		//location of the plate
	Uint8 		color; 				//color of the plate, see enum Color in common.h
	Uint8 		type;				//type of plate
	Uint8		laneId;				//way number, start from 0
	Uint8 		confidence;			//confidence of the result, 0~100
	Uint8 		brightness;			//brightness of the plate, unused
	Uint8  		direction;			//direction of the plate, unused
	Uint8 		vehicleBrightness;	//brightness of the vehicle, unused
	Uint8 		vehicleColor;		//color of vehicle, unused
	Int8 		reserved[8]; 		//reserved
}SinglePlateInfo;

/* Whole image info */
typedef struct {
	Uint32			totalNum;	//total num of plate recognized
	SinglePlateInfo info[CAM_MAX_PLATE_RECOG_NUM];
}LicensePlateInfo;

#endif

