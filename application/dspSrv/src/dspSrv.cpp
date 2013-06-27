#include "dspSrv.h"
#include <iostream>
#include "common.h"
#include "log.h"
#include "img_trans.h"
#include "sys_commu.h"
#include "net_utils.h"
#include "syslink_proto.h"
#include "netProxy.h"

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
	strncpy((char *)attrs.name, chanName.c_str(), sizeof(attrs.name));
	
	//Open this chan
	int fd = sys_commu_open(&attrs);
	if(fd < 0) {
		ERR("%s, open syslink chan failed!", chanName.c_str());
		return E_IO;
	}

	DBG("%s, open syslink dev ok, base: 0x%X.", chanName.c_str(), baseAddr);
	return fd;
}


/**
 * ProcessThread -- process proxy
 */
void *DspSrv::ProcessThread(void *arg)
{
	DspSrv *dspSrv = (DspSrv *)arg;

	// Open this syslink chan
	int fd = dspSrv->SyslinkOpen();

	if(fd < 0)
		goto exit;

	dspSrv->exit = FALSE;

	DBG("%s, start run process loop.", dspSrv->chanName.c_str());
	dspSrv->ProcessLoop(fd, dspSrv->bufSize);
	
exit:
	if(fd > 0)
		close(fd);

	cout << dspSrv->chanName << " ,dsp srv stop running..." << endl;

	pthread_exit(0);
}

/**
 * Run dsp server -- read img from dsp and send to network
 */
int DspSrv::Run()
{
	if(isRunning)
		return E_MODE;

	int err = pthread_create(&pid, NULL, ProcessThread, this);
	if(err < 0) {
		ERRSTR("%s, create process thread failed!", chanName.c_str());
		return E_IO;
	}

	isRunning = true;

	return E_NO;
}

/**
 * Stop -- stop running
 */
int DspSrv::Stop()
{
	if(!isRunning)
		return E_MODE;

	exit = 1;
	DBG("%s, stop running...", chanName.c_str());
	pthread_join(pid, NULL);

	isRunning = false;

	return E_NO;
}


