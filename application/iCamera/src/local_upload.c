/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : local_upload.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/5/4
  Last Modified :
  Description   : local file scan and upload
  Function List :
              local_dir_send
              local_upload_create
              local_upload_delete
              local_upload_thread
              single_file_send
  History       :
  1.Date        : 2012/5/4
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "local_upload.h"
#include "log.h"
#include <pthread.h>
#include "app_msg.h"
#include <dirent.h>
#include <sys/stat.h>

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
#define LOCAL_UPLOAD_CONNECT_TIMEOUT	30000	//timeout for connect
#define PRINT_FILE_INFO
#define IMG_FILE_SUFFIX					".jpg"
#define VID_FILE_SUFFIX					".h264"

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/


/* object for this module */
struct LocalUploadObj {
	const char 			*path;			//path for file scan
	const char			*msgName;		//msg name for thread
	pthread_t			pid;			//thread id
	Int32				flags;			//flags for bit ctrl
	Bool				exit;			//flags for exit
	UploadHandle		hUpload;		//handle for upload
	CamImgUploadProto	uploadProto;	//upload protocol
	BufHandle			hBuf;			//buf for file read
	Int32				bufSize;		//size of buf
};

/*****************************************************************************
 Prototype    : local_upload_update
 Description  : update upload module
 Input        : LocalUploadHandle hLocalUpload  
                UploadParams *params            
                MsgHandle hMsg                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/5/7
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 local_upload_update(LocalUploadHandle hLocalUpload, UploadParams *params, MsgHandle hMsg)
{
	/* get upload protol */
	Int32 	ret = E_NO;

	if(!hLocalUpload->hUpload || params->protol != hLocalUpload->uploadProto) {
		/* create upload handle */
		UploadAttrs uploadAttrs;

		/* delete old handle */
		if(hLocalUpload->hUpload)
			upload_delete(hLocalUpload->hUpload, hMsg);

		usleep(10000);
		
		/* create upload handle for sending */
		uploadAttrs.savePath = hLocalUpload->path;
		uploadAttrs.flags = UPLOAD_FLAG_NOT_SAVE;
		uploadAttrs.reConTimeout = 30;
		uploadAttrs.msgName = hLocalUpload->msgName;

		hLocalUpload->hUpload = upload_create(&uploadAttrs,  params);
		if(!hLocalUpload->hUpload) {
			ERR("create upload handle failed");
			return E_INVAL;
		}
		hLocalUpload->uploadProto = params->protol;
		usleep(10000);				
	}else {
		/* update params */
		ret = upload_update_params(hLocalUpload->hUpload, params);
	}

	return ret;
}

/*****************************************************************************
 Prototype    : local_upload_create
 Description  : create this module
 Input        : LocalUploadAttrs *attrs  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/5/4
    Author       : Sun
    Modification : Created function

*****************************************************************************/
LocalUploadHandle local_upload_create(LocalUploadAttrs *attrs, UploadParams *params)
{
	LocalUploadHandle hLocalUpload;

	if(!attrs || !params || attrs->maxFileSize <= 0)
		return NULL;

	hLocalUpload = calloc(1, sizeof(struct LocalUploadObj));
	if(!hLocalUpload) {
		ERR("alloc mem failed.");
		return NULL;
	}

	/* alloc buffer */
	hLocalUpload->hBuf = buffer_alloc(attrs->maxFileSize, NULL);
	if(!hLocalUpload->hBuf) {
		ERR("alloc buf for single file read failed");
		goto err_quit;
	}

	/* record params */
	hLocalUpload->hUpload = NULL;
	hLocalUpload->path = attrs->filePath;
	hLocalUpload->flags = attrs->flags;
	hLocalUpload->msgName = attrs->msgName;
	hLocalUpload->bufSize = attrs->maxFileSize;

	/* create upload handle for sending */
	if(local_upload_update(hLocalUpload, params, NULL) != E_NO) {
		goto err_quit;
	}
	
	return hLocalUpload;

err_quit:

	if(hLocalUpload->hBuf)
		buffer_free(hLocalUpload->hBuf);

	free(hLocalUpload);

	return NULL;
}

/*****************************************************************************
 Prototype    : local_upload_delete
 Description  : delete this module
 Input        : LocalUploadHandle hLocalUpload  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/5/4
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 local_upload_delete(LocalUploadHandle hLocalUpload, MsgHandle hCurMsg)
{
	if(!hLocalUpload)
		return E_INVAL;

	hLocalUpload->exit = TRUE;
	if(hLocalUpload->pid) {
		/* tell ansync thread to exit */
		app_hdr_msg_send(hCurMsg, hLocalUpload->msgName, APPCMD_EXIT, 0, 0);
		pthread_join(hLocalUpload->pid, NULL);
	}

	if(hLocalUpload->hUpload)
		upload_delete(hLocalUpload->hUpload, hCurMsg);

	if(hLocalUpload->hBuf)
		buffer_free(hLocalUpload->hBuf);

	free(hLocalUpload);

	return E_NO;
}


/*****************************************************************************
 Prototype    : single_file_send
 Description  : send single file
 Input        : LocalUploadHandle hLocalUpload  
                const char *fileName            
                MsgHandle hMsg                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/5/4
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 single_file_send(LocalUploadHandle hLocalUpload, const char *fileName, MsgHandle hMsg)
{
	Int8		*buf = buffer_get_user_addr(hLocalUpload->hBuf);
	Int32		dataLen;
	Int32		fd;		//fd for file
	Int32		ret;
	struct stat fstat;
	ImgMsg		img;	
	
	ret = stat(fileName, &fstat);
	if(ret < 0) {
		ERRSTR("stat <%s> err", fileName);
		return E_IO;
	}

	if(!S_ISREG(fstat.st_mode)) {
		/* ignore unregular file */
		return E_NO;
	}

	/* ignore file that has suffix is not *.jpg */
	size_t nameLen = strlen(fileName);
	size_t suffixLen = strlen(IMG_FILE_SUFFIX);
	if(strncmp(fileName + nameLen - suffixLen, IMG_FILE_SUFFIX, suffixLen) != 0)
		return E_NO;
	
	fd = open(fileName, O_RDONLY);
	if(fd < 0) {
	    ERRSTR("open <%s> err", fileName);
	    return E_NOTEXIST;
	}

	dataLen = fstat.st_size - sizeof(img);
	if(hLocalUpload->bufSize < dataLen) {
	    //ERR("file is too large: %d, max: %d", 
			//dataLen, hLocalUpload->bufSize);
		/* ignore large file */
		ret = E_NO;
		goto exit;
	}

	/* read header */
	ret = read(fd, &img, sizeof(img));
	if(ret != sizeof(img)) {
		//ERRSTR("read header err");
		goto exit;
	}

	/* validate header */
	dataLen = img.dimension.size;
	if(img.header.type != MSG_TYPE_REQU || dataLen > hLocalUpload->bufSize)
		goto exit;

	/* read data */
	ret = read(fd, buf, dataLen);
	if(ret < dataLen) {
	    ERRSTR("read data from <%s> err", fileName);
		goto exit;
	} 
	
	img.hBuf = hLocalUpload->hBuf;

	/* send file */
	ret = upload_run(hLocalUpload->hUpload, hMsg, &img);

	/* delete file if success or the file contains invalid data */
	if((!ret || ret == E_CHECKSUM) && (hLocalUpload->flags & LOCAL_FLAG_DEL_AFTER_SND))
		unlink(fileName);	
	
#ifdef PRINT_FILE_INFO
	if(!ret)
		DBG("local upload, send file <%s>, %d bytes", fileName, (int)fstat.st_size);
#endif
	
exit:		
	
	close(fd); 

	return ret;
	
}

/*****************************************************************************
 Prototype    : local_dir_send
 Description  : send dir
 Input        : LocalUploadHandle hLocalUpload  
                const char *dirName             
                MsgHandle hMsg                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/5/4
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 local_dir_send(LocalUploadHandle hLocalUpload, const char *dirName, MsgHandle hMsg)
{
	DIR 			*dir = NULL;
	struct dirent 	*dirent = NULL;
	char			pathName[FILENAME_MAX];
	Int32			err = E_NO;

	if (!dirName)
		return E_INVAL;

	dir = opendir(dirName);
	if(!dir) {
		//ERRSTR("opendir <%s> err", dirName);
		err = E_NOTEXIST;
		goto exit;
	}

	//DBG("read dir: %s", dirName);
	for (dirent = readdir(dir); dirent; ) {
		if ('.' != dirent->d_name[0]) {
			if (DT_DIR == dirent->d_type) {
				/* this is dir, goto this one */
				snprintf(pathName, sizeof(pathName), "%s/%s", dirName, dirent->d_name);
				err = local_dir_send(hLocalUpload, pathName, hMsg);
				if(err)
					break;
			} else if(DT_REG == dirent->d_type) {
				/* regular file, send this file */
				snprintf(pathName, sizeof(pathName), "%s/%s", dirName, dirent->d_name);
				err = single_file_send(hLocalUpload, pathName, hMsg);
				if(err)
					break;
			} else {
				/* other file, skip this one */
				continue;
			}
		}

		/* list another entry */
		dirent = readdir(dir);
	}


	if(!err) {
		/* delete this dir if it is not the top dir */
		if( strcmp(dirName, hLocalUpload->path) &&
			(hLocalUpload->flags & LOCAL_FLAG_DEL_AFTER_SND)) {
			rmdir(dirName);
		}
	}

exit:

	if (dir)
		closedir(dir);

	return err;
}


/*****************************************************************************
 Prototype    : local_upload_thread
 Description  : thread for file scan and upload
 Input        : void *arg  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/5/4
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static void *local_upload_thread(void *arg)
{
	assert(arg);
	
	LocalUploadHandle 	hLocalUpload = arg;
	Int32				err;
	MsgHandle			hMsg = msg_create(hLocalUpload->msgName, NULL, 0);
	Int32				delayTime;
	CommonMsg			msgBuf;
	
	while(!hLocalUpload->exit) {
		delayTime = 30;
		
		if(hLocalUpload->flags & LOCAL_FLAG_AUTO_UPLOAD_EN) {
			/* scan and send all files available */
			err = local_dir_send(hLocalUpload, hLocalUpload->path, hMsg);
			if(!err) {
				/* all file send success, disconnect server and wait longer time */
				upload_disconnect(hLocalUpload->hUpload);
				delayTime = 60;
			}
		}

		/* recv msg */
		msg_set_recv_timeout(hMsg, delayTime);
		err = msg_recv(hMsg, (MsgHeader *)&msgBuf, sizeof(msgBuf), 0);
		if(!err) {
			switch(msgBuf.header.cmd) {
			case APPCMD_EXIT:
				hLocalUpload->exit = TRUE;
				break;
			case APPCMD_SET_UPLOAD_PARAMS:
				if(msgBuf.header.dataLen == sizeof(UploadParams)) {
					local_upload_update(hLocalUpload, (UploadParams *)msgBuf.buf, hMsg);
				} else
					ERR("invalid len for local upload update");
				break;
			case APPCMD_SEND_DIR:
				local_dir_send(hLocalUpload, msgBuf.buf, hMsg);
				break;
			case APPCMD_SEND_FILE:
				single_file_send(hLocalUpload, msgBuf.buf, hMsg);
				break;
			default:
				break;
			}
		}
	}

	
	DBG("local upload thread exit...");
	msg_delete(hMsg);
	
	pthread_exit(0);

}

/*****************************************************************************
 Prototype    : local_upload_run
 Description  : start running this module
 Input        : LocalUploadHandle hLocalUpload  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/5/4
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 local_upload_run(LocalUploadHandle hLocalUpload)
{
	Int32 err;
	
	if(!hLocalUpload)
		return E_INVAL;

	if(!hLocalUpload->pid) {
		/* create thread to run */
		err = pthread_create(&hLocalUpload->pid, NULL, local_upload_thread, hLocalUpload);
		if(err < 0) {
			ERRSTR("create thread failed");
			return E_IO;
		}
	}

	return E_NO;
}

/*****************************************************************************
 Prototype    : local_upload_cfg
 Description  : cfg upload params
 Input        : LocalUploadHandle hLocalUpload  
                UploadParams *params            
                MsgHandle hCurMsg               
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/5/7
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 local_upload_cfg(LocalUploadHandle hLocalUpload, UploadParams *params, Int32 flags, MsgHandle hCurMsg)
{
	if(!hLocalUpload || !params)
		return E_INVAL;

	Int32 ret = E_NO;
	
	if(hLocalUpload->pid > 0 && hCurMsg) {
		struct {
			MsgHeader 		hdr;
			UploadParams	params;
		}msg;

		/* send msg to our thread to update params */
		msg.hdr.cmd = APPCMD_SET_UPLOAD_PARAMS;
		msg.hdr.index = 0;
		msg.hdr.dataLen = sizeof(UploadParams);
		msg.hdr.type = MSG_TYPE_REQU;
		msg.params = *params;

		/* save ctrl flags */
		hLocalUpload->flags = flags;
		
		ret = msg_send(hCurMsg, hLocalUpload->msgName, (MsgHeader *)&msg, 0);
	}

	return ret;
}

/*****************************************************************************
 Prototype    : local_upload_send
 Description  : ansync send file or dir to server
 Input        : LocalUploadHandle hLocalUpload  
                Bool isDir                      
                const char *pathName            
                MsgHandle hCurMsg               
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/5/7
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 local_upload_send(LocalUploadHandle hLocalUpload, Bool isDir, const char *pathName, MsgHandle hCurMsg)
{
	if(!hLocalUpload || !pathName || !hCurMsg) {
		//ERR("local upload send, inv val: %d-%d-%d", (int)hLocalUpload, (int)pathName, (int)hCurMsg);
		return E_INVAL;
	}

	if(hLocalUpload->pid == 0) {
		ERR("local upload has not start running.");
		return E_MODE;
	}
	
	/* send cmd for ansync upload */
	CommonMsg msg;
	
	memset(&msg, 0, sizeof(msg));
	msg.header.cmd = isDir ? APPCMD_SEND_DIR : APPCMD_SEND_FILE;
	msg.header.dataLen = strlen(pathName) + 1;
	msg.header.type = MSG_TYPE_REQU;
	if(msg.header.dataLen > sizeof(msg.buf)) {
		ERR("path name too long.");
		return E_NOMEM;
	}

	/* copy path name */
	strcpy(msg.buf, pathName);

	return msg_send(hCurMsg, hLocalUpload->msgName, (MsgHeader *)&msg, 0);
}


#include "ftp_upload.h"
#include "tcp_upload.h"

/*****************************************************************************
 Prototype    : local_upload_test
 Description  : Test this module
 Input        : const char *pathName  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/5/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 local_upload_test(const char *pathName)
{
	LocalUploadHandle 	hLocal;
	LocalUploadAttrs	attrs;
	UploadParams		params;
	FtpUploadParams		ftpUpload;
	ImgTcpUploadParams	tcpUpload;

	assert(pathName);
	attrs.filePath = pathName;
	attrs.flags = LOCAL_FLAG_AUTO_UPLOAD_EN;
	attrs.maxFileSize = 2 * 1024 * 1024; 
	attrs.msgName = "/tmp/localUpload";

	params.protol = CAM_UPLOAD_PROTO_FTP;
	bzero(&ftpUpload, sizeof(ftpUpload));
	strcpy(ftpUpload.roadInfo.roadName, "Test_Road");
	strcpy(ftpUpload.srvInfo.serverIP, "192.168.0.15");
	strcpy(ftpUpload.srvInfo.userName, "test");
	strcpy(ftpUpload.srvInfo.password, "123456");
	ftpUpload.srvInfo.serverPort = 21;
	ftpUpload.size = sizeof(ftpUpload);

	bzero(&tcpUpload, sizeof(tcpUpload));
	tcpUpload.size = sizeof(tcpUpload);
	strcpy(tcpUpload.srvInfo.serverIP, "192.168.0.15");
	tcpUpload.srvInfo.serverPort = 9300;

	bzero(params.paramsBuf, sizeof(params.paramsBuf));
	memcpy(params.paramsBuf, &ftpUpload, sizeof(ftpUpload));
	hLocal = local_upload_create(&attrs, &params);
	assert(hLocal);

	Int32 err;
	MsgHandle hMsg = msg_create("/tmp/localUploadTest", NULL, 0);

	assert(hMsg);

	err = local_upload_run(hLocal);
	assert(err == E_NO);
	sleep(1);
	
	int i = 0;

	while(i++ < 100) {
		//sleep(i + 1);
		#if 1
		bzero(params.paramsBuf, sizeof(params.paramsBuf));
		if(params.protol == CAM_UPLOAD_PROTO_FTP) {
			// change to tcp
			params.protol = CAM_UPLOAD_PROTO_TCP;
			memcpy(params.paramsBuf, &tcpUpload, sizeof(tcpUpload));
		} else {
			params.protol = CAM_UPLOAD_PROTO_FTP;
			memcpy(params.paramsBuf, &ftpUpload, sizeof(ftpUpload));
		}
		#endif
		
		err = local_upload_cfg(hLocal, &params, attrs.flags, hMsg);
		//err = local_upload_update(hLocal, &params, hMsg);
		assert(err == E_NO);
		
	}
	
	err = local_upload_delete(hLocal, hMsg);
	assert(err == E_NO);

	return E_NO;
}


