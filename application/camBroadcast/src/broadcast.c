/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : broadcast.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/8/30
  Last Modified :
  Description   : cam info broadcast
  Function List :
              broadcast_info_init
              broadcast_info_update
              main
              main_loop
              usage
  History       :
  1.Date        : 2012/8/30
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "broadcast.h"
#include "icam_ctrl.h"
#include "net_utils.h"
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
#define PROGRAM_NAME		"camBroadcast"
#define CAM_TCP_SRV_PORT	9200
#define BROADCAST_INTERVAL	5	
#define MSG_NAME			"/tmp/"PROGRAM_NAME
#define MSG_TRANS_TIMEOUT	6
#define DEST_PORT			(9800)
#define ERR_CNT_TO_EXIT		5
#define INFO_SND_TIMEOUT	10		// second

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/* params for main_loop */
typedef struct {
	Uint16		destPort;
	Uint16		tcpSrvPort;
	Uint32		interval;
}CamBroadcastParams;

/*****************************************************************************
 Prototype    : broadcast_info_init
 Description  : init broadcast info
 Input        : ICamCtrlHandle hCamCtrl  
                CamBroadcastInfo *info   
                Uint16 tcpSrvPort        
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/30
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 broadcast_info_init(ICamCtrlHandle hCamCtrl, CamBroadcastInfo *info, Uint16 tcpSrvPort)
{
	bzero(info, sizeof(*info));
	
	info->magicNum = CAM_BROADCAST_MAGIC;
	info->bootTime = time(NULL);
	info->tcpSrvPort = tcpSrvPort;

	/* sleep a while so that icam will start */
	sleep(2);

	/* get device info */
	CamDeviceInfo devInfo;
	Int32 err = icam_get_dev_info(hCamCtrl, &devInfo);
	if(err)
		return err;
	info->devSN = devInfo.deviceSN;
	strncpy(info->description, devInfo.name, sizeof(info->description));
	strncpy(info->location, devInfo.location, sizeof(info->location));
	
	return E_NO;
	
}

/*****************************************************************************
 Prototype    : broadcast_info_update
 Description  : update broadcast info
 Input        : ICamCtrlHandle hCamCtrl  
                CamBroadcastInfo *info   
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/30
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 broadcast_info_update(ICamCtrlHandle hCamCtrl, CamBroadcastInfo *info)
{
	CamVersionInfo versionInfo;
	Int32 err = icam_get_version(hCamCtrl, &versionInfo);
	if(err) {
		ERR("get version failed...");
		return err;
	}
	info->version = versionInfo.armVer;
	info->runTime = time(NULL) - info->bootTime;
	return E_NO;
}

/*****************************************************************************
 Prototype    : main_loop
 Description  : mian loop of this program
 Input        : CamBroadcastParams *params  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/30
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 main_loop(CamBroadcastParams *params)
{
	int 	sock = -1;
	int 	ret;	
	Bool	reset = FALSE;
	ICamCtrlHandle hCamCtrl;

	/* create icam ctrl object for commu */
	hCamCtrl = icam_ctrl_create(MSG_NAME, 0, MSG_TRANS_TIMEOUT);
	if(!hCamCtrl) {
		ERR("create cam ctrl handle failed!");
		ret = E_INVAL;
		goto exit;
	}

	/* create broadcast socket */
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0) {
		ERRSTR("socket");
		ret = E_IO;
		goto exit;
	}

	/* set opt for broadcast */
	int opt = 1;
	ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));
	if(ret < 0) {
		ERRSTR("set sock as broadcast failed");
		ret = E_IO;
		goto exit;
	}

	/* set send timeout */
	set_sock_send_timeout(sock, INFO_SND_TIMEOUT);

	/* set dest addr & port */
	struct sockaddr_in addr; 
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET; 
	addr.sin_addr.s_addr = htonl(INADDR_BROADCAST); 
	addr.sin_port = htons(params->destPort);

	/* init broadcast info */
	CamBroadcastInfo info;
	ret = broadcast_info_init(hCamCtrl, &info, params->tcpSrvPort);
	if(ret) {
		ERR("init broadcast info failed!");
		goto exit;
	}

	/* send upd broadcast every a while */
	Int32 	errCnt = 0;
	while(1) {
		/* update broadcast info */
		ret = broadcast_info_update(hCamCtrl, &info);
		if(ret < 0) {
			if(++errCnt > ERR_CNT_TO_EXIT) {
				WARN("update info err too many times, reboot system");
				reset = TRUE;
				break;
			}
		} else {
			errCnt = 0;
		}

		/* send broadcast info */
		ret = sendto(sock, &info, sizeof(info), 0, (struct sockaddr *)&addr, sizeof(addr)); 
		if(ret < 0)
			DBG("send broadcast failed");

		/* sleep a while */
		sleep(params->interval);
	}

exit:
	if(hCamCtrl)
		icam_ctrl_delete(hCamCtrl);
	
	if(sock > 0)
		close(sock);

	if(reset)
		system("shutdown -r now\n");
	
	return ret;
}

/*****************************************************************************
 Prototype    : usage
 Description  : usage of this program
 Input        : void  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/30
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static void usage(void)
{
    INFO("%s Compiled on %s %s with gcc %s", PROGRAM_NAME, __DATE__, __TIME__, __VERSION__);
    INFO("Usage:");
    INFO("./%s [options]", PROGRAM_NAME);
    INFO("Options:");
    INFO("\t-h get help");
	INFO("\t-p dest port, default: %u", DEST_PORT);
	INFO("\t-l cam tcp server listen port, default: %d", CAM_TCP_SRV_PORT);
	INFO("\t-t broadcast interval, unit: secod, default: %d", BROADCAST_INTERVAL);
    INFO("\tuse default params: ./%s", PROGRAM_NAME);
    INFO("\tuse specific params: ./%s -p 5400", PROGRAM_NAME);
}

/*****************************************************************************
 Prototype    : main
 Description  : Main function
 Input        : Int32 argc   
                char **argv  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/5
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 main(Int32 argc, char **argv)
{
	Int32 c;
    const char *options = "p:t:l:h";
	CamBroadcastParams params;
	Int32 ret;

	/* Init params */
	bzero(&params, sizeof(params));
	
	params.tcpSrvPort = CAM_TCP_SRV_PORT;
	params.interval = BROADCAST_INTERVAL;
	params.destPort = DEST_PORT;

	while ((c = getopt(argc, argv, options)) != -1) {
		switch (c) {
		case 'p':
			params.destPort = atoi(optarg);
			break;
		case 'l':
			params.tcpSrvPort = atoi(optarg);
			break;
		case 't':
			params.interval = atoi(optarg);
			break;
		case 'h':
		default:
			usage();
			return -1;
		}
	}

	ret = main_loop(&params);

	INFO("%s exit, status: %d...\n", PROGRAM_NAME, ret);

	exit(0);
}


