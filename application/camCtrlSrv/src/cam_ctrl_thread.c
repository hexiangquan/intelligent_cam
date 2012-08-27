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
#include <linux/types.h>
#include "net_cmds.h"
#include "crc16.h"
#include "sd_ctrl.h"

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

#define		CMD_FLAG_USE_PARAM0		(1 << 0)
#define 	CMD_FLAG_USE_PARAM1		(1 << 1)
#define		CMD_FLAG_CHECKSUM		(1 << 2)

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

typedef struct {
	Uint32			tcpCmd;			//cmd from client
	ICamCtrlCmd		camCmd;			//camera ctrl cmd
	Int32			flags;			//flags for data parse
	Int32			requLen;		//data len needed for request, if set to -1, means variable length
	Int32			respLen;		//data len needed for reply, if set to -1, means variable length
}CmdInfo;

const static CmdInfo s_cmdInfo[] = {
	/* capture ctrl */
	{.tcpCmd = TC_FUN_CAPTURECTRL, .camCmd = ICAMCMD_S_CAPCTRL, 
	 .flags = CMD_FLAG_USE_PARAM0, .requLen = sizeof(Int32), .respLen = 0,},
	/* restore default config params  */
	{.tcpCmd = TC_FUN_RESTORETODEFAULT, .camCmd = ICAMCMD_S_RESTORECFG, 
	 .flags = 0, .requLen = 0, .respLen = 0,},
	/* get firmware version */
	{.tcpCmd = TC_GET_VERINFO, .camCmd = ICAMCMD_G_VERSION, 
	 .flags = 0, .requLen = 0, .respLen = sizeof(CamVersionInfo),},
	/* get temperature */
	{.tcpCmd = TC_GET_TEMPERATURE, .camCmd = ICAMCMD_G_TEMP, 
	 .flags = 0, .requLen = 0, .respLen = sizeof(Int32),}, 
	/* get system time */
	{.tcpCmd = TC_GET_TIME, .camCmd = ICAMCMD_G_DATETIME, 
	 .flags = 0, .requLen = 0, .respLen = sizeof(CamDateTime),},
	/* set system time */
	{.tcpCmd = TC_SET_TIME, .camCmd = ICAMCMD_S_DATETIME, 
	 .flags = 0, .requLen = sizeof(CamDateTime), .respLen = 0,},
	/* get network info */
	{.tcpCmd = TC_GET_NETWORKINFO, .camCmd = ICAMCMD_G_NETWORKINFO, 
	 .flags = 0, .requLen = 0, .respLen = sizeof(CamNetworkInfo),},
	/* set network info */
	{.tcpCmd = TC_SET_NETWORKINFO, .camCmd = ICAMCMD_S_NETWORKINFO, 
	 .flags = 0, .requLen = sizeof(CamNetworkInfo), .respLen = 0,},
	/* get osd params */
	{.tcpCmd = TC_GET_OSDINFO, .camCmd = ICAMCMD_G_OSDPARAMS, 
	 .flags = 0, .requLen = 0, .respLen = sizeof(CamOsdParams),},
	/* set osd params */
	{.tcpCmd = TC_SET_OSDINFO, .camCmd = ICAMCMD_S_OSDPARAMS, 
	 .flags = 0, .requLen = sizeof(CamOsdParams), .respLen = 0,},
	/* get upload protocol */
	{.tcpCmd = TC_GET_IMAGEUPLOADPROTOCOL, .camCmd = ICAMCMD_G_UPLOADPROTO, 
	 .flags = 0, .requLen = 0, .respLen = sizeof(CamImgUploadCfg),},
	/* set upload protocol */
	{.tcpCmd = TC_SET_IMAGEUPLOADPROTOCOL, .camCmd = ICAMCMD_S_UPLOADPROTO, 
	 .flags = 0, .requLen = sizeof(CamImgUploadCfg), .respLen = 0,}, 
	/* get ntp server info */
	{.tcpCmd = TC_GET_NTPSERVERINFO, .camCmd = ICAMCMD_G_NTPSRVINFO, 
	 .flags = 0, .requLen = 0, .respLen = sizeof(CamNtpServerInfo),},
	/* set ntp server info */
	{.tcpCmd = TC_SET_NTPSERVERINFO, .camCmd = ICAMCMD_S_NTPSRVINFO, 
	 .flags = 0, .requLen = sizeof(CamNtpServerInfo), .respLen = 0,}, 
	/* get tcp image server info */
	{.tcpCmd = TC_GET_TCPIMAGESERVERINFO, .camCmd = ICAMCMD_G_IMGSRVINFO, 
	 .flags = 0, .requLen = 0, .respLen = sizeof(CamTcpImageServerInfo),},
	/* set tcp image server info */
	{.tcpCmd = TC_SET_TCPIMAGESERVERINFO, .camCmd = ICAMCMD_S_IMGSRVINFO, 
	 .flags = 0, .requLen = sizeof(CamTcpImageServerInfo), .respLen = 0,}, 
	/* get ftp image server info */
	{.tcpCmd = TC_GET_FTPSERVERINFO, .camCmd = ICAMCMD_G_FTPSRVINFO, 
	 .flags = 0, .requLen = 0, .respLen = sizeof(CamFtpImageServerInfo),},
	/* set ftp image server info */
	{.tcpCmd = TC_SET_FTPSERVERINFO, .camCmd = ICAMCMD_S_FTPSRVINFO, 
	 .flags = 0, .requLen = sizeof(CamFtpImageServerInfo), .respLen = 0,},
	/* get dev info */
	{.tcpCmd = TC_GET_DEVICEINFO, .camCmd = ICAMCMD_G_DEVINFO, 
	 .flags = 0, .requLen = 0, .respLen = sizeof(CamDeviceInfo),},
	/* set dev info */
	{.tcpCmd = TC_SET_DEVICEINFO, .camCmd = ICAMCMD_S_DEVINFO, 
	 .flags = 0, .requLen = sizeof(CamDeviceInfo), .respLen = 0,}, 
	/* get road info */
	{.tcpCmd = TC_GET_ROADINFO, .camCmd = ICAMCMD_G_ROADINFO, 
	 .flags = 0, .requLen = 0, .respLen = sizeof(CamRoadInfo),},
	/* set road info */
	{.tcpCmd = TC_SET_ROADINFO, .camCmd = ICAMCMD_S_ROADINFO, 
	 .flags = 0, .requLen = sizeof(CamRoadInfo), .respLen = 0,},
	/* get rtp params */
	{.tcpCmd = TC_GET_RTPPARAMS, .camCmd = ICAMCMD_G_RTPPARAMS, 
	 .flags = 0, .requLen = 0, .respLen = sizeof(CamRtpParams),},
	/* set rtp params */
	{.tcpCmd = TC_SET_RTPPARAMS, .camCmd = ICAMCMD_S_RTPPARAMS, 
	 .flags = 0, .requLen = sizeof(CamRtpParams), .respLen = 0,},
	/* get exposure params */
	{.tcpCmd = TC_GET_EXPROSUREPARAM, .camCmd = ICAMCMD_G_EXPPARAMS, 
	 .flags = 0, .requLen = 0, .respLen = sizeof(CamExprosureParam),},
	/* set exposure params */
	{.tcpCmd = TC_SET_EXPROSUREPARAM, .camCmd = ICAMCMD_S_EXPPARAMS, 
	 .flags = 0, .requLen = sizeof(CamExprosureParam), .respLen = 0,},
	/* get rgb gains */
	{.tcpCmd = TC_GET_RGBGAIN, .camCmd = ICAMCMD_G_RGBGAINS, 
	 .flags = 0, .requLen = 0, .respLen = sizeof(CamRGBGains),},
	/* set rgb gains */
	{.tcpCmd = TC_SET_RGBGAIN, .camCmd = ICAMCMD_S_RGBGAINS, 
	 .flags = 0, .requLen = sizeof(CamRGBGains), .respLen = 0,},
	/* get image enc params */
	{.tcpCmd = TC_GET_IMGENCPARAMS, .camCmd = ICAMCMD_G_IMGENCPARAMS, 
	 .flags = 0, .requLen = 0, .respLen = sizeof(CamImgEncParams),},
	/* set image enc params */
	{.tcpCmd = TC_SET_IMGENCPARAMS, .camCmd = ICAMCMD_S_IMGENCPARAMS, 
	 .flags = 0, .requLen = sizeof(CamImgEncParams), .respLen = 0,},
	/* get h.264 enc params */
	{.tcpCmd = TC_GET_H264_ENC_PARAMS, .camCmd = ICAMCMD_G_H264PARAMS, 
	 .flags = 0, .requLen = 0, .respLen = sizeof(CamH264Params),},
	/* set h.264 enc params */
	{.tcpCmd = TC_SET_H264_ENC_PARAMS, .camCmd = ICAMCMD_S_H264PARAMS, 
	 .flags = 0, .requLen = sizeof(CamH264Params), .respLen = 0,},
	/* get av type params */
	{.tcpCmd = TC_GET_AV_TYPE, .camCmd = ICAMCMD_G_AVPARAMS, 
	 .flags = 0, .requLen = 0, .respLen = sizeof(CamAVParam),},
	/* set av type params */
	{.tcpCmd = TC_SET_AV_TYPE, .camCmd = ICAMCMD_S_AVPARAMS, 
	 .flags = 0, .requLen = sizeof(CamAVParam), .respLen = 0,},
	/* get traffic light regions */
	{.tcpCmd = TC_GET_TRAFFIC_LIGHT_REGS, .camCmd = ICAMCMD_G_TRAFLIGHTREG, 
	 .flags = 0, .requLen = 0, .respLen = sizeof(CamTrafficLightRegions),},
	/* set traffic light regions */
	{.tcpCmd = TC_SET_TRAFFIC_LIGHT_REGS, .camCmd = ICAMCMD_S_TRAFLIGHTREG, 
	 .flags = 0, .requLen = sizeof(CamTrafficLightRegions), .respLen = 0,},
	/* get img enhance params */
	{.tcpCmd = TC_GET_IMG_ENHANCE_PARAMS, .camCmd = ICAMCMD_G_IMGENHANCE, 
	 .flags = 0, .requLen = 0, .respLen = sizeof(CamImgEnhanceParams),},
	/* set img enhance params */
	{.tcpCmd = TC_SET_IMG_ENHANCE_PARAMS, .camCmd = ICAMCMD_S_IMGENHANCE, 
	 .flags = 0, .requLen = sizeof(CamImgEnhanceParams), .respLen = 0,},
	/* get special capture params */
	{.tcpCmd = TC_GET_SPEC_CAP_PARAMS, .camCmd = ICAMCMD_G_SPECCAPPARAMS, 
	 .flags = 0, .requLen = 0, .respLen = sizeof(CamSpecCapParams),},
	/* set special capture params */
	{.tcpCmd = TC_SET_SPEC_CAP_PARAMS, .camCmd = ICAMCMD_S_SPECCAPPARAMS, 
	 .flags = 0, .requLen = sizeof(CamSpecCapParams), .respLen = 0,},
	/* get work status */
	{.tcpCmd = TC_GET_WORKSTATUS, .camCmd = ICAMCMD_G_WORKSTATUS, 
	 .flags = 0, .requLen = 0, .respLen = sizeof(Int32),},
	/* get work mode */
	{.tcpCmd = TC_GET_WORKMODE, .camCmd = ICAMCMD_G_WORKMODE, 
	 .flags = 0, .requLen = 0, .respLen = sizeof(CamWorkMode),},
	/* set work mode */
	{.tcpCmd = TC_SET_WORKMODE, .camCmd = ICAMCMD_S_WORKMODE, 
	 .flags = 0, .requLen = sizeof(CamWorkMode), .respLen = 0,},
	/* get input info */
	{.tcpCmd = TC_GET_INPUT_INFO, .camCmd = ICAMCMD_G_INPUTINFO, 
	 .flags = 0, .requLen = 0, .respLen = sizeof(CamInputInfo),},
	/* get gpio info */
	{.tcpCmd = TC_GET_IOINFO, .camCmd = ICAMCMD_G_IOCFG, 
	 .flags = 0, .requLen = 0, .respLen = sizeof(CamIoCfg),},
	/* set gpio info */
	{.tcpCmd = TC_SET_IOINFO, .camCmd = ICAMCMD_S_IOCFG, 
	 .flags = 0, .requLen = sizeof(CamIoCfg), .respLen = 0,},
	/* get strobe params */
	{.tcpCmd = TC_GET_STROBEPARAM, .camCmd = ICAMCMD_G_STROBEPARAMS, 
	 .flags = 0, .requLen = 0, .respLen = sizeof(CamStrobeCtrlParam),},
	/* set strobe params */
	{.tcpCmd = TC_SET_STROBEPARAM, .camCmd = ICAMCMD_S_STROBEPARAMS, 
	 .flags = 0, .requLen = sizeof(CamStrobeCtrlParam), .respLen = 0,},
	/* get vehicle detector params */
	{.tcpCmd = TC_GET_DETECTORPARAM, .camCmd = ICAMCMD_G_DETECTORPARAMS, 
	 .flags = 0, .requLen = 0, .respLen = sizeof(CamDetectorParam),},
	/* set vehicle detector params */
	{.tcpCmd = TC_SET_DETECTORPARAM, .camCmd = ICAMCMD_S_DETECTORPARAMS, 
	 .flags = 0, .requLen = sizeof(CamDetectorParam), .respLen = 0,},
	/* get AE params */
	{.tcpCmd = TC_GET_AUTOEXPROSUREPARAM, .camCmd = ICAMCMD_G_AEPARAMS, 
	 .flags = 0, .requLen = 0, .respLen = sizeof(CamAEParam),},
	/* set AE params */
	{.tcpCmd = TC_SET_AUTOEXPROSUREPARAM, .camCmd = ICAMCMD_S_AEPARAMS, 
	 .flags = 0, .requLen = sizeof(CamAEParam), .respLen = 0,},
	/* get AWB params */
	{.tcpCmd = TC_GET_AUTOWHITEBALANCEPARAM, .camCmd = ICAMCMD_G_AWBPARAMS, 
	 .flags = 0, .requLen = 0, .respLen = sizeof(CamAWBParam),},
	/* set AWB params */
	{.tcpCmd = TC_SET_AUTOWHITEBALANCEPARAM, .camCmd = ICAMCMD_S_AWBPARAMS, 
	 .flags = 0, .requLen = sizeof(CamAWBParam), .respLen = 0,},
	/* get Day night mode params */
	{.tcpCmd = TC_GET_DAYNIGHTMODEPARAMS, .camCmd = ICAMCMD_G_DAYNIGHTMODE, 
	 .flags = 0, .requLen = 0, .respLen = sizeof(CamDayNightModeCfg),},
	/* set Day night mode params */
	{.tcpCmd = TC_SET_DAYNIGHTMODEPARAMS, .camCmd = ICAMCMD_S_DAYNIGHTMODE, 
	 .flags = 0, .requLen = sizeof(CamDayNightModeCfg), .respLen = 0,},
	/* Update fpga, just send msg */
	{.tcpCmd = TC_FUN_UPDATE_FPGA, .camCmd = ICAMCMD_S_UPDATE_FPGA, 
	 .flags = 0, .requLen = 0, .respLen = 0,},
	/* send sd dir */
	{.tcpCmd = TC_SET_RD_SD_DIR, .camCmd = ICAMCMD_S_SNDDIR, 
	 .flags = 0, .requLen = -1, .respLen = 0,},
	/* send sd file */
	{.tcpCmd = TC_SET_RD_SD_FILE, .camCmd = ICAMCMD_S_SNDFILE, 
	 .flags = 0, .requLen = -1, .respLen = 0,},
};

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
	params->reboot = FALSE;

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
		params->reboot = TRUE;
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
	cmdHdr.checkSum = 0;
	tcp_cmd_reply(sock, &cmdHdr, NULL, result);
	return ret;
}

/*****************************************************************************
 Prototype    : cam_cmd_process
 Description  : process cam cmd
 Input        : ICamCtrlHandle hCamCtrl  
                TcpCmdHeader *cmdHdr     
                void *dataBuf            
                Int32 bufLen             
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 cam_cmd_process(ICamCtrlHandle hCamCtrl, TcpCmdHeader *cmdHdr, Int8 *dataBuf, Int32 bufLen)
{
	
	Int32			index;
	const CmdInfo 	*cmdInfo = NULL;
	MsgHeader		*msgHdr = (MsgHeader *)dataBuf;
	Int32		    *data = (Int32 *)(dataBuf + sizeof(MsgHeader));

	/* find cmd info */
	for(index = 0; index < ARRAY_SIZE(s_cmdInfo); index++) {
		if(cmdHdr->cmd == s_cmdInfo[index].tcpCmd) {
			cmdInfo = &s_cmdInfo[index];
			break;
		}
	}

	if(!cmdInfo) {
		/* unsupport cmd */
		ERR("unsupported cmd: 0x%04X", (__u32)cmdHdr->cmd);
		return E_UNSUPT;
	}

	if(cmdInfo->requLen > 0 && cmdInfo->requLen != cmdHdr->dataLen) {
		ERR("invalid request len %d for cmd: 0x%04X, need: %d", 
			cmdHdr->dataLen, (__u32)cmdHdr->cmd, cmdInfo->requLen);
		return E_INVAL;
	}

	msgHdr->cmd = cmdInfo->camCmd;
	msgHdr->index = cmdHdr->serial;
	msgHdr->dataLen = cmdHdr->dataLen;

	if(cmdInfo->flags & CMD_FLAG_CHECKSUM) {
		/* do checksum */
	}
	
	if(cmdInfo->flags & CMD_FLAG_USE_PARAM0)
		msgHdr->param[0] = *data++;
	if(cmdInfo->flags & CMD_FLAG_USE_PARAM1)
		msgHdr->param[1] = *data++;

	/* call cam ctrl */
	Int32 ret = icam_ctrl_run(hCamCtrl, msgHdr, bufLen);

	if(ret) {
		ERR("cmd 0x%04X process err: %d", (__u32)cmdHdr->cmd, ret);
	} else if(cmdInfo->respLen > 0 && cmdInfo->respLen != msgHdr->dataLen) {
		ERR("invalid resp len %d for cmd: 0x%04X, need: %d", 
			msgHdr->dataLen, (__u32)cmdHdr->cmd, cmdInfo->respLen);
		return E_TRANS;
	}

	/* we only need to set reply data len */
	cmdHdr->dataLen = msgHdr->dataLen;
	cmdHdr->checkSum = 0;
	return ret;
}

/*****************************************************************************
 Prototype    : firmware_update
 Description  : update firmware file
 Input        : const char *name  
                const void *data  
                Int32 len         
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/5/2
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 firmware_update(const char *name, const void *data, Int32 len, Uint32 checksum)
{
	FILE 	*fp;
	Int32 	err;
	Int8 	*buf = NULL;

	Uint32	crc = crc16(data, len);

	if(crc != checksum) {
		ERR("check sum: 0x%04X diff of cmd hdr: 0x%04X, len: %d", crc, checksum, len);
		return E_CHECKSUM;
	}

	/* open file */
	fp = fopen(name, "wb");
	if(!fp) {
		ERRSTR("can't open %s for write", name);
		return E_IO;
	}

	DBG("open file %s for update", name);

	/* set to start */
	fseek(fp, 0, SEEK_SET);

	/* write data */
	const Int8 *firmware = (const Int8 *)data + UPDATE_DATA_OFFSET;
	size_t wrLen = len - UPDATE_DATA_OFFSET;
	err = fwrite(firmware, wrLen, 1, fp);
	if(err < 0) {
		ERRSTR("write data failed");
		err = E_IO;
		goto exit;
	}

	/* sync to hard disk & close file */
	fclose(fp);
	fp = NULL;
	sync();
	
#if 0
	/* open for read */
	fp = fopen(name, "rb");
	if(!fp) {
		ERRSTR("can't open %s for read", name);
		return E_IO;
	}

	/* read back & compare */
	buf = malloc(len);
	if(!buf) {
		ERR("alloc buf for cmp failed");
		err = E_NOMEM;
		goto exit;
	}

	/* go back to start */
	fseek(fp, 0, SEEK_SET);
	err = fread(buf, len, 1, fp);
	if(err < 0) {
		ERRSTR("read data failed");
		err = E_IO;
		goto exit;
	}

	/* compare data */
	if(memcmp(data + UPDATE_DATA_OFFSET, buf, len)) {
		ERR("data cmp failed");
		err = E_CHECKSUM;
		goto exit;
	}
#endif

	err = E_NO;
	DBG("update %s success...", name);

exit:	

	if(fp)
		fclose(fp);

	if(buf)
		free(buf);		

	return err;

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
static Int32 ctrl_cmd_process(ICamCtrlHandle hCamCtrl, TcpCmdHeader *cmdHdr, CamCtrlThrParams *params)
{
	Int32 	ret = E_INVAL;
	Int8    *data = (Int8 *)(params->dataBuf) + sizeof(MsgHeader);

	switch(cmdHdr->cmd) {
	case TC_FUN_CAMERARESET:
		ret = E_REBOOT;
		cmdHdr->dataLen = 0;
		break;
	case TC_FUN_HEARTBEATSINGAL:
		ret = E_NO;
		cmdHdr->dataLen = 0;
		break;
	case TC_FUN_UPDATE_DSP:
		ret = dsp_update(data, cmdHdr->dataLen, cmdHdr->checkSum);
		cmdHdr->dataLen = 0;
		break;
	case TC_FUN_UPDATE_ARM: {
		char fname[128];
		if(strncmp(data, "lib", 3))	
			snprintf(fname, sizeof(fname), "/home/root/update/%s", data + UPDATE_NAME_OFFSET);
		else {
			snprintf(fname, sizeof(fname), "/usr/lib/%s", data + UPDATE_NAME_OFFSET);
		}
		ret = firmware_update(fname, data, cmdHdr->dataLen, cmdHdr->checkSum);
		cmdHdr->dataLen = 0;
		if(ret == E_NO)
			params->reboot = TRUE;
		break;
	}
	case TC_FUN_UPDATE_FPGA:
		ret = firmware_update(params->fpgaFirmware, data, cmdHdr->dataLen, cmdHdr->checkSum);
		cmdHdr->dataLen = 0;
		/* if file update ok, tell icam prog to reload */
		if(ret == E_NO)
			params->reboot = TRUE;
		break;
	case TC_GET_SD_ROOT_PATH:
		/* get path of sd mount dir */
		ret = sd_get_root_path(*(Int32 *)data, data, params->bufLen);
		if(ret > 0) {
			cmdHdr->dataLen = ret;
			ret = E_NO;
		}
		break;
	case TC_GET_SD_DIR_INFO:
		/* get path of sd mount dir */
		ret = sd_get_dir_info((char *)data, data, params->bufLen);
		if(ret > 0) {
			cmdHdr->dataLen = ret;
			ret = E_NO;
		}
		break;
	case TC_SET_DEL_SD_DIR:
		/* delete dir in sd card */
		ret = sd_del_dir((char *)data);
		cmdHdr->dataLen = 0;
		break;
	case TC_SET_FORMAT_SD:
		/* format sd card */
		ret = sd_format(*(Int32 *)data);
		cmdHdr->dataLen = 0;
		break;
	default:
		ret = cam_cmd_process(hCamCtrl, cmdHdr, params->dataBuf, params->bufLen);
		break;
	}

	/* If opt is failed, return no additive data */
	if(ret) 
		cmdHdr->dataLen = 0;
	cmdHdr->checkSum = 0;
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
	/* leave space for msg header */
	void				*dataBuf = (Int8 *)(params->dataBuf) + sizeof(MsgHeader);
	Int32				bufLen = params->bufLen - sizeof(MsgHeader);
	int					sock = params->sock;
	Int32				errCnt = 0;
	
	assert(params && params->hCamCtrl && params->mutex && params->dataBuf);

	DBG("cam ctrl thread %u start", (__u32)pthread_self());

	/* init thread */
	ret = thread_init(params);
	if(ret) {
		goto exit;
	}

	/* start loop until connect id is changed */
	while(s_curThread == pthread_self() && errCnt < MAX_ERR_CNT && !params->reboot) {
		/* recv and process cmd */
		ret = tcp_cmd_recv(sock, &cmdHdr, dataBuf, bufLen);

		if(!ret) {
			/* process cmd */
			ret = ctrl_cmd_process(params->hCamCtrl, &cmdHdr, params);

			/* reply cmd */
			ret = tcp_cmd_reply(sock, &cmdHdr, dataBuf, ret);
			if(ret)
				errCnt++;
			else 
				errCnt = 0;
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

	if(params->reboot)
		sleep(3);
	
	close(sock);

	if(params->reboot)
		system("shutdown -r now\n");

	free(arg);

	DBG("<%u> cam ctrl thread exit...", (__u32)pthread_self());
	pthread_exit(0);
	
}

