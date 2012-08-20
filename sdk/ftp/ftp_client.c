/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : ftp_client.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/1/14
  Last Modified :
  Description   : Ftp client module
  Function List :
              ftp_change_working_dir
              ftp_check_connect_status
              ftp_connect_server
              ftp_create
              ftp_delete
              ftp_delete_dir
              ftp_delete_file
              ftp_disconnect_server
              ftp_download_file
              ftp_enter_transfer_mode
              ftp_get_list
              ftp_get_working_dir
              ftp_keep_alive
              ftp_make_dir
              ftp_recv_data
              ftp_rename
              ftp_send_cmd
              ftp_send_make_dir_cmd
              ftp_set_password
              ftp_set_server_ip
              ftp_set_server_port
              ftp_set_user_name
              ftp_upload_files
  History       :
  1.Date        : 2012/1/14
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "common.h"
#include "net_utils.h"
#include "ftp_client.h"

/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/

/*----------------------------------------------*
 * external routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/
 
/* Private functions declare */
//static char *itoa(int value, char *string, int radix);
static inline Int32 ftp_send_cmd(FtpHandle hFtp,const char * cmd,Int32 cmdLen);
static Int32 ftp_recv_data(FtpHandle hFtp, char * buf, Uint32 bufSize, Uint32 * recvLen);
static Bool ftp_check_connect_status(FtpHandle hFtp);
static Int32 ftp_enter_transfer_mode(FtpHandle hFtp);

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
//#define FTP_CHECK_SIZE 		1
#define FTP_MAX_RETRY_CNT	5

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/*****************************************************************************
 Prototype    : ftp_set_server_ip
 Description  : Set server IP
 Input        : FtpHandle hFtp     
                const Int8 *ipStr  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 ftp_set_server_ip(FtpHandle hFtp, const Int8 *ipStr)
{
	if(!hFtp || !ipStr)
		return E_INVAL;

	if(hFtp->status == FTP_STAT_TRANSFER) {
		ERR("a transfer is still in progess.");
		return E_MODE;
	}

	Uint32 tmp = inet_addr(ipStr);
	if(FTP_INADDR_NONE == tmp)
		return E_INVAL;

	if(hFtp->status == FTP_STAT_IDLE)
		ftp_disconnect_server(hFtp);
	
	hFtp->serverIP = tmp;
#ifdef FTP_DEBUG_EN
	char buf[16];
	DBG("ftp server ip %s", inet_ntop(AF_INET, &tmp, buf, sizeof(buf)));
#endif

	return E_NO;
}

/*****************************************************************************
 Prototype    : ftp_set_server_port
 Description  : Set server port
 Input        : FtpHandle hFtp  
                Uint16 port     
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 ftp_set_server_port(FtpHandle hFtp, Uint16 port)
{
	if(!hFtp)
		return E_INVAL;

	if(hFtp->status == FTP_STAT_TRANSFER) {
		ERR("a transfer is still in progess.");
		return E_MODE;
	}

	if(hFtp->status == FTP_STAT_IDLE)
		ftp_disconnect_server(hFtp);
	
	hFtp->serverPort = htons(port);
#ifdef FTP_DEBUG_EN
	DBG("ftp server port %u", port);
#endif

	return E_NO;
}

/*****************************************************************************
 Prototype    : ftp_set_user_name
 Description  : Set login user name
 Input        : FtpHandle hFtp        
                const Int8 *userName  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 ftp_set_user_name(FtpHandle hFtp, const Int8 *userName)
{
	if(!hFtp || !userName)
		return E_INVAL;

	if(hFtp->status == FTP_STAT_TRANSFER) {
		ERR("a transfer is still in progess.");
		return E_MODE;
	}

	if(hFtp->status == FTP_STAT_IDLE)
		ftp_disconnect_server(hFtp);
	
	if (strlen(userName) > FTP_MAX_USERNAME_LEN - 1)
		return E_NOMEM;
	
	strcpy(hFtp->userName, userName);
#ifdef FTP_DEBUG_EN
	DBG("ftp user name %s", hFtp->userName);
#endif
	return E_NO;
}

/*****************************************************************************
 Prototype    : ftp_set_password
 Description  : Set password for login
 Input        : FtpHandle hFtp        
                const Int8 *password  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 ftp_set_password(FtpHandle hFtp, const Int8 *password)
{
	if(!hFtp || !password)
		return E_INVAL;

	if(hFtp->status == FTP_STAT_TRANSFER) {
		ERR("a transfer is still in progess.");
		return E_MODE;
	}

	if(hFtp->status == FTP_STAT_IDLE)
		ftp_disconnect_server(hFtp);

	if (strlen(password) > FTP_MAX_PASSWD_LEN - 1)
		return E_NOMEM;
	
	strcpy(hFtp->passWord, password);
#ifdef FTP_DEBUG_EN
		DBG("ftp password %s", hFtp->passWord);
#endif

	return E_NO;
}

/*****************************************************************************
 Prototype    : ftp_set_trans_timeout
 Description  : set send and recv timeout
 Input        : FtpHandle hFtp     
                Uint32 sndTimeout  
                Uint32 rcvTimeout  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/20
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 ftp_set_trans_timeout(FtpHandle hFtp, Uint32 sndTimeout, Uint32 rcvTimeout)
{
	if(!hFtp)
		return E_INVAL;

	hFtp->sndTimeout = sndTimeout;
	hFtp->recvTimeout = rcvTimeout;

	return E_NO;
}

/*****************************************************************************
 Prototype    : ftp_create
 Description  : Create ftp client handle
 Input        : const Int8 *userName  
                const Int8 *password  
                const Int8 *serverIP  
                Uint16 serverPort     
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
FtpHandle ftp_create(const Int8 *userName, const Int8 *password, const Int8 *serverIP, Uint16 serverPort)
{
	FtpHandle	hFtpClient;
	Int32		err = E_NO;
	
	hFtpClient = (FtpHandle)calloc(1, sizeof(FtpObject));
	if(!hFtpClient) {
		ERRSTR("mem alloc FtpObject failed.");
		return NULL;
	}

	hFtpClient->status = FTP_STAT_UNCONNECTED;
	hFtpClient->connectMode = FTP_PASSIVE_MODE;
	hFtpClient->cmdSock = -1;
	hFtpClient->dataSock = -1;
	hFtpClient->sndTimeout = FTP_SEND_TIMEOUT;
	hFtpClient->recvTimeout = FTP_RECV_TIMEOUT;

	if(userName != NULL)
		err |= ftp_set_user_name(hFtpClient, userName);
	if(password != NULL)
		err |= ftp_set_password(hFtpClient, password);
	if(serverIP != NULL)
		err |= ftp_set_server_ip(hFtpClient, serverIP);
	if(serverPort != 0)
		err |= ftp_set_server_port(hFtpClient, serverPort);

	if(err) {
		ERR("Ftp_create, set param failed.");
		free(hFtpClient);
		return NULL;
	}

	return hFtpClient;
}

/*****************************************************************************
 Prototype    : ftp_delete
 Description  : Delete ftp client handle
 Input        : FtpHandle hFtp  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 ftp_delete(FtpHandle hFtp)
{
	Int32 err;
	
	err = ftp_disconnect_server(hFtp);
	
	if(hFtp)
		free(hFtp);

	return err;
}

/*****************************************************************************
 Prototype    : ftp_connect_server
 Description  : Connect ftp server
 Input        : FtpHandle hFtp      
                Uint32 unTimeoutMs  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 ftp_connect_server(FtpHandle hFtp, Int32 timeoutSec)
{
	Int32	cmdSock;
	Int32	ret;
	Int32 	i = 0;
	Int8 	*pData;
	struct sockaddr_in serverAddr;

	if(!hFtp)
		return E_INVAL;

	/* Set server addr */
	serverAddr.sin_addr.s_addr= hFtp->serverIP;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = hFtp->serverPort;
	
	/* create socket */
	if((cmdSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		ERRSTR("create cmd sock failed.");
		return E_IO;
	}
		
	if(timeoutSec > 0)
		ret = connect_nonblock(cmdSock,(struct sockaddr *)&serverAddr, sizeof(serverAddr), timeoutSec);
	else
		ret = connect(cmdSock, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

	if(ret) {
		ret = E_CONNECT;
		ERR("connect error.");
		goto connect_server_quit;
	}

	ret = 0;
	if(hFtp->sndTimeout)
		ret |= set_sock_send_timeout(cmdSock, hFtp->sndTimeout);
	if(hFtp->recvTimeout)
		ret |= set_sock_recv_timeout(cmdSock, hFtp->recvTimeout);
	if(ret) {
		ret = E_INVAL;
		ERR("set sock opt error.");
		goto connect_server_quit;
	}

	/* recv cmd */
	ret = recv(cmdSock, hFtp->cmdBuf, FTP_MAX_LINE_SIZE, 0);
	if(ret <= 0) {
		ret = E_TRANS;
		ERR("recv after connect error.");
		goto connect_server_quit;
	}

	#ifdef FTP_DEBUG_EN
	//DBG("connect to server ok, recv %s", hFtp->cmdBuf);
	#endif
	
	if(strncmp(hFtp->cmdBuf, "220", 3) != 0) {
		ret = E_CONNECT;
		ERR("recv incorrect data.");
		goto connect_server_quit;
	}

	/* send username */
	int len = sprintf(hFtp->cmdBuf, "USER %s\r\n", hFtp->userName);
	hFtp->cmdSock = cmdSock;

	ret = ftp_send_cmd(hFtp, hFtp->cmdBuf, len);
	if(ret) {
		ERR("send username error.");
		goto connect_server_quit;
	}
	
	if(strncmp(hFtp->cmdBuf, "331", 3) != 0) {
		ret = E_INVUSER;
		ERR("invalid user name.");
		goto connect_server_quit;
	}

	/* send password */
	len = sprintf(hFtp->cmdBuf, "PASS %s\r\n", hFtp->passWord);
	ret = ftp_send_cmd(hFtp, hFtp->cmdBuf, len);
	if(ret) {
		ERR("send passwd error.");
		goto connect_server_quit;
	}
	
	if(strncmp(hFtp->cmdBuf, "230", 3) && strncmp(hFtp->cmdBuf, "231", 3)) {
		ret = E_INVPASSWD;
		ERR("invalid passwd.");
		goto connect_server_quit;
	}

	#ifdef FTP_DEBUG_EN
	DBG("ftp_connect_server, successfully login...");
	#endif

	/* get current dir */
	ret = ftp_send_cmd(hFtp, "PWD \r\n",  6);
	if (ret) {
		ERR("send pwd cmd err.");
		goto connect_server_quit;
	}

	/* Record current dir */
	if(strncmp(hFtp->cmdBuf, "257", 3) == 0) {
	
		pData = hFtp->cmdBuf + 5;
		while(*(pData) != '\"' && i < FTP_MAX_LINE_SIZE - 5)
		{
			*(hFtp->currentDir + i) = *(pData);
			pData++;
			i++;
		}
		*(hFtp->currentDir + i) = '\0';
	}
	#ifdef FTP_DEBUG_EN
	DBG("current dir is: %s", hFtp->currentDir);
	#endif

	//set transfer mode: binary
	ret = ftp_send_cmd(hFtp, "TYPE I\r\n", 8);
	if(ret) {
		ERR("send type I cmd err.");
		goto connect_server_quit;
	}
	
	hFtp->cmdSock = cmdSock;
	hFtp->status = FTP_STAT_IDLE;
	return E_NO;

connect_server_quit:
	close(cmdSock);
	hFtp->cmdSock = -1;
	hFtp->status = FTP_STAT_UNCONNECTED;
	return ret;
}

/*****************************************************************************
 Prototype    : ftp_disconnect_server
 Description  : Disconnect with server
 Input        : FtpHandle hFtp  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 ftp_disconnect_server(FtpHandle hFtp)
{
	Int32	err;
		
	if(!hFtp)
		return E_INVAL;

	/* Still connected with server */
	if(ftp_check_connect_status(hFtp)) {
		/* Send quit cmd */
		err = ftp_send_cmd(hFtp, "QUIT \r\n", 7);
		if (err)
			ERR("send quit cmd error.");
		
		if(strncmp(hFtp->cmdBuf, "221", 3) != 0)
			ERR("reply data incorrect.");

#ifdef FTP_DEBUG_EN
		DBG("ftp_disconnect_server, disconnect server...");
#endif
		err = close(hFtp->cmdSock);

		if(hFtp->dataSock >= 0)
			close(hFtp->dataSock);
		
		hFtp->cmdSock = hFtp->dataSock = -1;
		hFtp->status = FTP_STAT_UNCONNECTED;
		
		if (err)
			return E_TRANS;
	}

	return E_NO;
}

/*****************************************************************************
 Prototype    : ftp_send_make_dir_cmd
 Description  : send make dir cmd once
 Input        : FtpHandle hFtp  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 ftp_send_make_dir_cmd(FtpHandle hFtp, const Int8 *dirName)
{
	Int32 err;
	
	Int32 len = sprintf(hFtp->cmdBuf, "MKD %s\r\n", dirName); 
	DBG("send cmd: %s", hFtp->cmdBuf);
	err = ftp_send_cmd(hFtp, hFtp->cmdBuf, len);
	if(err)
		return err;

	/* check reply */
	if(strncmp(hFtp->cmdBuf, "550 ", 3) == 0) {
		ERR("make dir %s is dennied.", dirName);
		return E_REFUSED;
	}

	if(strncmp(hFtp->cmdBuf, "257 ", 3) == 0)
		return E_NO;
	else {
		ERR("unkown reply data.");
		return E_AGAIN;
	}
}

/*****************************************************************************
 Prototype    : ftp_make_dir
 Description  :  make dir recursivly
 Input        : FtpHandle hFtp  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 ftp_make_dir(FtpHandle hFtp, const Int8 *ftpDir)
{
	if (!hFtp || !ftpDir || *ftpDir == '\0') {
		ERR("ftp_make_dir, invalid dir name.");
		return E_INVAL;
	}

	if(strlen(ftpDir) > FTP_MAX_DIR_LEN) {
		ERR("dir name too long.");
		return E_NOMEM;
	}

	Int32	err;
	
	if (!ftp_check_connect_status(hFtp)) {
		WARN("ftp_make_dir, checkstatus failed, reconnect server.");
		err = ftp_connect_server(hFtp, FTP_RECONNECT_TIMEOUT);
		if (err) {
			ERR("ftp_make_dir, reconnect server failed.");
			return err;
		}
	}

	if(ftpDir[0] == '/' && ftpDir[1] == '\0') //need not make top dir
		return E_NO;

	Int32	i = 0;
	Int8	dirName[FTP_MAX_DIR_LEN];
	
	bzero(dirName, sizeof(dirName));
	if(ftpDir[0] == '/')
	{
		dirName[0] = '/';	//top dir, needn't make
		i = 1;
	}

	//printf("%s\n", ftpDir);
	
	while(1) {
		while(ftpDir[i] != '/' && ftpDir[i] != '\0') {
			dirName[i] = ftpDir[i];
			i++;
		}
		
		dirName[i] = '\0';
		if((err = ftp_send_make_dir_cmd(hFtp, dirName)) != E_NO) {
			if(err != E_REFUSED) {
				ERR("Make dir in ftp server failed: %u!", err);
				break;
			}
		}
		
		if(ftpDir[i] == '\0') //end of string
			break;

		dirName[i] = '/';
		i++;
	}
	
	#ifdef FTP_DEBUG_EN
	if(err == E_NO)
		DBG("ftp_make_dir, make dir %s success", ftpDir);
	#endif

	if(err == E_NO)
		err = ftp_get_working_dir(hFtp, NULL, 0); //update current dir
	return err;
}

/*****************************************************************************
 Prototype    : ftp_change_working_dir
 Description  :  change current working dir on server
 Input        : FtpHandle hFtp  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 ftp_change_working_dir(FtpHandle hFtp, const char *newDir)
{
	Int32	err;
	
	if(!hFtp || !newDir || *newDir == '\0')
		return E_INVAL;

	if(strlen(newDir) > FTP_MAX_DIR_LEN) {
		ERR("dir name too long.");
		return E_INVPATH;
	}

	if (!ftp_check_connect_status(hFtp)) {
		WARN("ftp_change_working_dir, checkstatus failed, reconnect server.");
		err = ftp_connect_server(hFtp, FTP_RECONNECT_TIMEOUT);
		if (err)
			return err;
	}

	Int32 len = snprintf(hFtp->cmdBuf, sizeof(hFtp->cmdBuf), "CWD %s\r\n", newDir);
	err = ftp_send_cmd(hFtp, hFtp->cmdBuf, len);
	if (err)
		return err;
	
	if (strncmp(hFtp->cmdBuf, "250", 3))
	{
		ERR("ftp_change_working_dir, CWD cmd is dennied.");
		return E_REFUSED;
	}
	
	strcpy(hFtp->currentDir, newDir);
	#ifdef FTP_DEBUG_EN
	DBG("ftp_change_working_dir, change dir to %s", newDir);
	#endif
	
	return E_NO;
}

/*****************************************************************************
 Prototype    : ftp_rename
 Description  :  rename a dir
 Input        : FtpHandle hFtp  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 ftp_rename(FtpHandle hFtp, const Int8 *newPathName, const Int8 *oldPathName)
{
	Int32	err;

	if(!hFtp || !newPathName || !oldPathName)
		return E_INVAL;
	
	if(strlen(newPathName) > FTP_MAX_DIR_LEN || strlen(oldPathName) > FTP_MAX_DIR_LEN) {
		ERR("dir path name too long.");
		return E_INVPATH;
	}
	
	if(!ftp_check_connect_status(hFtp)) {
		err = ftp_connect_server(hFtp, FTP_RECONNECT_TIMEOUT);
		if (err)
			return err;
	}

	Int32 len = sprintf(hFtp->cmdBuf, "RNFR %s\r\n", oldPathName);

	err = ftp_send_cmd(hFtp, hFtp->cmdBuf, len);
	if(err)
		return err;
	
	if(strncmp(hFtp->cmdBuf, "350", 3) != 0){
		ERR(" RNFR cmd is dennied.");
		return E_REFUSED; 
	}
	
	/* Send new dir name */
	len = sprintf(hFtp->cmdBuf, "RNTO %s\r\n", newPathName);
	err = ftp_send_cmd(hFtp, hFtp->cmdBuf, len);
	if(err)
		return err;
	
	if(strncmp(hFtp->cmdBuf, "250", 3) != 0) {
		ERR("RNTO cmd is dennied.");
		return E_REFUSED;
	}
	
	/* Check if current working dir is renamed */
	if(strcmp(oldPathName, hFtp->currentDir) == 0) 
	{
		bzero(hFtp->currentDir, FTP_MAX_LINE_SIZE);
		strcpy(hFtp->currentDir, newPathName);
	}

	#ifdef FTP_DEBUG_EN
	DBG("ftp_rename, rename dir %s to %s success", oldPathName, newPathName);
	#endif
	return E_NO;
}

/*****************************************************************************
 Prototype    : ftp_get_working_dir
 Description  :  get current working dir
 Input        : FtpHandle hFtp  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 ftp_get_working_dir(FtpHandle hFtp, Int8 *dirNameBuf, Uint32 bufLen)
{
	Int32	err;

	if(!hFtp)
		return E_INVAL;

	if(!ftp_check_connect_status(hFtp)) {
		WARN("ftp_get_working_dir, checkstatus failed, reconnect server.");
		err = ftp_connect_server(hFtp, FTP_RECONNECT_TIMEOUT); 
		if (err)
			return err;
	}

	err = ftp_send_cmd(hFtp, "PWD \r\n", 6);
	if(err)
		return err;

	/*  Record current working dir */
	Int32	i = 0;
	if(strncmp(hFtp->cmdBuf, "257", 3) == 0) {
		Int8	*pTemp = hFtp->cmdBuf + 5;
		while(*(pTemp) != '\"' && i < FTP_MAX_LINE_SIZE - 5) {
			*(hFtp->currentDir + i) = *(pTemp);
			pTemp++;
			i++;
		}
		*(hFtp->currentDir + i) = '\0';
	} else {
		ERR("PWD cmd is dennied.");
		return E_REFUSED;
	}
	
	if(dirNameBuf) {
		strncpy(dirNameBuf, hFtp->currentDir, bufLen);
	}
	
	#ifdef FTP_DEBUG_EN
	DBG("ftp_get_working_dir, current dir is %s", hFtp->currentDir);
	#endif
	
	return E_NO;
}

/*****************************************************************************
 Prototype    : ftp_delete_dir
 Description  : delete an empty dir
 Input        : FtpHandle hFtp  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 ftp_delete_dir(FtpHandle hFtp, const Int8 *ftpDir)
{
	if(!hFtp || !ftpDir)
		return E_INVAL;
	
	if (strlen(ftpDir) > FTP_MAX_DIR_LEN) {
		ERR("dir name too long.");
		return E_INVPATH;
	}

	Int32 err;
	if (!ftp_check_connect_status(hFtp)) {
		WARN("ftp_delete_dir, checkstatus failed reconnect server.");
		err = ftp_connect_server(hFtp, FTP_RECONNECT_TIMEOUT);
		if (err)
			return err;
	}
	

	Int32 len = sprintf(hFtp->cmdBuf, "RMD %s\r\n", ftpDir);
	
	err = ftp_send_cmd(hFtp, hFtp->cmdBuf, len);
	if(err)
		return err;
	
	if(strncmp(hFtp->cmdBuf, "250", 3)) {
		ERR("ftp_delete_dir, RMD cmd is dennied: %s.", hFtp->cmdBuf);
		return E_REFUSED;
	}

	if(strcmp(ftpDir, hFtp->currentDir) == 0)  {
		/* Current working dir is deleted */
		err = ftp_send_cmd(hFtp, "PWD \r\n", 6);
		if(err)
			return err;

		if(strncmp(hFtp->cmdBuf, "257", 3) == 0) {
			/* Record current dir */
			char 	*pTemp = hFtp->cmdBuf + 5;
			Int32	i = 0;
			while(*(pTemp) != '\"' && i < FTP_MAX_LINE_SIZE - 5) {
				*(hFtp->currentDir + i) = *(pTemp);
				pTemp++;
				i++;
			}
			*(hFtp->currentDir + i) = '\0';
		} else {
			ERR("PWD cmd is denied: %s", hFtp->cmdBuf);
			return E_REFUSED;
		}
	}

	#ifdef FTP_DEBUG_EN
	DBG("ftp_delete_dir, Delete dir %s success", ftpDir);
	#endif
	return E_NO;
}

/*****************************************************************************
 Prototype    : ftp_delete_file
 Description  : delete a file
 Input        : FtpHandle hFtp  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/

Int32 ftp_delete_file(FtpHandle hFtp, const Int8 *pathName)
{
	Int32	err;
	
	if(!hFtp || !pathName)
		return E_INVAL;
	
	if (strlen(pathName) > FTP_MAX_LINE_SIZE - 5) {
		ERR("file path name too long.");
		return E_INVNAME;
	}
	
	if (!ftp_check_connect_status(hFtp)) {
		err = ftp_connect_server(hFtp, FTP_RECONNECT_TIMEOUT);
		WARN("ftp_delete_file, reconnect server...");
		if (err)
			return err;
	}

	Int32 len = sprintf(hFtp->cmdBuf, "DELE %s\r\n", pathName);

	err = ftp_send_cmd(hFtp, hFtp->cmdBuf, len);
	if (err)
		return err;
	
	if (strncmp(hFtp->cmdBuf, "250", 3) != 0) {
		ERR("DELE cmd is denied: %s", hFtp->cmdBuf);
		return E_REFUSED;
	}

	#ifdef FTP_DEBUG_EN
	//DBG("ftp_delete_file, delete file %s ok.\n", pathName);
	#endif
	return E_NO;	
}

/*****************************************************************************
 Prototype    : ftp_get_file_size
 Description  : get file size on server
 Input        : FtpHandle hFtp  
 				const Int8 *ftpPathName, file pathname on ftp server
 Output       : None
 Return Value : file size if success or error code if failed
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 ftp_get_file_size(FtpHandle hFtp, const char *ftpPathName)
{
	int ret;
	
	/* Read file size */
	int len = sprintf(hFtp->cmdBuf, "SIZE %s\r\n", ftpPathName);
	ret = ftp_send_cmd(hFtp, hFtp->cmdBuf, len);
	if(ret)
		return ret;

	if (strncmp(hFtp->cmdBuf, "213 ", 3) == 0) {
		ret = atoi(hFtp->cmdBuf + 4);
	} else {
		#ifdef FTP_DEBUG_EN
		ERR("SIZE cmd was denied.");
		#endif
		ret = E_REFUSED;
	}

	return ret;
}

/*****************************************************************************
 Prototype    : ftp_upload_file
 Description  : upload a file to server
 Input        : FtpHandle hFtp  
 				const Int8 *fileBuf, buffer of file data
 				Uint32 fileSize, size of file
 				const Int8 *ftpPathName, file pathname on ftp server
 Output       : None
 Return Value : error code
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 ftp_upload_file(FtpHandle hFtp, const Int8 *fileBuf, Uint32 fileSize, const Int8 *ftpPathName)
{
	Int32	err = E_IO;
	
	if(!hFtp || !fileBuf || !fileSize || !ftpPathName)
		return E_NO;
	
	if(strlen(ftpPathName) > FTP_MAX_DIR_LEN) {
		ERR("invalid data or ftpPathName too long");
		return E_INVNAME;
	}

	if(hFtp->status == FTP_STAT_TRANSFER) {
		ERR("there is another data transferring.");
		return E_AGAIN;
	}

	hFtp->status = FTP_STAT_TRANSFER;
	Bool hasTriedBefore = FALSE;
	int dataSocket = -1;
	
transfer:	

	err = ftp_enter_transfer_mode(hFtp);
	if(err) {
		ERR("enter tansfer mode failed!");
		goto err_quit;
	}
	
	int len = snprintf(hFtp->cmdBuf, sizeof(hFtp->cmdBuf), "STOR %s\r\n", ftpPathName);
	dataSocket = hFtp->dataSock;

	err = ftp_send_cmd(hFtp, hFtp->cmdBuf, len);
	if(err)
		goto err_quit;

	//DBG("STOR cmd reply: %s", hFtp->cmdBuf);
	if(strncmp(hFtp->cmdBuf, "550", 2)== 0) {
		/* Upload was refused, Check if we need to overwrite same file */
		err = ftp_get_file_size(hFtp, ftpPathName);
		
		if (!err) {
			/* We don't have right to overwrite this file */
			#ifdef FTP_DEBUG_EN
			ERR("same file name already exits or size cmd error.");
			#endif
			err = E_REFUSED;
			goto err_quit;
		} else {
			if(hFtp->cmdBuf[0] == '5' && hFtp->cmdBuf[1] == '5')
				err = E_INVPATH;
			else
				err = E_REFUSED;
			#ifdef FTP_DEBUG_EN
			ERR("upload is denied, path name invalid?");
			#endif
			goto err_quit;
		}
	}

	if(strncmp(hFtp->cmdBuf, "150", 3) || strncmp(hFtp->cmdBuf, "125", 3)) {
		if(hFtp->connectMode == FTP_ACTIVE_MODE) {
			/* Waiting coming connection, NOT TESTED! */
			int listenSocket = hFtp->dataSock;
			socklen_t len = sizeof(hFtp->dataAddr);
			
			dataSocket = accept(listenSocket, (struct sockaddr *)&(hFtp->dataAddr), &len);
			if(dataSocket < 0) {
				ERR("accept error.");
				close(listenSocket);
				hFtp->dataSock = -1;
				return E_CONNECT;
			}
			set_sock_send_timeout(dataSocket, FTP_SEND_TIMEOUT);
			close(listenSocket); //no more listening
		}
	
		if(sendn(dataSocket, fileBuf, fileSize, 0) != fileSize) {			
				ERR("send file failed.");
				err = E_TRANS;
				goto err_quit;
		}

		//DBG("send file %s, %d bytes transfered.", ftpPathName, fileSize);
		/* Waiting data send completed */
		shutdown(dataSocket, SHUT_WR);

		/* Waiting server response */
		int cnt = 3 + (fileSize >> 20);
		while (1) {
			if((err = recv(hFtp->cmdSock, hFtp->cmdBuf, FTP_MAX_LINE_SIZE, 0)) > 0) {
				if(strncmp(hFtp->cmdBuf, "226", 3) == 0) {
					err = E_NO;
					break;
				}
			}
			
			if(!err || cnt-- < 0) {
				/* Timeout */
				ERR("ftp_upload_file, recv timeout.");
				err = E_REFUSED;
				goto err_quit;
			}
		}

#ifdef FTP_CHECK_SIZE
		/* Read file size */
		DBG("get file size");
		int ftpSize = ftp_get_file_size(hFtp, ftpPathName);
		if(ftpSize < 0) {
			err = E_REFUSED;
			goto err_quit;
		}
		
		if(ftpSize == fileSize) {
			/* The same */
			#ifdef FTP_DEBUG_EN
			//DBG("send file %s ok.",ftpPathName);
			#endif
			err = E_NO;
		} else {	
			/* Different size */
			if(!hasTriedBefore) {
				hasTriedBefore = TRUE;
				goto transfer;
			}
			ftp_delete_file(hFtp, ftpPathName);
			ERR("file size isn't the same.");
			err = E_AGAIN;
		}
#endif

	}else{
		ERR("STOR cmd was denied.");
		err = E_REFUSED;
	}


err_quit:
	hFtp->status = FTP_STAT_IDLE;
	if(dataSocket >= 0)
		close(dataSocket);
		
	hFtp->dataSock = -1;
	return err;
	
}

/*****************************************************************************
 Prototype    : ftp_download_file
 Description  : download a file from server
 Input        : FtpHandle hFtp  
 				Uint32 bufSize, size of buffer			
 				const Int8 *ftpPathName, file pathname on ftp server
 Output       : Int8 *fileBuf, buffer of file data
 				Uint32 fileSize, download size of file
 Return Value : error code
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 ftp_download_file(FtpHandle hFtp, Int8 *fileBuf, Uint32 bufSize, const Int8 *ftpPathName, Uint32 *ftpFileSize)
{
	Int32	err;

	if(!fileBuf  || !bufSize || !ftpPathName || strlen(ftpPathName) > FTP_MAX_DIR_LEN) {
		ERR("invalid buffer or pathName.");
		return E_INVAL;
	}

	if(hFtp->status == FTP_STAT_TRANSFER) {
		ERR("there is another data transferring.");
		return E_AGAIN;
	}

	hFtp->status = FTP_STAT_TRANSFER;

	int ftpSize = ftp_get_file_size(hFtp, ftpPathName);
	if(ftpSize < 0 || ftpSize > bufSize) {
		ERR("Can't find file or buffer too small.");
		err = E_INVAL;
		goto err_quit;
	}

	err = ftp_enter_transfer_mode(hFtp);
	if(err) {
		ERR("ftp_enter_transfer_mode failed!");
		goto err_quit;
	}

	Bool hasTriedBefore = FALSE;
	int len;
	
retransfer:	

	len = sprintf(hFtp->cmdBuf, "RETR %s\r\n", ftpPathName);

	err = ftp_send_cmd(hFtp, hFtp->cmdBuf, len);
	if(err)
		goto err_quit;

	if(strncmp(hFtp->cmdBuf, "550", 3) == 0) {
		ERR("RETR cmd is denied, invalid path name?");
		err = E_NOTEXIST;
		goto err_quit;
	}

	if (strncmp(hFtp->cmdBuf, "150", 3)) {
		ERR("RETR cmd is denied.");
		err = E_REFUSED;
		goto err_quit;
	}

	if((err = ftp_recv_data(hFtp, fileBuf, bufSize, ftpFileSize)) || *ftpFileSize != ftpSize) {
		ERR("recv data failed.");
		if(!hasTriedBefore) {
			hasTriedBefore = TRUE;
			goto retransfer;
		}
		if(err)
			goto err_quit;
	}

	err = E_NO;

	#ifdef FTP_DEBUG_EN
	DBG("ftp_download_file, recv file %s ok.", ftpPathName);
	#endif

err_quit:
	hFtp->status = FTP_STAT_IDLE;
	close(hFtp->dataSock);
	hFtp->dataSock = -1;
	return err;
	
}

/*****************************************************************************
 Prototype    : ftp_download_file
 Description  : Get dir and file list of current dir
 Input        : FtpHandle hFtp  
 				Uint32 bufSize, size of buffer			
 				const Int8 *ftpPath, dir pathname on ftp server
 Output       : Int8 *buf, buffer of file list data
 				Uint32 recvLen, actual recv len
 Return Value : error code
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/

Int32 ftp_get_list(FtpHandle hFtp, Int8 *buf, Uint32 bufSize, const Int8 *ftpPath, Uint32 *recvLen)
{
	Int32 	err;
	
	if(!buf  || !bufSize || strlen(ftpPath) > sizeof(hFtp->cmdBuf) - 8) {
		ERR("invalid buffer or path too long.");
		return E_INVAL;
	}

	if(hFtp->status == FTP_STAT_TRANSFER) {
		ERR("there is another data transferring.");
		return E_AGAIN;
	}

	hFtp->status = FTP_STAT_TRANSFER;

	err = ftp_enter_transfer_mode(hFtp);
	if(err) {
		ERR("ftp_enter_transfer_mode failed!");
		goto err_quit;
	}

	Int32 len = sprintf(hFtp->cmdBuf, "LIST %s\r\n", ftpPath);

	err = ftp_send_cmd(hFtp, hFtp->cmdBuf, len);
	if(err)
		goto err_quit;

	if(strncmp(hFtp->cmdBuf, "150 ", 3) != 0) {
		ERR("LIST cmd is denied.");
		err = E_REFUSED;
		goto err_quit;
	}
	
	if((err = ftp_recv_data(hFtp, buf, bufSize, recvLen)) != E_NO) {	
		ERR("recv data failed.");
		goto err_quit;
	}

	err = E_NO;
#ifdef FTP_DEBUG_EN
	int i = 0;
	DBG("ftp_get_list for %s, recv:", ftpPath);
	while(i < *recvLen) {
		DBG("  %s", buf + i);
		i += strlen(buf + i);
		while(i < *recvLen && *(buf + i) == 0)
			i++;
	}
#endif

err_quit:
	hFtp->status = FTP_STAT_IDLE;
	close(hFtp->dataSock);
	hFtp->dataSock = -1;
	return err;
}

/*****************************************************************************
 Prototype    : ftp_keep_alive
 Description  : Keep connect with server
 Input        : FtpHandle hFtp  
 Output       : None
 Return Value : error code
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 ftp_keep_alive(FtpHandle hFtp)
{
	Int32 err = E_NO;

	if(!hFtp)
		return E_INVAL;
	
	if(!ftp_check_connect_status(hFtp)) {
		WARN("reconnect server...");
		err = ftp_connect_server(hFtp, FTP_RECONNECT_TIMEOUT);
	}

	return err;
}

/*****************************************************************************
 Prototype    : ftp_send_cmd
 Description  : Send cmd for ftp and recv response
 Input        : FtpHandle hFtp  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline Int32 ftp_send_cmd(FtpHandle hFtp, const char *cmd, Int32 cmdLen)
{
	Int32 len = cmdLen > 0 ? cmdLen : strlen(cmd) + 1;

	/* Send cmd string */
	//DBG("ftp send %s", cmd);
	
	if(send(hFtp->cmdSock, cmd, len, 0) != len) {
		ERRSTR("send returns err");
		return E_TRANS;
	}

	/* Recv response */
	if(recv(hFtp->cmdSock, hFtp->cmdBuf, sizeof(hFtp->cmdBuf), 0) <= 0)
		return E_TRANS;

	//DBG("ftp recv %s", hFtp->cmdBuf);
	return E_NO;
}

/*****************************************************************************
 Prototype    : ftp_recv_data
 Description  : Recv data from server
 Input        : FtpHandle hFtp  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/

static Int32 ftp_recv_data(FtpHandle hFtp, Int8 *buf, Uint32 bufSize, Uint32 *recvLen)
{
	if(hFtp->connectMode == FTP_ACTIVE_MODE) {
		int	listenSock = hFtp->dataSock;
		socklen_t len = sizeof(struct sockaddr_in);
		if((hFtp->dataSock = accept(listenSock, (struct sockaddr *)&(hFtp->dataAddr), &len)) < 0) {
			ERR("ftp_recv_data, accept error.");
			close(listenSock);
			return E_IO;
		}
	}

	set_sock_recv_timeout(hFtp->dataSock, FTP_RECV_TIMEOUT);

	*recvLen = 0;
	while(1) {
		/* Recv data */
		int recCnt = recv(hFtp->dataSock, buf + *recvLen, bufSize - *recvLen, 0); 
		if(recCnt < 0) {
			ERR("ftp_recv_data, recv data error.");
			return E_TRANS;
		}

		if(recCnt == 0)
			break;

		*recvLen += recCnt;

		if(*recvLen > bufSize) {
			ERR("recv data is bigger than buffer len.");
			return E_NOMEM;
		}
	}

	#ifdef FTP_DEBUG_EN
	//bzero(cRecvLine, sizeof(cRecvLine));
	#endif
	
	if(recv(hFtp->cmdSock, hFtp->cmdBuf, FTP_MAX_LINE_SIZE, 0) <= 0) {
		ERRSTR("recv cmd error");
		return E_TRANS;
	}

	#ifdef FTP_DEBUG_EN
	//DBG("ftp_recv_data, recv from cmd socket: %s", hFtp->cmdBuf);
	#endif
	return E_NO;
}


/*****************************************************************************
 Prototype    : ftp_enter_passive_mode
 Description  : Enter passive mode
 Input        : FtpHandle hFtp  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/17
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline Int32 ftp_enter_passive_mode(FtpHandle hFtp) 
{
	/* Passive transfer mode */
	bzero(hFtp->cmdBuf, sizeof(hFtp->cmdBuf));
	
	int err = ftp_send_cmd(hFtp, "PASV \r\n", 7);
	if (err)
		return err;

	if(strncmp(hFtp->cmdBuf, "227", 3)) {
		ERR("enter passive mode is denied: %s.", hFtp->cmdBuf);
		return E_REFUSED;
	}

	/* Get port number */
	char *pTemp1 = hFtp->cmdBuf + strlen(hFtp->cmdBuf);
	while(*pTemp1 != ')' && pTemp1 != hFtp->cmdBuf)
		pTemp1--;

	if(pTemp1 == hFtp->cmdBuf) {
		ERR("can't find ')'");
		return E_MODE;
	}

	char *pTemp2 = pTemp1;
	while(*pTemp2 != ',' && pTemp2 != hFtp->cmdBuf)
		pTemp2--;

	if(pTemp2 == hFtp->cmdBuf) {
		ERR("can't find ','");
		return E_MODE;
	}

	Int8 numBuf[8] = {0};
	strncpy(numBuf, pTemp2 + 1, pTemp1 - pTemp2 - 1);

	Uint16 port = atoi(numBuf);

	pTemp2--;
	pTemp1 = pTemp2;
	pTemp2++;
	while(*pTemp1 != ',' && pTemp1 != hFtp->cmdBuf)
		pTemp1--;

	if(pTemp1 == hFtp->cmdBuf) {
		ERR("can't find ','");
		return E_MODE;
	}

	strncpy(numBuf, pTemp1 + 1, pTemp2 - pTemp1 - 1);

	port = (atoi(numBuf) << 8) + port;

	int dataSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (dataSocket < 0) {
		ERR("alloc dataSocket failed.");
		return E_IO;
	}

	struct  sockaddr_in srvAddr;
	srvAddr.sin_addr.s_addr = hFtp->serverIP;
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(port);

	err = connect_nonblock(dataSocket, (struct sockaddr *)&srvAddr, sizeof(srvAddr), FTP_RECONNECT_TIMEOUT << 2);		
	if(err < 0) {
		close(dataSocket);
		return E_CONNECT;
	}
	
	hFtp->dataSock = dataSocket;
	//err = set_sock_send_timeout(dataSocket, FTP_SEND_TIMEOUT);
	//err |= set_sock_recv_timeout(dataSocket, FTP_RECV_TIMEOUT);
	
	return E_NO;
}

/*****************************************************************************
 Prototype    : ftp_enter_active_mode
 Description  : Enter active transfer mode, Not Tested!
 Input        : FtpHandle hFtp  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/17
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline Int32 ftp_enter_active_mode(FtpHandle hFtp)
{
	/* Active transfer mode */
	int dataSocket = socket_tcp_server(0, 1);
	if (dataSocket < 0) {
		ERRSTR("alloc listenSocket failed");
		return E_IO;
	}

	struct sockaddr_in srvAddr;
	socklen_t addrLen = sizeof(srvAddr);
	int err = getsockname(dataSocket, (struct sockaddr *)&srvAddr, &addrLen);
	if(err < 0) {
		ERRSTR("getsockname failed");
		close(dataSocket);
		return E_IO;
	}

	Uint16 hightPort = ntohs(srvAddr.sin_port) / 256;
	Uint16 lowPort = ntohs(srvAddr.sin_port) % 256;

	if(getsockname(hFtp->cmdSock, (struct sockaddr *)&srvAddr, &addrLen) < 0) {
		ERRSTR("getsockname failed");
		close(dataSocket);
		return E_IO;
	}

	char ipString[16];
	inet_ntop(AF_INET, &srvAddr.sin_addr.s_addr, ipString, sizeof(ipString));

	int len = sprintf(hFtp->cmdBuf, "PORT %s,%u,%u\r\n", ipString, hightPort, lowPort);
	err = ftp_send_cmd(hFtp, hFtp->cmdBuf, len);
	if(err) {
		close(dataSocket);
		return err;
	}

	if (strncmp(hFtp->cmdBuf ,"200", 3) != 0) {
		ERR("operation denied by server: %s", hFtp->cmdBuf);
		close(dataSocket);
		return E_REFUSED;
	}

	/* We have entered active mode */
	hFtp->dataSock = dataSocket;
	hFtp->dataAddr = srvAddr;
	#ifdef FTP_DEBUG_EN
	//printf("ftp_enter_transfer_mode, enter active mode ok.");
	#endif

	return E_NO;
}

/*****************************************************************************
 Prototype    : ftp_send_cmd
 Description  : enter active or passive transfer mode
 Input        : FtpHandle hFtp  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 ftp_enter_transfer_mode(FtpHandle hFtp)
{
	Int32	err;

	//DBG("enter transfer check status");
	if (!ftp_check_connect_status(hFtp)) {
		ERR("reconnect server...");
		err = ftp_connect_server(hFtp, FTP_RECONNECT_TIMEOUT);		
		if (err)
			return err;
	}
	
	if(hFtp->connectMode == FTP_PASSIVE_MODE) {
		Int32 cnt = 0;
		//DBG("enter transfer, enter passive mode");
		while(cnt++ < FTP_MAX_RETRY_CNT) {
			err = ftp_enter_passive_mode(hFtp);
			if(!err)
				break;
			usleep(5000); //wait a while
		}
	}else{
		err = ftp_enter_active_mode(hFtp);
	}

	//DBG("enter transfer set linger");
	if(!err) {
		/* Set linger */
		err = set_sock_linger(hFtp->dataSock, TRUE, 1);
		if(err) {
			close(hFtp->dataSock);
			ERR("set linger failed.");
			return E_IO;
		}
	}

	//DBG("enter transfer returns");
	return err;
}

/*return TRUE if connection is ok, otherwise return FALSE */
static Bool ftp_check_connect_status(FtpHandle hFtp)
{
	Int32 	err;

	if(hFtp->status == FTP_STAT_UNCONNECTED)
		return FALSE;
	
	err = ftp_send_cmd(hFtp, "NOOP \r\n", 7);
	if(err) {
		ERR("send noop failed...");
		goto close_socket;		
	}

	/* Correct Response */
	if (strncmp(hFtp->cmdBuf, "200", 3) == 0) 
		return TRUE;
	
	ERR("data replied incorrect.");
	
close_socket:
	close(hFtp->cmdSock);
	hFtp->cmdSock = -1;
	hFtp->status = FTP_STAT_UNCONNECTED;
	return FALSE;
}


#if 0
/* convert integer to ascii */

static Int8 *itoa(Int32 value, Int8 *string, Int32 radix)
{
	Int8	tmp[33];
	Int8	*tp = tmp;
	Int32	i;
	Uint32	v;
	Int32	sign;
	Int8	*sp;

	if(string == NULL)
		return NULL;

	sign = (radix == 10 && value < 0);
	if (sign)
		v = -value;
	else
		v = (unsigned)value;
	while (v || tp == tmp)
	{
		i = v % radix;
		v = v / radix;
		if (i < 10)
			*tp++ = i+'0';
		else
			*tp++ = i + 'a' - 10;
	}

	sp = string;

	if (sign)
		*sp++ = '-';
	while (tp > tmp)
		*sp++ = *--tp;
	*sp = 0;
	return string;
}

#endif


