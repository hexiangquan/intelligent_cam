/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : upload.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/10
  Last Modified :
  Description   : upload.c header file
  Function List :
  History       :
  1.Date        : 2012/3/10
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __UPLOAD_H__
#define __UPLOAD_H__

#include "common.h"
#include "app_msg.h"
#include "cam_upload.h"

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
#define UPLOAD_FLAG_ANSYNC		(1 << 0)	//using ansync upload 
#define UPLOAD_FLAG_FREE_BUF	(1 << 1)	//free buffer after send
#define UPLOAD_FLAG_NOT_SAVE	(1 << 2)	//do not save if err occurs
#define UPLOAD_FLAG_WAIT_INFO	(1 << 3)	//wait capture info from co-processor

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

typedef struct UploadObj *UploadHandle;

typedef struct _UploadFxns {
	/* create function, returns sub object handle  */
	void *(*create)(void *params);

	/* Connect server, 1st: sub object handle, 2nd: timeout, ms */
	Int32  (*connect)(void *handle, Uint32 timeoutMs);

	/* Disconnect server, 1st: sub object handle */
	Int32  (*disconnect)(void *handle);

	/* Send a frame,, 1st: sub object handle, 2nd: frame data, 3rd: data len */
	Int32  (*sendFrame)(void *handle, const ImgMsg *frame);

	/* Save a frame,, 1st: sub object handle, 2nd: frame data, 3rd: data len, when send fails */
	Int32  (*saveFrame)(void *handle, const ImgMsg *frame);

	/* Send heart beat, , 1st: sub object handle, can be NULL */
	Int32  (*sendHeartBeat)(void *handle);
	
	/* Set params, , 1st: sub object handle, 2nd: New Init params */	
	Int32  (*setParams)(void *handle, const void *params);

	/* Flush cache buffer frames , all send to net */
	Int32  (*disableCache)(void *handle);

	/* Ctrl cmds, defined by lower level */
	Int32  (*ctrl)(void *handle, Int32 cmd, void *arg);

	/* Delete param, 1st: sub object handle*/
	Int32  (*delete)(void *handle);
}UploadFxns;

/* attrs for create */
typedef struct {
	Uint32					reConTimeout;	//reconnect timeout in second	
	Int32					flags;			//ctrl flags
	const char				*msgName;		//msg name for ansyn thread
	const char				*savePath;		//path for save frame when err happens
}UploadAttrs;

/* params can be set at run time */
typedef struct {
	CamImgUploadProto 		protol;			//protocol to send
	Int8					paramsBuf[512]; //low level params for TCP, FTP etc. transfer
}UploadParams;

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

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
extern UploadHandle upload_create(UploadAttrs *attrs, UploadParams *params);

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
extern Int32 upload_delete(UploadHandle hUpload, MsgHandle hCurMsg);

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
extern Int32 upload_connect(UploadHandle hUpload, Uint32 timeoutMs);

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
extern Int32 upload_disconnect(UploadHandle hUpload);

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
extern Bool upload_get_connect_status(UploadHandle hUpload);

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
extern Int32 upload_send_heartbeat(UploadHandle hUpload);

/*****************************************************************************
 Prototype    : upload_update_params
 Description  : update params
 Input        : UploadHandle hUpload  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/21
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 upload_update_params(UploadHandle hUpload, UploadParams *params);

/*****************************************************************************
 Prototype    : upload_send_frame
 Description  : send one frame
 Input        : UploadHandle hUpload  
                const ImgMsg *data    
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 upload_run(UploadHandle hUpload, MsgHandle hCurMsg, const ImgMsg *data);

/*****************************************************************************
 Prototype    : upload_control
 Description  : upload module ctrl
 Input        : UploadHandle hUpload  
                Int32 cmd             
                void *arg             
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/7/3
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 upload_control(UploadHandle hUpload, Int32 cmd, void *arg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __UPLOAD_H__ */
