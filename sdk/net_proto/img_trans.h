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
