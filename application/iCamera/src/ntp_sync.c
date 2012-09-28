/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : ntp_sync.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/9/28
  Last Modified :
  Description   : ntp time sync module
  Function List :
              ntp_sync_config
              ntp_sync_create
              ntp_sync_delete
              ntp_sync_run
              ntp_sync_thread
              ntp_sync_time
  History       :
  1.Date        : 2012/9/28
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "ntp_sync.h"
#include "ntp.h"
#include "cam_time.h"
#include "log.h"
#include <pthread.h>
#include "app_msg.h"

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
#define MSG_NTP				"/tmp/msgNtpSync"
#define NTP_SYNC_MIN_PRD	1

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/* Object of this module */
struct _NtpSync {
	NtpHandle 		hNtp;
	pthread_mutex_t mutex;
	Uint16			syncPrd;
	pthread_t		pid;
	MsgHandle		hMsg;
	Bool			exit;
};

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
NtpSyncHandle ntp_sync_create(CamNtpServerInfo *info)
{
	if(!info)
		return NULL;

	NtpSyncHandle hNtpSync = calloc(1, sizeof(struct _NtpSync));
	if(!hNtpSync) {
		ERR("alloc mem failed!");
		return NULL;
	}

	hNtpSync->hNtp = ntp_create(info->serverIP, info->serverPort);
	if(!hNtpSync->hNtp) {
		ERR("create ntp handle failed!");
		goto exit;
	}

	hNtpSync->hMsg = msg_create(MSG_NTP, NULL, 0);
	if(!hNtpSync->hMsg)
		goto exit;

	if(pthread_mutex_init(&hNtpSync->mutex, NULL) < 0)
		goto exit;

	hNtpSync->syncPrd = MAX(info->syncPrd, NTP_SYNC_MIN_PRD);
	hNtpSync->exit = FALSE;

	return hNtpSync;
	
exit:
	if(hNtpSync->hNtp)
		ntp_delete(hNtpSync->hNtp);
	free(hNtpSync);
	return NULL;
}

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
Int32 ntp_sync_delete(NtpSyncHandle hNtpSync, MsgHandle hCurMsg)
{	
	if(!hNtpSync)
		return E_INVAL;

	hNtpSync->exit = TRUE;
	if(hNtpSync->pid) {
		/* tell thead to exit */
		app_hdr_msg_send(hCurMsg, MSG_NTP, APPCMD_EXIT, 0, 0);
		pthread_join(hNtpSync->pid, NULL);
	}
	
	ntp_delete(hNtpSync->hNtp);
	msg_delete(hNtpSync->hMsg);
	
	pthread_mutex_destroy(&hNtpSync->mutex);
	free(hNtpSync);

	return E_NO;
}

/*****************************************************************************
 Prototype    : ntp_sync_time
 Description  : get time from server and sync with system
 Input        : NtpSyncHandle hNtpSync  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/9/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 ntp_sync_time(NtpSyncHandle hNtpSync)
{
	Int32		err;
	DateTime 	dateTime;

	pthread_mutex_lock(&hNtpSync->mutex);
	err = ntp_get_time(hNtpSync->hNtp, &dateTime);
	pthread_mutex_unlock(&hNtpSync->mutex);
	
	if(err)
		return err;

	/* sync time with system */
	err = cam_set_time(&dateTime);
	if(!err)
		INFO("ntp sync time success...");

	return err;
}

/*****************************************************************************
 Prototype    : ntp_sync_thread
 Description  : thread for period running
 Input        : void *arg  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/9/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static void *ntp_sync_thread(void *arg)
{
	assert(arg);

	NtpSyncHandle hNtpSync = (NtpSyncHandle)arg;
	Int32 ret;

	INFO("ntp sync thread start...");

	while(!hNtpSync->exit) {

		/* sync time */
		ntp_sync_time(hNtpSync);

		/* set recv timeout as sync period */
		Uint32 recvTimeout = HOUR_TO_SEC(hNtpSync->syncPrd, 0, 0);
		MsgHeader msg;
		
		msg_set_recv_timeout(hNtpSync->hMsg, recvTimeout);

		ret = msg_recv(hNtpSync->hMsg, &msg, sizeof(msg), 0);
		if(ret >= 0) {
			/* process msg */
			switch(msg.cmd) {
				case APPCMD_EXIT:
					hNtpSync->exit = TRUE;
					break;
				case APPCMD_NTP_SYNC:
					break;
				default:
					break;
			}
		}
	}

	INFO("ntp thread exit...");
	pthread_exit(0);
}

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
Int32 ntp_sync_run(NtpSyncHandle hNtpSync)
{
	if(!hNtpSync || !hNtpSync->hNtp)
		return E_INVAL;

	if(hNtpSync->pid) {
		ERR("ntp sync has already been running.");
		return E_MODE;
	}

	int err = pthread_create(&hNtpSync->pid, NULL, ntp_sync_thread, hNtpSync);
	if(err < 0) {
		ERRSTR("create thread failed!");
		return E_IO;
	}

	return E_NO;
}

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
Int32 ntp_sync_config(NtpSyncHandle hNtpSync, const CamNtpServerInfo *info, MsgHandle hCurMsg)
{
	if(!hNtpSync || !info)
		return E_INVAL;

	pthread_mutex_lock(&hNtpSync->mutex);
	Int32 err = ntp_set_server_info(hNtpSync->hNtp, info->serverIP, info->serverPort);
	pthread_mutex_unlock(&hNtpSync->mutex);

	hNtpSync->syncPrd = MAX(info->syncPrd, NTP_SYNC_MIN_PRD);

	err = app_hdr_msg_send(hCurMsg, MSG_NTP, APPCMD_NTP_SYNC, 0, 0);
	return err;
}

