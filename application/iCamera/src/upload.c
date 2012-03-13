/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : upload.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/10
  Last Modified :
  Description   : abstrac level for frame upload
  Function List :
              upload_connect
              upload_create
              upload_delete
              upload_disconnect
              upload_get_connect_status
              upload_send_frame
              upload_send_heartbeat
              upload_set_params
  History       :
  1.Date        : 2012/3/10
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "upload.h"
#include "log.h"
#include "tcp_upload.h"
#include "ftp_upload.h"

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

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/
/* structure for this module */
struct UploadObj {
	const UploadFxns	*fxns;
	Bool				isConnected;
	void				*handle;
	Uint32				connectTimeout;
};


/*****************************************************************************
 Prototype    : upload_create
 Description  : create upload module
 Input        : const UploadFxns *fxns  
                void *params            
                Uint32 size             
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
UploadHandle upload_create(CamImageUploadProtocol protol, void *params, Uint32 reConTimeout)
{
	UploadHandle hUpload;

	hUpload = calloc(1, sizeof(struct UploadObj));
	if(!hUpload) {
		ERR("alloc memory failed.");
		return NULL;
	}

	hUpload->isConnected = FALSE;
	hUpload->connectTimeout = reConTimeout;

	switch(protol) {
	case CAM_UPLOAD_PROTO_TCP:
		hUpload->fxns = &TCP_UPLOAD_FXNS;
		break;
	case CAM_UPLOAD_PROTO_FTP:
		hUpload->fxns = &FTP_UPLOAD_FXNS;
		break;
	case CAM_UPLOAD_PROTO_NONE:
	default:
		hUpload->fxns = NULL;
		break;
	}

	if(hUpload->fxns && hUpload->fxns->create) {
		hUpload->handle = hUpload->fxns->create(params);
		if(!hUpload->handle) {
			ERR("create sub object failed.");
			goto err_quit;
		}
	}

	return hUpload;

err_quit:
	
	free(hUpload);
	return NULL;
	
}

/*****************************************************************************
 Prototype    : upload_delete
 Description  : delete upload module
 Input        : UploadHandle hUpload  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 upload_delete(UploadHandle hUpload)
{
	if(hUpload == NULL)
		return E_INVAL;

	if(hUpload->isConnected)
		upload_disconnect(hUpload);

	if(hUpload->fxns && hUpload->fxns->delete)
		hUpload->fxns->delete(hUpload->handle);
	
	free(hUpload);

	return E_NO;
}

/*****************************************************************************
 Prototype    : upload_connect
 Description  : connect server
 Input        : UploadHandle hUpload  
                Uint32 timeoutMs      
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 upload_connect(UploadHandle hUpload, Uint32 timeoutMs)
{
	Int32 err;
	
	if(!hUpload)
		return E_INVAL;

	if(!hUpload->fxns || !hUpload->fxns->connect) {
		hUpload->isConnected = FALSE;
		return E_UNSUPT;
	}

	if(hUpload->isConnected)
		upload_disconnect(hUpload);
		
	if((err = hUpload->fxns->connect(hUpload->handle, timeoutMs)))
		return err;

	hUpload->isConnected = TRUE;

	return E_NO;
}

/*****************************************************************************
 Prototype    : upload_disconnect
 Description  : disconnect server
 Input        : UploadHandle hUpload  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 upload_disconnect(UploadHandle hUpload)
{
	Int32 err;
	
	if(!hUpload)
		return E_INVAL;

	if(!hUpload->fxns || !hUpload->fxns->disconnect)
		return E_NO;

	if(hUpload->isConnected &&
		(err = hUpload->fxns->disconnect(hUpload->handle)) != E_NO)
		return err;

	hUpload->isConnected = FALSE;
	return E_NO;
}

/*****************************************************************************
 Prototype    : upload_get_connect_status
 Description  : get if the server is connected
 Input        : UploadHandle hUpload  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Bool upload_get_connect_status(UploadHandle hUpload)
{
	if(!hUpload)
		return FALSE;
	return hUpload->isConnected;
}

/*****************************************************************************
 Prototype    : upload_send_frame
 Description  : send one frame
 Input        : UploadHandle hUpload  
                Ptr pFrameBuf         
                Uint32 unLen          
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 upload_send_frame(UploadHandle hUpload, const ImgMsg *data)
{
	if(!hUpload)
		return E_INVAL;

	if(!hUpload->fxns || !hUpload->fxns->sendFrame)
		return E_TRANS;

	if(!hUpload->isConnected) {
		Int32 err = upload_connect(hUpload, hUpload->connectTimeout);
		if(err) {
			ERR("not connected server");
			return err;
		}
	}

	return hUpload->fxns->sendFrame(hUpload->handle, data);
}


/*****************************************************************************
 Prototype    : upload_send_heartbeat
 Description  : keep connect with server
 Input        : UploadHandle hUpload  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 upload_send_heartbeat(UploadHandle hUpload)
{
	if(!hUpload)
		return E_INVAL;

	if(!hUpload->fxns || !hUpload->fxns->sendHeartBeat)
		return E_NO;

	if(!hUpload->isConnected) {
		ERR("server not connected ");
		return E_CONNECT;
	}

	return hUpload->fxns->sendHeartBeat(hUpload->handle);
}

/*****************************************************************************
 Prototype    : upload_set_params
 Description  : update params
 Input        : UploadHandle hUpload  
                Ptr params            
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 upload_set_params(UploadHandle hUpload, Ptr params)
{
	if(!hUpload)
		return E_INVAL;

	if(!hUpload->fxns || !hUpload->fxns->setParams)
		return E_NO;

	if(hUpload->isConnected)
		upload_disconnect(hUpload);

	return hUpload->fxns->setParams(hUpload->handle, params);
}


