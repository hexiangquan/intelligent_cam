/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : main.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/4/23
  Last Modified :
  Description   : main file of cam ctrl server
  Function List :
              main
              main_loop
              tcp_accept
              usage
  History       :
  1.Date        : 2012/4/23
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "icam_ctrl.h"
#include "net_utils.h"
#include <pthread.h>
#include "cam_ctrl_thread.h"
#include "udp_process.h"
#include "sys_commu.h"

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
#define PROGRAM_NAME				"camCtrlSrv"
#define CAM_CTRL_MSG				"/tmp/camCtrlSrv"
	
#define CAM_CTRL_SRV_PORT			9200
#define CAM_UDP_SRV_PORT			9200
#define CAM_CTRL_LISTEN_NUM			5
#define CAM_CTRL_MAX_CONNECT_NUM	5
#define CAM_CTRL_TIMEOUT			5	//second
#define CAM_CTRL_BUF_SIZE			(8 * 1024 * 1024)
#define ARM_PROG_NAME				"/home/root/iCamera"
#define FPGA_FIRMAWRE				"/home/root/fpga.rbf"

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/* env of this module */
typedef struct {
	Uint16			tcpPort;		//tcp listen port
	Uint16			udpPort;		//udp listen port
	Uint32			bufLen;			//size of buffer
	Int8			*transBuf;		//buf 
	ICamCtrlHandle	hCamCtrl;		//handle for cam ctrl
	pthread_mutex_t	mutex;			//mutex for sync
	const char		*armProg;		//name of arm program for update
	const char 		*fpgaFirmware;	//name of FPGA firmware
}CamCtrlSrvEnv;

/*****************************************************************************
 Prototype    : tcp_accept
 Description  : accept tcp connection
 Input        : CamCtrlSrvEnv *envp  
                int listenSock       
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/23
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 tcp_accept(CamCtrlSrvEnv *envp, int listenSock)
{
	int					connectSock;
	socklen_t 			len;
	struct sockaddr_in  clientAddr;
	pthread_t			pid;
	
	/* Accept connections */
	len = sizeof(clientAddr);
	bzero(&clientAddr, sizeof(clientAddr));
	connectSock = accept(listenSock, (struct sockaddr *)&(clientAddr), &len);

	if(connectSock < 0) {
		ERRSTR("accept err");
		return E_CONNECT;
	}
	
	DBG("%s, %s connected...", PROGRAM_NAME, inet_ntoa(clientAddr.sin_addr));
	
	/* create new thread to reponse this connection */
	CamCtrlThrParams *params = malloc(sizeof(CamCtrlThrParams));
	if(!params) {
		ERR("can't alloc param for thread....");
		close(connectSock);
		return E_NOMEM;
	}

	params->sock = connectSock;
	params->dataBuf = envp->transBuf;
	params->bufLen = envp->bufLen;
	params->hCamCtrl = envp->hCamCtrl;
	params->mutex = &envp->mutex;
	params->armProg = envp->armProg;
	params->fpgaFirmware = envp->fpgaFirmware;

	if(pthread_create(&pid, NULL, cam_ctrl_thread, params) < 0) {
		ERRSTR("create thread failed");
		close(connectSock);
		return E_NOSPC;
	}

	return E_NO;
}

/*****************************************************************************
 Prototype    : main_loop
 Description  : main loop for cam ctrl server
 Input        : CamCtrlSrvEnv *envp  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/23
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 main_loop(CamCtrlSrvEnv *envp)
{
	int 			ret = E_NO;
	Bool			exit = FALSE;
	int				listenSock, udpScok, fdMax;
	fd_set			rdSet;
	UdpProcEnv		udpProcEnv;

	/* alloc buffer for communication */
	envp->transBuf = calloc(1, envp->bufLen);
	assert(envp->transBuf);
	
	/* create socket */
	listenSock = socket_tcp_server(envp->tcpPort, CAM_CTRL_LISTEN_NUM);
	if(listenSock < 0) {
		ERR("create server socket failed");
		return E_IO;
	}
	/* set to non-blocking and use select for wait */
	set_sock_block(listenSock, FALSE);

	/* create udp server socket */
	udpScok = socket_udp_server(envp->udpPort);
	if(udpScok < 0) {
		ERR("create udp server socket failed");
		return E_IO;
	}
	/* set timeout */
	set_sock_send_timeout(udpScok, CAM_CTRL_TIMEOUT);
	set_sock_recv_timeout(udpScok, CAM_CTRL_TIMEOUT);
	udpProcEnv.cmdListenPort = envp->tcpPort;
	udpProcEnv.needReboot = FALSE;

	/* create icam ctrl handle */
	envp->hCamCtrl = icam_ctrl_create(CAM_CTRL_MSG, 0, CAM_CTRL_TIMEOUT);
	if(!envp->hCamCtrl) {
		ERR("create cam ctrl handle failed");
		ret = E_IO;
		goto quit;
	}

	/* init mutex for sync */
	pthread_mutex_init(&envp->mutex, NULL);
	
	/* ignore signal when socket is closed by server */
	signal(SIGPIPE, SIG_IGN); 

	DBG("%s, port %u waiting for connection...", PROGRAM_NAME, envp->tcpPort);

	/* choose max fd */
	fdMax = MAX(listenSock, udpScok) + 1;

	while(!exit) {	
		/* wait data ready */
		FD_ZERO(&rdSet);
		FD_SET(listenSock, &rdSet);
		FD_SET(udpScok, &rdSet);
		
		ret = select(fdMax, &rdSet, NULL, NULL, NULL);
		if(ret < 0 && errno != EINTR) {
			ERRSTR("select err");
			break;
		}

		/* no data ready */
		if(!ret)
			continue;

		/* check which is ready */
		if(FD_ISSET(listenSock, &rdSet)){
			/* accept connection */
			tcp_accept(envp, listenSock);
		}

		if(FD_ISSET(udpScok, &rdSet)){
			/* process udp cmd */
			udp_process(udpScok, &udpProcEnv);
			if(udpProcEnv.needReboot)
				break;
		}	
	}

quit:

	if(listenSock > 0)
		close(listenSock);

	if(envp->hCamCtrl)
		icam_ctrl_delete(envp->hCamCtrl);

	pthread_mutex_destroy(&envp->mutex);

	free(envp->transBuf);

	if(udpProcEnv.needReboot)
		system("shutdown -r now\n");
	
	return ret;
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
	INFO(" -p tcp listen port, default: %d", CAM_CTRL_SRV_PORT);
	INFO(" -u udp listen port, default: %d", CAM_UDP_SRV_PORT);
	INFO(" -l buffer size for communication, default: %d bytes", CAM_CTRL_BUF_SIZE);
	INFO(" -a arm program file name, default: %s", ARM_PROG_NAME);
	INFO(" -f fpga firmware file name, default: %s", FPGA_FIRMAWRE);
    INFO(" use default params: ./%s", PROGRAM_NAME);
    INFO(" use specific params: ./%s -p 5400", PROGRAM_NAME);
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
    const char *options = "p:u:l:a:f:h";
	CamCtrlSrvEnv env;
	Int32 ret;

	/* Init params */
	memset(&env, 0, sizeof(env));
	
	env.tcpPort = CAM_CTRL_SRV_PORT;
	env.bufLen = CAM_CTRL_BUF_SIZE;
	env.udpPort = CAM_UDP_SRV_PORT;
	env.armProg = ARM_PROG_NAME;
	env.fpgaFirmware = FPGA_FIRMAWRE;

	while ((c = getopt(argc, argv, options)) != -1) {
		switch (c) {
		case 'p':
			env.tcpPort = atoi(optarg);
			break;
		case 'u':
			env.udpPort = atoi(optarg);
			break;
		case 'l':
			if(atoi(optarg) > 0)
				env.bufLen = atoi(optarg);
			break;
		case 'a':
			env.armProg = optarg;
			break;
		case 'f':
			env.fpgaFirmware = optarg;
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

