#ifndef __CAM_ENC_H__
#define __CAM_ENC_H__

#include "common.h"

/*
  * image encode params, if width or height set to 0, use capture resolution
  */
typedef struct {
	Uint16	width;	
	Uint16	height;
	Uint16	encQuality;	//0~97
	Uint16	rotation;	//only support 0, 90, 180, 270, anti-clockwise
}CamImgEncParams;

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

#define CAM_H264_MAX_BIT_RATE	6000	//max bit rate

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

#endif

