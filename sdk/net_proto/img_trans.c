/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : img_trans.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/23
  Last Modified :
  Description   : protocol stack for image upload using own TCP protocol
  Function List :
              img_trans_connect
              img_trans_create
              img_trans_delete
              img_trans_disconnect
              img_trans_send
              img_trans_set_src_desp
              img_trans_set_srv_info
  History       :
  1.Date        : 2012/3/23
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "img_trans.h"
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

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/* header for sync */
typedef struct {
	Int8		syncStart[4];		// sync start, "<I> " (0x3C, 0x49, 0x3E, 0x20)
	Int8		imageSource[16];	// src of image 
	ImgHdrInfo	info;
}ImgTransHeader;

/* object of this module */
struct ImgTransObj {
	Int32				sock;
	struct sockaddr_in	srvAddr;
	ImgTransHeader		transHdr;
	Uint32				timeout;
	Int32				flags;
	Bool				connected;
};



/*****************************************************************************
 Prototype    : img_trans_set_srv_info
 Description  : set server IP, port
 Input        : ImgTransHandle hTrans  
                const char *ipString   
                Uint16 port            
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/23
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 img_trans_set_srv_info(ImgTransHandle hTrans, const char *ipString, Uint16 port)
{
	struct sockaddr_in *addr = &hTrans->srvAddr;

	if(!hTrans || !ipString)
		return E_INVAL;

	if(hTrans->connected)
		img_trans_disconnect(hTrans);

	/* Set server addr */
	bzero(addr, sizeof(struct sockaddr_in));
	addr->sin_addr.s_addr= inet_addr(ipString);
	addr->sin_family = AF_INET;
	if(!port) {
		WARN("img_trans_set_srv_info, using default port");
		port = IMG_TRANS_SRV_PORT_DEFAULT;
	}
	addr->sin_port = htons(port);

	return E_NO;
}

/*****************************************************************************
 Prototype    : img_trans_create
 Description  : create module
 Input        : const char *ipString  
                Uint16 port           
                const char *srcDesp   
                Uint32 timeout        
                Int32 flags           
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/23
    Author       : Sun
    Modification : Created function

*****************************************************************************/
ImgTransHandle img_trans_create(const char *ipString, Uint16 port, const char *srcDesp, Uint32 timeout, Int32 flags)
{
	ImgTransHandle	hTrans;
	const Int8 syncStart[] = {0x3C, 0x49, 0x3E, 0x20};

	if(!ipString)
		return NULL;

	hTrans = calloc(1, sizeof(struct ImgTransObj));
	if(!hTrans) {
		ERR("alloc mem for this module error.");
		return NULL;
	}

	/* set fields in handle */
	hTrans->timeout = timeout;
	hTrans->flags = flags;
	
	img_trans_set_srv_info(hTrans, ipString, port);

	/* set trans header */
	memcpy(hTrans->transHdr.syncStart, syncStart, sizeof(syncStart));
	if(srcDesp)
		strncpy(hTrans->transHdr.imageSource, srcDesp, sizeof(hTrans->transHdr.imageSource));

	return hTrans;
}

/*****************************************************************************
 Prototype    : img_trans_delete
 Description  : delete module
 Input        : ImgTransHandle hTrans  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/23
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 img_trans_delete(ImgTransHandle hTrans)
{
	if(hTrans->connected)
		img_trans_disconnect(hTrans);

	if(hTrans->sock > 0)
		close(hTrans->sock);

	free(hTrans);
	return E_NO;
}

/*****************************************************************************
 Prototype    : img_trans_connect
 Description  : connect server
 Input        : ImgTransHandle hTrans  
                Uint32 timeoutSec      
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/23
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 img_trans_connect(ImgTransHandle hTrans, Uint32 timeoutSec)
{
	Int32 sock, ret;
	struct sockaddr_in *addr = &hTrans->srvAddr;

	/* disconnect first */
	if(hTrans->connected)
		img_trans_disconnect(hTrans);
	
	if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		ERRSTR("create sock failed.");
		return E_IO;
	}
	
	if(timeoutSec > 0)
		ret = connect_nonblock(sock,(struct sockaddr *)addr, sizeof(*addr), timeoutSec);
	else
		ret = connect(sock, (struct sockaddr *)addr, sizeof(*addr));
	
	if(ret < 0) {
		//ERRSTR("connect error.");
		close(sock);
		return E_CONNECT;
	}

	//DBG("img trans, connect to img server ok");

	if(hTrans->timeout)
		set_sock_send_timeout(sock, hTrans->timeout);

	set_sock_linger(sock, TRUE, hTrans->timeout);
	
	hTrans->connected = TRUE;
	hTrans->sock = sock;

	return E_NO;
}

/*****************************************************************************
 Prototype    : img_trans_disconnect
 Description  : disconnect server
 Input        : ImgTransHandle hTrans  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/23
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 img_trans_disconnect(ImgTransHandle hTrans)
{
	if(!hTrans)
		return E_INVAL;
	
	if(!hTrans->connected)
		return E_NO;
	
	shutdown(hTrans->sock, SHUT_WR);
	close(hTrans->sock);
	hTrans->sock = -1;
	hTrans->connected = FALSE;
	
	return E_NO;
}
/*****************************************************************************
 Prototype    : img_trans_send
 Description  : send image
 Input        : ImgTransHandle hTrans   
                const ImgHdrInfo *info  
                const void *data        
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/23
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 img_trans_send(ImgTransHandle hTrans, const ImgHdrInfo *info, const void *data)
{
	const Int8 	syncEnd[] = {0x3C, 0x2F, 0x49, 0x3E};
	Int32		err;

	if(!hTrans || !info || !data || !info->imageLen)
		return E_INVAL;

	if(!hTrans->connected) {
		err = img_trans_connect(hTrans, 5);
		if(err) {
			ERR("srv should be connect first");
			return err;
		}
	}

	hTrans->transHdr.info = *info;
	if(send(hTrans->sock, &hTrans->transHdr, sizeof(hTrans->transHdr), 0) == sizeof(hTrans->transHdr))
		if(sendn(hTrans->sock, data, info->imageLen, 0) == info->imageLen)
			if(send(hTrans->sock, syncEnd, sizeof(syncEnd), 0) == sizeof(syncEnd))
				return E_NO;

	//ERRSTR("send data error");
	return E_TRANS;
}

/*****************************************************************************
 Prototype    : img_trans_set_src_desp
 Description  : set our description
 Input        : ImgTransHandle hTrans  
                const char *srcDesp    
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/23
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 img_trans_set_src_desp(ImgTransHandle hTrans, const char *srcDesp)
{
	if(!hTrans || !srcDesp)
		return E_INVAL;
	
	strncpy(hTrans->transHdr.imageSource, srcDesp, sizeof(hTrans->transHdr.imageSource));

	return E_NO;
}

