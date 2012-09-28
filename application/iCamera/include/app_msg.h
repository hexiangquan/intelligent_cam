#ifndef ___APP_MSG__
#define ___APP_MSG__

#include "msg.h"
#include "buffer.h"
#include "cam_detector.h"
#include "cam_info.h"
#include "cap_info_parse.h"

#define MSG_MAX_LEN		1024

/* Name of msg module */
#define MSG_ROOT		"/tmp"
#define MSG_MAIN		MSG_ROOT"/msgMain"
#define MSG_CAP			MSG_ROOT"/msgCap"
#define MSG_CONV		MSG_ROOT"/msgConv"
#define MSG_IMG_ENC		MSG_ROOT"/msgImgEnc"
#define MSG_VID_ENC		MSG_ROOT"/msgVidEnc"
#define MSG_IMG_TX		MSG_ROOT"/msgImgTx"
#define MSG_CTRL		MSG_ROOT"/iCamCtrl"
#define MSG_LOCAL		MSG_ROOT"/msgLocal"


/* enable crc check sum of data between threads */
//#define CRC_EN

/* some common macros */
#define IMG_MAX_WIDTH			2560
#define IMG_MAX_HEIGHT			2048

#define IMG_ENC_NAME			"imgEncoder"
#define VID_ENC_NAME			"vidEncoder"

#define IMG_ENC_POOL_BUF_NUM	2
#define VID_ENC_POOL_BUF_NUM	0

#define SD0_MNT_PATH			"/media/mmcblk0"
#define SD1_MNT_PATH			"/media/mmcblk1"

/* Cmd of this application */
typedef enum _AppCmd {
	APPCMD_EXIT = 0x1000,		//program exit
	APPCMD_REBOOT,				//system reboot
	APPCMD_CAP_EN,				//capture ctrl, start, stop, restart
	APPCMD_RAW_DATA,			//new raw data
	APPCMD_FREE_RAW,			//free raw data
	APPCMD_NEW_DATA,			//new image data
	APPCMD_SET_IMG_CONV,		//set img adjust params
	APPCMD_SET_STREAM2,			//set 2nd stream params
	APPCMD_SET_ENC_PARAMS,		//set encode params for image & h.264 enc thread
	APPCMD_SET_WORK_MODE,		//set work mode
	APPCMD_SET_UPLOAD_PARAMS,	//set img upload params
	APPCMD_SET_TRIG_PARAMS,		//set trigger params
	APPCMD_SET_AE_PARAMS,		//set auto exposure params
	APPCMD_SET_AWB_PARAMS,		//set auto white balance params
	APPCMD_SET_CAP_INPUT,		//update cap input info
	APPCMD_SET_ROAD_INFO,		//update road info
	APPCMD_SEND_DIR,			//send all file in a dir
	APPCMD_SEND_FILE,			//send specified file
	APPCMD_UPLOAD_CTRL,			//upload ctrl cmds
	APPCMD_DAY_NIGHT_SWITCH,	//switch day/night mode
	APPCMD_RESTORE_CFG,			//restore cfg to default value
	APPCMD_NTP_SYNC,			//sync ntp time
	APPCMD_MAX
}AppCmd;

/* General msg data, can be used for recv */
typedef struct _CommonMsg {
	MsgHeader	header;				/* Msg header, must be the first field */
	Int8		buf[MSG_MAX_LEN];	/* Buffer for append data */
}CommonMsg;

/* Msg structure for new frame */
typedef struct _ImgMsg {
	MsgHeader		header;				/* Msg header, must be the first field */
	BufHandle		hBuf;				/* Buffer handle */
	Int32			index;				/* Frame index */
	ImgDimension	dimension;			/* Format info of image */
	DateTime		timeStamp;			/* Capture time */
	struct timeval	timeCode;			/* Another time stamp */
	Int32			frameType;			/* Frame type for video */
	CamRoadInfo		roadInfo;			/* Road info */
	RawCapInfo		rawInfo;			/* raw capture info from hardware */
	CaptureInfo		capInfo;			/* capture info of this frame */
}ImgMsg;

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/* api for send only header msg */
extern Int32 app_hdr_msg_send(MsgHandle hMsg, const char *dstName, Uint16 cmd, Int32 param0, Int32 param1);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /*___APP_MSG__ */

