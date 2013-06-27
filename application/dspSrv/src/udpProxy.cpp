#include "udpProxy.h"
#include "log.h"
#include "net_utils.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;

/**
 * InitProxy, create socket 
 */
int UdpProxy::InitProxy()
{
	sendCmd = SYS_CMD_NET_SENDTO; 
	recvCmd = SYS_CMD_NET_RECVFROM;
	if(sock > 0)
		close(sock);

	// create socket 
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sock < 0) {
		ERRSTR("%s, create sock failed", name.c_str());
		return E_IO;
	}

	return E_NO;
}

/**
 * Send data to target
 */
int UdpProxy::SendData(const void *pData, size_t len)
{
	const SysNetAddr *pAddr = (SysNetAddr *)pData;
	const uint8_t *pBuf = (const uint8_t *)pData + sizeof(SysNetAddr);
	int ret = 0;
	SysMsg msg;

	bzero(&msg, sizeof(msg));
	msg.cmd = SYS_CMD_SENDTO_RET;
	
	if(len <= sizeof(NetAddrMsg)) {
		ERR("%s, data too short, len: %d...", name.c_str(), len);
		msg.params[0] = SYS_ERR_NOSPC;	
	} else {
		// send data
		struct sockaddr_in sockaddr;
		socklen_t socklen = sizeof(sockaddr);
		init_sock_addr(&sockaddr, pAddr->addr, pAddr->port);
		//DBG("%s, sendto %s:%u, len: %u", name.c_str(), pAddr->addr, pAddr->port, len - sizeof(SysNetAddr));
		ret = sendto(sock, pBuf, len - sizeof(SysNetAddr), 0, 
				(struct sockaddr *)&sockaddr, socklen);
		if(ret < 0) {
			ERRSTR("%s, sendto failed", name.c_str());
			msg.params[0] = SYS_ERR_TRANS;	
		} else {
			msg.params[0] = ret;
			//DBG("%s, send to len: %u, ret: %d", name.c_str(), len - sizeof(SysNetAddr), ret);
		}
	}

	//reply msg
	ret = sys_commu_write(syslink, &msg);
	return ret;
}

/**
 * RecvData from target
 */
int UdpProxy::RecvData(void *pBuf, size_t bufLen)
{
	NetAddrMsg *pMsg = (NetAddrMsg *)pBuf;
	uint8_t *pData = (uint8_t *)pBuf + sizeof(NetAddrMsg);
	int ret = 0;
	size_t rcvLen = MIN(GetBufSize() - sizeof(SysNetAddr), bufLen);

	bzero(pMsg, sizeof(*pMsg));
	pMsg->header.cmd = SYS_CMD_RECVFROM_RET;
	
	// recv data
	assert(sock > 0);
	struct sockaddr_in sockaddr;
	socklen_t socklen = sizeof(sockaddr);

	//DBG("%s, start recvfrom data, len: %u", name.c_str(), rcvLen);

	bzero(&sockaddr, sizeof(sockaddr));
	ret = recvfrom(sock, pData, rcvLen, 0, 
			(struct sockaddr *)&sockaddr, &socklen);
	if(ret < 0) {
		ERRSTR("%s, recvfrom failed", name.c_str());
		pMsg->header.params[0] = SYS_ERR_TRANS;	
	} else {
		pMsg->header.params[0] = ret;
		pMsg->header.dataLen = ret + sizeof(SysNetAddr);
		inet_ntop(AF_INET, &sockaddr.sin_addr, pMsg->addrInfo.addr, sizeof(pMsg->addrInfo.addr));
		pMsg->addrInfo.port = ntohs(sockaddr.sin_port);
		//DBG("%s, recvfrom len: %u, ret: %d.", name.c_str(), bufLen, ret);
	}

	//reply msg
	ret = sys_commu_write(syslink, &pMsg->header);
	return ret;
}

/*********************************************************************************/

/**
 * Do some work for bind port
 */
int UdpServer::InitTrasnfer()
{
	DBG("udp server run...");
	return StartListen();
}

/**
 * bind port for recv data
 */
int UdpServer::BindPort(uint16_t port)
{
	if(sock > 0)
		close(sock);
	
	sock = socket_udp_server(port);

	if(sock < 0) {
		ERRSTR("bind port failed");
		return E_IO;
	} else {
		DBG("udp server, bind port %u ok.", port);
	}

	return E_NO;	
}

