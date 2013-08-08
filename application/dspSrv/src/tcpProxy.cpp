#include "tcpProxy.h"
#include <iostream>
#include "common.h"
#include "log.h"
#include "sys_commu.h"
#include "net_utils.h"
#include "syslink_proto.h"

using std::string;
using std::cin;
using std::cout;
using std::endl;

#define LISTEN_MAX_NUM	3
#define MAX_ERR_CNT		1000

/**
 * SendData -- send data by tcp
 */
int TcpProxy::SendData(const void *pData, size_t len)
{
	int ret;
	SysMsg msg;

	bzero(&msg, sizeof(msg));
	msg.cmd = SYS_CMD_SEND_RET;

	ret = sendn(sock, pData, len, 0);

	if(ret < 0) {
		ERRSTR("%s, send data failed", name.c_str());
		msg.params[0] = (errno == EWOULDBLOCK ? SYS_ERR_TIMEOUT : SYS_ERR_TRANS);
	} else {
		msg.params[0] = ret;
	}

	//reply msg
	ret = sys_commu_write(syslink, &msg);

	return ret;	
}

/**
 * RecvData -- recv data by tcp
 */
int TcpProxy::RecvData(void *pBuf, size_t bufLen)
{
	int ret;
	SysMsg *pMsg = (SysMsg *)pBuf;
	void *pData = (uint8_t *)pBuf + sizeof(*pMsg);

	bzero(pMsg, sizeof(*pMsg));
	pMsg->cmd = SYS_CMD_RECV_RET;

	ret = recvn(sock, pData, bufLen, 0);

	if(ret <= 0) {
		pMsg->params[0] = (errno == EWOULDBLOCK ? SYS_ERR_TIMEOUT : SYS_ERR_TRANS);
		pMsg->dataLen = 0;
		ERRSTR("%s, recv err, ret: %d", name.c_str(), ret);
	} else {
		pMsg->params[0] = ret;
		pMsg->dataLen = ret;
	}

	//reply msg
	ret = sys_commu_write(syslink, pMsg);
	if(ret < 0) {
		ERR("%s, reply msg failed", name.c_str());
	}

	return ret;	
}

/****************************************************************/

/**
 * BindPort -- open listen socket and bind port
 */
int TcpServer::BindPort(uint16_t port)
{
	if(listenFd > 0)
		close(listenFd);

	// create listen port
	listenFd = socket_tcp_server(port, LISTEN_MAX_NUM);
	if(listenFd < 0) {
		ERR("%s, listen failed.", name.c_str());
		return E_IO;
	}

	DBG("%s, listen port: %u ok...", name.c_str(), port);

	return E_NO;
}

/**
 * EstablishLink -- accept new connections
 */
int TcpServer::EstablishLink()
{
	// Wait listen and bind cmd
	SysMsg msg;
	int err;

	DBG("%s, wait accept cmd...", name.c_str());
	err = WaitCmd(SYS_CMD_ACCEPT, msg);
	if(err) {
		ERR("%s, wait accept cmd failed.", name.c_str());
		return err;
	}

	// check timeout value
	int timeout = (int)msg.params[0];
	struct timeval tv, *pTimeval = NULL;

	if(timeout > 0) {
		tv.tv_sec = timeout/1000;
		tv.tv_usec = (timeout%1000)*1000;
		pTimeval = &tv;
	}

	DBG("%s, start accept, timeout: %d ms", name.c_str(), timeout);

	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);

	while(!Exit()) {
		// wait connections
		fd_set rdSet;
		int fdMax = MAX(listenFd, syslink) + 1;

		FD_ZERO(&rdSet);
		FD_SET(listenFd, &rdSet);
		FD_SET(syslink, &rdSet);
		err = select(fdMax, &rdSet, NULL, NULL, pTimeval);
		if(err < 0 ) {
			ERRSTR("%s, select err", name.c_str());
			err = E_IO;
			msg.params[0] = SYS_ERR_TIMEOUT;
			break;
		}

		if(FD_ISSET(listenFd, &rdSet)) {
			// accept incoming connections
			bzero(&addr, sizeof(addr));
			sock = accept(listenFd, (struct sockaddr *)&addr, &len);
			if(sock < 0) {
				ERRSTR("%s, accept err.", name.c_str());
				msg.params[0] = SYS_ERR_IO;
				break;
			}	
			msg.params[0] = SYS_ERR_NO;
			DBG("%s, tcp server proxy, %s connected...", name.c_str(), inet_ntoa(addr.sin_addr));
			break;
		}
		
		if(FD_ISSET(syslink, &rdSet)) {
			// maybe DSP restart
			WaitCmd(0, msg, sizeof(msg));
		}
	}

	msg.cmd = SYS_CMD_ACCEPT_RET;
	msg.dataLen = 0;
	err = sys_commu_write(syslink, &msg);

	return err;
}

/**
 * InitTrasnfer -- do init work before transfer, start listen
 */
int TcpServer::InitTrasnfer()
{
	DBG("%s, tcp server run ...", name.c_str());

	waitAcceptCmd = true;

	//listen
	int err = StartListen();
	return err;
}

/************************************************************/

/**
 * EstablishLink -- connect server
 */
int TcpClient::EstablishLink()
{
	return ConnectServer();	
}

/**
 * ConnectServer -- connect server
 */
int TcpClient::ConnectServer()
{
	// Wait connect cmd
	NetAddrMsg msg;
	int err;

	err = WaitCmd(SYS_CMD_CONNECT, msg.header, sizeof(msg));
	if(err)
		return err;

	// parse server ip and port
	DBG("%s, tcp client, server info: %s:%u", name.c_str(), msg.addrInfo.addr, msg.addrInfo.port);

	if(sock > 0)
		close(sock);

	// create socket 
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock < 0) {
		ERRSTR("%s, create sock failed", name.c_str());
		return E_IO;
	}

	// connect server
	struct sockaddr_in addr;
	init_sock_addr(&addr, msg.addrInfo.addr, msg.addrInfo.port);
	err = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
	if(err < 0) {
		msg.header.params[0] = SYS_ERR_REFUSED;
		ERRSTR("%s, connect server failed", name.c_str());
		err = E_REFUSED;
	} else {
		DBG("%s, connect server ok.", name.c_str());
		msg.header.params[0] = SYS_ERR_NO;
		err = E_NO;
	}

	//reply msg
	msg.header.cmd = SYS_CMD_CONNECT_RET;
	msg.header.dataLen = 0;
	sys_commu_write(syslink, &msg.header);

	return err;
}


