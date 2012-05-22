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
#include "cam_file.h"
#include "app_msg.h"
#include "buffer.h"
#include <signal.h>
#include "ctrl_server.h"
#include "icam_ctrl.h"

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
#define PROGRAM_NAME			"iCamera"
#define CONFIG_FILE				"./cam.cfg"

#define MAIN_FLAG_TEST_EN		(1 << 0)

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/* Params used in this module */
typedef struct {
	const char 		*cfgFileName;
	const char 		*savePath;
	Bool			exit;
	Bool			reboot;
	CtrlSrvHandle	hCtrlSrv;
	Int32			flags;
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
	
	/* create ctrl server */
	CtrlSrvAttrs ctrlSrvAttrs;

	ctrlSrvAttrs.cfgFileName = envp->cfgFileName;
	ctrlSrvAttrs.msgName = ICAM_MSG_NAME;
	
	envp->hCtrlSrv = ctrl_server_create(&ctrlSrvAttrs);
	if(!envp->hCtrlSrv) {
		return E_IO;
	}

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

	/* run ctrl server */
	err = ctrl_server_run(envp->hCtrlSrv);
	if(err) {
		ERR("ctrl server run failed...");
		return err;
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
	/* call ctrl server exit */
	if(envp->hCtrlSrv)
		ctrl_server_delete(envp->hCtrlSrv, hMsg);

	if(hMsg)
		msg_delete(hMsg);

	alg_exit();

	DBG("app exit...");

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
		DBG("got INT signal: %d", sig);
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
	hMsg = msg_create(MSG_MAIN, NULL, 0);
	if(!hMsg) {
		ERR("create msg failed");
		goto exit;
	}

	msg_set_recv_timeout(hMsg, 1);

	/* catch signals */
	signal(SIGINT, sig_handler);

	/* ignore signal when socket is closed by server */
	signal(SIGPIPE, SIG_IGN); 

	/* run our modules  */
	status = app_run(envp);
	if(status)
		goto exit;

	/* start main loop */
	while(!s_exit && !envp->exit) {
		/* feed dog */

		/* recv msg */
		status = msg_recv(hMsg, (MsgHeader *)&msgBuf, sizeof(msgBuf), 0);
		if(status >= 0) {
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
	INFO(" -t self-test enable, 1-detector test, 2-path name test, 4-ftp upload test");
    INFO(" use default params: ./%s", PROGRAM_NAME);
    INFO(" use specific config file: ./%s -f ./cfg/myCfg", PROGRAM_NAME);
	INFO(" do detector and path name test: ./%s -t 3", PROGRAM_NAME);
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
	Int32 		c;
    const char 	*options = "f:p:t:h";
	MainEnv 	env;
	int 		ret, testFlags = 0;

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
		case 't':
			testFlags = atoi(optarg);
			break;
		case 'h':
		default:
			usage();
			return -1;
		}
	}

	/* do self test if enabled */
	if(testFlags)
		ret = self_test(testFlags);
	else
		ret = main_loop(&env);

	INFO("%s exit, status: %d...\n", PROGRAM_NAME, ret);

	exit(0);
}

/*****************************************************************************
 Prototype    : app_hdr_msg_send
 Description  : api for send only msg header
 Input        : MsgHandle hMsg       
                const char *dstName  
                Uint16 cmd           
                Int32 param0         
                Int32 param1         
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/22
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 app_hdr_msg_send(MsgHandle hMsg, const char *dstName, Uint16 cmd, Int32 param0, Int32 param1)
{
	MsgHeader msg;

	msg.cmd = cmd;
	msg.dataLen = 0;
	msg.param[0] = param0;
	msg.param[1] = param1;
	msg.type = MSG_TYPE_REQU;
	msg.index = 0;

	return msg_send(hMsg, dstName, &msg, 0);
}

