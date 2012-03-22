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

	MsgHandle hMsg = calloc(1, sizeof(MsgObj));
	if(!hMsg) {
		ERR("Alloc mem failed.");
		return NULL;
	}
	
	int fd, size;
	struct sockaddr_un un;

	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;
	strncpy(un.sun_path, srcName, sizeof(un.sun_path));
	
	if ((fd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
		ERRSTR("socket error");
		goto err_quit;
	}
	size = SUN_LEN(&un);
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

#if 0
/*****************************************************************************
 Prototype    : send_msg
 Description  : Send msg data and header
 Input        : MsgHandle hMsg            
                struct sockaddr *srvAddr  
                const MsgHeader *header   
                const void *data          
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline Int32 send_msg(MsgHandle hMsg, struct sockaddr *srvAddr, const MsgHeader *header, const void *data)
{
	if(sendto(hMsg->fd, header, sizeof(header), 0, 
		srvAddr, sizeof(struct sockaddr)) < 0) {
		ERRSTR("sendto header err");
		return E_IO;
	}

	if(header->dataLen > 0 && data) {
		if(sendto(hMsg->fd, data, header->dataLen, 0, 
			srvAddr, sizeof(struct sockaddr)) < 0) {
			ERRSTR("sendto data err");
			return E_IO;
		}
	}

	return E_NO;
}
#endif

/*****************************************************************************
 Prototype    : msg_send
 Description  : Send request msg
 Input        : MsgHandle hMsg           
                const char *dstName, if this is NULL, send to default dst      
                const MsgHeader *header  
                const void *data         
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 msg_send(MsgHandle hMsg, const char *dstName, const void *data, Int32 dataLen)
{
	if(!hMsg || hMsg->fd < 0 || !data || dataLen < sizeof(MsgHeader))
		return E_INVAL;

	MsgHeader *header = (MsgHeader *)data;

	if(header->magicNum != MSG_MAGIC_SEND && 
		header->magicNum != MSG_MAGIC_RESP) {
		ERR("invalid magic");
		return E_CHECKSUM;
	}

	struct sockaddr_un *dstAddr;
	struct sockaddr_un serverAddr;

	if(dstName) {
		/* Use input dest name */
		memset(&serverAddr, 0, sizeof(serverAddr));
		serverAddr.sun_family = AF_UNIX;
		strncpy(serverAddr.sun_path, dstName, sizeof(serverAddr.sun_path));
		dstAddr = &serverAddr;
	} else if(header->magicNum == MSG_MAGIC_SEND) {
		/* Use default dest name */
		dstAddr = &hMsg->dstAddr;
	} else {
		/* Use last src that sent msg to us */
		dstAddr = &hMsg->recvAddr;
	}

	/* Align send len */
	dataLen = ROUND_UP(dataLen, MSG_LEN_ALIGN);
	header->dataLen = ROUND_UP(header->dataLen, MSG_LEN_ALIGN);
	if(dataLen < header->dataLen + sizeof(MsgHeader))
		WARN("send msg, warning: dataLen: %d is smaller than msgHeader.dataLen: %d...",
			dataLen, header->dataLen);
	//DBG("send len: %d", dataLen);
	if(sendto(hMsg->fd, data, dataLen, 0, 
			(struct sockaddr *)dstAddr, sizeof(struct sockaddr)) != dataLen) {
		ERRSTR("sendto data err");
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
Int32 msg_recv(MsgHandle hMsg, void *buf, Int32 bufLen)
{
	if(!hMsg || !buf || bufLen < sizeof(MsgHeader))
		return E_INVAL;

    socklen_t len;  
    len = sizeof(hMsg->recvAddr);
	int rcvLen;
	MsgHeader *header = (MsgHeader *)buf;

	rcvLen = recvfrom(hMsg->fd, buf, bufLen, 0, (struct sockaddr *)&hMsg->recvAddr, &len);
	if(rcvLen < 0) {
		//ERRSTR("recv header err");
		return E_IO;
	}

	Int32 offset;
	for(offset = 0; offset < rcvLen; offset += sizeof(Uint32)) {
		/* Check header magic */
		if( *(Uint32 *)(buf + offset) == MSG_MAGIC_SEND || 
			*(Uint32 *)(buf + offset) == MSG_MAGIC_RESP ) {
				break;
		}
	}

	if(offset >= rcvLen || rcvLen - offset < sizeof(MsgHeader)) {
		ERR("can't sync header");
		return E_CHECKSUM;
	}

	/* Skip invalid data */
	if(offset) {
		DBG("offset is %d", offset);
		memmove(buf, buf + offset, offset);
		rcvLen -= offset; //rcvLen = effective data len
	}	

	if(header->dataLen > rcvLen - sizeof(MsgHeader)) {
		WARN("msg recv, warning: msgHeader.dataLen: %d is bigger than rcv len: %d!",
			header->dataLen, rcvLen - sizeof(MsgHeader));
		//return E_NOSPC;
	}

	return rcvLen;
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
	
	if(dstName)
		strncpy(hMsg->dstAddr.sun_path, dstName, sizeof(hMsg->dstAddr.sun_path));

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

