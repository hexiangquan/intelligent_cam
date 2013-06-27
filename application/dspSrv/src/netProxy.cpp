#include "netProxy.h"
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

#define MAX_ERR_CNT		1000

/**
 * CheckExit -- check exit cmd
 */
int NetProxy::CheckExit(uint32_t cmd)
{
	SysMsg msg;
	int err;

	bzero(&msg, sizeof(msg));
	if(cmd == SYS_CMD_NET_EXIT) {
		// reply cmd
		msg.cmd = SYS_CMD_NET_EXIT_RET;
		msg.params[0] = SYS_ERR_NO;
		closeChan = true;
		DBG("%s, got chan exit cmd...", name.c_str());
		err = E_NO;
	} else {
		DBG("%s, waitCmd, unexpected cmd: 0x%X", name.c_str(), cmd);
		msg.cmd = cmd + 1; 
		msg.params[0] = SYS_ERR_MODE;
		err = E_INVAL;
		if(cmd == SYS_CMD_NET_OPEN) {
			ERR("%s, got open cmd, exit chan...", name.c_str());
			closeChan = true;
		}

		if(cmd > SYS_CMD_NET_EXIT || cmd < SYS_CMD_NET_OPEN)
			return E_INVAL; //invalid cmd value, need not reply
	}

	sys_commu_write(syslink, &msg);

	return err;
}

/**
 * CloseSock -- process close socket cmd
 */
int NetProxy::CloseSock()
{
	if(sock >= 0) {
		close(sock);
		sock = -1;
	}

	// reply cmd
	SysMsg msg;
	bzero(&msg, sizeof(msg));
	msg.cmd = SYS_CMD_SOCK_CLOSE_RET;
	msg.params[0] = SYS_ERR_NO;
	sys_commu_write(syslink, &msg);

	DBG("%s, close socket...", name.c_str());

	return E_NO;
}

/**
 * Set socket timeout
 */
int NetProxy::SetTimeout(int sendTimeout, int recvTimeout)
{
	if(sock < 0)
		return E_INVAL;
	
	int err = 0;
	if(sendTimeout > 0)
		err = set_sock_send_timeout(sock, sendTimeout/1000);	
	if(recvTimeout > 0)
		err |= set_sock_recv_timeout(sock, recvTimeout/1000);	

	DBG("%s, set timeout, tx/rx: %d, %d ms.", name.c_str(), sendTimeout, recvTimeout);
		
	assert(err == E_NO);
	return err ? E_IO : E_NO;
}

/**
 * WaitCmd -- wait a certain cmd without addtional data 
 */
int NetProxy::WaitCmd(uint32_t cmd, SysMsg& msg, size_t len)
{
	int err = E_ABORT;
	int cnt = 0;

	while(!Exit()) {
		// wait cmd from dsp
		err = sys_commu_read(syslink, &msg, len);
		if(err < 0 ) {
			if( ++cnt > MAX_ERR_CNT) {
				ERR("%s, read msg failed.", name.c_str());
				break;
			} else {
				continue;
			}
		}
		//check cmd
		if(msg.cmd == cmd) {
			err = E_NO;
			break;
		} else if(CheckExit(msg.cmd) == E_NO) {
			err = E_ABORT;
			break;
		} 
	}

	if(Exit())
		return E_ABORT;

	return err;
}

/**
 * TransferLoop -- Loop send and receive 
 */
int NetProxy::TransferLoop()
{
	// alloc buffer for the first time use
	if(!transBuf) {
		transBuf = calloc(1, bufSize);
		if(!transBuf) {
			ERR("%s, alloc mem failed!", name.c_str());
			return E_NOMEM;
		}
	}

	// loop for send and recv data
	SysMsg *pMsg = (SysMsg *)transBuf;
	int err = E_ABORT;
	int cnt = 0;

	while(!Exit()) {
		// wait cmd from dsp
		err = sys_commu_read(syslink, pMsg, bufSize);
		if(err < 0 ) {
			usleep(1000);
			if(++cnt > MAX_ERR_CNT) {
				//ERR("read msg failed.");
				cnt = 0;
				continue;//break;
			} else {
				continue;
			}
		}
		//check cmd
		if(pMsg->cmd == sendCmd) {
			uint8_t *pData = (uint8_t *)transBuf + sizeof(*pMsg);
			err = SendData(pData, pMsg->dataLen);
		} else if(pMsg->cmd == recvCmd) {
			err = RecvData(transBuf, pMsg->params[0]);
		} else if(pMsg->cmd == SYS_CMD_SOCK_CLOSE) {
			err = CloseSock();
			break;
		} else if(pMsg->cmd == SYS_CMD_SET_NET_TIMEOUT) {
			err = SetTimeout((int)pMsg->params[0], (int)pMsg->params[1]);
		} else {
			CheckExit(pMsg->cmd);
		}
	}

	return err;
}

/**
 * Run -- Run the net server 
 */
int NetProxy::Run()
{
	DBG("%s, net proxy run ...", name.c_str());

	int err = E_ABORT;

	//listen
	err = InitTrasnfer();
	if(err < 0)
		return err;

	while(!Exit()) {
		//establish link, such as connect, accept
		err = EstablishLink();
		if(err < 0)
			continue;

		//start transfer loop
		err = TransferLoop();
	}

	return err;
}

/**
 * Do some work for bind port
 */
int NetProxy::StartListen()
{
	// Wait listen and bind cmd
	SysMsg msg;
	int err = E_ABORT;
	uint16_t port;

	while(!Exit()) {
		// wait cmd from dsp
		err = WaitCmd(SYS_CMD_LISTEN, msg);
		if(err == E_NO) {
			// process listen cmd
			port = msg.params[0];
			err = BindPort(port);
			// reply msg
			msg.cmd = SYS_CMD_LISTEN_RET;
			msg.params[0] = err ? SYS_ERR_REFUSED : SYS_ERR_NO;
			msg.dataLen = 0;
			sys_commu_write(syslink, &msg);

			if(err == E_NO)
				break;
		}
	}

	return err;
}


