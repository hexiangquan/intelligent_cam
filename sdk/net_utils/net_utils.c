/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : net_utils.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/1/13
  Last Modified :
  Description   : Some socket APIs
  Function List :
              connect_nonblock
              recvn
              sendn
              set_sock_block
              set_sock_buf_size
              set_sock_linger
              set_sock_recv_timeout
              set_sock_send_timeout
              socket_server
  History       :
  1.Date        : 2012/1/13
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "sysnet.h"
#include "net_utils.h"
#include <ifaddrs.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <net/if.h>

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
#ifdef _NET_DBG
#define _D(fmt, args...)	DBG(fmt, ##args)
#define _W(fmt, args...)	INFO(fmt, ##args)
#else
#define _D(fmt, args...)
#define _W(fmt, args...)
#endif
	
#define _E(fmt, args...)	ERR(fmt, ##args)
#define _ES(fmt, args...)	ERRSTR(fmt, ##args)

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/


/*****************************************************************************
 Prototype    : connect_nonblock
 Description  : Nonblocking connect
 Input        : int sockfd        
                PSA *addr         
                int len           
                Uint32 timeoutMs  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/13
    Author       : Sun
    Modification : Created function

*****************************************************************************/
int connect_nonblock(int sockfd, struct sockaddr *addr, socklen_t len, Uint32 timeoutSec)
{
	/* Get status of current socket */
	int flags = fcntl(sockfd, F_GETFL, 0);

	/* Set current socket as nonblocking */
	fcntl(sockfd, F_SETFL, flags|O_NONBLOCK);

	/* Connect server */
	if(connect(sockfd, addr, len) < 0) {
		if(errno != EINPROGRESS) {
			_ES("connect_nonblock, unexpect connect error");
			return E_CONNECT;
		}
		
		fd_set fdSet;
		struct timeval tmVal;
		tmVal.tv_sec = timeoutSec;
		tmVal.tv_usec = 0;
		FD_ZERO(&fdSet);
		FD_SET(sockfd, &fdSet);
		
		if(select(sockfd + 1, NULL, &fdSet, NULL, &tmVal) <= 0) {
			_D("connect_nonblock, time out.");
			return E_CONNECT;
		} else {
			int error;
			len = sizeof(error); 
			getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len); 
			if (error) { 
				_D("Error of connect: %s", strerror(error));
				return E_CONNECT;
			}
		}
	}

	/* set to previous status, blocking */
	fcntl(sockfd, F_SETFL, flags);

	return E_NO;	
}

/*****************************************************************************
 Prototype    : sendn
 Description  : Continous send data
 Input        : int sockfd       
                const void *buf  
                size_t nbytes    
                int flags        
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/13
    Author       : Sun
    Modification : Created function

*****************************************************************************/
ssize_t sendn(int sockfd, const void *buf, size_t nbytes, int flags)
{
	size_t left = nbytes;
	ssize_t sent;

	if(!buf) {
		_E("sendn, invalid input args.");
		return E_INVAL;
	}
	
	while(left > 0){
		if((sent = send(sockfd, buf, left, flags)) < 0) {
			_ES("sendn, send data error");
			return E_TRANS;
		} else if(sent > left) {
			_E("sendn, Send Data Error, send size > left.");
			return E_TRANS;
		}

		left -= sent;
		buf = (Int8 *)buf + sent;
	}

	return nbytes;
}

/*****************************************************************************
 Prototype    : recvn
 Description  : Continously recv data
 Input        : int sockfd     
                void *buf      
                size_t nbytes  
                int flags      
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/13
    Author       : Sun
    Modification : Created function

*****************************************************************************/
ssize_t recvn(int sockfd, void *buf, size_t nbytes, int flags)
{
	size_t remain = nbytes;
	ssize_t recvCnt;

	if(!buf) {
		_E("recvn, invalid input args.");
		return E_INVAL;
	}
	
	/* Because of streaming service, we must receive repeated */
	while(remain > 0) {
		recvCnt = recv(sockfd, (Int8 *)buf + nbytes - remain, remain, flags); 
		if(recvCnt <= 0){
			_ES("recvn, recv return error");
			return recvCnt;
		}
		remain -= recvCnt;
	}
			
	return  nbytes;
}	 

/*****************************************************************************
 Prototype    : socket_tcp_server
 Description  : Create IPV4 TCP server socket and start listen
 Input        : Uint16 port          
                Int32 listenBacklog  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/13
    Author       : Sun
    Modification : Created function

*****************************************************************************/
int socket_tcp_server(Uint16 port, Int32 listenBacklog)
{
	int sockfd;

	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sockfd < 0) {
		_ES("socktet error");
		return sockfd;
	}


	/* set ip as reusable so we can rebind it */
	int opt = 1; 
	int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); 
	if(ret < 0)
		_ES("set opt for addr reuse failed!");
	else
		_D("set opt for addr reuse ok.");

	struct sockaddr_in servAddr;

	bzero(&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(port);

	if(bind(sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
		_ES("bind error");
		goto exit;
	}

	if(listen(sockfd, listenBacklog) < 0) {
		_ES("listen error");
		goto exit;
	}

	return sockfd;

exit:
	close(sockfd);
	return E_IO;
}

/*****************************************************************************
 Prototype    : socket_udp_server
 Description  : Create IPV4 UPD server
 Input        : Uint16 port  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
int socket_udp_server(Uint16 port)
{
	int sockfd;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0) {
		_ES("socktet error");
		return sockfd;
	}

	struct sockaddr_in servAddr;

	bzero(&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(port);

	if(bind(sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
		_ES("bind error");
		goto exit;
	}

	return sockfd;

exit:
	close(sockfd);
	return E_IO;
}

/*****************************************************************************
 Prototype    : get_local_ip
 Description  : get local IP
 Input        : Int8* buf      
                Int32 bufSize  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 get_local_ip(Int8* buf, Int32 bufSize)
{
#if 0
	Int32 i = 0;
	int sockfd;
	struct ifconf ifconf;
	struct ifreq *ifreq;
	char* ip;
	
	/* init ifconf */
	ifconf.ifc_len = 512;
	ifconf.ifc_buf = buf;

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		ERRSTR("create sock failed");
		return E_IO;
	}

	/* get interface info */
	ioctl(sockfd, SIOCGIFCONF, &ifconf);

	close(sockfd);

	/* parse info */
	ifreq = (struct ifreq*)buf;
	for(i= (ifconf.ifc_len/sizeof(struct ifreq)); i > 0; i--) {
		ip = inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr);

		/* skip local loop ip */
		if(strcmp(ip, "127.0.0.1")==0) {
			ifreq++;
			continue;
		}
		
		strncpy(buf,ip, bufSize);
		return E_NO;
	}

	/* can't find local IP */
	return E_NOTEXIST;
#else
	struct ifaddrs 	*ifAddrStruct = NULL;
	void 			*tmpAddrPtr = NULL;
	Int32			ret = E_NOTEXIST;

	getifaddrs(&ifAddrStruct);

	while(ifAddrStruct) {
		/* check it is IP4 */
		if(ifAddrStruct->ifa_addr->sa_family == AF_INET) {
			/* is a valid IP4 Address */
			tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
			inet_ntop(AF_INET, tmpAddrPtr, buf, bufSize);
			//_D("%s IP Address %s", ifAddrStruct->ifa_name, buf);
			if(strcmp(buf, "127.0.0.1")) {
				ret = E_NO;
				break;
			}
		} else if(ifAddrStruct->ifa_addr->sa_family==AF_INET6) {
			/* is a valid IP6 Address */
			tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
			inet_ntop(AF_INET6, tmpAddrPtr, buf, bufSize);
			//_D("%s IP Address %s", ifAddrStruct->ifa_name, buf); 
		} 
		
		ifAddrStruct = ifAddrStruct->ifa_next;
	}
	return ret;
#endif
}

/*****************************************************************************
 Prototype    : sys_set_ip
 Description  : set system ip addr and netmask
 Input        : Int32 ethId          
                const char *ipaddr   
                const char *netmask  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 sys_set_ip(Int32 ethId, const char *ipaddr, const char *netmask)
{
	char buf[128];
	size_t offset = 0;

	if(ethId > 10)
		return E_INVAL;

	offset = snprintf(buf, sizeof(buf), "ifconfig eth%d", ethId);

	if(ipaddr)
		offset += snprintf(buf + offset, sizeof(buf) - offset, " %s", ipaddr);

	if(netmask)
		offset += snprintf(buf + offset, sizeof(buf) - offset, " netmask %s", netmask);

	strcat(buf, "\n");
	
	Int32 err = system(buf);

	if(err < 0) {
		ERRSTR("set ip failed");
		return E_IO;
	}

	DBG("sys ip change to %s:%s", ipaddr, netmask);

	return E_NO;
}

/*****************************************************************************
 Prototype    : sys_set_gateway
 Description  : set system gateway addr
 Input        : const char *gateway  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 sys_set_gateway(const char *gateway, const char *netmask)
{
	char	buf[128];
	Int32	err;
	
	if(!gateway) 
		return E_INVAL;

	size_t offset;
	
	offset = snprintf(buf, sizeof(buf), "route add default gw %s", gateway);

	if(netmask)
		snprintf(buf + offset, sizeof(buf) - offset, " netmask %s", netmask);
	
	strcat(buf, "\n");
	
	err = system(buf);
	if(err < 0) {
		ERRSTR("set gateway failed");
		return E_IO;
	}

	DBG("sys new gateway: %s:%s", gateway, netmask);
	return E_NO;
}

/*****************************************************************************
 Prototype    : sys_set_domain_info
 Description  : set system net domain info
 Input        : const char *hostname  
                const char *dns       
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 sys_set_domain_info(const char *hostname, const char *dns)
{
	char 	buf[128];
	Int32	err;

	if(hostname) {
		snprintf(buf, sizeof(buf), "hostname %s\n", hostname);
		err = system(buf);
		if(err < 0) {
			ERRSTR("set host name err");
		}
	}

	if(dns) {
		snprintf(buf, sizeof(buf), "echo nameserver %s > /etc/resolv.conf\n", dns);
		err = system(buf);
		if(err < 0) {
			ERRSTR("set dns server err");
			return E_IO;
		}
	}

	DBG("sys hostname: %s, dns: %s", hostname, dns);
	return E_NO;
}

/*****************************************************************************
 Prototype    : sys_get_mac
 Description  : get system mac addr
 Input        : const char *ethName  
                Uint8 *buf           
                size_t len           
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/9/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 sys_get_mac(const char *ethName, Uint8 *buf, size_t len)
{
	if(!ethName || !buf || len < ETH_ALEN)
		return E_INVAL;
	
	int s = socket(AF_INET, SOCK_DGRAM, 0);
	struct ifreq req;
	int err;

	/* copy device name */
	strncpy(req.ifr_name, ethName, sizeof(req.ifr_name));
	err = ioctl(s, SIOCGIFHWADDR, &req);

	close(s);
	if(err < 0) {
		ERRSTR("get mac addr failed");
		return E_IO;
	}

	/* copy mac out */
	memcpy(buf, req.ifr_hwaddr.sa_data, ETH_ALEN);
	return E_NO;
}

