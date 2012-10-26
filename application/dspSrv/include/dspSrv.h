#ifndef __DSP_SRV_H__
#define __DSP_SRV_H__

#include <iostream>
#include <string>
#include <unistd.h>
#include "icam_ctrl.h"
#include "img_trans.h"

#define DSP_SRV_BUF_SIZE	(3 * 1024 * 1024)

class DspSrv {
public:
	DspSrv(): srvPort(0), srvIp(""), hCamCtrl(NULL), bufSize(DSP_SRV_BUF_SIZE), buf(NULL) {}
	DspSrv(const std::string &ip, uint16_t port): srvPort(port), srvIp(ip), hCamCtrl(NULL), bufSize(DSP_SRV_BUF_SIZE), buf(NULL) {}
	~DspSrv() 
		{ if(hCamCtrl) icam_ctrl_delete(hCamCtrl); }
	int Run();
	void SetSrvInfo(const std::string &ip, uint16_t port)
		{ srvIp = ip; srvPort = port; }
	static void Stop()
		{ exit = TRUE; }

private:
	uint16_t srvPort;	
	std::string srvIp;
	ICamCtrlHandle hCamCtrl;
	size_t bufSize;
	void *buf;
	static bool exit;

private:
	int GetSrvInfo();
	int ReadDspImg(int &fd);
	int SendImg(ImgTransHandle hImgTrans, bool &isConnected, int &imgCnt);
};

#endif
