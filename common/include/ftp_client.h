/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : ftp_client.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/1/14
  Last Modified :
  Description   : ftp_client.c header file
  Function List :
  History       :
  1.Date        : 2012/1/14
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __FTP_CLIENT_H__
#define __FTP_CLIENT_H__

#include "common.h"
#include "sysnet.h"

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
#define FTP_INADDR_NONE 			0xFFFFFFFF
#define FTP_MAX_USERNAME_LEN		128
#define FTP_MAX_PASSWD_LEN			128
#define FTP_MAX_LINE_SIZE			512
#define FTP_MAX_DIR_LEN				(FTP_MAX_LINE_SIZE - 7)
#define FTP_RECONNECT_TIMEOUT		10u		//s
#define FTP_SEND_TIMEOUT			10u 	//s
#define FTP_RECV_TIMEOUT			15u		//s

/* FTP connection mode */
#define	FTP_ACTIVE_MODE				0			//active mode
#define	FTP_PASSIVE_MODE			1			//passive mode

/* Value of current connect status */
#define	FTP_STAT_UNCONNECTED 		0		//unconnected server
#define	FTP_STAT_IDLE				1		//connected server and login, without data transfer
#define	FTP_STAT_TRANSFER			2		//connected and tranfering data

//#define FTP_DEBUG_EN

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/* Handle for all ftp operations */
typedef struct _FtpObj{
	Uint32				serverIP; 							//server IP in net format
	Uint16 				serverPort;							//server port in net format
	Uint16				connectMode;						//connect mode: 1: active mode, 0: passive mode
	Uint32 				status;								//current connect status
	Int32 				cmdSock;							//command socket
	Int32 				dataSock;							//data socket
	struct sockaddr_in	dataAddr;							//data socket addr
	Int8 				userName[FTP_MAX_USERNAME_LEN];		//user name
	Int8				passWord[FTP_MAX_PASSWD_LEN];	 	//pass word
	Int8 				currentDir[FTP_MAX_DIR_LEN];		//current dir	
	Int8				cmdBuf[FTP_MAX_LINE_SIZE];
	Uint32				sndTimeout;							//send timeout, second
	Uint32				recvTimeout;						//recv timeout, second
}FtpObject, *FtpHandle;


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*****************************************************************************
 Prototype    : ftp_create
 Description  : create ftp handle
 Input        : Int8 *userName, user name string
                Int8 *password, password string
                Int8 *serverIP, IP addr string
                Uint16 serverPort, server port, host format
 Output       : None
 Return Value : NULL if fails, otherwise, returns FtpHandle for other operations 
 Calls        : 
 Called By    : 
 
 History        :
 1.Date         : 2011/3/15
   Author       : Sun
   Modification : Created function

*****************************************************************************/
extern FtpHandle ftp_create(const Int8 *userName, const Int8 *password, const Int8 *serverIP, Uint16 serverPort);

/*****************************************************************************
 Prototype    : ftp_delete
 Description  : delete handle created by ft_create
 Input        : FtpHandle hFtp  
 Output       : None
 Return Value : ftp error code
 Calls        : 
 Called By    : 
 
 History        :
 1.Date         : 2011/3/15
   Author       : Sun
   Modification : Created function

*****************************************************************************/
extern Int32 ftp_delete(FtpHandle hFtp);

/*****************************************************************************
 Prototype    : ftp_connect_server
 Description  : connect ftp server
 Input        : FtpHandle hFtp , handle create by ftp_create   
                Uint32 timeoutMs, timeout for connect, if this is zero, will use default timeout
 Output       : None
 Return Value : ftp error code
 Calls        : 
 Called By    : 
 
 History        :
 1.Date         : 2011/3/15
   Author       : Sun
   Modification : Created function

*****************************************************************************/
extern Int32 ftp_connect_server(FtpHandle hFtp, Int32 timeoutSec);

/*****************************************************************************
 Prototype    : ftp_disconnect_server
 Description  : disconnect with server
 Input        : FtpHandle hFtp  
 Output       : None
 Return Value : error code
 Calls        : 
 Called By    : 
 
 History        :
 1.Date         : 2011/3/15
   Author       : Sun
   Modification : Created function

*****************************************************************************/
extern Int32 ftp_disconnect_server(FtpHandle hFtp);

/*****************************************************************************
 Prototype    : ftp_get_working_dir
 Description  : get current working dir, if has not connected server, this function will connect server first
 Input        : FtpHandle hFtp  , ftp handle   
                Uint32 bufLen, buffer size in bytes 
 Output       : Int8 *dirNameBuf, string of working dir 
 Return Value : error code
 Calls        : 
 Called By    : 
 
 History        :
 1.Date         : 2011/3/15
   Author       : Sun
   Modification : Created function

*****************************************************************************/
extern Int32 ftp_get_working_dir(FtpHandle hFtp, Int8 *dirNameBuf, Uint32 bufLen);

/*****************************************************************************
 Prototype    : ftp_change_working_dir
 Description  : change working dir
 Input        : FtpHandle hFtp       
                const Int8 *pNewDir, new working dir name
 Output       : None
 Return Value : error code
 Calls        : 
 Called By    : 
 
 History        :
 1.Date         : 2011/3/15
   Author       : Sun
   Modification : Created function

*****************************************************************************/
extern Int32 ftp_change_working_dir(FtpHandle hFtp, const Int8 *newDir);

/*****************************************************************************
 Prototype    : ftp_upload_file
 Description  : upload file to server
 Input        : FtpHandle hFtp            
                const Uint8 *fileBuf, buffer for file data     
                Uint32 fileSize, size of file     
                const Int8 *ftpPathName, path and file name string  
 Output       : None
 Return Value : error code
 Calls        : 
 Called By    : 
 
 History        :
 1.Date         : 2011/3/15
   Author       : Sun
   Modification : Created function

*****************************************************************************/
extern Int32 ftp_upload_file(FtpHandle hFtp, const Int8 *fileBuf, Uint32 fileSize, const Int8 *ftpPathName);

/*****************************************************************************
 Prototype    : ftp_download_file
 Description  : download a file from server
 Input        : FtpHandle hFtp                            
                Uint32 bufSize          
                const Int8 *ftpPathName, path and file name for download
                Uint32 *ftpFileSize      
 Output       : Int8 *fileBuf, file downloaded to this buffer
 Return Value : common error code
 Calls        : 
 Called By    : 
 
 History        :
 1.Date         : 2011/3/15
   Author       : Sun
   Modification : Created function

*****************************************************************************/
extern Int32 ftp_download_file(FtpHandle hFtp, Int8 *fileBuf, Uint32 bufSize, const Int8 *ftpPathName, Uint32 *ftpFileSize);

/*****************************************************************************
 Prototype    : ftp_make_dir
 Description  : make a new dir, if the dir in middle of the path doesn't exist, it will also be created
 Input        : FtpHandle hFtp       
                const Int8 *ftpDir, dir path name to be created, could start with '/' for absolute path  
 Output       : None
 Return Value : error code
 Calls        : 
 Called By    : 
 
 History        :
 1.Date         : 2011/3/15
   Author       : Sun
   Modification : Created function

*****************************************************************************/
extern Int32 ftp_make_dir(FtpHandle hFtp, const Int8 *ftpDir);

/*****************************************************************************
 Prototype    : ftp_rename
 Description  : rename a dir or file
 Input        : FtpHandle hFtp            
                const Int8 *newPathName  
                const Int8 *oldPathName  
 Output       : None
 Return Value : error code
 Calls        : 
 Called By    : 
 
 History        :
 1.Date         : 2011/3/15
   Author       : Sun
   Modification : Created function

*****************************************************************************/
extern Int32 ftp_rename(FtpHandle hFtp, const Int8 *newPathName, const Int8 *oldPathName);

/*****************************************************************************
 Prototype    : ftp_delete_dir
 Description  : delete dir, this dir MUST be empty
 Input        : FtpHandle hFtp       
                const Int8 *ftpDir  
 Output       : None
 Return Value : error code
 Calls        : 
 Called By    : 
 
 History        :
 1.Date         : 2011/3/15
   Author       : Sun
   Modification : Created function

*****************************************************************************/
extern Int32 ftp_delete_dir(FtpHandle hFtp, const Int8 *ftpDir);

/*****************************************************************************
 Prototype    : ftp_delete_file
 Description  : delete file
 Input        : FtpHandle hFtp            
                const Int8 *ftpFilePath, path and name of the file, e.g. "/name/1.txt" 
 Output       : None
 Return Value : error code
 Calls        : 
 Called By    : 
 
 History        :
 1.Date         : 2011/3/15
   Author       : Sun
   Modification : Created function

*****************************************************************************/
extern Int32 ftp_delete_file(FtpHandle hFtp, const Int8 *ftpFilePath);

/*****************************************************************************
 Prototype    : ftp_get_list
 Description  : get file and dir list in current dir
 Input        : FtpHandle hFtp            
                Uint32 bufSize   
                const Int8 *ftpPath  
                     
 Output       : Int8 *buf, buffer for list
 				Uint32 *recvLen, len of data recieved
 Return Value : error code
 Calls        : 
 Called By    : 
 
 History        :
 1.Date         : 2011/3/15
   Author       : Sun
   Modification : Created function

*****************************************************************************/
extern Int32 ftp_get_list(FtpHandle hFtp, Int8 *buf, Uint32 bufSize, const Int8 *ftpPath, Uint32 *recvLen);

/*****************************************************************************
 Prototype    : ftp_set_server_ip
 Description  : set server ip 
 Input        : FtpHandle hFtp        
                const Int8 *ipAddr, string of server ip, e.g. "192.168.1.11"  
 Output       : None
 Return Value : error code
 Calls        : 
 Called By    : 
 
 History        :
 1.Date         : 2011/3/15
   Author       : Sun
   Modification : Created function

*****************************************************************************/
extern Int32 ftp_set_server_ip(FtpHandle hFtp, const Int8 *ipAddr);

/*****************************************************************************
 Prototype    : ftp_set_server_port
 Description  : set port of ftp server
 Input        : FtpHandle hFtp  
                Uint16 port, server listen port
 Output       : None
 Return Value : error code
 Calls        : 
 Called By    : 
 
 History        :
 1.Date         : 2011/3/15
   Author       : Sun
   Modification : Created function

*****************************************************************************/
extern Int32 ftp_set_server_port(FtpHandle hFtp, Uint16 port);

/*****************************************************************************
 Prototype    : ftp_set_user_name
 Description  : set username for login
 Input        : FtpHandle hFtp         
                const Int8 *userName  
 Output       : None
 Return Value : error code
 Calls        : 
 Called By    : 
 
 History        :
 1.Date         : 2011/3/15
   Author       : Sun
   Modification : Created function

*****************************************************************************/
extern Int32 ftp_set_user_name(FtpHandle hFtp, const Int8 *userName);

/*****************************************************************************
 Prototype    : ftp_set_password
 Description  : set password
 Input        : FtpHandle hFtp         
                const Int8 *password  
 Output       : None
 Return Value : error code
 Calls        : 
 Called By    : 
 
 History        :
 1.Date         : 2011/3/15
   Author       : Sun
   Modification : Created function

*****************************************************************************/
extern Int32 ftp_set_password(FtpHandle hFtp, const Int8 *password);


/*****************************************************************************
 Prototype    : ftp_keep_alive
 Description  : send "NOOP" cmd  to server so that this connection will not be
                closed 
 Input        : FtpHandle hFtp  
 Output       : None
 Return Value : error code
 Calls        : 
 Called By    : 
 
 History        :
 1.Date         : 2011/4/7
   Author       : Sun
   Modification : Created function

*****************************************************************************/
extern Int32 ftp_keep_alive(FtpHandle hFtp);

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
extern Int32 ftp_set_trans_timeout(FtpHandle hFtp, Uint32 sndTimeout, Uint32 rcvTimeout);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __FTP_CLIENT_H__ */
