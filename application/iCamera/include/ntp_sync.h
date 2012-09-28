/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : ntp_time_sync.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/9/28
  Last Modified :
  Description   : ntp_time_sync.c header file
  Function List :
  History       :
  1.Date        : 2012/9/28
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __NTP_SYNC_H__
#define __NTP_SYNC_H__

#include "common.h"
#include "cam_upload.h"
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

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

typedef struct _NtpSync *NtpSyncHandle;

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*****************************************************************************
 Prototype    : ntp_sync_create
 Description  : create ntp sync module
 Input        : CamNtpServerInfo *info  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/9/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern NtpSyncHandle ntp_sync_create(CamNtpServerInfo *info);

/*****************************************************************************
 Prototype    : ntp_sync_delete
 Description  : delete ntp sync module
 Input        : NtpSyncHandle hNtpSync  
                MsgHandle hCurMsg       
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/9/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 ntp_sync_delete(NtpSyncHandle hNtpSync, MsgHandle hCurMsg);

/*****************************************************************************
 Prototype    : ntp_sync_run
 Description  : start running ntp sync
 Input        : NtpSyncHandle hNtpSync  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/9/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 ntp_sync_run(NtpSyncHandle hNtpSync);

/*****************************************************************************
 Prototype    : ntp_sync_config
 Description  : config this module
 Input        : NtpSyncHandle hNtpSync        
                const CamNtpServerInfo *info  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/9/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 ntp_sync_config(NtpSyncHandle hNtpSync, const CamNtpServerInfo *info, MsgHandle hCurMsg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __NTP_TIME_SYNC_H__ */
