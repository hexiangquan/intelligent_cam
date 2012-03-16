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
#include "data_collector.h"
#include "img_enc_thr.h"
#include <signal.h>
#include "vid_enc_thr.h"
#include "img_snd_thr.h"
#include "ctrl_thr.h"
#include "img_convert.h"

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
#define CAP_DEVICE			"/dev/video0"
#define CAP_BUF_NUM			4

	
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
	CapHandle		hCap;
	AlgHandle		hImgConv;
	CollectorHandle	hCollector;
}MainEnv;

static Bool s_exit = FALSE;

/*****************************************************************************
 Prototype    : module_init
 Description  : init modules
 Input        : MainEnv *envp  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/16
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 app_init(MainEnv *envp)
{
	Int32 			ret;

	/* init low level modules */
	ret = alg_init();
	ret |= buffer_init();

	if(ret) {
		ERR("module init failed!");
		return ret;
	}

	/* read params */
	envp->hParamsMng = params_mng_create(envp->cfgFileName);
	if(!envp->hParamsMng) {
		ERR("create params failed.");
		return E_IO;
	}
	
	/* get current work mode  & img enhance params */
	CapAttrs		capAttrs;
	CapInputInfo	inputInfo;

	/* fill attrs for capture create */
	capAttrs.devName = CAP_DEVICE;
	capAttrs.inputType = CAP_INPUT_CAMERA;
	capAttrs.std = CAP_STD_FULL_FRAME;
	capAttrs.mode = CAM_CAP_MODE_CONTINUE;
	capAttrs.userAlloc = TRUE;
	capAttrs.bufNum = CAP_BUF_NUM;
	capAttrs.defRefCnt = 1;

	/* create capture object */
	envp->hCap = capture_create(&capAttrs);
	if(!envp->hCap) {
		ERR("create capture handle failed...");
		return E_IO;
	}

	capture_get_input_info(envp->hCap, &inputInfo);

	/* set capture info to params manager */
	ret = params_mng_control(envp->hParamsMng, PMCMD_S_CAPINFO, &inputInfo, sizeof(CapInputInfo));
	assert(ret == E_NO);

	/* create img conv module */
	ImgConvInitParams convInitParams;
	ImgConvDynParams convDynParams;
	
	convInitParams.prevDevName = NULL;
	convInitParams.rszDevName = NULL;
	convInitParams.size = sizeof(convInitParams);

	/* get img convert dynamic params */
	ret = params_mng_control(envp->hParamsMng, PMCMD_G_IMGCONVDYN, &convDynParams, sizeof(convDynParams));
	if(ret)
		return ret;

	/* create image convert object */
	envp->hImgConv = img_conv_create(&convInitParams, &convDynParams);
	if(!envp->hImgConv) {
		ERR("create img convert handle failed...");
		return E_IO;
	}

	/* create data capture module (App module) */
	CollectorAttrs collectorAttrs;

	collectorAttrs.hParamsMng = envp->hParamsMng;
	collectorAttrs.hCap = envp->hCap;
	envp->hCollector = data_collector_create(&collectorAttrs);
	if(!envp->hCollector) {
		ERR("create data capture failed");
		return E_INVAL;
	}

	
	/* create frame dispatch module */
	FrameDispInfo info;
	info.hParamsMng = envp->hParamsMng;
	info.savePath = envp->savePath;
	envp->hDispatch = frame_disp_create(FT_SRV_UNCONNECTED, FRAME_ENC_ON, &info);

	return E_NO;
}

/*****************************************************************************
 Prototype    : start_running
 Description  : start running system
 Input        : MainEnv *envp  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/16
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 app_run(MainEnv *envp)
{
	Int32 		err;

	/* start data capture */
	err = data_collector_run(envp->hCollector);
	if(err ) {
		ERR("data collector run failed...");
		return err;
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

	/* create video encode thread */
	VidEncThrArg *vidEncArg = malloc(sizeof(VidEncThrArg));
	assert(vidEncArg);

	vidEncArg->hDispatch = envp->hDispatch;
	vidEncArg->hParamsMng = envp->hParamsMng;
	err = pthread_create(&envp->pid[2], NULL, vid_enc_thr, vidEncArg);
	if(err < 0) {
		free(vidEncArg);
		ERRSTR("create vid enc thread failed...");
		return E_NOMEM;
	}

#if 1
	/* create image send thread */
	ImgSndThrArg *imgSndArg = malloc(sizeof(ImgSndThrArg));
	assert(imgSndArg);

	imgSndArg->hDispatch = envp->hDispatch;
	imgSndArg->hParamsMng = envp->hParamsMng;
	err = pthread_create(&envp->pid[3], NULL, img_snd_thr, imgSndArg);
	if(err < 0) {
		free(imgSndArg);
		ERRSTR("create img snd thread failed...");
		return E_NOMEM;
	}
#endif

	/* create ctrl thread */
	CtrlThrArg *ctrlThrArg = malloc(sizeof(CtrlThrArg));
	assert(ctrlThrArg);

	ctrlThrArg->hDispatch = envp->hDispatch;
	ctrlThrArg->hParamsMng = envp->hParamsMng;
	err = pthread_create(&envp->pid[4], NULL, ctrl_thr, ctrlThrArg);
	if(err < 0) {
		free(ctrlThrArg);
		ERRSTR("create ctrl thread failed...");
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
static void app_exit(MainEnv *envp, MsgHandle hMsg)
{
	//Int32 i = 0;
	MsgHeader msg;

	DBG("delete threads...");

	/* send msg to all tasks to exit */
	msg.cmd = APPCMD_EXIT;
	msg.index = 0;
	msg.dataLen = 0;
	msg.magicNum = MSG_MAGIC_SEND;
	msg_send(hMsg, MSG_IMG_ENC, &msg, sizeof(msg));
	pthread_join(envp->pid[1], NULL);
	msg_send(hMsg, MSG_VID_ENC, &msg, sizeof(msg));
	pthread_join(envp->pid[2], NULL);
	msg_send(hMsg, MSG_IMG_TX, &msg, sizeof(msg));
	pthread_join(envp->pid[3], NULL);
	msg_send(hMsg, MSG_CTRL, &msg, sizeof(msg));
	pthread_join(envp->pid[4], NULL);

	if(envp->hCollector)
		data_collector_delete(envp->hCollector, hMsg);

	if(hMsg)
		msg_delete(hMsg);

	if(envp->hParamsMng)
		params_mng_delete(envp->hParamsMng);

	if(envp->hDispatch)
		frame_disp_delete(envp->hDispatch);

	if(envp->hCap)
		capture_delete(envp->hCap);

	if(envp->hImgConv)
		img_conv_delete(envp->hImgConv);

	alg_exit();

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

	/* init and create modules for application */
	status = app_init(envp);
	if(status)
		goto exit;

	/* create msg */
	hMsg = msg_create(MSG_MAIN, NULL, MSG_FLAG_NONBLK);
	if(!hMsg) {
		ERR("create msg failed");
		goto exit;
	}

	/* catch signals */
	//signal(SIGINT, sig_handler);

	/* run our modules  */
	status = app_run(envp);
	if(status)
		goto exit;

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

	if(envp->reboot) {
		INFO("we are going to reboot system");
		system("shutdown -r now");
	}
	
exit:

	/* exit modules */
	app_exit(envp, hMsg);

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

