/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : main.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/5
  Last Modified :
  Description   : main 
  Function List :
              main
              main_loop
              msg_process
              threads_create
              threads_delete
              usage
  History       :
  1.Date        : 2012/3/5
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "common.h"
#include "log.h"
#include "alg.h"
#include <pthread.h>
#include "cam_time.h"
#include "app_msg.h"
#include "params_mng.h"
#include "buffer.h"
#include "capture_thr.h"
#include "img_enc_thr.h"
#include "signal.h"

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
#define PROGRAM_NAME		"iCamera"
#define CONFIG_FILE			"./cam.cfg"
#define FILE_SAVE_PATH		"/media/mmc"
	
#define CAM_MAX_THREAD_NUM	8

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/* Params used in this module */
typedef struct {
	const char *cfgFileName;
	const char *savePath;
	Bool		exit;
	Bool		reboot;
	pthread_t	pid[CAM_MAX_THREAD_NUM];
	ParamsMngHandle hParamsMng;
	FrameDispHandle	hDispatch;
}MainEnv;

static Bool s_exit = FALSE;

/*****************************************************************************
 Prototype    : threads_create
 Description  : create threads 
 Input        : SetupParams *params  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/5
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 threads_create(MainEnv *envp)
{
	Int32 		err;

	/* create capture thread */
	CapThrArg 	*capArg = malloc(sizeof(CapThrArg));
	assert(capArg);

	capArg->hParamsMng = envp->hParamsMng;
	capArg->hDispatch = envp->hDispatch;
	err = pthread_create(&envp->pid[0], NULL, capture_thr, capArg);
	if(err < 0) {
		free(capArg);
		ERRSTR("create capture thread failed...");
		return E_NOMEM;
	}

	/* create image encode thread */
	ImgEncThrArg *imgEncArg = malloc(sizeof(ImgEncThrArg));
	assert(imgEncArg);

	imgEncArg->hDispatch = envp->hDispatch;
	imgEncArg->hParamsMng = envp->hParamsMng;
	err = pthread_create(&envp->pid[1], NULL, img_enc_thr, imgEncArg);
	if(err < 0) {
		free(imgEncArg);
		ERRSTR("create img enc thread failed...");
		return E_NOMEM;
	}
	
	return E_NO;
}

/*****************************************************************************
 Prototype    : threads_delete
 Description  : delete threads 
 Input        : SetupParams *params  
                MsgHandle hMsg       
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/5
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static void threads_delete(MainEnv *envp, MsgHandle hMsg)
{
	Int32 i = 0;
	MsgHeader msg;

	DBG("delete threads...");

	/* send msg to all tasks to exit */
	msg.cmd = APPCMD_EXIT;
	msg.index = 0;
	msg.dataLen = 0;
	msg.magicNum = MSG_MAGIC_SEND;
	msg_send(hMsg, MSG_CAP, &msg, sizeof(msg));
	msg_send(hMsg, MSG_IMG_ENC, &msg, sizeof(msg));

	/* wait all threads exit */
	while(envp->pid[i]) {
		pthread_join(envp->pid[i], NULL);
		i++;
	}
}

/*****************************************************************************
 Prototype    : msg_process
 Description  : process msg recieved
 Input        : CommonMsg *msg       
                SetupParams *params  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/5
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 msg_process(CommonMsg *msg, MainEnv *envp) 
{
	Int32 err = E_NO;
	
	switch(msg->header.cmd) {
	case APPCMD_EXIT:
		envp->exit = TRUE;
		break;
	case APPCMD_REBOOT:
		envp->exit = TRUE;
		envp->reboot = TRUE;
		break;
	default:
		break;
	}

	return err;
}

/*****************************************************************************
 Prototype    : sig_handler
 Description  : Signal handler
 Input        : int sig  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/9
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static void sig_handler(int sig)
{
	if(sig == SIGINT || sig == SIGABRT) {
		DBG("got INT signal");
		s_exit = TRUE;
	}
} 

/*****************************************************************************
 Prototype    : main_loop
 Description  : main loop
 Input        : SetupParams *params  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/5
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 main_loop(MainEnv *envp)
{
	Int32 			status;
	MsgHandle 		hMsg = NULL;
	CommonMsg		msgBuf;
	CamDateTime		curTime;

	/* init modules */
	status = alg_init();
	status |= buffer_init();

	if(status) {
		ERR("module init failed!");
		goto exit;
	}

	/* create msg */
	hMsg = msg_create(MSG_MAIN, NULL, MSG_FLAG_NONBLK);
	if(!hMsg) {
		ERR("create msg failed");
		goto exit;
	}

	/* read params */
	envp->hParamsMng = params_mng_create(envp->cfgFileName);
	if(!envp->hParamsMng) {
		ERR("create params failed.");
		goto exit;
	}

	/* create frame dispatch module */
	FrameDispInfo info;
	info.hParamsMng = envp->hParamsMng;
	info.savePath = envp->savePath;
	envp->hDispatch = frame_disp_create(FT_SRV_UNCONNECTED, FRAME_ENC_ON, &info);

	/* catch signals */
	DBG("enable signal");
	signal(SIGINT, sig_handler);

	/* create threads */
	status = threads_create(envp);

	/* start main loop */
	while(!s_exit && !envp->exit) {
		/* feed dog */

		/* recv msg */
		status = msg_recv(hMsg, &msgBuf, sizeof(msgBuf));
		if(!status) {
			msg_process(&msgBuf, envp);
		}
			
		/* get current time */
    	status = cam_get_time(&curTime);
		if(!status) {
			/* check if it is time to reboot */
		}
		
		/* wait a while */
		sleep(1);
	}

	/* delete threads */
	threads_delete(envp, hMsg);

	if(envp->reboot) {
		INFO("we are going to reboot system");
		system("shutdown -r now");
	}
	
exit:

	if(hMsg)
		msg_delete(hMsg);

	if(envp->hParamsMng)
		params_mng_delete(envp->hParamsMng);

	if(envp->hDispatch)
		frame_disp_delete(envp->hDispatch);

	buffer_exit();
	alg_exit();

	return status;
	
}

/*****************************************************************************
 Prototype    : usage
 Description  : Help info
 Input        : void  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/5
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static void usage(void)
{
    INFO("%s Compiled on %s %s with gcc %s", PROGRAM_NAME, __DATE__, __TIME__, __VERSION__);
    INFO("Usage:");
    INFO("./%s [options]", PROGRAM_NAME);
    INFO("Options:");
    INFO(" -h get help");
	INFO(" -f input file name, default: %s", CONFIG_FILE);
	INFO(" -p local file save path, default: %s", FILE_SAVE_PATH);
    INFO(" use default params: ./%s", PROGRAM_NAME);
    INFO(" use specific params: ./%s -f ./cfg/myCfg", PROGRAM_NAME);
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
    const char *options = "f:p:h";
	MainEnv env;
	int ret;

	/* Init params */
	memset(&env, 0, sizeof(env));
	
	env.cfgFileName = CONFIG_FILE;
	env.savePath = FILE_SAVE_PATH;
	env.exit = FALSE;
	env.reboot = FALSE;

	while ((c = getopt(argc, argv, options)) != -1) {
		switch (c) {
		case 'f':
			env.cfgFileName = optarg;
			break;
		case 'p':
			env.savePath = optarg;
			break;
		case 'h':
		default:
			usage();
			return -1;
		}
	}

	ret = main_loop(&env);

	INFO("%s exit, status: %d...\n", PROGRAM_NAME, ret);

	exit(0);
}

