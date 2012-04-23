#include "net_cmds.h"
#include "udp_cmd_trans.h"
#include "udp_process.h"
#include "net_utils.h"
#include "log.h"

static Int32 get_cam_info(UdpProcEnv *env, void *buf, Uint32 *len)
{
	CamListenInfo *info = buf;

	info->listenPort = env->cmdListenPort;
	get_local_ip(info->ipString, sizeof(info->ipString));
	*len = sizeof(CamListenInfo);

	return E_NO;
}

Int32 udp_process(int sock, UdpProcEnv *env)
{
	Int32 				ret;
	struct sockaddr_in 	addr;
	UdpCmdHeader		header;
	Int8				buf[64];

	/* recv cmds */
	ret = udp_cmd_recv(sock, &header, NULL, 0, &addr);
	if(ret < 0) {
		ERR("recv cmd err");
		return ret;
	}

	DBG("recv udp cmd: 0x%x", header.cmd);
	
	/* process cmd */
	switch(header.cmd) {
	case UC_GET_CAMERAINFO:
		ret = get_cam_info(env, buf, &header.dataLen);
		break;
	case UC_RESET_CAMERA:
		env->needReboot = TRUE;
		header.dataLen = 0;
		break;
	default:
		ret = E_INVAL;
		header.dataLen = 0;
		DBG("invalid cmd: 0x%X", header.cmd);
		break;
	}

	ret = udp_cmd_send(sock, &header, buf, &addr);
	return ret;
}

