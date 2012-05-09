/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : local_upload.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/5/4
  Last Modified :
  Description   : local_upload.c header file
  Function List :
  History       :
  1.Date        : 2012/5/4
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __LOCAL_UPLOAD_H__
#define __LOCAL_UPLOAD_H__

#include "common.h"
#include "upload.h"
#include "msg.h"

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
#define LOCAL_FLAG_AUTO_UPLOAD_EN	(1 << 0)	//enable automatic upload
#define LOCAL_FLAG_DEL_AFTER_SND	(1 << 1)	//delete file after send 
/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/* Handle for this obj */
typedef struct LocalUploadObj *LocalUploadHandle;

typedef struct {
	const char 		*filePath;		//path of local file to upload
	const char		*msgName;		//msg name for IPC
	Int32			flags;			//flags for bit ctrl
	Int32			maxFileSize;	//size for single file
}LocalUploadAttrs;

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*****************************************************************************
 Prototype    : local_upload_create
 Description  : create this module
 Input        : LocalUploadAttrs *attrs  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/5/4
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern LocalUploadHandle local_upload_create(LocalUploadAttrs *attrs, UploadParams *params);

/*****************************************************************************
 Prototype    : local_upload_delete
 Description  : delete this module
 Input        : LocalUploadHandle hLocalUpload  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/5/4
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 local_upload_delete(LocalUploadHandle hLocalUpload, MsgHandle hCurMsg);

/*****************************************************************************
 Prototype    : local_upload_run
 Description  : start running this module
 Input        : LocalUploadHandle hLocalUpload  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/5/4
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 local_upload_run(LocalUploadHandle hLocalUpload);

/*****************************************************************************
 Prototype    : local_upload_cfg
 Description  : cfg upload params
 Input        : LocalUploadHandle hLocalUpload  
                UploadParams *params            
                MsgHandle hCurMsg               
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/5/7
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 local_upload_cfg(LocalUploadHandle hLocalUpload, UploadParams *params, Int32 flags, MsgHandle hCurMsg);

/*****************************************************************************
 Prototype    : local_upload_send
 Description  : ansync send file or dir to server
 Input        : LocalUploadHandle hLocalUpload  
                Bool isDir                      
                const char *pathName            
                MsgHandle hCurMsg               
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/5/7
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 local_upload_send(LocalUploadHandle hLocalUpload, Bool isDir, const char *pathName, MsgHandle hCurMsg);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __LOCAL_UPLOAD_H__ */
