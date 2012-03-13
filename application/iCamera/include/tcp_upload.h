/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : tcp_upload.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/12
  Last Modified :
  Description   : image upload using tcp protol
  Function List :
  History       :
  1.Date        : 2012/3/12
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __TCP_UPLOAD_H__
#define __TCP_UPLOAD_H__
	
#include "upload.h"
#include "cam_params.h"

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
/* module params for create and config */
typedef struct {
	Int32					size;		//size of this struct
	CamTcpImageServerInfo 	srvInfo;
	CamDeviceInfo			devInfo;
}ImgTcpUploadParams;

/* fxns for this module */
const UploadFxns TCP_UPLOAD_FXNS ;

#endif

