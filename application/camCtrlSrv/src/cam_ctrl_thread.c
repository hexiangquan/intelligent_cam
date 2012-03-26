/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : cam_ctrl_thread.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/26
  Last Modified :
  Description   : thread for cam ctrl commu
  Function List :
              cam_ctrl_thread
              ctrl_cmd_process
              kill_old_connect
              set_cur_thr_id
              thread_init
  History       :
  1.Date        : 2012/3/26
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "cam_ctrl_thread.h"
#include "tcp_cmd_trans.h"
#include "net_utils.h"

/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/

/*----------------------------------------------*
 * external routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/
static pthread_t	s_curThread = 0;

/*----------------------------------------------*
 * project-wide global variables                *
 *----------------------------------------------*/

/*----------------------------------------------*
 * module-wide global variables                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * constants                                    *
 *----------------------------------------------*/

/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/
#define 	CMD_RECV_TIMEOUT	15
#define		CMD_SEND_TIMEOUT	10
#define		MAX_ERR_CNT			10

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/


/*****************************************************************************
 Prototype    : set_cur_thr_id
 Description  : set thread id to current thread id
 Input        : pthread_mutex_t *mutex  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/23
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 set_cur_thr_id(pthread_t newId, pthread_mutex_t *mutex)
{
	pthread_mutex_lock(mutex);
	s_curThread = newId;
	pthread_mutex_unlock(mutex);
	
	return E_NO;
}


/*****************************************************************************
 Prototype    : kill_old_connect
 Description  : kill old thread
 Input        : CamCtrlThrParams *params  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/23
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 kill_old_connect(CamCtrlThrParams *params)
{
	pthread_t oldThrId = s_curThread;
	Int32 ret = E_NO;

	if(pthread_self() == oldThrId)
		return E_NO;	//same thread

	/* set id to current thread */
	set_cur_thr_id(pthread_self(), params->mutex);
	if(s_curThread != pthread_self()) {
		/* this happens when multi-threads setting id at the same time */
		ret = E_BUSY;
		goto exit;
	}

	if(oldThrId == 0) {
		/* Old task is already killed */
		ret = E_NO;
		goto exit;
	}
	
	/* wait a while for old thread exit */
	sleep(1);
	
exit:
	
	return ret;
}


/*****************************************************************************
 Prototype    : thread_init
 Description  : init thread
 Input        : CamCtrlThrParams *params  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/23
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 thread_init(CamCtrlThrParams *params)
{
	TcpCmdHeader 	cmdHdr;
	Int32 			ret, result;
	Uint16			cmdData;
	int				sock = params->sock;

	/* detatch ourself */
	pthread_detach(pthread_self());

	/* Set trans timeout */
	ret = set_sock_recv_timeout(sock, CMD_RECV_TIMEOUT);
	ret |= set_sock_send_timeout(sock, CMD_SEND_TIMEOUT);
	ret |= set_sock_linger(sock, TRUE, 0) ;
	if(ret) {
		ERR("set sock timeout failed...");
		return E_IO;
	}

	/* Recv start cmd */
	ret = tcp_cmd_recv(sock, &cmdHdr, &cmdData, sizeof(cmdData));
	if(ret != E_NO)
		return ret;

	/* Check if start cmd is valid */
	if(cmdHdr.cmd != TC_FUN_CONNECTREQUEST || cmdHdr.dataLen != sizeof(Int16)) {
		ERR("start cmd is invalid");
		result = E_CHECKSUM;
		ret = E_CHECKSUM;
		goto reply_cmd;
	}

	if(cmdData == CONNECT_MODE_RESET) {
		result = E_NO;
		ret = E_MODE; //set to fail so the port will be closed soon
		INFO("client send reset cmd.");
		goto reply_cmd;
	}

	if(s_curThread) {
		/* There is already an connection, check the control data */
		if(cmdData == CONNECT_MODE_FORCE) {
			/* Kill running task */
			if((ret = kill_old_connect(params)) != E_NO) {
				result = E_REFUSED;
				goto reply_cmd;
			}
		} else {
			ret = E_REFUSED;
			result = E_REFUSED;
			goto reply_cmd;
		}
	} else if((ret = set_cur_thr_id(pthread_self(), params->mutex)) != E_NO) {
		ERR("set connect id failed when connect id is none.");
		result = E_REFUSED;
		goto reply_cmd;
	}

	result = E_NO;

reply_cmd:
	cmdHdr.dataLen = 0;
	tcp_cmd_reply(sock, &cmdHdr, NULL, result);
	return ret;
}

/*****************************************************************************
 Prototype    : ctrl_cmd_process
 Description  : process cmd
 Input        : TcpCmdHeader *cmdHdr  
                void *dataBuf         
                Int32 bufLen          
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/23
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 ctrl_cmd_process(ICamCtrlHandle hCamCtrl, TcpCmdHeader *cmdHdr, void *dataBuf, Int32 bufLen)
{
	Int32 ret = E_INVAL;
	
	switch(cmdHdr->cmd) {
	case TC_FUN_CAMERARESET:
		ret = E_REBOOT;
		cmdHdr->dataLen = 0;
		break;
	case TC_FUN_HEARTBEATSINGAL:
		ret = E_NO;
		cmdHdr->dataLen = 0;
		break;
	case TC_FUN_STARTCAPTURE:
		ret = icam_capture_ctrl(hCamCtrl, CAM_CAP_START);
		cmdHdr->dataLen = 0;
		break;
	case TC_FUN_STOPCAPTURE:
		ret = icam_capture_ctrl(hCamCtrl, CAM_CAP_STOP);
		cmdHdr->dataLen = 0;
		break;
	case TC_FUN_CAPTURETRIGGER:
		ret = icam_capture_ctrl(hCamCtrl, CAM_CAP_TRIG);
		cmdHdr->dataLen = 0;
		break;
	case TC_FUN_SPECCAPTURE:
		ret = icam_capture_ctrl(hCamCtrl, CAM_CAP_SPEC_TRIG);
		cmdHdr->dataLen = 0;
		break;
	case TC_FUN_RESTORETODEFAULT:
		ret = icam_restore_cfg(hCamCtrl);
		cmdHdr->dataLen = 0;
		break;
	case TC_FUN_UPGRADE_ARM:
		ret = icam_firmware_update(hCamCtrl, CAM_UPDATE_ARM, dataBuf, cmdHdr->dataLen);
		cmdHdr->dataLen = 0;
		break;
	case TC_FUN_UPGRADE_DSP:
		ret = icam_firmware_update(hCamCtrl, CAM_UPDATE_DSP, dataBuf, cmdHdr->dataLen);
		cmdHdr->dataLen = 0;
		break;
	case TC_FUN_UPGRADE_FPGA:
		ret = icam_firmware_update(hCamCtrl, CAM_UPDATE_FPGA, dataBuf, cmdHdr->dataLen);
		cmdHdr->dataLen = 0;
		break;
	case TC_GET_VERINFO:
		ret = icam_get_version(hCamCtrl, (CamVersionInfo *)dataBuf);
		cmdHdr->dataLen = sizeof(CamVersionInfo);
		break;
	case TC_GET_TEMPERATURE:
		*(Uint32 *)dataBuf = 0;
		cmdHdr->dataLen = sizeof(Uint32);
		break;
	case TC_GET_TIME:
		ret = icam_get_time(hCamCtrl, (CamDateTime *)dataBuf);
		cmdHdr->dataLen = sizeof(CamDateTime);
		break;
	case TC_SET_TIME:
		if(cmdHdr->dataLen == sizeof(CamDateTime))
			ret = icam_set_time(hCamCtrl, (CamDateTime *)dataBuf);
		cmdHdr->dataLen = 0;
		break;
	case TC_GET_NETWORKINFO:
		ret = icam_get_network_info(hCamCtrl, (CamNetworkInfo *)dataBuf);
		cmdHdr->dataLen = sizeof(CamNetworkInfo);
		break;
	case TC_SET_NETWORKINFO:
		if(cmdHdr->dataLen == sizeof(CamNetworkInfo))
			ret = icam_set_network_info(hCamCtrl, (CamNetworkInfo *)dataBuf);
		cmdHdr->dataLen = 0;
		break;
	case TC_GET_OSDINFO:
		ret = icam_get_osd_params(hCamCtrl, (CamOsdParams*)dataBuf);
		cmdHdr->dataLen = sizeof(CamOsdParams);
		break;
	case TC_SET_OSDINFO:
		if(cmdHdr->dataLen == sizeof(CamOsdParams))
			ret = icam_set_osd_params(hCamCtrl, (CamOsdParams *)dataBuf);
		cmdHdr->dataLen = 0;
		break;
	case TC_GET_IMAGEUPLOADPROTOCOL:
		ret = icam_get_upload_proto(hCamCtrl, dataBuf);
		cmdHdr->dataLen = sizeof(Int32);
		break;
	case TC_SET_IMAGEUPLOADPROTOCOL:
		if(cmdHdr->dataLen == sizeof(Int32))
			ret = icam_set_upload_proto(hCamCtrl, *(CamImgUploadProto *)dataBuf);
		cmdHdr->dataLen = 0;
		break;
	case TC_GET_NTPSERVERINFO:
		ret = icam_get_ntp_srv_info(hCamCtrl, (CamNtpServerInfo *)dataBuf);
		cmdHdr->dataLen = sizeof(CamNtpServerInfo);
		break;
	case TC_SET_NTPSERVERINFO:
		if(cmdHdr->dataLen == sizeof(CamNtpServerInfo))
			ret = icam_set_ntp_srv_info(hCamCtrl, (CamNtpServerInfo *)dataBuf);
		cmdHdr->dataLen = 0;
		break;
	case TC_GET_TCPIMAGESERVERINFO:
		ret = icam_get_img_srv_info(hCamCtrl, (CamTcpImageServerInfo *)dataBuf);
		cmdHdr->dataLen = sizeof(CamTcpImageServerInfo);
		break;
	case TC_SET_TCPIMAGESERVERINFO:
		if(cmdHdr->dataLen == sizeof(CamTcpImageServerInfo))
			ret = icam_set_img_srv_info(hCamCtrl, (CamTcpImageServerInfo *)dataBuf);
		cmdHdr->dataLen = 0;
		break;
	case TC_GET_FTPSERVERINFO:
		ret = icam_get_ftp_srv_info(hCamCtrl, (CamFtpImageServerInfo *)dataBuf);
		cmdHdr->dataLen = sizeof(CamFtpImageServerInfo);
		break;
	case TC_SET_FTPSERVERINFO:
		if(cmdHdr->dataLen == sizeof(CamFtpImageServerInfo))
			ret = icam_set_ftp_srv_info(hCamCtrl, (CamFtpImageServerInfo *)dataBuf);
		cmdHdr->dataLen = 0;
		break;
	case TC_GET_DEVICEINFO:
		ret = icam_get_dev_info(hCamCtrl, (CamDeviceInfo *)dataBuf);
		cmdHdr->dataLen = sizeof(CamDeviceInfo);
		break;
	case TC_SET_DEVICEINFO:
		if(cmdHdr->dataLen == sizeof(CamDeviceInfo))
			ret = icam_set_dev_info(hCamCtrl, (CamDeviceInfo *)dataBuf);
		cmdHdr->dataLen = 0;
		break;
	case TC_GET_ROADINFO:
		ret = icam_get_road_info(hCamCtrl, (CamRoadInfo *)dataBuf);
		cmdHdr->dataLen = sizeof(CamRoadInfo);
		break;
	case TC_SET_ROADINFO:
		if(cmdHdr->dataLen == sizeof(CamRoadInfo))
			ret = icam_set_road_info(hCamCtrl, (CamRoadInfo *)dataBuf);
		cmdHdr->dataLen = 0;
		break;
	case TC_GET_RTPPARAMS:
		ret = icam_get_rtp_params(hCamCtrl, (CamRtpParams *)dataBuf);
		cmdHdr->dataLen = sizeof(CamRtpParams);
		break;
	case TC_SET_RTPPARAMS:
		if(cmdHdr->dataLen == sizeof(CamRtpParams))
			ret = icam_set_rtp_params(hCamCtrl, (CamRtpParams *)dataBuf);
		cmdHdr->dataLen = 0;
		break;
	case TC_GET_EXPROSUREPARAM:
		ret = icam_get_exposure_params(hCamCtrl, (CamExprosureParam *)dataBuf);
		cmdHdr->dataLen = sizeof(CamExprosureParam);
		break;
	case TC_SET_EXPROSUREPARAM:
		if(cmdHdr->dataLen == sizeof(CamExprosureParam))
			ret = icam_set_exposure_params(hCamCtrl, (CamExprosureParam *)dataBuf);
		cmdHdr->dataLen = 0;
		break;
	case TC_GET_RGBGAIN:
		ret = icam_get_rgb_gains(hCamCtrl, (CamRGBGains *)dataBuf);
		cmdHdr->dataLen = sizeof(CamRGBGains);
		break;
	case TC_SET_RGBGAIN:
		if(cmdHdr->dataLen == sizeof(CamRGBGains))
			ret = icam_set_rgb_gains(hCamCtrl, (CamRGBGains *)dataBuf);
		cmdHdr->dataLen = 0;
		break;
	case TC_GET_DRCPARAM:
		ret = icam_get_drc_params(hCamCtrl, (CamDRCParam *)dataBuf);
		cmdHdr->dataLen = sizeof(CamDRCParam);
		break;
	case TC_SET_DRCPARAM:
		if(cmdHdr->dataLen == sizeof(CamDRCParam))
			ret = icam_set_drc_params(hCamCtrl, (CamDRCParam *)dataBuf);
		cmdHdr->dataLen = 0;
		break;
	case TC_GET_JPEGCOMPRESSION:
		ret = icam_get_img_enc_params(hCamCtrl, (CamImgEncParams *)dataBuf);
		cmdHdr->dataLen = sizeof(Int32);
		break;
	case TC_SET_JPEGCOMPRESSION:
		if(cmdHdr->dataLen == sizeof(CamImgEncParams))
			ret = icam_set_img_enc_params(hCamCtrl, (CamImgEncParams *)dataBuf);
		cmdHdr->dataLen = 0;
		break;
	case TC_GET_LUT_DAY:
	case TC_SET_LUT_DAY:
	case TC_GET_LUT_NIGHT:
	case TC_SET_LUT_NIGHT:	
		ret = E_UNSUPT;
		break;
	case TC_GET_H264_ENC_PARAMS:
		ret = icam_get_h264_params(hCamCtrl, (CamH264Params *)dataBuf);
		cmdHdr->dataLen = sizeof(CamH264Params);
		break;
	case TC_SET_H264_ENC_PARAMS:
		if(cmdHdr->dataLen == sizeof(CamH264Params))
			ret = icam_set_h264_params(hCamCtrl, (CamH264Params *)dataBuf);
		cmdHdr->dataLen = 0;
		break;
	case TC_GET_AV_TYPE:
		ret = icam_get_av_params(hCamCtrl, (CamAVParam *)dataBuf);
		cmdHdr->dataLen = sizeof(CamAVParam);
		break;
	case TC_SET_AV_TYPE:
		if(cmdHdr->dataLen == sizeof(CamAVParam))
			ret = icam_set_av_params(hCamCtrl, (CamAVParam *)dataBuf);
		cmdHdr->dataLen = 0;
		break;
	case TC_GET_TRAFFIC_LIGHT_REGS:
		ret = icam_get_traffic_light_regions(hCamCtrl, (CamTrafficLightRegions *)dataBuf);
		cmdHdr->dataLen = sizeof(CamTrafficLightRegions);
		break;
	case TC_SET_TRAFFIC_LIGHT_REGS:
		if(cmdHdr->dataLen == sizeof(CamTrafficLightRegions))
			ret = icam_set_traffic_light_regions(hCamCtrl, (CamTrafficLightRegions *)dataBuf);
		cmdHdr->dataLen = 0;
		break;
	case TC_GET_IMG_ENHANCE_PARAMS:
		ret = icam_get_img_enhance_params(hCamCtrl, (CamImgEnhanceParams *)dataBuf);
		cmdHdr->dataLen = sizeof(CamImgEnhanceParams);
		break;
	case TC_SET_IMG_ENHANCE_PARAMS:
		if(cmdHdr->dataLen == sizeof(CamImgEnhanceParams))
			ret = icam_set_img_enhance_params(hCamCtrl, (CamImgEnhanceParams *)dataBuf);
		cmdHdr->dataLen = 0;
		break;
	case TC_GET_SPEC_CAP_PARAMS:
		ret = icam_get_spec_cap_params(hCamCtrl, (CamSpecCapParams *)dataBuf);
		cmdHdr->dataLen = sizeof(CamSpecCapParams);
		break;
	case TC_SET_SPEC_CAP_PARAMS:
		if(cmdHdr->dataLen == sizeof(CamSpecCapParams))
			ret = icam_set_spec_cap_params(hCamCtrl, (CamSpecCapParams *)dataBuf);
		cmdHdr->dataLen = 0;
		break;
	case TC_GET_WORKSTATUS:
		ret = icam_get_work_status(hCamCtrl, (CamWorkStatus *)dataBuf);
		cmdHdr->dataLen = sizeof(CamWorkStatus);
		break;
	case TC_GET_WORKMODE:
		ret = icam_get_work_mode(hCamCtrl, (CamWorkMode *)dataBuf);
		cmdHdr->dataLen = sizeof(CamWorkMode);
		break;
	case TC_SET_WORKMODE:
		if(cmdHdr->dataLen == sizeof(CamWorkMode))
			ret = icam_set_work_mode(hCamCtrl, (CamWorkMode *)dataBuf);
		cmdHdr->dataLen = 0;
		break;
	case TC_GET_IOINFO:
		ret = icam_get_io_config(hCamCtrl, (CamIoCfg *)dataBuf);
		cmdHdr->dataLen = sizeof(CamWorkMode);
		break;
	case TC_SET_IOINFO:
		if(cmdHdr->dataLen == sizeof(CamIoCfg))
			ret = icam_set_io_config(hCamCtrl, (CamIoCfg *)dataBuf);
		cmdHdr->dataLen = 0;
		break;
	case TC_GET_STROBEPARAM:
		ret = icam_get_strobe_params(hCamCtrl, (CamStrobeCtrlParam *)dataBuf);
		cmdHdr->dataLen = sizeof(CamStrobeCtrlParam);
		break;
	case TC_SET_STROBEPARAM:
		if(cmdHdr->dataLen == sizeof(CamStrobeCtrlParam))
			ret = icam_set_strobe_params(hCamCtrl, (CamStrobeCtrlParam *)dataBuf);
		cmdHdr->dataLen = 0;
		break;
	case TC_GET_DETECTORPARAM:
		ret = icam_get_detector_params(hCamCtrl, (CamDetectorParam *)dataBuf);
		cmdHdr->dataLen = sizeof(CamDetectorParam);
		break;
	case TC_SET_DETECTORPARAM:
		if(cmdHdr->dataLen == sizeof(CamDetectorParam))
			ret = icam_set_detector_params(hCamCtrl, (CamDetectorParam *)dataBuf);
		cmdHdr->dataLen = 0;
		break;
	case TC_GET_AUTOEXPROSUREPARAM:
		ret = icam_get_ae_params(hCamCtrl, (CamAEParam *)dataBuf);
		cmdHdr->dataLen = sizeof(CamAEParam);
		break;
	case TC_SET_AUTOEXPROSUREPARAM:
		if(cmdHdr->dataLen == sizeof(CamAEParam))
			ret = icam_set_ae_params(hCamCtrl, (CamAEParam *)dataBuf);
		cmdHdr->dataLen = 0;
		break;
	case TC_GET_AUTOWHITEBALANCEPARAM:
		ret = icam_get_awb_params(hCamCtrl, (CamAWBParam *)dataBuf);
		cmdHdr->dataLen = sizeof(CamAWBParam);
		break;
	case TC_SET_AUTOWHITEBALANCEPARAM:
		if(cmdHdr->dataLen == sizeof(CamAWBParam))
			ret = icam_set_awb_params(hCamCtrl, (CamAWBParam *)dataBuf);
		cmdHdr->dataLen = 0;
		break;
	case TC_GET_DAYNIGHTCHANGETIME:
		ret = icam_get_day_night_mode_params(hCamCtrl, (CamDayNightModeCfg *)dataBuf);
		cmdHdr->dataLen = sizeof(CamDayNightModeCfg);
		break;
	case TC_SET_DAYNIGHTCHANGETIME:
		if(cmdHdr->dataLen == sizeof(CamDayNightModeCfg))
			ret = icam_set_day_night_mode_params(hCamCtrl, (CamDayNightModeCfg *)dataBuf);
		cmdHdr->dataLen = 0;
		break;
	case TC_GET_SD_ROOT_PATH:
		ret = icam_get_sd_root_path(hCamCtrl, dataBuf, &bufLen);
		cmdHdr->dataLen = bufLen;
		break;
	case TC_GET_SD_DIR_INFO:
		ret = icam_get_sd_dir_info(hCamCtrl, dataBuf, dataBuf, &bufLen);
		cmdHdr->dataLen = bufLen;
		break;
	case TC_SET_RD_SD_FILES_BY_DIR:
	case TC_SET_DEL_SD_DIR:
	case TC_SET_FORMAT_SD:
	case TC_GET_RESOLUTION:
	default:
		ret = E_UNSUPT;
		break;
	}

	/* If opt is failed, return no additive data */
	if(ret) 
		cmdHdr->dataLen = 0;

	return ret;
}

/*****************************************************************************
 Prototype    : cam_ctrl_thread
 Description  : thread for ctrl cam
 Input        : void *arg  
 Output       : None
 Return Value : void
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/23
    Author       : Sun
    Modification : Created function

*****************************************************************************/
void *cam_ctrl_thread(void *arg)
{
	CamCtrlThrParams 	*params = arg;
	Int32				ret;
	TcpCmdHeader		cmdHdr;
	void				*dataBuf = params->dataBuf;
	Int32				bufLen = params->bufLen;
	int					sock = params->sock;
	Int32				errCnt = 0;
	Bool				reboot = FALSE;
	
	assert(params && params->hCamCtrl && params->mutex && params->dataBuf);

	/* init thread */
	ret = thread_init(params);
	if(ret) {
		goto exit;
	}

	/* start loop until connect id is changed */
	while(s_curThread == pthread_self() && errCnt < MAX_ERR_CNT) {
		/* recv and process cmd */
		ret = tcp_cmd_recv(sock, &cmdHdr, dataBuf, bufLen);

		if(!ret) {
			/* process cmd */
			ret = ctrl_cmd_process(params->hCamCtrl, &cmdHdr, dataBuf, bufLen);
			if(ret == E_REBOOT)
				reboot = TRUE;

			/* reply cmd */
			ret = tcp_cmd_reply(sock, &cmdHdr, dataBuf, ret);
			if(ret) {
				errCnt++;
			}
		} else if(ret == E_CONNECT)
			break;	//connection error directly break
		else
			errCnt++;
		
	}

	if(s_curThread == pthread_self()) {
		/* set back to no connection */
		set_cur_thr_id(0, params->mutex);
	}

exit:

	free(arg);
	close(sock);

	if(ret == E_REBOOT || reboot)
		system("shutdown -r now");

	pthread_exit(0);
	
}

