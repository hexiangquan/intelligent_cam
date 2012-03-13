/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : camCtl.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2011/10/11
  Last Modified :
  Description   : camCtl.c header file
  Function List :
  History       :
  1.Date        : 2011/10/11
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __CAM_PARAMS_H__
#define __CAM_PARAMS_H__

#include "common.h"

/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/

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
 * routines' implementations                    *
 *----------------------------------------------*/

/*
  * Firmware version info 
  */
typedef struct {
	Uint32	dspVer;
	Uint32	fpgaVer;
	Uint32	armVer;
} CamVersionInfo;

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

/* 
  * Network info 
  */
typedef struct {
	Uint8	ipAddr[16];  	// Set to "0.0.0.0" for DHCP    
	Uint8	ipMask[16];  	// Not used when using DHCP
	Uint8	gatewayIP[16];  // Not used when using DHCP
	Uint8	domainName[20];	// Not used when using DHCP
	Uint8	hostName[20];	// Host name
	Uint8	dnsServer[16];	// Used when set to anything but zero
} CamNetworkInfo;

/* 
  * Device info 
  */
typedef struct {
	Uint8	macAddr[8];		//Mac Addr
	Uint32	deviceSN;		//Device Serial Number
} CamDeviceInfo;


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
	CamOsdInfo	imgOsd;		//image osd params
	CamOsdInfo	vidOsd;		//video osd params
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


/* 
  * Road info for this device
  */
#define MAX_ROAD_NAME_SIZE 		64

typedef struct {
	Int8	roadName[MAX_ROAD_NAME_SIZE];	//road info
	Uint16	roadNum;						//Road Number
	Uint16	directionNum;					//Direction Number
	Uint16	devSN;							//Divice Serial Num
	Uint16	reserved;
} CamRoadInfo;

/* 
  * RTP params for H.264 transfer 
  */
typedef struct {
	Int8	dstIp[16];		//Destination IP address
	Uint16	dstPort;		//Destination port
	Uint16	localPort;		//Local port for RTP sending
	Uint16	flag;			//Control flag 
	Uint8	payloadType;	//Payload type, 0~200	
	Uint8	reserved;
} CamRtpParams;

#define CAM_RTP_FLAG_EN		(1 << 0) //Enable Rtp send

/* 
  * Image upload type 
  */
typedef enum {
	CAM_UPLOAD_PROTO_TCP = 0,
	CAM_UPLOAD_PROTO_FTP =1,	
	CAM_UPLOAD_PROTO_NONE,		//don't upload, save to local file system
	CAM_UPLOAD_PROTO_MAX
} CamImageUploadProtocol;

/* 
  * TCP image recv server info 
  */
typedef struct {
	Int8	serverIP[16];	//Image server IP
	Uint16	serverPort;		//Image server listen port
	Uint16	flag;			//control flag, reserved
} CamTcpImageServerInfo;

/* 
  * FTP image server info 
  */
#define CAM_FTP_MAX_USERNAME_LEN		32
#define CAM_FTP_MAX_PASSWORD_LEN		32
#define CAM_FTP_MAX_PATHNAME_PATTERN	256
#define CAM_FTP_MAX_ROOT_DIR_LEN		64

typedef struct {
	Int8	serverIP[16];						//Ftp server Ip
	Uint16	serverPort;							//Listen port
	Uint16	flag;								//Flag for other function
	Int8	userName[CAM_FTP_MAX_USERNAME_LEN];	//User name
	Int8	password[CAM_FTP_MAX_PASSWORD_LEN];	//password
	Int8	rootDir[CAM_FTP_MAX_ROOT_DIR_LEN];	//root dir name
	Int8	pathNamePattern[CAM_FTP_MAX_PATHNAME_PATTERN];	//pathname generate pattern
} CamFtpImageServerInfo;

/*
  * NTP info for sync
  */
typedef struct {
	Uint8	serverIP[16];	//NTP server IP
	Uint16	serverPort;		//Listen port
	Uint16	syncPrd;		//sync interval period, unit: hour, set to 0 to disable sync
} CamNtpServerInfo;

/* 
  * Exposure params for camera control 
  */
typedef struct {
	Uint32	shutterTime;		//shutter time, unit: us
	Uint16	globalGain;			//0~1023, equals 6db~42db
	Uint16	reserved;
} CamExprosureParam;

/* 
  * RGB gains
  */
typedef struct {
	Uint16	redGain;			//Red gain, 0~511
	Uint16	greenGain[2];		//Green gain, two channels, 0~511
	Uint16	blueGain;			//Blue gain, 0~511
} CamRGBGains;

/* 
  * Dynamic range compression params 
  */
typedef struct {
	Uint8	strength;			//Adjust strength, 0~255
	Uint8	flag;				//Flag for function enable
	Uint8	reserved[8];		//reserved
} CamDRCParam;

#define DRC_FLAG_EN		(1 << 0)	//DRC enable

/* 
  * Rectangle region in image, used in other structures 
  */
typedef struct
{
	Uint16	startX;		//X position of Start point
	Uint16	startY;		//Y position of Start point
	Uint16	endX;		//X position of End point
	Uint16	endY;		//Y position of End point
} CamRect;

/* 
  * Traffic light correct regions 
  */
typedef struct {
	CamRect 	region[3];		//redlight regions
} CamTrafficLightRegions;

/* 
  * Image enhance params 
  */
typedef struct {
	Uint16	flags;			//control flag
	Uint16	contrast;		//value of contrast 
	Uint16	sharpness;		//value of sharpness
	Uint16	brightness;		//value of brightness, unused at current ver.
	Uint16	saturation;		//value of saturation, unused at current ver.
	Uint16	digiGain;		//digital gain
	Uint16	reserved[2];	//reserved
} CamImgEnhanceParams;

/* Flags for image enhance */
#define CAM_IMG_CONTRAST_EN		(1 << 0)	//enable contrast adjust
#define CAM_IMG_SHARP_EN		(1 << 1)	//enable sharpness adjust
#define CAM_IMG_GAMMA_EN		(1 << 2)	//enable brightness adjust
#define CAM_IMG_SAT_EN			(1 << 3)	//enable saturation adjust
#define CAM_IMG_MED_FILTER_EN	(1 << 4)	//enable median filter
#define CAM_IMG_NF_EN			(1 << 5)	//enable noise filter

/* 
  * H.264 encode params  
  */

enum eCamH264RateControl
{
	CAM_H264_RC_CBR = 0, 	/**< constant Bitrate rate control */
	CAM_H264_RC_VBR,		/**< Variable Bitrate control.*/
	CAM_H264_RC_FQP,		/**< Constant quality, fixed QP */
	CAM_H264_RC_CVBR,		/**< Constrained variable bitrate */
	CAM_H264_RC_FFS,		/**< Fixed  Frame  size  Rate  Control */
	CAM_H264_RC_CUSTOM_CBR,	/**< customized version of CBR,  
						  	*    reduce the breathing artifacts in encoded videos
						  	*/
	CAM_H264_RC_CUSTOM_VBR,	/**< customized version of VBR, targeted for sequences 
						 	 *	 with varying complexity distribution 
						  	*/
	CAM_H264_RC_MAX,
};

enum eCamH264Resolution
{
	H264_RES_1920X1080 = 0,		//1080P
	H264_RES_1280X720 = 1,		// 720P
	H264_RES_1600X1200 = 2,		//200W
	H264_RES_720X480 = 3,		//D1
	H264_RES_MAX,
};

typedef struct {
	Uint8	resolution;			//H.264 video resolution, see eH264 Resolution
	Uint8	frameRate;			//Encode frame rate, 1~30fps
	Uint8	rateControl;		//Rate control type£¬see eH264RateControl
	Uint8	forceIFrame;		//When set to non zero, force every frame to be I frame
	Uint16	bitRate;			//Target bit rate, Uint: Kbpss
	Uint16	IPRatio;			//I frame and P frame interval, default: 30
	Uint8	QPInit;				//Init QP value, 0~51
	Uint8	QPMax;				//Max QP value, QPMin~51
	Uint8	QPMin;				//Min QP value, 0~QPMax
	Uint8	encPreset;			//Enc type preset, Not supported 
	Uint16	packetSize;			//Percentage of detach one frame
	Uint16	videoLen;			//Time of a video file after one trigger
	Uint32	flags;				//Reserved for future use 
} CamH264Params;

/* 
  * Analog video params 
  */
typedef struct {
	Uint32	avType;		//AV type, see enum eAvType
	Uint32	flags;		//Flags reserved
} CamAVParam;

enum eAvType {
	AV_TYPE_NONE = 0,	//No output
	AV_TYPE_NTSC = 1, 	//NTSC standard
	AV_TYPE_PAL = 2,	//PAL standard
	AV_TYPE_MAX,
};

/* Len for LUT update */
#define NET_LUT_LEN		4096

/* 
  * Current camera status
  */
typedef enum 
{
	WORK_STATUS_IDLE = 0,		//Idle, do nothing
	WORK_STATUS_RUNNING,		//Capturing and encoding
	WORK_STATUS_UPDATING		//Updating program
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
  * image encode params, if width or height set to 0, use capture resolution
  */
typedef struct {
	Uint16	width;	
	Uint16	height;
	Uint16	encQuality;	//0~97
	Uint16	rotation;	//only support 0, 90, 270, anti-clockwise
}CamImgEncParams;

/*
  * special capture params 
 */
typedef struct {
	Uint32	expTime;		// Unit: us
	Uint16	globalGain;		// Range: 0~1023
	Uint16	strobeEn;		// Bit[0:1]- strobe0, strobe1, 1-enable, 1-disable
	Uint32	reserved[2];
}CamSpecCapParams;

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
#define CAM_STROBE_FLAG_STROBE1	(1 << 8)	//enable strobe0 in strobe mode
#define CAM_STROBE_FLAG_STROBE2	(1 << 9)	//enable strobe1 in strobe mode

typedef enum {
	CAM_STROBE_SWITCH_AUTO = 0, //switch automatically
	CAM_STROBE_SWITCH_TIME, 	//switch according to system time
}CamStrobeSwitchMode;

typedef struct {
	Uint16	status;			//current status, bit[0:1]--strobe[0:1]
	Uint16	switchMode;		//switch mode, see CamStrobeSwitchMode
	Uint16	ctrlFlags;		//bit[0:1]--strobe[0:1] enable
	Uint16	threshold;		//threshold for auto switch
	Uint8	enStartHour;	//start hour, 0~23
	Uint8	enStartMin;		//start minute, 0~59
	Uint8	enEndHour;		//end hour
	Uint8	enEndMin;		//end minute, 0~59
} CamStrobeCtrlParam;


/*
  * Vehicle detector config params
  */
  
/* Max Trigger ways support */
#define APP_MAX_CAP_CNT			4

typedef struct {
	Uint16  detecotorId;			//Type of detector
	Uint16	capDelayTime;			//3rd Image capture delay time, Uint: ms
	Uint16	redLightCapFlag;		//Flag for red light capture strategy 
	Uint16	greenLightCapFlag;		//Flag for green light capture strategy 
	Uint16	retrogradeCapFlag;		//Flag for retrograde capture strategy 
	Uint16	loopDist[APP_MAX_CAP_CNT];//distance between two loops, Uint: cm
	Uint8	limitSpeed;				//nominal limited speed, Uint: km/h
	Uint8	calcSpeed;				//actual speed limitation that triggers a capture, Uint: km/h
	Uint8	speedModifyRatio[APP_MAX_CAP_CNT]; //Ratio for speed modification
	Uint32	trigVideoLen;			//How much time to trigger H264 video, unit: Second, if set to 0, not trigger
	Int8	reserved[8];			//Reserved for future use
} CamDetectorParam;

enum VehicleDetectorId
{
	DETECTOR_IO = 0,			//IO trigger
	DETECTOR_TORY_EP,			//Tory epolice detector
	DETECTOR_TORY_CP,			//Tory checkpost detector
	DETECTOR_TORY_EP_NEW,		//Tory new epolice detector
	DETECTOR_VIDEO_TRIG,		//Video trigger detector
	DETECTOR_MAX
};

/* Trigger Position bits define */
#define DETECTOR_FLAG_LOOP1_POS_CAP		(1 << 0)	//Capture @ positive edge of loop1
#define DETECTOR_FLAG_LOOP2_POS_CAP		(1 << 1)	//Capture @ positive edge of loop2
#define DETECTOR_FLAG_LOOP1_NEG_CAP		(1 << 2)	//Capture @ negative edge of loop1
#define DETECTOR_FLAG_LOOP2_NEG_CAP		(1 << 3)	//Capture @ negative edge of loop2
#define DETECTOR_FLAG_OVERSPEED_CAP		(1 << 4)	//capture when overspeed
#define DETECTOR_FLAG_DELAY_CAP			(1 << 5)	//delay capture


/*
 * Auto exposure params
 */
#define CAM_AE_MAX_ROI_NUM			5

#define CAM_AE_FLAG_AE_EN			(1 << 0)	//enable Auto Exposure
#define CAM_AE_FLAG_AG_EN			(1 << 1)	//enable Auto Gain
#define CAM_AE_FLAG_AA_EN			(1 << 2)	//enable Auto Aperture

typedef struct
{
	Uint16		flags;					//Control flag
	Uint16 		targetValue;	 		//Target brightness, 0~255
	Uint32 		minShutterTime;			//Minimun shutter time, us
	Uint32 		maxShutterTime;			//Maximum shutter time, us
	Uint16 		minGainValue;			//Minimum global gain, 1~1023
	Uint16 		maxGainValue;			//Maxmum global gain, usMinGainValue~1023
	CamRect 	roi[CAM_AE_MAX_ROI_NUM]; //ROI region
	Uint8		minAperture;			//Min aperture value
	Uint8		maxAperture;			//Max aperture value
	Uint16		reserved;
}CamAEParam;

/*
 * Auto white balance params
 */
#define AWB_FLAG_EN		(1 << 0)		//enable AWB

typedef struct {
	Uint16		flags;					//control flag
	Uint16		maxValueR;				//max Red gain
    Uint16		maxValueG;				//max green gain, unused in current version
    Uint16		maxValueB;				//max blue gain
    Uint16		minValueR;				//min red gain
    Uint16		minValueG;				//min green gain, unused in current version
	Uint16		minValueB;				//min blue gain
	Uint16		redModifyRatio;			//red gain modify ratio
	Uint16		greenModifyRatio;		//green gain modify ratio, unused in current version
	Uint16		blueModifyRatio;		//blue gain modify ratio
	Uint16		initValueR[2];			//Init Red value at day and night, 0-day, 1-night 
	Uint16		initValueG[2];			//Init Green  value at day and night, 0-day, 1-night 
	Uint16		initValueB[2];			//Init Blue value at day and night, 0-day, 1-night 
} CamAWBParam;


/*
 *  Day night mode cfg
 */
typedef enum {
	CAM_DN_SWT_OFF = 0,		//don't switch
	CAM_DN_SWT_AUTO = 1,	//switch according to environment light
	CAM_DN_SWT_TIME = 2,	//switch according to time  
}CamDayNightSwitchMethod;

typedef struct {
	Int32	switchMethod;		// See enum CamDayNightSwitchMethod
	Uint8	dayModeStartHour;	//Day  mode start hour, 0~23
	Uint8	dayModeStartMin;	//Day mode end hour, 0~59
	Uint8	nightModeStartHour;	//Night mode start hour, 0~23
	Uint8	nightModeStartMin;	//Night mode start minute, 0~59
} CamDayNightModeCfg;

/*
 * Cam local file params 
 */
#define CAM_SD_NUM	1

typedef struct _CamFileInfoHdr
{
	Uint32	type;				//Type of this file, 0--file, 1--dir
	Uint32	size;				//Size of this file in bytes
}CamFileInfoHdr;

enum CamFileType
{
	FILE_TYPE_NORMAL = 0,		//Normal file
	FILE_TYPE_DIR = 1,			//Directory
	FILE_TYPE_OTHER,			//Other type
};

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __CAMCTL_H__ */
