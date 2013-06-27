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
		ERRSTR("send data failed");
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
		ERRSTR("recv err, ret: %d", ret);
	} else {
		pMsg->params[0] = ret;
		pMsg->dataLen = ret;
	}

	//reply msg
	ret = sys_commu_write(syslink, pMsg);
	if(ret < 0) {
		ERR("reply msg failed");
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
		ERR("listen failed.");
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

	DBG("start accept, timeout: %d ms", timeout);

	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);

	// wait connections
	fd_set rdSet;
	int fdMax = listenFd + 1;
	FD_ZERO(&rdSet);
	FD_SET(listenFd, &rdSet);
	err = select(fdMax, &rdSet, NULL, NULL, pTimeval);
	if(err < 0 || !FD_ISSET(listenFd, &rdSet)) {
		ERRSTR("select err");
		err = E_IO;
		msg.params[0] = SYS_ERR_TIMEOUT;
		goto reply_msg;
	}

	// accept incoming connections
	bzero(&addr, sizeof(addr));
	sock = accept(listenFd, (struct sockaddr *)&addr, &len);
	if(sock < 0) {
		ERRSTR("accept err.");
		msg.params[0] = SYS_ERR_IO;
		goto reply_msg;
	}	

	msg.params[0] = SYS_ERR_NO;
	DBG("tcp server proxy, %s connected...", inet_ntoa(addr.sin_addr));

reply_msg:

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
	DBG("tcp server run ...");

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
	DBG("tcp client, server info: %s:%u", msg.addrInfo.addr, msg.addrInfo.port);

	if(sock > 0)
		close(sock);

	// create socket 
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock < 0) {
		ERRSTR("create sock failed");
		return E_IO;
	}

	// connect server
	struct sockaddr_in addr;
	init_sock_addr(&addr, msg.addrInfo.addr, msg.addrInfo.port);
	err = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
	if(err < 0) {
		msg.header.params[0] = SYS_ERR_REFUSED;
		ERRSTR("tcp client, connect server failed");
		err = E_REFUSED;
	} else {
		DBG("tcp client, connect server ok.");
		msg.header.params[0] = SYS_ERR_NO;
		err = E_NO;
	}

	//reply msg
	msg.header.cmd = SYS_CMD_CONNECT_RET;
	msg.header.dataLen = 0;
	sys_commu_write(syslink, &msg.header);

	return err;
}


