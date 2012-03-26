/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : tcp_cmd_trans.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/23
  Last Modified :
  Description   : tcp cmd transfer
  Function List :
              recv_cmd_data
              recv_sync_end
              recv_sync_start
              tcp_cmd_recv
              tcp_cmd_reply
  History       :
  1.Date        : 2012/3/23
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "tcp_cmd_trans.h"
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
#define TCP_CMD_SYNC_START		0x203E433C
#define TCP_CMD_SYNC_END		0x3E432F3C

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/* 
  * error code of cmd 
  */
enum TcpCmdErrorCode
{
	NETERROR_NOERROR = 0x0000u,
	NETERROR_CONNECT_REFUSED = 0x0001u,

	NETERROR_INVALID_DATA_LENGTH = 0x2001u,
	NETERROR_INVALID_DATA_TYPE = 0x2002u,
	NETERROR_INVALID_COMMAND_TYPE = 0x2003u,
	NETERROR_INVALID_COMMAND = 0x2004u,
	NETERROR_INVALID_CHECKSUM = 0x2005u,
	NETERROR_MEMORY_ERROR = 0x2006u,
	NETERROR_NOSUPPORT_COMMAND = 0x2007u,
	NETERROR_INVALID_PARAMETER = 0x2008u,
	NETERROR_IO_OPERATION = 0x2009u,
	NETERROR_TIMEOUT = 0x200Au,
	NETERROR_BAD_MODE = 0x200Bu,
	NETERROR_NOT_READY = 0x200Cu,
	NETERROR_ABORT = 0x200Du,

	NETERROR_CAMERA_ALREADY_CAPTURING = 0x2101u,
	NETERROR_NETWORK = 0x2102u,
	NETERROR_SIZE_OVERFLOW = 0x2103u,
	NETERROR_CAMERA_NOT_CAPTURING = 0x2104u,
	NETERROR_CAMERA_NOT_SOFTTRIGGER = 0x2105u,
	NETERROR_CANT_CHANGE_PARAMETER = 0x2106u,
	NETERROR_MAX
};

/*****************************************************************************
 Prototype    : convert_err_code
 Description  : convert common error to cmd error code
 Input        : Int32 commErr  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/23
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Uint16 convert_err_code(Int32 commErr)
{
	Uint16 cmdErr;
	
	switch(commErr) {
	case E_NO:
		cmdErr = NETERROR_NOERROR;
		break;
	case E_INVAL:
		cmdErr = NETERROR_INVALID_PARAMETER;
		break;
	case EIO:
		cmdErr = NETERROR_IO_OPERATION;
		break;
	case E_NOMEM:
		cmdErr = NETERROR_MEMORY_ERROR;
		break;
	case E_TIMEOUT:
		cmdErr = NETERROR_TIMEOUT;
		break;
	case E_UNSUPT:
		cmdErr = NETERROR_INVALID_COMMAND;
		break;
	case E_CHECKSUM:
		cmdErr = NETERROR_INVALID_CHECKSUM;
		break;
	case E_MODE:
		cmdErr = NETERROR_BAD_MODE;
		break;
	case E_BUSY:
		cmdErr = NETERROR_NOT_READY;
		break;
	default:
		cmdErr = NETERROR_IO_OPERATION;
		break;
	}

	return cmdErr;
}


/*****************************************************************************
 Prototype    : recv_sync_start
 Description  : recv sync start
 Input        : int sock  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/23
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline Int32 recv_sync_start(int sock)
{
  Uint32 	data;
  Int32 	rc;

  //Sychronize every request packet
  while(1) {   
	  if((rc = recv(sock, &data, sizeof(data), 0)) != sizeof(data)) {
		  if(rc == 0)
			  return E_CONNECT;	//connection is closed 
		  else if(rc < 0)
			  return E_TIMEOUT;
	  } else {
		 if(data == TCP_CMD_SYNC_START)
		 	break;
	  } 	  
  }
  return E_NO;
}


/*****************************************************************************
 Prototype    : recv_sync_end
 Description  : recv sync end data
 Input        : int sock  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/23
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline Int32 recv_sync_end(int sock)
{
    Int32	rc;
    Uint32 	data;
   
    if((rc = recv(sock, &data, sizeof(data), 0)) != sizeof(data)) {
  	    ERRSTR("RecvSyncEnd, recv data error");
		if(rc == 0)
			return E_CONNECT;	//connection is closed 
		else
			return E_TIMEOUT;
    }
   
    if(data != TCP_CMD_SYNC_END) {
  	    ERR("TCP cmd sync end is invalid...");
  	    return E_CHECKSUM;
    }

    return E_NO;
}
 

/*****************************************************************************
 Prototype    : recv_cmd_data
 Description  : recv cmd data
 Input        : int sock           
                TcpCmdHeader *hdr  
                Int8 *dataBuf      
                Uint32 bufSize     
 Output       : None
 Return Value : inline
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/23
    Author       : Sun
    Modification : Created function

*****************************************************************************/
inline Int32 recv_cmd_data(int sock, TcpCmdHeader *hdr, Int8 *dataBuf, Uint32 bufSize)
{
	Int32 rc;
	
	/* Recieve command header */
	if((rc = recv(sock, hdr, sizeof(TcpCmdHeader), 0)) != sizeof(TcpCmdHeader))
		goto err_quit;

	/* Recieve data */
	if(hdr->dataLen) {
		if(!dataBuf || hdr->dataLen > bufSize) {
			ERR("data buffer is NULL or cmd data len %d > buf len %d", hdr->dataLen, bufSize);
			return E_NOMEM;
		}

		/* Recv additive data */
		if((rc = recvn(sock, dataBuf, hdr->dataLen, 0)) != (Int32)hdr->dataLen)
			goto err_quit;
	}

#ifdef TCP_CMD_DBG
	DBG("Recv Cmd 0x%X", hdr->cmd);
#endif
	return E_NO;

err_quit:

	ERRSTR("recv error");
	if(rc < 0)
		return E_TIMEOUT;
	else
		return E_CONNECT;
}

/*****************************************************************************
 Prototype    : tcp_cmd_recv
 Description  : recv cmd 
 Input        : int sock           
                TcpCmdHeader *hdr  
                void *dataBuf      
                Int32 bufLen       
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/23
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 tcp_cmd_recv(int sock, TcpCmdHeader *hdr, void *dataBuf, Int32 bufLen)
{
	Int32 ret;

	if(sock < 0 ||  !hdr)
		return E_INVAL;

	/* Recieve sync data */
	ret = recv_sync_start(sock);
	if(ret)
		return ret;

	/* Recieve cmd data */
	ret = recv_cmd_data(sock, hdr, dataBuf, bufLen);
	if(ret)
		return ret;
	
	/* Recieve sync end */
	return recv_sync_end(sock);
}

/*****************************************************************************
 Prototype    : tcp_cmd_reply
 Description  : reply tcp cmd
 Input        : int sock             
                TcpCmdHeader *hdr    
                const Int8 *dataBuf  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/23
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 tcp_cmd_reply(int sock, TcpCmdHeader *hdr, const Int8 *dataBuf, Int32 result)
{
	if(!hdr)
		return E_INVAL;

	Uint32 syncData = TCP_CMD_SYNC_START;
	
	hdr->type = TCP_CMD_TYEP_RESPONSE;
	hdr->error = convert_err_code(result);
	
	/* Send sync start */
	if(send(sock, &syncData, sizeof(syncData), 0) != sizeof(syncData))
		goto err_quit;

	/* Send cmd header */
	if(send(sock, (Int8 *)hdr, sizeof(TcpCmdHeader), 0) != sizeof(TcpCmdHeader))
		goto err_quit;

	/* Send cmd data */
	if(hdr->dataLen && dataBuf != NULL) {
		if(sendn(sock, dataBuf, hdr->dataLen, 0) != (Int32)hdr->dataLen)
			goto err_quit;
	}

	/* Send sync end */
	syncData = TCP_CMD_SYNC_END;
	if(send(sock, &syncData, sizeof(syncData), 0) != sizeof(syncData))
		goto err_quit;

	#ifdef TCP_CMD_DBG
	DBG("Repy Cmd 0x%X", hdr->cmd);
	#endif
			
	return E_NO;
	
err_quit:

	ERR("send error");

	return E_TRANS;
}


