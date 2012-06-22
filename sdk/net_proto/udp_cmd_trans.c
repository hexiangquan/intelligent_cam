/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : udp_cmd_trans.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/4/23
  Last Modified :
  Description   : udp net protol implentation
  Function List :
              recv_sync
              udp_cmd_recv
              udp_cmd_send
  History       :
  1.Date        : 2012/4/23
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "udp_cmd_trans.h"
#include "net_utils.h"
#include "log.h"

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
//#define OLD_PROTOCOL			1

#ifdef OLD_PROTOCOL
#define UDP_CMD_SYNC_START		0x203E553C
#define UDP_CMD_SYNC_END		0x3E552F3C
#define UDP_CMD_RESP_START		0x203E753C
#define UDP_CMD_RESP_END		0x3E752F3C
#else
#define UDP_CMD_SYNC_START		0x203E753C
#define UDP_CMD_SYNC_END		0x3E752F3C
#define UDP_CMD_RESP_START		0x203E753C
#define UDP_CMD_RESP_END		0x3E752F3C
#endif

#define UDP_BUF_LEN				1024


/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/* start port of a udp packet */
typedef struct {
	Uint32			syncCode;
	UdpCmdHeader	header;
}UdpCmd;

/*****************************************************************************
 Prototype    : udp_cmd_recv
 Description  : recv tdp cmd
 Input        : Int32 sock                
                UdpCmdHeader *header      
                void *buf                 
                Uint32 bufLen             
                struct sockaddr_in *addr  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/23
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 udp_cmd_recv(Int32 sock, UdpCmdHeader *header, void *buf, Uint32 bufLen, struct sockaddr_in *addr)
{	
	Int32		ret;
	socklen_t	len = sizeof(struct sockaddr_in);
	Int8		recvBuf[UDP_BUF_LEN];
	
	/* validate data */
	if(sock < 0 || !header || !addr)
		return E_INVAL;

	/* recv header */
	ret = recvfrom(sock, recvBuf, sizeof(recvBuf), 0, (struct sockaddr *)addr, &len); 
  	if(ret < 0) {
  		ERRSTR("recv header data failed: %d", ret);
		return E_TRANS;
	}

	/* validate data */
	if(*(Uint32 *)recvBuf != UDP_CMD_SYNC_START) {
		ERR("invalid sync start");
		return E_CHECKSUM;
	}

	*header = *(UdpCmdHeader *)(recvBuf + sizeof(Uint32));

	/* recv data */
	if( header->dataLen > 0) {
		if(ret - sizeof(*header) < header->dataLen) {
			ERR("recv data len: %d, not equal to len in header: %d",
				ret - sizeof(*header), header->dataLen);
			return E_TRANS;
		}
		if(!buf || bufLen < header->dataLen) {
			ERR("buf not enough for addtive data");
			return E_NOMEM;
		}
		memcpy(buf, recvBuf + sizeof(UdpCmd), header->dataLen);
	}

	return E_NO;
}

/*****************************************************************************
 Prototype    : udp_cmd_send
 Description  : send udp cmd
 Input        : Int32 sock                
                UdpCmdHeader *header      
                void *buf                 
                struct sockaddr_in *addr  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/23
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 udp_cmd_send(Int32 sock, UdpCmdHeader *header, void *buf, struct sockaddr_in *addr)
{
	Int32		ret;
	UdpCmd		*cmd;
	Int32		len;
	Int8		sendBuf[UDP_BUF_LEN];
	
	/* validate data */
	if(sock < 0 || !header || !addr)
		return E_INVAL;

	if(header->dataLen && !buf) {
		ERR("buf should not be null while datalen is bigger than 0");
		return E_INVAL;
	}

	/* set sync code and header */
	cmd = (UdpCmd *)sendBuf;
	cmd->syncCode = UDP_CMD_RESP_START;
	cmd->header = *header;
	if(header->dataLen) {
		if(header->dataLen > sizeof(sendBuf) - sizeof(UdpCmd) - sizeof(Uint32)) {
			ERR("not enough buf for send");
			return E_NOMEM;
		}
		memcpy(sendBuf + sizeof(UdpCmd), buf, header->dataLen);
	}
	
	len = sizeof(*cmd) + header->dataLen;
	*(Uint32 *)(sendBuf + len) = UDP_CMD_RESP_END;
	len += sizeof(Uint32);
	
	/* send data */
	ret = sendto(sock, sendBuf, len, 0, 
			(struct sockaddr *)addr, sizeof(struct sockaddr_in));
	if(ret != len) {
		ERRSTR("sendto error");
		return E_TRANS;
	}
	
	return E_NO;
}

