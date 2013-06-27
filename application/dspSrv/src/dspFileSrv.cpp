#include <iostream>
#include "common.h"
#include "log.h"
#include "dspFileSrv.h"
#include "syslink_proto.h"
#include "sys_commu.h"

using namespace std;

/**
 * DispactchImg -- deal with img sent by DSP
 */
int DspFileSrv::DispactchImg(BufItem& item)
{
	int err = E_NO;
	if(m_isConnected) {
		// put into send queue
		m_listUsed.push_back(item);
		// Delte this item
		m_listFree.pop_front();
	} else {
		// Save to local filesys
		err = SaveImg(item);
	}

	return err;
}

/**
 * HandleMsg -- handle received cmd
 */
int DspFileSrv::HandleMsg(BufItem& item)
{
	SysMsg *pMsg = (SysMsg *)item.addr;
	int err = E_NO;
	
	switch(pMsg->cmd) {
		case SYS_CMD_SND_IMG:
			DispactchImg(item);
			break;
		default:
			err = E_UNSUPT;
			break;
	}

	return err;
}

/**
 * Do the process loop
 */
int DspFileSrv::ProcessLoop(int syslink, size_t bufLen)
{
	int err = E_NO;

	while(!StopRunning()) {
		// Get free buffer
		if(m_listFree.empty()) {
			usleep(10000);
			continue;
		}
		BufItem item = m_listFree.front();

		// Wait cmd 
		err = sys_commu_read(syslink, (SysMsg *)item.addr, item.totalLen);
		if(err < 0) {
			continue;
		}

		// Process cmd
		HandleMsg(item);
	
	}
		
	return err;
}

/**
 * Initialize -- init params 
 */
int DspFileSrv::Initialize(size_t bufSize, int bufNum) 
{
	BufItem item;
	int err = 0;

	for(int i = 0; i != bufNum; ++i) {
		// alloc memory
		item.addr = new uint8_t[bufSize];
		if(!item.addr) {
			cout << "alloc mem failed!" << endl;
			err = E_NOMEM;
			break;
		}	
		item.totalLen = bufSize;
		
		// push into list
		m_listFree.push_back(item);
	}

	// create thread for upload
	m_isConnected = false;
	err = pthread_create(&m_pid, NULL, UploadThread, this);

	if(err < 0) {
		ERRSTR("create upload thread failed!");
	}

	// init params
	m_savePath = DSP_SAVE_PATH;
	return err;
}

/**
 * ~DspFileSrv -- deconstruct this object
 */
DspFileSrv::~DspFileSrv()
{
	ListBuf::iterator i;
	BufItem *item;

	while(!m_listFree.empty()) {
		item = &m_listFree.front();
		//free mem
		delete [] (uint8_t *)item->addr;	
		m_listFree.pop_front();
	}

	while(!m_listUsed.empty()) {
		item = &m_listUsed.front();
		//free mem
		delete [] (uint8_t *)item->addr;	
		m_listUsed.pop_front();
	}


	pthread_join(m_pid, NULL);
}

/**
 * Connect -- connect img server
 */
int DspFileSrv::Connect(ICamCtrlHandle hCamCtrl, ImgTransHandle hUpload)
{
	// Get latest server info
	CamTcpImageServerInfo info;
	int err = icam_get_img_srv_info(hCamCtrl, &info);

	if(err == E_NO) {
		//set current server info
		info.serverPort += 100;
		img_trans_set_srv_info(hUpload, info.serverIP, info.serverPort);
	}

	// Try connect server
	err = img_trans_connect(hUpload, 0);
	if(err < 0) {
		m_isConnected = false;
	} else {
		m_isConnected = true;
	}	

	return err;
}

/**
 * SaveImg -- save img file to local file system
 */
int DspFileSrv::SaveImg(const BufItem& item)
{

	return E_NO;
}

/**
 * SendImg -- send image to server
 */
int DspFileSrv::SendImg(ImgTransHandle hUpload, ImgHdrInfo& hdr)
{
	int err;
	BufItem item = m_listUsed.front();
	SysMsg *pMsg = (SysMsg *)item.addr;
	
	hdr.imageLen = pMsg->dataLen;
	++hdr.serialNumber;
	err = img_trans_send(hUpload, &hdr, item.addr + sizeof(SysMsg));
	
	if(err != E_NO) {
		// Send failed, disconnect srv and save file
		img_trans_disconnect(hUpload);
		m_isConnected = false;
		SaveImg(item);
	}	

	// Put processed buffer into free list
	m_listUsed.pop_front();
	m_listFree.push_back(item);

	return err;
}

/**
 * UploadThread -- connect server and upload img
 */
#define MSG_NAME 		"/tmp/dspFileSrv"
#define TRANS_TIMEOUT	5

void *DspFileSrv::UploadThread(void *arg)
{
	DspFileSrv *fileSrv = (DspFileSrv *)arg;
	ICamCtrlHandle hCamCtrl = NULL;
	ImgHdrInfo hdr;
	ImgTransHandle hUpload = NULL;

	memset(&hdr, 0, sizeof(hdr));

	// Create cam ctrl handle to get params
	hCamCtrl = icam_ctrl_create(MSG_NAME, 0, TRANS_TIMEOUT);
	if(!hCamCtrl) {
		ERR("create icam handle failed!");
		goto exit;
	}

	// Create handle for upload
	hUpload = img_trans_create(NULL, 0, "dspImg", TRANS_TIMEOUT, 0);
	if(!hUpload) {
		ERR("create img upload handle failed!");
		goto exit;
	}

	while(!fileSrv->StopRunning()) {
		// Connect server
		if(!fileSrv->m_isConnected) {
			fileSrv->Connect(hCamCtrl, hUpload);
			if(!fileSrv->m_isConnected) {
				// connect failed
				sleep(2);
				continue;
			}
		}

		// Wait img available
		if(fileSrv->m_listUsed.empty()) {
			usleep(10000);
			continue;
		}

		// Send img
		fileSrv->SendImg(hUpload, hdr);
	}

exit:
	if(hCamCtrl)
		icam_ctrl_delete(hCamCtrl);

	if(hUpload)
		img_trans_delete(hUpload);

	pthread_exit(0);
}


