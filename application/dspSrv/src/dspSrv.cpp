#include "dspSrv.h"
#include <iostream>
#include "common.h"
#include "log.h"
#include "img_trans.h"
#include "sys_commu.h"
#include "net_utils.h"
#include "syslink_proto.h"
#include "netProxy.h"

#define MSG_NAME			"/tmp/msgDspSrv"
#define MSG_TRANS_TIMEOUT	10
#define IMG_TRANS_TIMEOUT	10

/* Type of net proxy */
enum NetType {
	NET_TYPE_TCP_CLIENT = 0,
	NET_TYPE_TCP_SERVER,
	NET_TYPE_UDP_CLIENT,
	NET_TYPE_UDP_SERVER,
};

using std::string;
using std::cin;
using std::cout;
using std::endl;

/**
 * SyslinkOpen, open syslink chan
 */
int DspSrv::SyslinkOpen()
{
	//Set base addr
	struct syslink_attrs attrs;
	attrs.info_base = baseAddr;
	
	//Open this chan
	int fd = sys_commu_open(&attrs);
	if(fd < 0) {
		ERR("open syslink chan failed!");
		return E_IO;
	}

	DBG("open syslink dev ok.");
	return fd;
}

/**
 * WaitOpen, wait open cmd from dsp
 */
int DspSrv::WaitOpen(int fd, uint32_t& type)
{
	SysMsg msg;

	bzero(&msg, sizeof(msg));
	
	// Wait dsp send cmd
	int err = E_NO;
	while(1) {
		// Wait data from dsp
		fd_set rdSet;
		int fdMax = fd + 1;
		FD_ZERO(&rdSet);
		FD_SET(fd, &rdSet);
		err = select(fdMax, &rdSet, NULL, NULL, NULL);
		if(err < 0 || !FD_ISSET(fd, &rdSet)) {
			ERRSTR("select err");
			err = E_IO;
			break;
		}

		// Read cmd 
		err = sys_commu_read(fd, &msg, sizeof(msg));
		if(err < 0)
			break;	

		if(msg.cmd == SYS_CMD_NET_OPEN) {
			err = E_NO;
			type = msg.params[0];
			break;
		} 

		ERR("unexpected cmd: 0x%X", msg.cmd);
	}	

	return err;
}

static NetProxy *ProxyOpen(int type)
{
	NetProxy *proxy = NULL;

	return proxy;
}

/**
 * ProcessThread -- process proxy
 */
void * DspSrv::ProcessThread(void *arg)
{
	class DspSrv *dspSrv = (class DspSrv *)arg;
	int err = E_NO;

	// Open this syslink chan
	int fd = dspSrv->SyslinkOpen();

	if(fd < 0)
		goto exit;

	dspSrv->exit = FALSE;
	
	while(!dspSrv->exit) {
		// Wait net open cmd
		uint32_t netType;
		err = dspSrv->WaitOpen(fd, netType);
		if(err < 0)
			continue;

		// Open proxy
		NetProxy *proxy = ProxyOpen(netType);
		if(!proxy)
			continue;

		// Process this proxy
		err = proxy->Run(); 

		// Delte this proxy
		delete proxy;
	}
	
exit:
	if(fd > 0)
		close(fd);

	cout << "dsp srv, stop running..." << endl;

	pthread_exit(0);
}

/**
 * Run dsp server, read img from dsp and send to network
 */
int DspSrv::Run()
{
	if(isRunning)
		return E_MODE;

	int err = pthread_create(&pid, NULL, DspSrv::ProcessThread, this);
	if(err < 0) {
		ERRSTR("create process thread failed!");
		return E_IO;
	}

	isRunning = true;

	return E_NO;
}

/**
 * Stop, stop running
 */
int DspSrv::Stop()
{
	if(!isRunning)
		return E_MODE;

	exit = 1;
	pthread_join(pid, NULL);

	isRunning = false;

	return E_NO;
}


