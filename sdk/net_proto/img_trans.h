#ifndef __IMG_TRANS_H__
#define __IMG_TRANS_H__

#include "common.h"


#define IMG_TRANS_SRV_PORT_DEFAULT	9300	

typedef struct ImgTransObj *ImgTransHandle;

typedef struct {
	Uint32		serialNumber;		// index of image
	Uint32		imageType;			// image type, color space
	Uint16		imageWidth;			// width of image
	Uint16		imageHeight;		// height of image
	Uint32		imageLen;			// len of image
}ImgHdrInfo;

/* image format supported */
enum ImageType
{
	IMGTYPE_RAW = 0,
	IMGTYPE_YUV,
	IMGTYPE_JPEG,
	IMGTYPE_H264
};

/* info append at end of JPEG image */
#define IMG_APPEND_MAGIC	0xADD2BACE

typedef struct {
	Int8	roadName[64];		//road info
	Uint16	roadNum;			//Road Number
	Uint16	direction;			//Direction Number
	Uint16	devSN;				//Divice Serial Num
	Uint16	reserved;
} ImgRoadInfo;


typedef struct {
	ImgDimension 	dimension;		// dimension info, width, height, format, etc.
	Uint16			frameId;		// frame id, count by hardware
	Uint8			capMode;		// capture mode of this frame, 0--continue, 1--normal trig, 2--spec trig
	Uint8			strobeStat;		// strobe status, bit[0:3]--strobe[0:3]
	Uint32 			exposureTime;
	Uint16			globalGain;
	Uint16			avgLum;
	Uint16			rgbGains[3];	// 0--red, 1--green, 2--blue
	Uint16			frameType;		
	DateTime		timeStamp;	
	ImgRoadInfo		roadInfo;		// info of road
	Uint8			capInfo[1024];	// info of trigger such as way num,  trigger id, etc.
	Uint32			offset;			// offset to start of append info
	Uint32			magicNum;		// magic num for info recg
}ImgAppendInfo;


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/* create module */
extern ImgTransHandle img_trans_create(const char *ipString, Uint16 port, const char *srcDesp, Uint32 timeout, Int32 flags);

/* delete module */
extern Int32 img_trans_delete(ImgTransHandle hTrans);

/* set server IP, port */
extern Int32 img_trans_set_srv_info(ImgTransHandle hTrans, const char *ipString, Uint16 port);

/* connect server */
extern Int32 img_trans_connect(ImgTransHandle hTrans, Uint32 timeoutSec);

/* disconnect server */
extern Int32 img_trans_disconnect(ImgTransHandle hTrans);

/* send image */
extern Int32 img_trans_send(ImgTransHandle hTrans, const ImgHdrInfo *info, const void *data);

/* set our description */
extern Int32 img_trans_set_src_desp(ImgTransHandle hTrans, const char *srcDesp);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
