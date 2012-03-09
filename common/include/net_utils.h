/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : net_utils.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/1/14
  Last Modified :
  Description   : net_utils.c header file
  Function List :
  History       :
  1.Date        : 2012/1/14
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __NET_UTILS_H__
#define __NET_UTILS_H__

#include "sysnet.h"
#include "common.h"
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

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/



#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*****************************************************************************
 Prototype    : socket_tcp_server
 Description  : Create TCP IPV4 server socket, bind addr and start listen
 Input        : Uint16 port          
                Int32 listenBacklog  
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern int socket_tcp_server(Uint16 port, Int32 listenBacklog);

/*****************************************************************************
 Prototype    : socket_udp_server
 Description  : Create UDP IPV4  server socket and bind addr
 Input        : Uint16 port  
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern int socket_udp_server(Uint16 port);

/*****************************************************************************
 Prototype    : connect_nonblock
 Description  : Using nonblocking to connect server
 Input        : int sockfd        
                PSA *addr         
                int len           
                Uint32 timeoutMs  
 Output       : None
 Return Value : error code
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 connect_nonblock(int sockfd, struct sockaddr *addr, socklen_t len, Uint32 timeoutSec);

/*****************************************************************************
 Prototype    : init_sock_addr
 Description  : Init socket addr for IPV4
 Input        : struct sockaddr_in *addr  
                const char *ipString      
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
static inline Int32 init_sock_addr(struct sockaddr_in *addr, const char *ipString, Uint16 port)
{
	if(!addr || !ipString)
		return E_INVAL;

	bzero(addr, sizeof(struct sockaddr_in));
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
	if(inet_pton(AF_INET, ipString, &addr->sin_addr) < 0) {
		ERRSTR("inet_pton error");
		return E_INVAL;
	}

	return E_NO;
}

/*****************************************************************************
 Prototype    : recvn
 Description  : continously recv data 
 Input        : int sockfd     
                void *buf      
                size_t nbytes  
                int flags      
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern ssize_t recvn(int sockfd, void *buf, size_t nbytes, int flags);

/*****************************************************************************
 Prototype    : sendn
 Description  : continously send data
 Input        : int sockfd       
                const void *buf  
                size_t nbytes    
                int flags        
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern ssize_t sendn(int sockfd, const void *buf, size_t nbytes, int flags);

/*****************************************************************************
 Prototype    : set_sock_block
 Description  : Set socket to blocking or nonblocking
 Input        : int sockfd    
                Bool isBlock  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/13
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline Int32 set_sock_block(int sockfd, Bool isBlock)
{
	int flags = fcntl(sockfd, F_GETFL, 0);

	if(isBlock)
		flags &= ~O_NONBLOCK;
	else
		flags |= O_NONBLOCK;

	if(fcntl(sockfd, F_SETFL, flags) < 0) {
		ERRSTR("fcnt err");
		return E_IO;
	}
	
	return E_NO;
}

/*****************************************************************************
 Prototype    : set_sock_buf_size
 Description  : Set buffer size of a socket
 Input        : int sockfd          
                Int32 sendBufSize  
                Int32 recvBufSize  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/13
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline Int32 set_sock_buf_size(int sockfd, Int32 sendBufSize, Int32 recvBufSize)
{
	Int32 ret = 0;

	if(recvBufSize > 0) {
		ret = setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &recvBufSize, sizeof(recvBufSize));
		if(ret < 0) {
		   ERRSTR("Set socket rcvbuf size err");
		}
	}

	if(sendBufSize > 0) {
		ret = setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sendBufSize, sizeof(sendBufSize));
		if(ret < 0) {
		   ERRSTR("Set socket sndbuf size err");
		}
	}
	
	return ret;
}

/*****************************************************************************
 Prototype    : set_sock_linger
 Description  : Set linger for a socket
 Input        : int sockfd      
                Bool isOn       
                Uint32 waitSec  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/13
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline Int32 set_sock_linger(int sockfd, Bool isOn, Uint32 waitSec)
{
	struct linger stLinger;

	if(isOn) {
		stLinger.l_onoff = 1;			//linger on
		stLinger.l_linger = waitSec;	//Seconds to wait before close socket
	} else {
		stLinger.l_onoff = 0;			//linger off
		stLinger.l_linger = 0;
	}	
	
	if(setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &stLinger, sizeof(struct linger)) < 0) {
		ERRSTR("set_sock_linger, setsockopt failed");
		return E_IO;
	}

	return E_NO;	
}

/*****************************************************************************
 Prototype    : set_sock_recv_timeout
 Description  : Set recv timeout for a socket
 Input        : int sockfd         
                Uint32 timeoutSec  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/13
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline Int32 set_sock_recv_timeout(int sockfd, Uint32 timeoutSec)
{
	struct timeval tmVal;

	//Set recv timeout
	tmVal.tv_sec  = timeoutSec;
	tmVal.tv_usec = 0;
	if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tmVal, sizeof(tmVal)) < 0) {
		ERRSTR("set_sock_recv_timeout, setsockopt failed");
		return E_IO;
	}

	return E_NO;
}

/*****************************************************************************
 Prototype    : set_sock_send_timeout
 Description  : Set send timeout for a socket
 Input        : int sockfd         
                Uint32 timeoutSec  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/13
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline Int32 set_sock_send_timeout(int sockfd, Uint32 timeoutSec)
{
	struct timeval tmVal;

	//Set send timeout
	tmVal.tv_sec  = timeoutSec;
	tmVal.tv_usec = 0;
	if(setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tmVal, sizeof(tmVal)) < 0) {
		ERRSTR("set_sock_send_timeout, setsockopt failed");
		return E_IO;
	}

	return E_NO;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __NET_UTILS_H__ */
