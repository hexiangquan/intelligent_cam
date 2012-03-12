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
UploadHandle upload_create(const UploadFxns *fxns, void *params, Uint32 size)
{
	UploadHandle hUpload;

	if(!fxns || !fxns->create) {
		ERR("fxns must set");
		return NULL;
	}
	
	hUpload = calloc(1, sizeof(struct UploadObj));
	if(!hUpload) {
		ERR("alloc memory failed.");
		return NULL;
	}

	hUpload->isConnected = FALSE;
	hUpload->fxns = fxns;

	hUpload->handle = fxns->create(params, size);
	if(!hUpload->handle) {
		ERR("create sub object failed.");
		goto err_quit;
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

	if(hUpload->fxns->delete)
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

	if(!hUpload->fxns->connect)
		return E_CONNECT;

	if(!hUpload->isConnected)
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

	if(!hUpload->fxns->disconnect)
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

	if(!hUpload->fxns->sendFrame)
		return E_TRANS;

	if(!hUpload->isConnected) {
		ERR("not connected server");
		return E_CONNECT;
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

	if(!hUpload->fxns->sendHeartBeat)
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
Int32 upload_set_params(UploadHandle hUpload, Ptr params, Int32 size)
{
	if(!hUpload)
		return E_INVAL;

	if(!hUpload->fxns->setParams)
		return E_NO;

	if(hUpload->isConnected)
		upload_disconnect(hUpload);

	return hUpload->fxns->setParams(hUpload->handle, params, size);
}


