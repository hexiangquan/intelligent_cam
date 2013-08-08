#include <iostream>
#include "common.h"
#include "log.h"
#include "sys_commu.h"
#include "net_utils.h"
#include "syslink_proto.h"
#include "tcpProxy.h"
#include "udpProxy.h"
#include "dspNetSrv.h"

/* type of net proxy */
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
 * WaitOpen, wait open cmd from dsp
 */
int DspNetSrv::WaitOpen(int fd, uint32_t& type)
{
	SysMsg msg;

	bzero(&msg, sizeof(msg));
	
//	DBG("%s, wait syslink chan open...", chanName.c_str());
	// Wait dsp send cmd
	int err = E_NO;
	struct timeval timeout;

	timeout.tv_sec = 3;
	timeout.tv_usec = 0;
	while(1) {
		// Wait data from dsp
		fd_set rdSet;
		int fdMax = fd + 1;
		FD_ZERO(&rdSet);
		FD_SET(fd, &rdSet);
		err = select(fdMax, &rdSet, NULL, NULL, &timeout);
		if(err < 0 || !FD_ISSET(fd, &rdSet)) {
			//ERRSTR("select err");
			err = E_IO;
			break;
		}

		// Read cmd 
		bzero(&msg, sizeof(msg));
		err = sys_commu_read(fd, &msg, sizeof(msg));
		if(err < 0)
			break;	

		if(msg.cmd == SYS_CMD_NET_OPEN) {
			err = E_NO;
			type = msg.params[0];
			DBG("%s, got open cmd, type: %u...", chanName.c_str(), type);
			break;
		} 

		//Reply msg
		CmdReply(fd, msg.cmd, SYS_ERR_MODE);

		ERR("%s, unexpected cmd: 0x%X", chanName.c_str(), msg.cmd);
	}	

	return err;
}

/**
 * CmdReply, reply a syslink cmd
 */
int DspNetSrv::CmdReply(int fd, uint32_t cmd, int err)
{
	if(cmd > SYS_CMD_NET_EXIT || cmd < SYS_CMD_NET_OPEN)
		return E_INVAL; //invalid cmd value, need not reply

	SysMsg msg;

	bzero(&msg, sizeof msg);
	msg.cmd = cmd + 1; 
	msg.params[0] = err;

	return sys_commu_write(fd, &msg);
}


/**
 * ProxyOpen -- open a new net proxy
 */
NetProxy *DspNetSrv::ProxyOpen(uint32_t type, int syslinkFd, size_t bufLen)
{
	NetProxy *proxy = NULL;
	DBG("%s, open proxy type: %d", chanName.c_str(), type);
	string name(chanName);

	switch(type) {
		case  NET_TYPE_TCP_SERVER:
			proxy = new TcpServer(syslinkFd, bufLen);
			name += "-tcpsrv";
			break;
		case  NET_TYPE_TCP_CLIENT:
			proxy = new TcpClient(syslinkFd, bufLen);
			name += "-tcpclient";
			break;
		case NET_TYPE_UDP_SERVER:
			proxy = new UdpServer(syslinkFd, bufLen);
			name += "-udpsrv";
			break;
		case NET_TYPE_UDP_CLIENT:
			proxy = new UdpClient(syslinkFd, bufLen);
			name += "-udpclient";
			break;
		default:
			ERR("unknown net type: %d", type);
			break;
	}

	if(proxy) {
		proxy->SetName(name);
		CmdReply(syslinkFd, SYS_CMD_NET_OPEN, SYS_ERR_NO);
	} else {
		CmdReply(syslinkFd, SYS_CMD_NET_OPEN, SYS_ERR_IO);
	}

	return proxy;
}

/**
 * ProcessThread -- process proxy
 */
int DspNetSrv::ProcessLoop(int syslinkFd, size_t bufLen) 
{
	int err = E_NO;

	DBG("%s, start process... ", chanName.c_str());

	while(!StopRunning()) {

		sleep(1);

		// Wait net open cmd
		uint32_t netType;
		err = WaitOpen(syslinkFd, netType);
		if(err < 0)
			continue;

		// Open proxy
		proxy = ProxyOpen(netType, syslinkFd, bufLen);
		if(!proxy) {
			ERR("%s, open proxy failed...", chanName.c_str());
			continue;
		}

		// Process this proxy
		err = proxy->Run(); 

		// Delte this proxy
		DBG("delete proxy.");
		if(proxy)
			delete proxy;
		proxy = NULL;
	}

	DBG("%s, exit process loop.", chanName.c_str());

	return err;
}

/**
 * Stop -- stop running
 */
int DspNetSrv::Stop()
{
	int ret;

	if(proxy)
		proxy->Stop();

	ret = DspSrv::Stop();

	DBG("%s, stop running...", chanName.c_str());

	return ret;
}


