#include "dspSrv.h"
#include <iostream>
#include "common.h"
#include "log.h"
#include "img_trans.h"
#include "sys_commu.h"
#include "net_utils.h"

#define MSG_NAME			"/tmp/msgDspSrv"
#define MSG_TRANS_TIMEOUT	10
#define IMG_TRANS_TIMEOUT	10

using std::string;
using std::cin;
using std::cout;
using std::endl;

bool DspSrv::exit = FALSE;

/**
 * Run dsp server, read img from dsp and send to network
 */
int DspSrv::Run()
{
	int err = E_NO;
	int fdSyslink = -1;

	if(srvIp == "" || !srvPort)
		GetSrvInfo();

	cout << "dspSrv, run..." << endl;
	cout << "img srv: " << srvIp;
	cout << ":" << srvPort << endl; 

	//get local IP for description info
	char localIp[16];
	get_local_ip(localIp, sizeof(localIp));

	ImgTransHandle hImgTrans = img_trans_create(srvIp.c_str(), srvPort, localIp, IMG_TRANS_TIMEOUT, 0);
	if(!hImgTrans) {
		ERR("create img trans handle failed!");
		err = E_IO;
		goto exit;
	}

	exit = FALSE;
	
	while(!exit) {
		bool isConnected;
		int imgCnt = 0;

		//Read img from dsp
		err = ReadDspImg(fdSyslink);
		//assert(fdSyslink > 0);
		if(!err) {
			//Send to network	
			err = SendImg(hImgTrans, isConnected, imgCnt);
		}
	}
	
exit:
	if(fdSyslink > 0)
		close(fdSyslink);

	if(buf) {
		free(buf);
		buf = NULL;
	}

	if(hImgTrans)
		img_trans_delete(hImgTrans);

	cout << "dsp srv, stop running..." << endl;

	return err;	
}

/**
 * Get tcp image server info from cam server 
 */
int DspSrv::GetSrvInfo()
{
	if(!hCamCtrl) {
		hCamCtrl = icam_ctrl_create(MSG_NAME, 0, MSG_TRANS_TIMEOUT);
		if(!hCamCtrl) {
			ERR("create cam ctrl handle failed!");
			return E_NOMEM;	
		}
	}

	// Get current server info from cam server
	CamTcpImageServerInfo srvInfo;
	int err = icam_get_img_srv_info(hCamCtrl, &srvInfo);
	if(err) {
		ERR("get img srv info failed!");
		return err;
	}

	// Set to our variables
	srvIp = srvInfo.serverIP;
	srvPort = srvInfo.serverPort;

	return E_NO;
}

/**
 *  Read image from dsp
 */
int DspSrv::ReadDspImg(int& fd)
{
	// Open device for communication with DSP
	if(fd <= 0) {
		DBG("open syslink dev.");
		fd = open(SYSLINK_DEV, O_RDWR);
		if(fd < 0) {
			ERR("open %s failed!", SYSLINK_DEV);
			return E_IO;
		}
	}

	// alloc buffer for image read
	if(!buf) {
		DBG("alloc buffer.");
		buf = calloc(1, bufSize);
		if(!buf) {
			ERR("alloc buffer failed!");
			return E_NOMEM;
		}
	}

	// wait msg from dsp
	fd_set rdSet;
	int fdMax = fd + 1;
	FD_ZERO(&rdSet);
	FD_SET(fd, &rdSet);
	int err = select(fdMax, &rdSet, NULL, NULL, NULL);
	if(err < 0 || !FD_ISSET(fd, &rdSet)) {
		ERRSTR("select err");
		return E_IO;
	}

	// read from device
	SysMsg *msgBuf = static_cast<SysMsg *>(buf);
	err = sys_commu_read(fd, msgBuf, bufSize);
	if(err < 0) {
		ERR("read msg from dsp failed!");
		return err;
	}

	// check if this msg is new image 
	if(msgBuf->cmd != SYS_CMD_NEW_JPG) {
		return E_AGAIN;	
	}

	return E_NO;
}

/**
 * Send image to tcp server
 */
int DspSrv::SendImg(ImgTransHandle hImgTrans, bool &isConnected, int &imgCnt)
{
	int err;

	if(!isConnected) {
		//Get lastest server info
		if(GetSrvInfo() == E_NO) {
			img_trans_set_srv_info(hImgTrans, srvIp.c_str(), srvPort);
		}
		err = img_trans_connect(hImgTrans, IMG_TRANS_TIMEOUT);
		if(err) {
			ERR("connect tcp img srv failed...");
			return err;
		}
		isConnected = TRUE;
		DBG("connect srv ok...");
	}

	//Send image
	ImgHdrInfo info;
	SysMsg *msgBuf = static_cast<SysMsg *>(buf);

	info.serialNumber = ++imgCnt;
	info.imageType = IMGTYPE_JPEG;
	info.imageWidth = msgBuf->params[0];
	info.imageHeight = msgBuf->params[1];	
	info.imageLen = msgBuf->dataLen;

	void *imgData = msgBuf + 1;

	err = img_trans_send(hImgTrans, &info, imgData);
	if(err != E_NO) {
		ERR("send data failed!");
		img_trans_disconnect(hImgTrans);
		isConnected = FALSE;
	}	
	
	return err;
}

