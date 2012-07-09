/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : img_enc_thr.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/8
  Last Modified :
  Description   : img_enc_thr.c header file
  Function List :
  History       :
  1.Date        : 2012/3/8
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __IMG_ENC_THR_H__
#define __IMG_ENC_THR_H__

#include "alg.h"
#include <pthread.h>
#include "app_msg.h"
#include "osd.h"
#include "cam_osd.h"
#include "upload.h"

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

/* handle for this module */
typedef struct EncoderObj *EncoderHandle;

/* operations for encoder */
typedef struct {
	Int32 (*encProcess)(IN AlgHandle hEncode, IN AlgBuf *pInBuf, OUT AlgBuf *pOutBuf, INOUT ImgMsg *msg);
	Int32 (*saveFrame)(IN const ImgMsg *msg, const char *rootPath);
}EncoderOps;

/* create params for processor module */
typedef struct {
	const char			*name;				//object name
	void				*encInitParams;		//init params for encoder
	const AlgFxns		*encFxns;			//encode fxns
	const char			*msgName;			//our msg name
	const char			*dstName;			//default dest msg name
	Uint32				encBufSize;
	Int32				poolBufNum;			//num of buffers in buffer pool, if set to zero, no pool
	const EncoderOps	*encOps;			//opeations for specail process
	const char			*saveRootPath;		//save root path
	pthread_mutex_t		*mutex;
}EncoderAttrs;

/* params for update */
typedef struct {
	CamOsdInfo		osdInfo;		//osd info
	OsdDynParams	osdDyn;			//osd dyn params
	Int8			encDynBuf[128];	//encode dyn params
}EncoderParams;

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*****************************************************************************
 Prototype    : encoder_create
 Description  : create encoder module
 Input        : EncoderAttrs *attrs  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/17
    Author       : Sun
    Modification : Created function

*****************************************************************************/
EncoderHandle encoder_create(EncoderAttrs *attrs, EncoderParams *encParams, UploadParams *uploadParams);

/*****************************************************************************
 Prototype    : encoder_delete
 Description  : delete encoder module
 Input        : EncoderHandle hEnc  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/17
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 encoder_delete(EncoderHandle hEnc, MsgHandle hCurMsg);

/*****************************************************************************
 Prototype    : encoder_run
 Description  : run this module
 Input        : EncoderHandle hEnc  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/20
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 encoder_run(EncoderHandle hEnc);

/*****************************************************************************
 Prototype    : encoder_set_enc_params
 Description  : update params
 Input        : EncoderHandle hEnc  
                MsgHandle hCurMsg   
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/20
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 encoder_set_enc_params(EncoderHandle hEnc,MsgHandle hCurMsg,EncoderParams * params);

/*****************************************************************************
 Prototype    : encoder_set_upload
 Description  : set upload params
 Input        : EncoderHandle hEnc     
                MsgHandle hCurMsg      
                UploadParams * params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/13
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 encoder_set_upload(EncoderHandle hEnc,MsgHandle hCurMsg, UploadParams * params);

/*****************************************************************************
 Prototype    : encoder_upload_ctrl
 Description  : ctrl upload value
 Input        : EncoderHandle hEnc  
                MsgHandle hCurMsg   
                Int32 cmd           
                void *arg           
                Int32 len           
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/7/9
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 encoder_upload_ctrl(EncoderHandle hEnc, MsgHandle hCurMsg, Int32 cmd, void *arg, Int32 len);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __IMG_ENC_THR_H__ */
