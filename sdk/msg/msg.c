/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : msg.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/2/14
  Last Modified :
  Description   : Message module
  Function List :
              msg_create
              msg_delete
              msg_recv
              msg_send
              msg_set_recv_timeout
              msg_set_send_timeout
              send_msg
  History       :
  1.Date        : 2012/2/14
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "common.h"
#include "sysnet.h"
#include "log.h"
#include "msg.h"
#include "net_utils.h"
//#include <linux/un.h>

/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/

/*----------------------------------------------*
 * external routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/
typedef struct _MsgObj {
	int 				fd; 			/* Socket fd */
	const char *		name;			/* Path name for create and unlink */
	int 				flag;			/* Revserved */
	int 				dstAddrSize;	/* Size of dst addr */
	struct sockaddr_un	recvAddr;		/* Addr for recv */
	struct sockaddr_un	dstAddr;		/* Addr for send */
}MsgObj;

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
#ifndef SUN_LEN
//#define SUN_LEN(path)		(offsetof(struct sockaddr_un, sun_path) + strlen(path))
#endif

#define MSG_MAX_NAME_LEN	(UNIX_PATH_MAX - 1)

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/*****************************************************************************
 Prototype    : msg_create
 Description  : Create Msg Object
 Input        : const Int8 *name  
                Int32 flag        
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
MsgHandle msg_create(const Int8 *srcName, const Int8 *dstName, Int32 flag)
{
	
	if(!srcName) {
		return NULL;
	}

	if( strlen(srcName) > MSG_MAX_NAME_LEN || 
		(dstName && strlen(dstName) > MSG_MAX_NAME_LEN) ) {
		ERR("msg name too long, max: %d", MSG_MAX_NAME_LEN);
		return NULL;
	}

	MsgHandle hMsg = calloc(1, sizeof(MsgObj));
	if(!hMsg) {
		ERR("Alloc mem failed.");
		return NULL;
	}
	
	int fd, size;
	struct sockaddr_un un;

	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;
	strcpy(un.sun_path, srcName);
	
	if ((fd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
		ERRSTR("socket error");
		goto err_quit;
	}
	size = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path);
	unlink(srcName);
	if (bind(fd, (struct sockaddr *)&un, size) < 0) {
		ERRSTR("bind error");
		goto err_quit;
	}
	
	//DBG("UNIX domain socket bound");
	hMsg->fd = fd;
	hMsg->flag = flag;
	hMsg->name = srcName;
	hMsg->dstAddr.sun_family = AF_UNIX;
	hMsg->recvAddr.sun_family = AF_UNIX;
	if(dstName)
		strncpy(hMsg->dstAddr.sun_path, dstName, sizeof(un.sun_path));

	if(hMsg->flag & MSG_FLAG_NONBLK)
		set_sock_block(hMsg->fd, FALSE);

	return hMsg;

err_quit:
	
	if(fd > 0)
		close(fd);
	unlink(srcName);
	if(hMsg)
		free(hMsg);

	return NULL;
}

/*****************************************************************************
 Prototype    : msg_delete
 Description  : Delete Msg Object
 Input        : MsgHandle hMsg  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 msg_delete(MsgHandle hMsg)
{
	if(!hMsg)
		return E_INVAL;

	if(hMsg->fd > 0)
		close(hMsg->fd);
	unlink(hMsg->name);
	free(hMsg);

	return E_NO;
}

/*****************************************************************************
 Prototype    : msg_send
 Description  : Send request msg
 Input        : MsgHandle hMsg           
                const char *dstName, if this is NULL, send to default dst      
                const MsgHeader *header  
                const void *data 
                Int32 flags
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 msg_send(MsgHandle hMsg, const char *dstName, const MsgHeader *header, Int32 flags)
{
	if(!hMsg || !header || (header->type & MSG_SYNC_MARSK) != MSG_SYNC_HEAD)
		return E_INVAL;

	struct sockaddr_un *dstAddr;
	struct sockaddr_un serverAddr;

	if(dstName) {
		/* Use input dest name */
		memset(&serverAddr, 0, sizeof(serverAddr));
		serverAddr.sun_family = AF_UNIX;
		strncpy(serverAddr.sun_path, dstName, sizeof(serverAddr.sun_path));
		dstAddr = &serverAddr;
	} else if(header->type == MSG_TYPE_REQU) {
		/* Use default dest name */
		dstAddr = &hMsg->dstAddr;
	} else {
		/* Use last src that sent msg to us */
		dstAddr = &hMsg->recvAddr;
	}

	/* Align send len */
	Int32 transLen = sizeof(MsgHeader) + header->dataLen;

	/* send data */
	if(sendto(hMsg->fd, header, transLen, 0, 
			(struct sockaddr *)dstAddr, sizeof(struct sockaddr_un)) != transLen) {
		ERRSTR("<0> sendto data err");
		return E_IO;
	}
	
	return E_NO;		
}

/*****************************************************************************
 Prototype    : msg_recv
 Description  : Recv msg header and data
 Input        : MsgHandle hMsg     
                MsgHeader *header  
                void *dataBuf      
                Int32 bufLen       
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 msg_recv(MsgHandle hMsg, MsgHeader *header, Int32 bufLen, Int32 flags)
{
	if(!hMsg || !header || bufLen < sizeof(MsgHeader)) {
		ERR("invalid value");	
		return E_INVAL;
	}
	
    socklen_t addrLen;  
    addrLen = sizeof(hMsg->recvAddr);
	int ret;

	bzero(&hMsg->recvAddr.sun_path, sizeof(hMsg->recvAddr.sun_path));

	/* recv data */
	do {
		ret = recvfrom(hMsg->fd, header, bufLen, 0, (struct sockaddr *)&hMsg->recvAddr, &addrLen);
		if(ret <= 0) {
			//ERRSTR("recv header err: %d", ret);
			return E_TIMEOUT;
		}
	}while((header->type & MSG_SYNC_MARSK) != MSG_SYNC_HEAD);

	/* we have recv in continous buffer */
	if(ret < header->dataLen + sizeof(MsgHeader)) {
		WARN("rcvlen %d is not equal to data len in header %d", ret, header->dataLen);
		return E_TRANS;
	}
	
	return E_NO;
}


/*****************************************************************************
 Prototype    : msg_get_recv_src
 Description  : Get the name of module which sends last msg
 Input        : MsgHandle hMsg  
 Output       : None
 Return Value : const
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
const char *msg_get_recv_src(MsgHandle hMsg)
{
	if(hMsg)
		return hMsg->recvAddr.sun_path;

	return NULL;
}

/*****************************************************************************
 Prototype    : msg_set_send_timeout
 Description  : Set Send timeout in seconds
 Input        : MsgHandle hMsg    
                Int32 timeoutSec  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 msg_set_send_timeout(MsgHandle hMsg, Int32 timeoutSec)
{
	return set_sock_send_timeout(hMsg->fd, timeoutSec);
}

/*****************************************************************************
 Prototype    : msg_set_recv_timeout
 Description  : Set recv timeout in seconds
 Input        : MsgHandle hMsg    
                Int32 timeoutSec  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 msg_set_recv_timeout(MsgHandle hMsg, Int32 timeoutSec)
{
	return set_sock_recv_timeout(hMsg->fd, timeoutSec);
}

/*****************************************************************************
 Prototype    : msg_get_fd
 Description  : Get fd of this handle
 Input        : MsgHandle hMsg  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
int msg_get_fd(MsgHandle hMsg)
{
	return hMsg->fd;
}

/*****************************************************************************
 Prototype    : msg_set_default_dest
 Description  : set default dst name
 Input        : MsgHandle hMsg       
                const char *dstName  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/9
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 msg_set_default_dst(MsgHandle hMsg, const char *dstName)
{
	if(!hMsg || !dstName)
		return E_INVAL;
	
	if( strlen(dstName) > MSG_MAX_NAME_LEN ) {
		ERR("msg name too long, max: %d", MSG_MAX_NAME_LEN);
		return E_NOSPC;
	}

	strcpy(hMsg->dstAddr.sun_path, dstName);

	return E_NO;
}

/*****************************************************************************
 Prototype    : msg_get_name
 Description  : get name of this msg module
 Input        : MsgHandle hMsg  
 Output       : None
 Return Value : const
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/20
    Author       : Sun
    Modification : Created function

*****************************************************************************/
const char *msg_get_name(MsgHandle hMsg)
{	
	if(!hMsg)
		return NULL;

	return hMsg->name;
}

