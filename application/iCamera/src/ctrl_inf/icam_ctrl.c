#include "icam_ctrl.h"
#include "msg.h"
#include "log.h"
#include <pthread.h>

/* object of this module */
struct ICamCtrlObj {
	MsgHandle	hMsg;
	Int32		flags;
	Int32		cmdIndex;
	pthread_mutex_t mutex;
};


/*****************************************************************************
 Prototype    : icam_ctrl_create
 Description  : create cam ctrl handle
 Input        : const char *pathName  
                Int32 flags           
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
ICamCtrlHandle icam_ctrl_create(const char *pathName, Int32 flags, Int32 transTimeout)
{
	ICamCtrlHandle 	hCtrl;
	Int32			msgFlags = 0;

	hCtrl = calloc(1, sizeof(struct ICamCtrlObj));
	if(!hCtrl) {
		ERR("alloc mem failed.");
		return NULL;
	}

	if(flags & ICAM_FLAG_NONBLOCK) {
		msgFlags |= MSG_FLAG_NONBLK;
	}

	hCtrl->hMsg = msg_create(pathName, ICAM_MSG_NAME, msgFlags);

	if(!hCtrl->hMsg) {
		ERR("create msg failed.");
		free(hCtrl);
		return NULL;
	}

	if(transTimeout > 0) {
		msg_set_send_timeout(hCtrl->hMsg, transTimeout);
		msg_set_recv_timeout(hCtrl->hMsg, transTimeout);
	}
	
	pthread_mutex_init(&hCtrl->mutex, NULL);

	return hCtrl;
}

/*****************************************************************************
 Prototype    : icam_ctrl_delete
 Description  : delete cam ctrl handle
 Input        : ICamCtrlHandle hCtrl  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_ctrl_delete(ICamCtrlHandle hCtrl)
{
	if(!hCtrl) {
		return E_INVAL;
	}

	if(hCtrl->hMsg)
		msg_delete(hCtrl->hMsg);

	pthread_mutex_destroy(&hCtrl->mutex);

	free(hCtrl);

	return E_NO;
}

/*****************************************************************************
 Prototype    : icam_send_cmd
 Description  : send cmd and wait response
 Input        : ICamCtrlHandle hCtrl  
                Uint16 cmd            
                void *data            
                Int32 dataLen         
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 icam_send_cmd(ICamCtrlHandle hCtrl, Uint16 cmd, void *data, Int32 sndLen, Int32 rcvLen)
{
	MsgHeader   *hdr = (MsgHeader *)data;
	Int32 		err;

	/* fill header fileds */
	hdr->type = MSG_TYPE_REQU;
	hdr->index = hCtrl->cmdIndex++;
	hdr->dataLen = sndLen - sizeof(MsgHeader);
	hdr->cmd = cmd;

	/* send msg */
	err = msg_send(hCtrl->hMsg, NULL, hdr, 0);
	if(err)
		return err;

	/* wait response */
	err = msg_recv(hCtrl->hMsg, data, rcvLen, 0);

	if(err < 0)
		return err;

	/* validate data */
	if(hdr->cmd != cmd || hdr->type != MSG_TYPE_RESP)
		return E_TRANS;

	return E_NO;
}

/*****************************************************************************
 Prototype    : icam_get_version
 Description  : get software version
 Input        : ICamCtrlHandle hCtrl     
                CamVersionInfo *version  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_get_version(ICamCtrlHandle hCtrl, CamVersionInfo *buf)
{
	struct {
		MsgHeader 		hdr;
		CamVersionInfo	version;
	}verMsg;

	Int32 ret;

	if(!hCtrl || !buf)
		return E_INVAL;

	ret = icam_send_cmd(hCtrl, ICAMCMD_G_VERSION, &verMsg, sizeof(MsgHeader), sizeof(verMsg));
	if(ret)
		return ret;

	*buf = verMsg.version;

	/* result of this cmd */
	return verMsg.hdr.param[0];
	
}

/*****************************************************************************
 Prototype    : icam_get_work_status
 Description  : get work status
 Input        : ICamCtrlHandle hCtrl   
                CamWorkStatus *status  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_get_work_status(ICamCtrlHandle hCtrl, CamWorkStatus *buf)
{
	struct {
		MsgHeader 		hdr;
		CamWorkStatus	status;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !buf)
		return E_INVAL;

	ret = icam_send_cmd(hCtrl, ICAMCMD_G_WORKSTATUS, &msgBuf, sizeof(MsgHeader), sizeof(msgBuf));
	if(ret)
		return ret;

	*buf = msgBuf.status;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
	
}

/*****************************************************************************
 Prototype    : icam_get_input_info
 Description  : get input info
 Input        : ICamCtrlHandle hCtrl  
                CamInputInfo *buf     
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_get_input_info(ICamCtrlHandle hCtrl, CamInputInfo *buf)
{
	struct {
		MsgHeader 		hdr;
		CamInputInfo 	info;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !buf)
		return E_INVAL;

	ret = icam_send_cmd(hCtrl, ICAMCMD_G_INPUTINFO, &msgBuf, sizeof(MsgHeader), sizeof(msgBuf));
	if(ret)
		return ret;

	*buf = msgBuf.info;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
	
}



/*****************************************************************************
 Prototype    : icam_get_time
 Description  : get cam time
 Input        : ICamCtrlHandle hCtrl  
                CamDateTime *buf      
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_get_time(ICamCtrlHandle hCtrl, CamDateTime *buf)
{
	struct {
		MsgHeader 		hdr;
		CamDateTime		dateTime;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !buf)
		return E_INVAL;

	ret = icam_send_cmd(hCtrl, ICAMCMD_G_DATETIME, &msgBuf, sizeof(MsgHeader), sizeof(msgBuf));
	if(ret)
		return ret;

	*buf = msgBuf.dateTime;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
	
}

/*****************************************************************************
 Prototype    : icam_set_time
 Description  : set time to cam
 Input        : ICamCtrlHandle hCtrl         
                const CamDateTime *dateTime  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_set_time(ICamCtrlHandle hCtrl, const CamDateTime *dateTime)
{
	struct {
		MsgHeader 		hdr;
		CamDateTime		dateTime;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !dateTime)
		return E_INVAL;

	msgBuf.dateTime = *dateTime;

	ret = icam_send_cmd(hCtrl, ICAMCMD_S_DATETIME, &msgBuf, sizeof(msgBuf), sizeof(msgBuf));
	if(ret)
		return ret;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_get_network_info
 Description  : get network info
 Input        : ICamCtrlHandle hCtrl  
                CamNetworkInfo *buf   
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_get_network_info(ICamCtrlHandle hCtrl, CamNetworkInfo *buf)
{
	struct {
		MsgHeader 		hdr;
		CamNetworkInfo	info;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !buf)
		return E_INVAL;

	ret = icam_send_cmd(hCtrl, ICAMCMD_G_NETWORKINFO, &msgBuf, sizeof(MsgHeader), sizeof(msgBuf));
	if(ret)
		return ret;

	*buf = msgBuf.info;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
	
}

/*****************************************************************************
 Prototype    : icam_set_network_info
 Description  : set network info
 Input        : ICamCtrlHandle hCtrl        
                const CamNetworkInfo *info  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_set_network_info(ICamCtrlHandle hCtrl, const CamNetworkInfo *info)
{
	struct {
		MsgHeader 		hdr;
		CamNetworkInfo	info;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !info)
		return E_INVAL;

	msgBuf.info = *info;

	ret = icam_send_cmd(hCtrl, ICAMCMD_S_NETWORKINFO, &msgBuf, sizeof(msgBuf), sizeof(msgBuf));
	if(ret)
		return ret;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_get_dev_info
 Description  : get device info
 Input        : ICamCtrlHandle hCtrl  
                CamDeviceInfo *buf    
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_get_dev_info(ICamCtrlHandle hCtrl, CamDeviceInfo *buf)
{
	struct {
		MsgHeader 		hdr;
		CamDeviceInfo	info;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !buf)
		return E_INVAL;

	ret = icam_send_cmd(hCtrl, ICAMCMD_G_DEVINFO, &msgBuf, sizeof(MsgHeader), sizeof(msgBuf));
	if(ret)
		return ret;

	*buf = msgBuf.info;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
	
}

/*****************************************************************************
 Prototype    : icam_set_dev_info
 Description  : set device info
 Input        : ICamCtrlHandle hCtrl       
                const CamDeviceInfo *info  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_set_dev_info(ICamCtrlHandle hCtrl, const CamDeviceInfo *info)
{
	struct {
		MsgHeader 		hdr;
		CamDeviceInfo	info;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !info)
		return E_INVAL;

	msgBuf.info = *info;
	ret = icam_send_cmd(hCtrl, ICAMCMD_S_DEVINFO, &msgBuf, sizeof(msgBuf), sizeof(msgBuf));
	if(ret)
		return ret;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_get_osd_params
 Description  : get osd params
 Input        : ICamCtrlHandle hCtrl  
                CamOsdParams *buf     
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_get_osd_params(ICamCtrlHandle hCtrl, CamOsdParams *buf)
{
	struct {
		MsgHeader 		hdr;
		CamOsdParams	params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !buf)
		return E_INVAL;

	ret = icam_send_cmd(hCtrl, ICAMCMD_G_OSDPARAMS, &msgBuf, sizeof(MsgHeader), sizeof(msgBuf));
	if(ret)
		return ret;

	*buf = msgBuf.params;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_set_osd_params
 Description  : set osd params
 Input        : ICamCtrlHandle hCtrl        
                const CamOsdParams *params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_set_osd_params(ICamCtrlHandle hCtrl, const CamOsdParams *params)
{
	struct {
		MsgHeader 		hdr;
		CamOsdParams	params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !params)
		return E_INVAL;

	msgBuf.params = *params;
	ret = icam_send_cmd(hCtrl, ICAMCMD_S_OSDPARAMS, &msgBuf, sizeof(msgBuf), sizeof(msgBuf));
	if(ret)
		return ret;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_get_road_info
 Description  : get road info
 Input        : ICamCtrlHandle hCtrl  
                CamRoadInfo *buf      
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_get_road_info(ICamCtrlHandle hCtrl, CamRoadInfo *buf)
{
	struct {
		MsgHeader 		hdr;
		CamRoadInfo		info;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !buf)
		return E_INVAL;

	ret = icam_send_cmd(hCtrl, ICAMCMD_G_ROADINFO, &msgBuf, sizeof(MsgHeader), sizeof(msgBuf));
	if(ret)
		return ret;

	*buf = msgBuf.info;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_set_road_info
 Description  : set road info
 Input        : ICamCtrlHandle hCtrl     
                const CamRoadInfo *info  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_set_road_info(ICamCtrlHandle hCtrl, const CamRoadInfo *info)
{
	struct {
		MsgHeader 		hdr;
		CamRoadInfo		info;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !info)
		return E_INVAL;

	msgBuf.info = *info;
	ret = icam_send_cmd(hCtrl, ICAMCMD_S_ROADINFO, &msgBuf, sizeof(msgBuf), sizeof(msgBuf));
	if(ret)
		return ret;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_get_rtp_params
 Description  : get rtp params
 Input        : ICamCtrlHandle hCtrl  
                CamRtpParams *buf     
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_get_rtp_params(ICamCtrlHandle hCtrl, CamRtpParams *buf)
{
	struct {
		MsgHeader 		hdr;
		CamRtpParams	params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !buf)
		return E_INVAL;

	ret = icam_send_cmd(hCtrl, ICAMCMD_G_RTPPARAMS, &msgBuf, sizeof(MsgHeader), sizeof(msgBuf));
	if(ret)
		return ret;

	*buf = msgBuf.params;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_set_rtp_params
 Description  : set rtp params
 Input        : ICamCtrlHandle hCtrl        
                const CamRtpParams *params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_set_rtp_params(ICamCtrlHandle hCtrl, const CamRtpParams *params)
{
	struct {
		MsgHeader 		hdr;
		CamRtpParams	params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !params)
		return E_INVAL;

	msgBuf.params = *params;
	ret = icam_send_cmd(hCtrl, ICAMCMD_S_RTPPARAMS, &msgBuf, sizeof(msgBuf), sizeof(msgBuf));
	if(ret)
		return ret;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_get_upload_cfg
 Description  : get image upload cfg
 Input        : ICamCtrlHandle hCtrl         
                CamImgUploadProto *buf  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_get_upload_cfg(ICamCtrlHandle hCtrl, CamImgUploadCfg *buf)
{
	struct {
		MsgHeader 		hdr;
		CamImgUploadCfg	cfg;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !buf)
		return E_INVAL;

	ret = icam_send_cmd(hCtrl, ICAMCMD_G_UPLOADPROTO, &msgBuf, sizeof(MsgHeader), sizeof(msgBuf));
	if(ret)
		return ret;

	*buf = msgBuf.cfg;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_set_upload_cfg
 Description  : set image upload cfg
 Input        : ICamCtrlHandle hCtrl          
                CamImgUploadProto proto  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_set_upload_cfg(ICamCtrlHandle hCtrl, CamImgUploadCfg *cfg)
{
	struct {
		MsgHeader 			hdr;
		CamImgUploadCfg		cfg;
	}msgBuf;

	Int32 ret;

	if(!hCtrl)
		return E_INVAL;

	msgBuf.cfg = *cfg;
	ret = icam_send_cmd(hCtrl, ICAMCMD_S_UPLOADPROTO, &msgBuf, sizeof(msgBuf), sizeof(msgBuf));
	if(ret)
		return ret;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_get_img_srv_info
 Description  : get tcp img server info
 Input        : ICamCtrlHandle hCtrl            
                CamTcpImageServerInfo *srvInfo  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_get_img_srv_info(ICamCtrlHandle hCtrl, CamTcpImageServerInfo *srvInfo)
{
	struct {
		MsgHeader 				hdr;
		CamTcpImageServerInfo	srvInfo;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !srvInfo)
		return E_INVAL;

	ret = icam_send_cmd(hCtrl, ICAMCMD_G_IMGSRVINFO, &msgBuf, sizeof(MsgHeader), sizeof(msgBuf));
	if(ret)
		return ret;

	*srvInfo = msgBuf.srvInfo;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_set_img_srv_info
 Description  : set tcp img server info
 Input        : ICamCtrlHandle hCtrl                  
                const CamTcpImageServerInfo *srvInfo  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_set_img_srv_info(ICamCtrlHandle hCtrl, const CamTcpImageServerInfo *srvInfo)
{
	struct {
		MsgHeader 				hdr;
		CamTcpImageServerInfo	srvInfo;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !srvInfo)
		return E_INVAL;

	msgBuf.srvInfo = *srvInfo;
	ret = icam_send_cmd(hCtrl, ICAMCMD_S_IMGSRVINFO, &msgBuf, sizeof(msgBuf), sizeof(msgBuf));
	if(ret)
		return ret;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_get_ftp_srv_info
 Description  : get ftp server info
 Input        : ICamCtrlHandle hCtrl            
                CamFtpImageServerInfo *srvInfo  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_get_ftp_srv_info(ICamCtrlHandle hCtrl, CamFtpImageServerInfo *srvInfo)
{
	struct {
		MsgHeader 				hdr;
		CamFtpImageServerInfo	srvInfo;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !srvInfo)
		return E_INVAL;

	ret = icam_send_cmd(hCtrl, ICAMCMD_G_FTPSRVINFO, &msgBuf, sizeof(MsgHeader), sizeof(msgBuf));
	if(ret)
		return ret;

	*srvInfo = msgBuf.srvInfo;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_set_ftp_srv_info
 Description  : set ftp server info
 Input        : ICamCtrlHandle hCtrl                  
                const CamFtpImageServerInfo *srvInfo  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_set_ftp_srv_info(ICamCtrlHandle hCtrl, const CamFtpImageServerInfo *srvInfo)
{
	struct {
		MsgHeader 				hdr;
		CamFtpImageServerInfo	srvInfo;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !srvInfo)
		return E_INVAL;

	msgBuf.srvInfo = *srvInfo;
	ret = icam_send_cmd(hCtrl, ICAMCMD_S_FTPSRVINFO, &msgBuf, sizeof(msgBuf), sizeof(msgBuf));
	if(ret)
		return ret;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_get_ntp_srv_info
 Description  : get ntp server info
 Input        : ICamCtrlHandle hCtrl       
                CamNtpServerInfo *srvInfo  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_get_ntp_srv_info(ICamCtrlHandle hCtrl, CamNtpServerInfo *srvInfo)
{
	struct {
		MsgHeader 			hdr;
		CamNtpServerInfo	srvInfo;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !srvInfo)
		return E_INVAL;

	ret = icam_send_cmd(hCtrl, ICAMCMD_G_NTPSRVINFO, &msgBuf, sizeof(MsgHeader), sizeof(msgBuf));
	if(ret)
		return ret;

	*srvInfo = msgBuf.srvInfo;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_set_ntp_srv_info
 Description  : set ntp server info
 Input        : ICamCtrlHandle hCtrl             
                const CamNtpServerInfo *srvInfo  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_set_ntp_srv_info(ICamCtrlHandle hCtrl, const CamNtpServerInfo *srvInfo)
{
	struct {
		MsgHeader 			hdr;
		CamNtpServerInfo	srvInfo;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !srvInfo)
		return E_INVAL;

	msgBuf.srvInfo = *srvInfo;
	ret = icam_send_cmd(hCtrl, ICAMCMD_S_NTPSRVINFO, &msgBuf, sizeof(msgBuf), sizeof(msgBuf));
	if(ret)
		return ret;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_get_exposure_params
 Description  : get exposure params
 Input        : ICamCtrlHandle hCtrl       
                CamExprosureParam *params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_get_exposure_params(ICamCtrlHandle hCtrl, CamExprosureParam *params)
{
	struct {
		MsgHeader 			hdr;
		CamExprosureParam	params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !params)
		return E_INVAL;

	ret = icam_send_cmd(hCtrl, ICAMCMD_G_EXPPARAMS, &msgBuf, sizeof(MsgHeader), sizeof(msgBuf));
	if(ret)
		return ret;

	*params = msgBuf.params;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_set_exposure_params
 Description  : set exposure params
 Input        : ICamCtrlHandle hCtrl             
                const CamExprosureParam *params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_set_exposure_params(ICamCtrlHandle hCtrl, const CamExprosureParam *params)
{
	struct {
		MsgHeader 			hdr;
		CamExprosureParam	params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !params)
		return E_INVAL;

	msgBuf.params = *params;
	ret = icam_send_cmd(hCtrl, ICAMCMD_S_EXPPARAMS, &msgBuf, sizeof(msgBuf), sizeof(msgBuf));
	if(ret)
		return ret;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_get_rgb_gains
 Description  : get rgb gains
 Input        : ICamCtrlHandle hCtrl  
                CamRGBGains *params   
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_get_rgb_gains(ICamCtrlHandle hCtrl, CamRGBGains *params)
{
	struct {
		MsgHeader 			hdr;
		CamRGBGains			params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !params)
		return E_INVAL;

	ret = icam_send_cmd(hCtrl, ICAMCMD_G_RGBGAINS, &msgBuf, sizeof(MsgHeader), sizeof(msgBuf));
	if(ret)
		return ret;

	*params = msgBuf.params;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_set_rgb_gains
 Description  : set rgb gains
 Input        : ICamCtrlHandle hCtrl       
                const CamRGBGains *params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_set_rgb_gains(ICamCtrlHandle hCtrl, const CamRGBGains *params)
{
	struct {
		MsgHeader 			hdr;
		CamRGBGains			params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !params)
		return E_INVAL;

	msgBuf.params = *params;
	ret = icam_send_cmd(hCtrl, ICAMCMD_S_RGBGAINS, &msgBuf, sizeof(msgBuf), sizeof(msgBuf));
	if(ret)
		return ret;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_get_traffic_light_regions
 Description  : get traffic light regions
 Input        : ICamCtrlHandle hCtrl            
                CamTrafficLightRegions *params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_get_traffic_light_regions(ICamCtrlHandle hCtrl, CamTrafficLightRegions *params)
{
	struct {
		MsgHeader 				hdr;
		CamTrafficLightRegions	params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !params)
		return E_INVAL;

	ret = icam_send_cmd(hCtrl, ICAMCMD_G_TRAFLIGHTREG, &msgBuf, sizeof(MsgHeader), sizeof(msgBuf));
	if(ret)
		return ret;

	*params = msgBuf.params;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_set_traffic_light_regions
 Description  : set traffic light regions
 Input        : ICamCtrlHandle hCtrl                  
                const CamTrafficLightRegions *params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_set_traffic_light_regions(ICamCtrlHandle hCtrl, const CamTrafficLightRegions *params)
{
	struct {
		MsgHeader 				hdr;
		CamTrafficLightRegions	params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !params)
		return E_INVAL;

	msgBuf.params = *params;
	ret = icam_send_cmd(hCtrl, ICAMCMD_S_TRAFLIGHTREG, &msgBuf, sizeof(msgBuf), sizeof(msgBuf));
	if(ret)
		return ret;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_get_img_enhance_params
 Description  : get image enhance params
 Input        : ICamCtrlHandle hCtrl         
                CamImgEnhanceParams *params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_get_img_enhance_params(ICamCtrlHandle hCtrl, CamImgEnhanceParams *params)
{
	struct {
		MsgHeader 			hdr;
		CamImgEnhanceParams	params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !params)
		return E_INVAL;

	ret = icam_send_cmd(hCtrl, ICAMCMD_G_IMGENHANCE, &msgBuf, sizeof(MsgHeader), sizeof(msgBuf));
	if(ret)
		return ret;

	*params = msgBuf.params;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_set_img_enhance_params
 Description  : set image enhance params
 Input        : ICamCtrlHandle hCtrl               
                const CamImgEnhanceParams *params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_set_img_enhance_params(ICamCtrlHandle hCtrl, const CamImgEnhanceParams *params)
{
	struct {
		MsgHeader 			hdr;
		CamImgEnhanceParams	params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !params)
		return E_INVAL;

	msgBuf.params = *params;
	ret = icam_send_cmd(hCtrl, ICAMCMD_S_IMGENHANCE, &msgBuf, sizeof(msgBuf), sizeof(msgBuf));
	if(ret)
		return ret;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_get_h264_params
 Description  : get h.264 params
 Input        : ICamCtrlHandle hCtrl   
                CamH264Params *params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_get_h264_params(ICamCtrlHandle hCtrl, CamH264Params *params)
{
	struct {
		MsgHeader 		hdr;
		CamH264Params	params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !params)
		return E_INVAL;

	ret = icam_send_cmd(hCtrl, ICAMCMD_G_H264PARAMS, &msgBuf, sizeof(MsgHeader), sizeof(msgBuf));
	if(ret)
		return ret;

	*params = msgBuf.params;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_set_h264_params
 Description  : set h.264 params
 Input        : ICamCtrlHandle hCtrl         
                const CamH264Params *params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_set_h264_params(ICamCtrlHandle hCtrl, const CamH264Params *params)
{
	struct {
		MsgHeader 		hdr;
		CamH264Params	params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !params)
		return E_INVAL;

	msgBuf.params = *params;
	ret = icam_send_cmd(hCtrl, ICAMCMD_S_H264PARAMS, &msgBuf, sizeof(msgBuf), sizeof(msgBuf));
	if(ret)
		return ret;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_get_av_params
 Description  : get analog video params
 Input        : ICamCtrlHandle hCtrl  
                CamAVParam *params    
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_get_av_params(ICamCtrlHandle hCtrl, CamAVParam *params)
{
	struct {
		MsgHeader 	hdr;
		CamAVParam	params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !params)
		return E_INVAL;

	ret = icam_send_cmd(hCtrl, ICAMCMD_G_AVPARAMS, &msgBuf, sizeof(MsgHeader), sizeof(msgBuf));
	if(ret)
		return ret;

	*params = msgBuf.params;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_set_av_params
 Description  : set analog video params
 Input        : ICamCtrlHandle hCtrl      
                const CamAVParam *params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_set_av_params(ICamCtrlHandle hCtrl, const CamAVParam *params)
{
	struct {
		MsgHeader 	hdr;
		CamAVParam	params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !params)
		return E_INVAL;

	msgBuf.params = *params;
	ret = icam_send_cmd(hCtrl, ICAMCMD_S_AVPARAMS, &msgBuf, sizeof(msgBuf), sizeof(msgBuf));
	if(ret)
		return ret;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_get_work_mode
 Description  : get work mode
 Input        : ICamCtrlHandle hCtrl  
                CamWorkMode *params   
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_get_work_mode(ICamCtrlHandle hCtrl, CamWorkMode *params)
{
	struct {
		MsgHeader 	hdr;
		CamWorkMode	params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !params)
		return E_INVAL;

	ret = icam_send_cmd(hCtrl, ICAMCMD_G_WORKMODE, &msgBuf, sizeof(MsgHeader), sizeof(msgBuf));
	if(ret)
		return ret;

	*params = msgBuf.params;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_set_work_mode
 Description  : set work mode
 Input        : ICamCtrlHandle hCtrl       
                const CamWorkMode *params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_set_work_mode(ICamCtrlHandle hCtrl, const CamWorkMode *params)
{
	struct {
		MsgHeader 	hdr;
		CamWorkMode	params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !params)
		return E_INVAL;

	msgBuf.params = *params;
	ret = icam_send_cmd(hCtrl, ICAMCMD_S_WORKMODE, &msgBuf, sizeof(msgBuf), sizeof(msgBuf));
	if(ret)
		return ret;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_get_img_enc_params
 Description  : get image encode params
 Input        : ICamCtrlHandle hCtrl     
                CamImgEncParams *params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_get_img_enc_params(ICamCtrlHandle hCtrl, CamImgEncParams *params)
{
	struct {
		MsgHeader 		hdr;
		CamImgEncParams	params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !params)
		return E_INVAL;

	ret = icam_send_cmd(hCtrl, ICAMCMD_G_IMGENCPARAMS, &msgBuf, sizeof(MsgHeader), sizeof(msgBuf));
	if(ret)
		return ret;

	*params = msgBuf.params;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_set_img_enc_params
 Description  : set image encode params
 Input        : ICamCtrlHandle hCtrl           
                const CamImgEncParams *params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_set_img_enc_params(ICamCtrlHandle hCtrl, const CamImgEncParams *params)
{
	struct {
		MsgHeader 		hdr;
		CamImgEncParams	params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !params)
		return E_INVAL;

	msgBuf.params = *params;
	ret = icam_send_cmd(hCtrl, ICAMCMD_S_IMGENCPARAMS, &msgBuf, sizeof(msgBuf), sizeof(msgBuf));
	if(ret)
		return ret;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_get_spec_cap_params
 Description  : get special capture params
 Input        : ICamCtrlHandle hCtrl      
                CamSpecCapParams *params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_get_spec_cap_params(ICamCtrlHandle hCtrl, CamSpecCapParams *params)
{
	struct {
		MsgHeader 			hdr;
		CamSpecCapParams	params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !params)
		return E_INVAL;

	ret = icam_send_cmd(hCtrl, ICAMCMD_G_SPECCAPPARAMS, &msgBuf, sizeof(MsgHeader), sizeof(msgBuf));
	if(ret)
		return ret;

	*params = msgBuf.params;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_set_spec_cap_params
 Description  : set special capture params
 Input        : ICamCtrlHandle hCtrl            
                const CamSpecCapParams *params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_set_spec_cap_params(ICamCtrlHandle hCtrl, const CamSpecCapParams *params)
{
	struct {
		MsgHeader 			hdr;
		CamSpecCapParams	params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !params)
		return E_INVAL;

	msgBuf.params = *params;
	ret = icam_send_cmd(hCtrl, ICAMCMD_S_SPECCAPPARAMS, &msgBuf, sizeof(msgBuf), sizeof(msgBuf));
	if(ret)
		return ret;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_get_io_config
 Description  : get io config params
 Input        : ICamCtrlHandle hCtrl  
                CamIoCfg *params      
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_get_io_config(ICamCtrlHandle hCtrl, CamIoCfg *params)
{
	struct {
		MsgHeader 	hdr;
		CamIoCfg	params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !params)
		return E_INVAL;

	ret = icam_send_cmd(hCtrl, ICAMCMD_G_IOCFG, &msgBuf, sizeof(MsgHeader), sizeof(msgBuf));
	if(ret)
		return ret;

	*params = msgBuf.params;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_set_io_config
 Description  : set io config params
 Input        : ICamCtrlHandle hCtrl    
                const CamIoCfg *params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_set_io_config(ICamCtrlHandle hCtrl, const CamIoCfg *params)
{
	struct {
		MsgHeader 	hdr;
		CamIoCfg	params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !params)
		return E_INVAL;

	msgBuf.params = *params;
	ret = icam_send_cmd(hCtrl, ICAMCMD_S_IOCFG, &msgBuf, sizeof(msgBuf), sizeof(msgBuf));
	if(ret)
		return ret;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_get_strobe_params
 Description  : get strobe control params
 Input        : ICamCtrlHandle hCtrl        
                CamStrobeCtrlParam *params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_get_strobe_params(ICamCtrlHandle hCtrl, CamStrobeCtrlParam *params)
{
	struct {
		MsgHeader 			hdr;
		CamStrobeCtrlParam	params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !params)
		return E_INVAL;

	ret = icam_send_cmd(hCtrl, ICAMCMD_G_STROBEPARAMS, &msgBuf, sizeof(MsgHeader), sizeof(msgBuf));
	if(ret)
		return ret;

	*params = msgBuf.params;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_set_strobe_params
 Description  : set strobe control params
 Input        : ICamCtrlHandle hCtrl              
                const CamStrobeCtrlParam *params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_set_strobe_params(ICamCtrlHandle hCtrl, const CamStrobeCtrlParam *params)
{
	struct {
		MsgHeader 			hdr;
		CamStrobeCtrlParam	params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !params)
		return E_INVAL;

	msgBuf.params = *params;
	ret = icam_send_cmd(hCtrl, ICAMCMD_S_STROBEPARAMS, &msgBuf, sizeof(msgBuf), sizeof(msgBuf));
	if(ret)
		return ret;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_get_detector_params
 Description  : get vehicle detector params
 Input        : ICamCtrlHandle hCtrl      
                CamDetectorParam *params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_get_detector_params(ICamCtrlHandle hCtrl, CamDetectorParam *params)
{
	struct {
		MsgHeader 			hdr;
		CamDetectorParam	params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !params)
		return E_INVAL;

	ret = icam_send_cmd(hCtrl, ICAMCMD_G_DETECTORPARAMS, &msgBuf, sizeof(MsgHeader), sizeof(msgBuf));
	if(ret)
		return ret;

	*params = msgBuf.params;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_set_detector_params
 Description  : set vehicle detector params
 Input        : ICamCtrlHandle hCtrl            
                const CamDetectorParam *params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_set_detector_params(ICamCtrlHandle hCtrl, const CamDetectorParam *params)
{
	struct {
		MsgHeader 			hdr;
		CamDetectorParam	params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !params)
		return E_INVAL;

	msgBuf.params = *params;
	ret = icam_send_cmd(hCtrl, ICAMCMD_S_DETECTORPARAMS, &msgBuf, sizeof(msgBuf), sizeof(msgBuf));
	if(ret)
		return ret;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_get_ae_params
 Description  : get auto exposure params
 Input        : ICamCtrlHandle hCtrl  
                CamAEParam *params    
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_get_ae_params(ICamCtrlHandle hCtrl, CamAEParam *params)
{
	struct {
		MsgHeader 	hdr;
		CamAEParam	params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !params)
		return E_INVAL;

	ret = icam_send_cmd(hCtrl, ICAMCMD_G_AEPARAMS, &msgBuf, sizeof(MsgHeader), sizeof(msgBuf));
	if(ret)
		return ret;

	*params = msgBuf.params;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_set_ae_params
 Description  : set auto exposure params
 Input        : ICamCtrlHandle hCtrl      
                const CamAEParam *params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_set_ae_params(ICamCtrlHandle hCtrl, const CamAEParam *params)
{
	struct {
		MsgHeader 	hdr;
		CamAEParam	params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !params)
		return E_INVAL;

	msgBuf.params = *params;
	ret = icam_send_cmd(hCtrl, ICAMCMD_S_AEPARAMS, &msgBuf, sizeof(msgBuf), sizeof(msgBuf));
	if(ret)
		return ret;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_get_awb_params
 Description  : get AWB params
 Input        : ICamCtrlHandle hCtrl  
                CamAWBParam *params   
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_get_awb_params(ICamCtrlHandle hCtrl, CamAWBParam *params)
{
	struct {
		MsgHeader 	hdr;
		CamAWBParam	params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !params)
		return E_INVAL;

	ret = icam_send_cmd(hCtrl, ICAMCMD_G_AWBPARAMS, &msgBuf, sizeof(MsgHeader), sizeof(msgBuf));
	if(ret)
		return ret;

	*params = msgBuf.params;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_set_awb_params
 Description  : set AWB params
 Input        : ICamCtrlHandle hCtrl       
                const CamAWBParam *params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_set_awb_params(ICamCtrlHandle hCtrl, const CamAWBParam *params)
{
	struct {
		MsgHeader 	hdr;
		CamAWBParam	params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !params)
		return E_INVAL;

	msgBuf.params = *params;
	ret = icam_send_cmd(hCtrl, ICAMCMD_S_AWBPARAMS, &msgBuf, sizeof(msgBuf), sizeof(msgBuf));
	if(ret)
		return ret;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_get_day_night_mode_params
 Description  : get day night switch params
 Input        : ICamCtrlHandle hCtrl        
                CamDayNightModeCfg *params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_get_day_night_mode_params(ICamCtrlHandle hCtrl, CamDayNightModeCfg *params)
{
	struct {
		MsgHeader 			hdr;
		CamDayNightModeCfg	params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !params)
		return E_INVAL;

	ret = icam_send_cmd(hCtrl, ICAMCMD_G_DAYNIGHTMODE, &msgBuf, sizeof(MsgHeader), sizeof(msgBuf));
	if(ret)
		return ret;

	*params = msgBuf.params;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_set_day_night_mode_params
 Description  : set day night switch params
 Input        : ICamCtrlHandle hCtrl              
                const CamDayNightModeCfg *params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_set_day_night_mode_params(ICamCtrlHandle hCtrl, const CamDayNightModeCfg *params)
{
	struct {
		MsgHeader 			hdr;
		CamDayNightModeCfg	params;
	}msgBuf;

	Int32 ret;

	if(!hCtrl || !params)
		return E_INVAL;

	msgBuf.params = *params;
	ret = icam_send_cmd(hCtrl, ICAMCMD_S_DAYNIGHTMODE, &msgBuf, sizeof(msgBuf), sizeof(msgBuf));
	if(ret)
		return ret;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_capture_ctrl
 Description  : capture control
 Input        : ICamCtrlHandle hCtrl  
                CamCapCtrl ctrl       
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_capture_ctrl(ICamCtrlHandle hCtrl, CamCapCtrl ctrl)
{
	MsgHeader 	hdr;

	Int32 ret;

	if(!hCtrl)
		return E_INVAL;

	hdr.param[0] = ctrl;
	ret = icam_send_cmd(hCtrl, ICAMCMD_S_CAPCTRL, &hdr, sizeof(hdr), sizeof(hdr));
	if(ret)
		return ret;

	/* result of this cmd */
	return hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_restore_cfg
 Description  : restore cfg data
 Input        : ICamCtrlHandle hCtrl  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/30
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_restore_cfg(ICamCtrlHandle hCtrl)
{
	MsgHeader 	hdr;

	Int32 ret;

	if(!hCtrl)
		return E_INVAL;

	hdr.param[0] = 0;
	ret = icam_send_cmd(hCtrl, ICAMCMD_S_RESTORECFG, &hdr, sizeof(hdr), sizeof(hdr));
	if(ret)
		return ret;

	/* result of this cmd */
	return hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_sys_reset
 Description  : reset icamera and system
 Input        : ICamCtrlHandle hCtrl  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/30
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_sys_reset(ICamCtrlHandle hCtrl)
{
	MsgHeader 	hdr;

	Int32 ret;

	if(!hCtrl)
		return E_INVAL;

	hdr.param[0] = 0;
	ret = icam_send_cmd(hCtrl, ICAMCMD_S_RESET, &hdr, sizeof(hdr), sizeof(hdr));
	if(ret)
		return ret;

	/* result of this cmd */
	return hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_snd_dir
 Description  : send a dir that contains jpg file
 Input        : ICamCtrlHandle hCtrl  
                const char *dirPath   
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_snd_dir(ICamCtrlHandle hCtrl, const char *dirPath)
{
	struct {
		MsgHeader	hdr;
		char		buf[512];
	}msgBuf;
	
	Int32 ret;

	if(!hCtrl)
		return E_INVAL;

	Int32 len = strlen(dirPath) + 1;
	if(len > sizeof(msgBuf.buf)) {
		ERR("dir path is too long");
		return E_NOSPC;
	}
		
	strcpy(msgBuf.buf, dirPath);
	ret = icam_send_cmd(hCtrl, ICAMCMD_S_SNDDIR, &msgBuf, len + sizeof(MsgHeader), sizeof(msgBuf));
	if(ret)
		return ret;

	/* result of this cmd */
	return msgBuf.hdr.param[0];
}

/*****************************************************************************
 Prototype    : icam_get_sd_root_path
 Description  : get sd root path
 Input        : ICamCtrlHandle hCtrl  
                void *buf             
                Int32 bufLen          
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_get_sd_root_path(ICamCtrlHandle hCtrl, void *buf, Int32 *bufLen)
{
	MsgHeader 	*hdr = (MsgHeader *)buf;
	
	Int32 ret;

	if(!hCtrl || !buf || *bufLen <= 0)
		return E_INVAL;

	ret = icam_send_cmd(hCtrl, ICAMCMD_G_SDROOTPATH, buf, sizeof(MsgHeader), *bufLen);
	if(ret)
		return ret;

	if(hdr->param[0])
		return hdr->param[0];

	*bufLen = hdr->dataLen;

	/* remove msg header */
	memmove(buf, buf + sizeof(MsgHeader), sizeof(MsgHeader));

	/* result of this cmd */
	return E_NO;
}

#if 0
Int32 icam_firmware_update(ICamCtrlHandle hCtrl, CamUpdateType type, const void *buf, Int32 bufLen)
{
	Int32 ret;

	if(!hCtrl || !buf || bufLen <= 0)
		return E_INVAL;

	MsgHeader 	*hdr = malloc(sizeof(MsgHeader) + bufLen);

	if(!hdr)
		return E_NOMEM;

	/* use type for param0 */
	hdr->param[0] = type;
	memcpy((Int8 *)hdr + sizeof(MsgHeader), buf, bufLen);
	
	ret = icam_send_cmd(hCtrl, ICAMCMD_S_UPDATE, hdr, sizeof(MsgHeader) + bufLen, sizeof(MsgHeader));
	if(ret) {
		free(hdr);
		return ret;
	}

	ret= hdr->param[0];
	free(hdr);

	/* result of this cmd */
	return ret;
}
#endif

/*****************************************************************************
 Prototype    : icam_get_sd_dir_info
 Description  : get sd dir info
 Input        : ICamCtrlHandle hCtrl  
                const char *dirPath   
                void *buf             
                Int32 bufLen          
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_get_sd_dir_info(ICamCtrlHandle hCtrl, const char *dirPath, void *buf, Int32 *bufLen)
{
	MsgHeader 	*hdr = (MsgHeader *)buf;
	
	Int32 ret, len;

	len = strlen(dirPath) + 1;
	if(!hCtrl || !buf || *bufLen < sizeof(MsgHeader) + len)
		return E_INVAL;

	strcpy((char *)buf + sizeof(MsgHeader), dirPath);

	ret = icam_send_cmd(hCtrl, ICAMCMD_G_SDDIRINFO, buf, sizeof(MsgHeader) + len, *bufLen);
	if(ret)
		return ret;

	if(hdr->param[0])
		return hdr->param[0];

	*bufLen = hdr->dataLen;
	/* remove msg header */
	memmove(buf, buf + sizeof(MsgHeader), sizeof(MsgHeader));

	/* result of this cmd */
	return E_NO;
}

/*****************************************************************************
 Prototype    : icam_ctrl_run
 Description  : abstract api for snd cmd and wait reply
 Input        : IN ICamCtrlHandle hCtrl  
                INOUT MsgHeader *data    
                IN Uint32 bufLen         
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/26
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 icam_ctrl_run(IN ICamCtrlHandle hCtrl, INOUT MsgHeader *data, IN Uint32 bufLen)
{
	Int32 		err;
	Uint16		cmd;
	
	if(!hCtrl || !data || !bufLen)
		return E_INVAL;
	
	/* fill header fileds */
	data->type = MSG_TYPE_REQU;
	cmd = data->cmd;
	
	/* send msg */
	err = msg_send(hCtrl->hMsg, NULL, data, 0);
	if(err)
		return err;

	/* wait response */
	err = msg_recv(hCtrl->hMsg, data, bufLen, 0);
	if(err < 0) {
		ERR("recv msg err: %s", str_err(err));
		return err;
	}

	/* validate data */
	if(data->type != MSG_TYPE_RESP || cmd != data->cmd)
		return E_TRANS;

	return data->param[0];
}

