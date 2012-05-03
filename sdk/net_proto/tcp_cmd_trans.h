/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : cmd_trans.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/23
  Last Modified :
  Description   : cmd_trans.c header file
  Function List :
  History       :
  1.Date        : 2012/3/23
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __TCP_CMD_TRANS_H__
#define __TCP_CMD_TRANS_H__

#include "common.h"

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

/* 
 * Header of a command packet 
 */
typedef struct {
	Uint32		serial;			// Sequence number of command, start from 0
	Uint32		cmd;			// Command, see eTcpCommandType
	Uint16		type;			// Command type, see eTcpCommandType
	Uint16		error;			// Error code for response
	Uint32		flags;			// Flag for extra info
	Uint32		checkSum;		// Checksum
	Uint32		dataLen;		// Additive data, can be 0
}TcpCmdHeader;

/* bit function define for flag in TCP_COMMAND_HEADER */
#define TCP_CMD_FLAG_SAVE			(1 << 0)
#define TCP_CMD_FLAG_CHECKSUM		(1 << 1)

/* Command type */
enum TcpCommandType
{
    TCP_CMD_TYEP_UNDEFINED = 0,
    TCP_CMD_TYEP_REQUEST,
    TCP_CMD_TYEP_RESPONSE
};

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

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
extern Int32 tcp_cmd_recv(int sock, TcpCmdHeader *hdr, void *dataBuf, Int32 bufLen);

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
extern Int32 tcp_cmd_reply(int sock, TcpCmdHeader *hdr, const Int8 *dataBuf, Int32 result);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __CMD_TRANS_H__ */
