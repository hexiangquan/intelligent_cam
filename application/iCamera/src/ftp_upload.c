/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : ftp_upload.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/12
  Last Modified :
  Description   : image upload by ftp
  Function List :
              ftp_upload_connect
              ftp_upload_create
              ftp_upload_delete
              ftp_upload_disconnect
              ftp_upload_send
              ftp_upload_send_heartbeat
              ftp_upload_set_params
  History       :
  1.Date        : 2012/3/12
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "log.h"
#include "ftp_upload.h"
#include "ftp_client.h"
#include "path_name.h"

/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/

/*----------------------------------------------*
 * external routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/

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

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/
/* object of this module */
typedef struct _FtpUploadHandle
{
	FtpHandle		hFtp;
	Int8			rootPath[CAM_FTP_MAX_ROOT_DIR_LEN];
	PathNameHandle	hPathName;
}FtpUploadObj, *FtpUploadHandle;

/*****************************************************************************
 Prototype    : ftp_upload_create
 Description  : create ftp upload module
 Input        : void *params  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/21
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static void *ftp_upload_create(void *params)
{
	FtpUploadParams *uploadParams = (FtpUploadParams *)params;
	FtpUploadHandle	hUpload;
	
	if(!params || uploadParams->size != sizeof(FtpUploadParams)) {
		ERR("invalid params");
		return NULL;
	}

	/* Alloc run time params */
	hUpload = calloc(1, sizeof(FtpUploadObj));
	if(!hUpload) {
		ERR("not enough memory");
		return NULL;
	}

	/* Create ftp handle */
	hUpload->hFtp = ftp_create( uploadParams->srvInfo.userName, 
								uploadParams->srvInfo.password, 
								uploadParams->srvInfo.serverIP, 
								uploadParams->srvInfo.serverPort );
	
	if(!hUpload->hFtp) {
		ERR("create ftp handle failed");
		goto err_quit;
	}
	
	/* Copy root dir */
	strncpy(hUpload->rootPath, uploadParams->srvInfo.rootDir, CAM_FTP_MAX_ROOT_DIR_LEN);

	/* Init pattern for path name generate */
	hUpload->hPathName = path_name_create(uploadParams->srvInfo.pathNamePattern, 
							uploadParams->roadInfo.roadName);

	if(!hUpload->hPathName) {
		ERR("create path name failed");
		goto err_quit;
	}
	
	return (void *)hUpload;

err_quit:
	if(hUpload->hFtp)
		ftp_delete(hUpload->hFtp);

	if(hUpload->hPathName)
		path_name_delete(hUpload->hPathName);

	free(hUpload);

	return NULL;
}


/*****************************************************************************
 Prototype    : ftp_upload_connect
 Description  : connect server
 Input        : void *handle    
                Uint32 timeout  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 ftp_upload_connect(void *handle, Uint32 timeout)
{
	FtpUploadHandle hUpload = (FtpUploadHandle)handle;
	Int32 			err;

	if(!handle)
		return E_INVAL;

	/* Connect server */
	if((err = ftp_connect_server(hUpload->hFtp, timeout)) != E_NO) {
		ERR("connect err: %s", str_err(err));
		return E_CONNECT;
	}
	
	/* Change working dir */
	if(hUpload->rootPath[0] != 0 && hUpload->rootPath[0] != ' ') {
		err = ftp_change_working_dir(hUpload->hFtp, hUpload->rootPath);
		if(err == E_REFUSED) {
			/* if dir not exist, try to make one */
			if((err = ftp_make_dir(hUpload->hFtp, hUpload->rootPath)) == E_NO)
				err = ftp_change_working_dir(hUpload->hFtp, hUpload->rootPath);
		}
		DBG("ftp change working dir to %s", hUpload->rootPath);
	}

	if(err) {
		ERR("change working dir err: %s", str_err(err));
		return err;
	}
	
	return E_NO;
}

/*****************************************************************************
 Prototype    : ftp_upload_disconnect
 Description  : disconnect ftp server
 Input        : void *handle  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 ftp_upload_disconnect(void *handle)
{
	FtpUploadHandle hUpload = (FtpUploadHandle)handle;
	
	if(!handle)
		return E_INVAL;
	
	return ftp_disconnect_server(hUpload->hFtp);
}

/*****************************************************************************
 Prototype    : ftp_upload_send
 Description  : send one frame by ftp
 Input        : void *handle         
                const ImgMsg *frame  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 ftp_upload_send(void *handle, const ImgMsg *frame)
{
	FtpUploadHandle hUpload = (FtpUploadHandle)handle;
	Int32 			ret;
	PathNameInfo	nameInfo;
	Int8			fileNameBuf[PATHNAME_MAX_LINE_SIZE];
	Int8			*fileData = buffer_get_user_addr(frame->hBuf);
	Uint32			fileSize = frame->dimension.size;
	
	if(!handle || !frame)
		return E_INVAL;

	if(frame->dimension.colorSpace != FMT_JPG) {
		WARN("ftp upload only support jpeg format!");
		return E_NO;
	}

	/* Generate pathname */
	if((ret = path_name_generate(hUpload->hPathName, frame, &nameInfo)) != E_NO) {
		ERR("generate pathname error.");
		return ret;
	}

	/* check path name number */
	if(!nameInfo.fileNum || nameInfo.fileNum > PATHNAME_MAX_CNT_NUM) {
		ERR("generate pathname num is invalid: num of names is %d.", nameInfo.fileNum);
		return E_NOMEM;
	}

	/* send files */
	Int32 index = nameInfo.fileNum;
	ret = E_NO;
	while(index-- > 0) {
		if(*(nameInfo.path[index]) != '\0') {
			snprintf(fileNameBuf, sizeof(fileNameBuf), "%s/%s", nameInfo.path[index], nameInfo.fileName[index]);
			//printf("%s\n", cPathFile);
			ret = ftp_upload_file(hUpload->hFtp, fileData, fileSize, fileNameBuf);
			if(ret == E_INVPATH) {
				//make dir and upload again
				ret = ftp_make_dir(hUpload->hFtp, nameInfo.path[index]);
				if(!ret)
					ret = ftp_upload_file(hUpload->hFtp, fileData, fileSize, fileNameBuf);
			}
		}
		else
			ret = ftp_upload_file(hUpload->hFtp, fileData, fileSize, nameInfo.fileName[index]);

		if(ret) {
			break; //communication error break
		}
	}

	return ret;
}


/*****************************************************************************
 Prototype    : ftp_upload_delete
 Description  : delete ftp module
 Input        : void *handle  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 ftp_upload_delete(void *handle)
{
	FtpUploadHandle hUpload = (FtpUploadHandle)handle;
	Int32 			err;
	
	if(!hUpload)
		return E_INVAL;

	err = ftp_delete(hUpload->hFtp);
	if(err )
		WARN("ftp delete err");

	
	if(hUpload->hPathName)
		path_name_delete(hUpload->hPathName);

	free(hUpload);
	
	return E_NO;
}

/*****************************************************************************
 Prototype    : ftp_upload_set_params
 Description  : set upload params
 Input        : void *handle        
                const void *params  
                Uint32 size         
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 ftp_upload_set_params(void *handle, const void *params)
{
	FtpUploadHandle hUpload = (FtpUploadHandle)handle;
	FtpUploadParams *ftpParams = (FtpUploadParams *)params;
	Int32 			 err = E_NO;

	if(!ftpParams || !hUpload || ftpParams->size != sizeof(FtpUploadParams))
		return E_INVAL;

	/* Set ftp params */
	err |= ftp_set_server_ip(hUpload->hFtp, ftpParams->srvInfo.serverIP);
	err |= ftp_set_server_port(hUpload->hFtp, ftpParams->srvInfo.serverPort);
	err |= ftp_set_user_name(hUpload->hFtp, ftpParams->srvInfo.userName);
	err |= ftp_set_password(hUpload->hFtp, ftpParams->srvInfo.password);
	if(err) {
		ERR("set ftp params error.");
		return E_INVAL;
	}

	/* copy root dir */
	if(strlen(ftpParams->srvInfo.rootDir) < CAM_FTP_MAX_ROOT_DIR_LEN) {
		strcpy(hUpload->rootPath, ftpParams->srvInfo.rootDir);
		DBG("ftp current root dir: %s", hUpload->rootPath);
	} else {
		WARN("FtpUpload_set_params, root dir is too long: %d", strlen(ftpParams->srvInfo.rootDir));
	}

	/* Set path name generate params */
	path_name_config(hUpload->hPathName, ftpParams->srvInfo.pathNamePattern, ftpParams->roadInfo.roadName);

	return E_NO;
}

/*****************************************************************************
 Prototype    : FtpUpload_send_heartbeat
 Description  : send heart beat to server
 Input        : void *handle  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 ftp_upload_send_heartbeat(void *handle)
{
	FtpUploadHandle hUpload = (FtpUploadHandle)handle;
	Int32 			err;

	if(!handle)
		return E_INVAL;

	err = ftp_keep_alive(hUpload->hFtp);
	return err;
}

/* fxns for ftp upload */
const UploadFxns FTP_UPLOAD_FXNS = {
	.create = ftp_upload_create,
	.delete = ftp_upload_delete,
	.connect = ftp_upload_connect,
	.disconnect = ftp_upload_disconnect,
	.sendFrame = ftp_upload_send,
	.sendHeartBeat = ftp_upload_send_heartbeat,
	.setParams = ftp_upload_set_params,
	.saveFrame = NULL,
};

/*****************************************************************************
 Prototype    : ftp_upload_test
 Description  : test ftp upload
 Input        : const char *username  
                const char *passwd    
                const char *srvip     
                Uint16 port           
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/5/23
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 ftp_upload_test(const char *username, const char *passwd, const char *srvip, Uint16 port)
{
	FtpUploadParams 	params;
	void				*hFtpUpload;
	Int32				err;

	/* init params */
	bzero(&params, sizeof(params));
	params.size = sizeof(params);
	strncpy(params.srvInfo.serverIP, srvip, sizeof(params.srvInfo.serverIP));
	params.srvInfo.serverPort = port;
	strncpy(params.srvInfo.userName, username, sizeof(params.srvInfo.userName));
	strncpy(params.srvInfo.password, passwd, sizeof(params.srvInfo.password));
	//strcpy(params.srvInfo.rootDir, "/");
	strncpy(params.srvInfo.pathNamePattern, "<OSD>/<Y><M><D>/<DT>/<H><m><S><s>-<WN>-<SN>-<FN>", 
		sizeof(params.srvInfo.pathNamePattern));
	strncpy(params.roadInfo.roadName, "Test_Road", sizeof(params.roadInfo.roadName));
	params.roadInfo.devSN = 2012;
	params.roadInfo.roadNum = 5;
	params.roadInfo.directionNum = 1;
	
	int i;

	for(i = 0; i < 100; i++) {
		//params.srvInfo.pathNamePattern[0] = 0;
		hFtpUpload = ftp_upload_create(&params);
		assert(hFtpUpload);
		err = ftp_delete(hFtpUpload);
		assert(err == E_NO);
	}

	/* create ftp upload */
	hFtpUpload = ftp_upload_create(&params);
	assert(hFtpUpload);

	/* connect the server */
	err = ftp_upload_connect(hFtpUpload, 15);
	if(err) {
		ERR("connect failed");
		goto exit;
	} else
		DBG("connect ftp server %s ok", srvip);

	/* send file */
	ImgMsg img;
	CaptureInfo *capInfo = &img.capInfo;
	bzero(&img, sizeof(img));
	img.dimension.colorSpace = FMT_JPG;
	img.dimension.width = img.dimension.bytesPerLine = 1600;
	img.dimension.height = 1200;
	img.dimension.size = 500 * 1024; // Bytes

	BufHandle hBuf = buffer_alloc(2 * 1024 * 1024, NULL);
	assert(hBuf);

	memset(buffer_get_user_addr(hBuf), 0x55, buffer_get_size(hBuf));
	img.hBuf = hBuf;

	TriggerInfo *trigInfo = &capInfo->triggerInfo[0];
	capInfo->capCnt = 1;
	capInfo->limitSpeed = 80;
	trigInfo->frameId = FRAME_TRIG_BASE + 2;
	trigInfo->groupId = 1;
	trigInfo->redlightTime = 200;
	trigInfo->speed = 0;
	trigInfo->wayNum = 2;
	trigInfo->flags = TRIG_INFO_RED_LIGHT;

	capInfo->triggerInfo[1] = capInfo->triggerInfo[2] = *trigInfo;

	err = ftp_upload_send_heartbeat(hFtpUpload);
	assert(err == E_NO);

	for(i = 0; i < 100; i++) {

		/* get current time */	
		cam_get_time(&img.timeStamp);

		/* calc time cost */
		struct timeval tmStart,tmEnd; 
		float   timeUse;

		capInfo->capCnt = 1;
		trigInfo = &capInfo->triggerInfo[0];
		trigInfo->frameId = FRAME_TRIG_BASE;
		trigInfo->groupId++;
		if((i % 3) == 0) {
			trigInfo->wayNum++;
		}
		gettimeofday(&tmStart,NULL);
		err = ftp_upload_send(hFtpUpload, &img);
		gettimeofday(&tmEnd,NULL);
		
		assert(err == E_NO);

		timeUse = 1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec;
		DBG("<%d-1> upload ftp file size: %d KB cost: %.2f ms", i,  
			img.dimension.size/1000, timeUse/1000);	

		usleep(100000);

		/* upload image which has two cap info */
		capInfo->capCnt = 2;
		trigInfo->frameId++;
		trigInfo = &capInfo->triggerInfo[1];
		trigInfo->groupId = 23 + i;
		trigInfo->redlightTime = 0;
		trigInfo->wayNum = (i % 3) + 1;
		trigInfo->flags = TRIG_INFO_OVERSPEED | TRIG_INFO_LAST_FRAME;
		trigInfo->speed = 102;
		trigInfo->frameId = FRAME_TRIG_BASE;

		/* get current time */	
		cam_get_time(&img.timeStamp);

		/* send this frame */
		gettimeofday(&tmStart,NULL);
		err = ftp_upload_send(hFtpUpload, &img);
		gettimeofday(&tmEnd,NULL);
		
		assert(err == E_NO);

		timeUse = 1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec;
		DBG("upload ftp file size: %d KB cost: %.2f ms", 
			img.dimension.size/1000, timeUse/1000);

		usleep(300000);

		/* upload image which has two cap info */
		capInfo->capCnt = 3;
		capInfo->triggerInfo[0].frameId++;
		trigInfo->wayNum++;
		trigInfo = &capInfo->triggerInfo[2];
		trigInfo->groupId = 77 + i;
		trigInfo->redlightTime = 0;
		trigInfo->wayNum = (i + 1) % 3 + 1;
		trigInfo->flags = TRIG_INFO_RETROGRADE | TRIG_INFO_LAST_FRAME;
		trigInfo->speed = 102;

		/* get current time */	
		cam_get_time(&img.timeStamp);

		/* send this frame */
		gettimeofday(&tmStart,NULL);
		err = ftp_upload_send(hFtpUpload, &img);
		gettimeofday(&tmEnd,NULL);
		
		assert(err == E_NO);

		timeUse = 1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec;
		DBG("upload ftp file size: %d KB cost: %.2f ms", 
			img.dimension.size/1000, timeUse/1000);	

		sleep(1);

	}
	
	if(err == E_NO) {
		err = ftp_upload_disconnect(hFtpUpload);
		assert(err == E_NO);
	}

exit:
	err = ftp_upload_delete(hFtpUpload);
	assert(err == E_NO);

	return E_NO;
}

