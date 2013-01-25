#include "target_ctrl.h"
#include "syslink.h"
#include "sys_commu.h"
#include "syslink_proto.h"
#include "log.h"
#include <sys/ioctl.h>
#include "cam_detector.h"

static int s_fdSyslink = -1; 

/**
 * target_ctrl_init -- init target ctrl module 
 */
Int32 target_ctrl_init(Uint32 timeout)
{
	if(s_fdSyslink > 0)
		return E_NO;

	struct syslink_attrs attrs;
	attrs.info_base = LINK_CTRL_BASE;
	s_fdSyslink = sys_commu_open(&attrs);

	if(s_fdSyslink <= 0)
		return E_IO;

	/* set timeout */
	ioctl(s_fdSyslink, SYSLINK_S_TIMEOUT, &timeout);
	
	return E_NO;
}

/**
 * target_ctrl_exit -- exit target ctrl module 
 */
Int32 target_ctrl_exit()
{
	return sys_commu_close(s_fdSyslink);
}

/**
 * target_set_params -- cfg params to target
 */
Int32 target_set_params(const void *data, size_t dataLen)
{
	
	Int32 fd = s_fdSyslink;
	if(fd <= 0) {
		return E_IO;
	}

	if(dataLen > 10*1024) {
		ERR("data too long");
		return E_NOSPC;
	}

	/* send cfg to the target */
	Int8 *buf = malloc(sizeof(SysMsg) + dataLen);
	SysMsg *msg = (SysMsg *)buf;

	if(!buf) {
		ERR("alloc mem failed");
		return E_NOMEM;
	}

	bzero(msg, sizeof(*msg));
	msg->cmd = SYS_CMD_SET_CFG;
	msg->dataLen = dataLen;
	if(dataLen)
		memcpy(buf + sizeof(SysMsg), data, dataLen);

	Int32 err = sys_commu_write(fd, msg);
	if(err) {
		ERR("send msg failed!");
	}

	free(buf);
	return err;
}

/**
 * target_get_params -- get params from target
 */
Int32 target_get_params(const void *info, size_t infoLen, void *data, size_t dataLen)
{
	Int32 fd = s_fdSyslink;
	if(fd <= 0) {
		return E_IO;
	}

	size_t len = MAX(infoLen, dataLen);
	if(len > 2048)
		return E_NOSPC;

	/* send cfg to the target */
	Int8 *buf = malloc(sizeof(SysMsg) + len);
	SysMsg *msg = (SysMsg *)buf;

	if(!buf)
		return E_NOMEM;

	bzero(msg, sizeof(*msg));
	msg->cmd = SYS_CMD_GET_CFG;
	msg->dataLen = infoLen;
	if(dataLen)
		memcpy(buf + sizeof(SysMsg), info, infoLen);

	Int32 err = sys_commu_write(fd, msg);
	if(err)
		goto exit;

	/* wait reply */
	err = sys_commu_read(fd, msg, dataLen + sizeof(SysMsg));
	if(err)
		goto exit;

	/* cpy data */
	if(msg->dataLen && dataLen >= msg->dataLen)
		memcpy(data, buf + sizeof(SysMsg), msg->dataLen);

exit:
	free(buf);
	return err;
}

/**
 * target_reset -- reset target
 */
Int32 target_reset()
{
	SysMsg msg;
	int fd = s_fdSyslink;

	if(fd <= 0)
		return E_IO;

	bzero(&msg, sizeof(msg));
	msg.cmd = SYS_CMD_SYS_RESET;

	Int32 err = sys_commu_write(fd, &msg);

	return err;
}

/**
 * target_day_night_switch -- switch day night mode
 */
Int32 target_day_night_switch(Bool isNightMode)
{
	SysMsg msg;
	int fd = s_fdSyslink;

	if(fd <= 0)
		return E_IO;

	bzero(&msg, sizeof(msg));
	msg.cmd = SYS_CMD_DAY_NIGHT_SWITCH;
	msg.params[0] = isNightMode;

	Int32 err = sys_commu_write(fd, &msg);

	return err;
}

Int32 target_ctrl_test()
{
	Int32 err = target_ctrl_init(5000);
	assert(err == E_NO);

	int i = 0, errCnt = 0;
	Int8 bufOut[1024];
	Int8 bufIn[1024];

#if 0
	for(i = 0; i < 1000; i++) {
		bzero(bufOut, sizeof(bufOut));	
		sprintf(bufOut, "%d target ctrl params test...", i);
		err = target_set_params(bufOut, sizeof(bufOut));
		assert(err == E_NO);
		bzero(bufIn, sizeof(bufIn));
		sprintf(bufIn, "%d get param", i);
		err = target_get_params(bufIn, 32, bufIn, sizeof(bufIn));
		assert(err == E_NO);
		DBG("<%d> get param reply: %s", i, bufIn);
		sleep(1);
	}
#endif

	bzero(bufIn, sizeof(bufIn));
	
	SysMsg *msg = (SysMsg *)bufIn;
	sprintf(bufIn + sizeof(*msg), "%d get param", i);
	msg->cmd = SYS_CMD_GET_CFG;
	msg->dataLen = 32;
	err = sys_commu_write(s_fdSyslink, msg);
	assert(err == E_NO);
	
	int fdMax, fdCap;
	fd_set rdSet;
	
	struct syslink_attrs attrs;
	attrs.info_base = LINK_CAP_BASE;
	fdCap = sys_commu_open(&attrs);
	assert(fdCap > 0);
	fdMax = MAX(fdCap, s_fdSyslink) + 1;	

	for(i = 0; i < 1000; ++i) {
		FD_ZERO(&rdSet);
		FD_SET(fdCap, &rdSet);
		FD_SET(s_fdSyslink, &rdSet);
		err = select(fdMax, &rdSet, NULL, NULL, NULL);
		if(err < 0 && errno != EINTR) {
			ERRSTR("select err");
			usleep(1000);
			continue;
		}

		if(FD_ISSET(fdCap, &rdSet)) {
			err = sys_commu_read(fdCap, msg, sizeof(bufIn));
			if(msg->cmd == SYS_CMD_CAP_FRAME) {
				CaptureInfo *info = (CaptureInfo *)(bufIn + sizeof(*msg));
				TriggerInfo *trig = &info->triggerInfo[0];
				DBG("got trig cmd, cnt:%d, way: %d, frame id: %d/%d", info->capCnt,
					trig->wayNum, trig->frameId, trig->groupId);
			} else {
				DBG("invalid cmd: 0x%X", msg->cmd);
			}
		}

		
		if(FD_ISSET(s_fdSyslink, &rdSet)) {
			err = sys_commu_read(s_fdSyslink, msg, sizeof(bufIn));
			if(msg->cmd == SYS_CMD_CFG_RET) {
				DBG("<%d> get param reply: %s", i, bufIn + sizeof(*msg));
			} else {
				DBG("ctrl chan, invalid cmd: 0x%X", msg->cmd);
			}
			sprintf(bufIn + sizeof(*msg), "%d get param", i);
			msg->cmd = SYS_CMD_GET_CFG;
			msg->dataLen = 32;
			err = sys_commu_write(s_fdSyslink, msg);
			assert(err == E_NO);
		}
		
	}

	DBG("err cnt: %d", errCnt);

	close(fdCap);
	target_ctrl_exit();

	return E_NO;
}

