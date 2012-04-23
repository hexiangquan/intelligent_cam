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
#ifndef __UDP_CMD_TRANS_H__
#define __UDP_CMD_TRANS_H__

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

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/* 
 * Header of a command packet 
 */
typedef struct {
	Uint32		cmd;		// Command Id
	Uint32		dataLen;	// Additive data Len
}UdpCmdHeader;

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*****************************************************************************
 Prototype    : udp_cmd_recv
 Description  : recv udp cmd
 Input        : Int32 sock                
                UdpCmdHeader *header      
                void *buf                 
                Uint32 bufLen             
                struct sockaddr_in *addr  
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/23
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 udp_cmd_recv(Int32 sock, UdpCmdHeader *header, void *buf, Uint32 bufLen, struct sockaddr_in *addr);

/*****************************************************************************
 Prototype    : udp_cmd_send
 Description  : send udp cmd
 Input        : Int32 sock                
                UdpCmdHeader *header      
                void *buf                 
                struct sockaddr_in *addr  
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/23
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 udp_cmd_send(Int32 sock, UdpCmdHeader *header, void *buf, struct sockaddr_in *addr);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __CMD_TRANS_H__ */

