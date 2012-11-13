#include "target_ctrl.h"
#include "syslink.h"
#include "sys_commu.h"
#include "log.h"

/**
 * target_ctrl_process -- recv and process msg from target processor
 */
Int32 target_ctrl_process(Int32 fd, TargetInfo *info)
{
	if(fd < 0 || !info)
		return E_INVAL;

	Int8 buf[1024];
	SysMsg *msg = (SysMsg *)buf;

	/* recv msg */
	Int32 ret = sys_commu_read(fd, msg, sizeof(buf));
	if(ret < 0) {
		DBG("target ctrl, process msg failed!");
		return ret;
	}

	/* process msg */
	switch(msg->cmd) {
	case SYS_CMD_VIDDETECT_RET:		
		/* result of video detect */
		if(msg->dataLen == sizeof(VideoDetectResult)) {
			memcpy(&info->vidDetectInfo, buf + sizeof(*msg), msg->dataLen);
			if(info->vidDetectInfo.capInfo.capCnt)
				info->vidDetectFlag = TRUE;
		} else {
			ERR("invalid len of video detect result!");
			ret = E_CHECKSUM;
		}
		break;
	case SYS_CMD_VLPR_RET:
		/* result of license plate */
		if(msg->dataLen == sizeof(LicensePlateInfo)) {
			memcpy(&info->plateInfo, buf + sizeof(*msg), msg->dataLen);
			info->vidDetectFlag = TRUE;
		} else {
			ERR("invalid len of plate info!");
			ret = E_CHECKSUM;
		}
		break;
	default:
		WARN("target ctrl, drop cmd: 0x%X", msg->cmd);
		ret = E_UNSUPT;
		break;
	}

	return ret;
}

/**
 * target_params_cfg -- cfg params to target
 */
Int32 target_params_cfg(Int32 fdSyslink, Uint32 cmd, const Uint32 *params, const void *data, size_t dataLen)
{
	
	Int32 fd = fdSyslink;
	if(fd <= 0) {
		fd = open(SYSLINK_DEV_NAME, O_RDWR);
		if(fd < 0)
			return E_IO;
	}

	/* send cfg to the target */
	Int8 buf[1024];
	SysMsg *msg = (SysMsg *)buf;

	bzero(msg, sizeof(*msg));
	msg->cmd = cmd;
	if(params) {
		msg->params[0] = params[0];
		msg->params[1] = params[1];
		msg->params[2] = params[2];
	}
	msg->dataLen = dataLen;
	if(dataLen && dataLen < sizeof(buf) - sizeof(SysMsg))
		memcpy(buf + sizeof(SysMsg), data, dataLen);

	Int32 err = sys_commu_write(fd, msg);

	/* close device if opened in this function */
	if(fdSyslink < 0)
		close(fd);
	
	return err;
}


/**
 * target_plate_recog_cfg -- cfg plate recog alg
 */
Int32 target_plate_recog_cfg(Int32 fdSyslink, const CamPlateRecogCfg *cfg)
{
	if(!cfg)
		return E_INVAL;
	
	return target_params_cfg(fdSyslink, SYS_CMD_VLPR_CFG, NULL, cfg, sizeof(*cfg));
}


/**
 * target_plate_recog_cfg -- cfg plate recog alg
 */
Int32 target_vid_detect_cfg(Int32 fdSyslink, const CamVidDetectCfg *cfg)
{
	if(!cfg)
		return E_INVAL;
	
	return target_params_cfg(fdSyslink, SYS_CMD_VIDDETECT_CFG, NULL, cfg, sizeof(*cfg));
}

/**
 * target_day_night_cfg -- notify target to switch day night mode
 */
Int32 target_day_night_cfg(Int32 fdSyslink, CamDayNightMode mode)
{
	Uint32 params[3] = {0, 0, 0};
	params[0] = (mode == CAM_DAY_MODE) ? 0 : 1;
	
	return target_params_cfg(fdSyslink, SYS_CMD_DAY_NIGHT_SWITCH, params, NULL, 0);
}

