/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : tcp_upload.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/12
  Last Modified :
  Description   : image upload using TCP protocol
  Function List :
              tcp_upload_connect
              tcp_upload_create
              tcp_upload_delete
              tcp_upload_disconnect
              tcp_upload_send
              tcp_upload_set_params
  History       :
  1.Date        : 2012/3/12
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "tcp_upload.h"
#include "img_trans.h"
#include "log.h"

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
 * macros                                       *
 *----------------------------------------------*/

 #define TRANS_TIMEOUT	0 //seconds

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/


/*****************************************************************************
 Prototype    : tcp_upload_create
 Description  : create handle
 Input        : void *params  
                Uint32 size   
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static void *tcp_upload_create(void *params)
{
	ImgTransHandle hTrans;
	ImgTcpUploadParams *uploadParams = (ImgTcpUploadParams *)params;

	if(!params || uploadParams->size != sizeof(ImgTcpUploadParams)) {
		ERR("invalid params size");
		return NULL;
	}

	hTrans = img_trans_create(uploadParams->srvInfo.serverIP,
				uploadParams->srvInfo.serverPort,
				(const char *)(uploadParams->devInfo.macAddr),
				TRANS_TIMEOUT, 0);
	if(!hTrans) {
		ERR("create img trans handle failed");
		return NULL;
	}

	return hTrans;
}

/*****************************************************************************
 Prototype    : tcp_upload_send
 Description  : send one frame
 Input        : void *handle         
                const ImgMsg *frame  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 tcp_upload_send(void *handle, const ImgMsg *frame)
{
	ImgHdrInfo info;

	info.serialNumber = frame->index;
	if(frame->dimension.colorSpace == FMT_BAYER_RGBG)
		info.imageType = IMGTYPE_RAW;
	else
		info.imageType = IMGTYPE_JPEG;
	info.imageWidth = frame->dimension.width;
	info.imageHeight = frame->dimension.height;
	info.imageLen = frame->dimension.size;

	Int32 err;

	err = img_trans_send((ImgTransHandle)handle,
						&info,
						buffer_get_user_addr(frame->hBuf));

	return err;
}

/*****************************************************************************
 Prototype    : tcp_upload_set_params
 Description  : set params
 Input        : void *handle        
                const void *params  
                Int32 size          
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 tcp_upload_set_params(void *handle, const void *params)
{
	ImgTransHandle hTrans = (ImgTransHandle)handle;
	ImgTcpUploadParams *uploadParams = (ImgTcpUploadParams *)params;

	if(!hTrans || !params || uploadParams->size != sizeof(ImgTcpUploadParams))
		return E_INVAL;

	Int32 err;
	
	err = img_trans_set_srv_info(hTrans, uploadParams->srvInfo.serverIP, uploadParams->srvInfo.serverPort);
	return err;
}

/*****************************************************************************
 Prototype    : tcp_upload_delete
 Description  : delete obj
 Input        : void *handle  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 tcp_upload_delete(void *handle)
{
	return img_trans_delete((ImgTransHandle)handle);
}

/*****************************************************************************
 Prototype    : tcp_upload_connect
 Description  : connect server
 Input        : void *handle    
                Uint32 timeout  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 tcp_upload_connect(void *handle, Uint32 timeout)
{
	return img_trans_connect((ImgTransHandle)handle, timeout);
}

/*****************************************************************************
 Prototype    : tcp_upload_disconnect
 Description  : disconnect server
 Input        : void *handle  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 tcp_upload_disconnect(void *handle)
{
	return img_trans_disconnect((ImgTransHandle)handle);
}

/* fxns for tcp upload */
const UploadFxns TCP_UPLOAD_FXNS = {
	.create = tcp_upload_create,
	.delete = tcp_upload_delete,
	.connect = tcp_upload_connect,
	.disconnect = tcp_upload_disconnect,
	.sendFrame = tcp_upload_send,
	.sendHeartBeat = NULL,
	.setParams = tcp_upload_set_params,
	.saveFrame = NULL,
};

