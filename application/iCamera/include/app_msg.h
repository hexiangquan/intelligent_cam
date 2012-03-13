#ifndef ___APP_MSG__
#define ___APP_MSG__

#include "msg.h"
#include "buffer.h"
#include "cam_params.h"
#include "capture.h"

#define MSG_MAX_LEN		1024

/* Name of msg module */
#define MSG_ROOT		"/tmp"
#define MSG_MAIN		MSG_ROOT"/msgMain"
#define MSG_CAP			MSG_ROOT"/msgCap"
#define MSG_IMG_ENC		MSG_ROOT"/msgImgEnc"
#define MSG_VID_ENC		MSG_ROOT"/msgVidEnc"
#define MSG_IMG_TX		MSG_ROOT"/msgImgTx"

/* enable crc check sum of data between threads */
//#define CRC_EN


/* Cmd of this application */
typedef enum _AppCmd {
	APPCMD_EXIT = 0x1000,		//program exit
	APPCMD_REBOOT,				//system reboot
	APPCMD_CAP_RDY,				//capture is ready
	APPCMD_NEW_DATA,			//new image data
	APPCMD_SET_IMG_CONV,		//set img adjust params
	APPCMD_SET_STREAM2,			//set 2nd stream params
	APPCMD_SET_IMG_ENC_PARAMS,	//set encode params for image enc thread
	APPCMD_SET_VID_ENC_PARAMS,	//set encode params for h.264 enc thread
	APPCMD_SET_WORK_MODE,		//set work mode
	APPCMD_SET_UPLOAD_PARAMS,	//set img upload params
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
	FrameBuf		rawFrame;			/* raw frame if the format is not encoded */
	Int32			index;				/* Frame index */
	ImgDimension	dimension;			/* Format info of image */
	CamDateTime		timeStamp;			/* Capture time */
	Int32			frameType;			/* Frame type for video */
}ImgMsg;



#endif

