#include "icam_ctrl.h"
#include "net_utils.h"
#include <pthread.h>
#include "cam_ctrl_thread.h"

#define PROGRAM_NAME				"camCtrlSrv"
#define CAM_CTRL_MSG				"/tmp/camCtrlSrv"

#define CAM_CTRL_SRV_PORT			9300
#define CAM_CTRL_LISTEN_NUM			5
#define CAM_CTRL_MAX_CONNECT_NUM	5
#define CAM_CTRL_TIMEOUT			5	//second
#define CAM_CTRL_BUF_SIZE			(2 * 1024 * 0124)

typedef struct {
	Uint16		port;		//listen port
	Uint32		bufLen;		//size of buffer
}CamCtrlSrvEnv;

static Int32 main_loop(CamCtrlSrvEnv *envp)
{
	int 				ret = E_NO;
	Bool				exit = FALSE;
	int					listenSock, connectSock;
	socklen_t 			len;
	struct sockaddr_in  clientAddr;
	pthread_mutex_t		mutex;
	ICamCtrlHandle		hCamCtrl = NULL;
	Int8				*transBuf;
	pthread_t			pid;

	/* alloc buffer for communication */
	transBuf = calloc(1, envp->bufLen);
	assert(transBuf);
	
	/* create socket */
	listenSock = socket_tcp_server(envp->port, CAM_CTRL_LISTEN_NUM);
	if(listenSock < 0) {
		ERR("create server socket failed");
		return E_IO;
	}

	/* create icam ctrl handle */
	hCamCtrl = icam_ctrl_create(CAM_CTRL_MSG, 0, CAM_CTRL_TIMEOUT);
	if(!hCamCtrl) {
		ERR("create cam ctrl handle failed");
		ret = E_IO;
		goto quit;
	}

	/* init mutex for sync */
	pthread_mutex_init(&mutex, NULL);

	/* ignore signal when socket is closed by server */
	signal(SIGPIPE, SIG_IGN); 

	DBG("%s, port %u waiting for connection...", PROGRAM_NAME, envp->port);

	while(!exit) {	
		/* Accept connections */
		len = sizeof(clientAddr);
		bzero(&clientAddr, sizeof(clientAddr));
		connectSock = accept(listenSock, (struct sockaddr *)&(clientAddr), &len);

		if(connectSock < 0) {
			ERRSTR("accept err");
			continue;
		}
		
		DBG("%s, %s connected...", PROGRAM_NAME, inet_ntoa(clientAddr.sin_addr));
		
		/* create new thread to reponse this connection */
		CamCtrlThrParams *params = malloc(sizeof(CamCtrlThrParams));
		if(!params) {
			ERR("can't alloc param for thread....");
			close(connectSock);
			continue;
		}

		params->sock = connectSock;
		params->dataBuf = transBuf;
		params->bufLen = envp->bufLen;
		params->hCamCtrl = hCamCtrl;
		params->mutex = &mutex;

		if(pthread_create(&pid, NULL, cam_ctrl_thread, params) < 0) {
			ERRSTR("create thread failed");
			close(connectSock);
		}
	}

quit:

	if(listenSock > 0)
		close(listenSock);

	if(hCamCtrl)
		icam_ctrl_delete(hCamCtrl);

	pthread_mutex_destroy(&mutex);

	free(transBuf);
	
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
	INFO(" -p listen port, default: %d", CAM_CTRL_SRV_PORT);
	INFO(" -l buffer size for communication, default: %d bytes", CAM_CTRL_BUF_SIZE);
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
    const char *options = "p:l:h";
	CamCtrlSrvEnv env;
	Int32 ret;

	/* Init params */
	memset(&env, 0, sizeof(env));
	
	env.port = CAM_CTRL_SRV_PORT;
	env.bufLen = CAM_CTRL_BUF_SIZE;

	while ((c = getopt(argc, argv, options)) != -1) {
		switch (c) {
		case 'p':
			env.port = atoi(optarg);
			break;
		case 'l':
			if(atoi(optarg) > 0)
				env.bufLen = atoi(optarg);
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

