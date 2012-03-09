/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : msg.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/2/14
  Last Modified :
  Description   : msg.c header file
  Function List :
  History       :
  1.Date        : 2012/2/14
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __MSG_H__
#define __MSG_H__

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
typedef struct _MsgObj *MsgHandle;

/* Header of message */
typedef struct _MsgHeader {
	Uint32	magicNum;		/* Magic Number for sync */
	Uint16	cmd;			/* Command Id */
	Uint16	index;			/* Unique index of this command */
	long	param[2];		/* General params */
	Int32	dataLen;		/* Len of append data */
	/* Int8 dataBuf[...] */		/* Append data should be add here */
}MsgHeader;

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
#define MSG_MAGIC_SEND			0x20120FED		/* Magic num for request msg */
#define MSG_MAGIC_RESP			0x20120BAC		/* Magic num for response msg */

#define MSG_MAX_DATA_LEN		(128 * 1024)	/* Maximum of data size to send */

#define MSG_FLAG_NONBLK			(1 << 0)		/* Flag for non-blocking Send and Recv Msg */
#define MSG_LEN_ALIGN			4				/* All msg len should be for bytes aligned */

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*****************************************************************************
 Prototype    : msg_create
 Description  : Create msg handle
 Input        : const Int8 *srcName, our name  
                const Int8 *dstName, default dest name  
                Int32 flag, reserved           
 Output       : none
 Return Value : MsgHandle if Success, otherwise return NULL
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern MsgHandle msg_create(const Int8 *srcName, const Int8 *dstName, Int32 flag);

/*****************************************************************************
 Prototype    : msg_delete
 Description  : Delete msg handle
 Input        : MsgHandle hMsg, handle created by msg_create  
 Output       : None
 Return Value : common error code
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 msg_delete(MsgHandle hMsg);

/*****************************************************************************
 Prototype    : msg_recv
 Description  : Recv a msg
 Input        : 	MsgHandle hMsg, handle created by msg_create     
                	Int32 bufLen, len of dataBuf, must no less than sizeof(MsgHeader)      
 Output       :  void *dataBuf, buffer for header and append data, if any
 			the first field of this buffer will be filled with MsgHeader
 Return Value : common error code
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 msg_recv(MsgHandle hMsg, void *buf, Int32 bufLen);

/*****************************************************************************
 Prototype    : msg_send
 Description  : Send a msg
 Input        : MsgHandle hMsg, handle created by msg_create 
 
                	const char *dstName,  name of dest, must have been created by msg_create     
					if this is set to NULL, the module use default dest name when send request, 
					or use last msg recv name when send reply
 
                	const void *data, data to send, include header and append data, must contain header as the 
                			first field, i.e., define this structure 
                			struct MsgData {
						MsgHeader 	header;
						Int8			buf[128];
                			}
                	Int32 dataLen, length of data to send, Unit bytes, including the header
                			normally, it is equal to sizeof(MsgHeader) + header.dataLen, 
                			should be 4 bytes aligned 
 Output       : None
 Return Value : fails: common error code, success: num of bytes received 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 msg_send(MsgHandle hMsg, const char *dstName, const void *data, Int32 dataLen);

/*****************************************************************************
 Prototype    : msg_set_recv_timeout
 Description  : Set recv timeout in seconds
 Input        : MsgHandle hMsg    
                Int32 timeoutSec  
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 msg_set_recv_timeout(MsgHandle hMsg, Int32 timeoutSec);

/*****************************************************************************
 Prototype    : msg_set_send_timeout
 Description  : Set send timeout in seconds
 Input        : MsgHandle hMsg    
                Int32 timeoutSec  
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 msg_set_send_timeout(MsgHandle hMsg, Int32 timeoutSec);

/*****************************************************************************
 Prototype    : msg_get_fd
 Description  : Get fd for select, etc.
 Input        : MsgHandle hMsg  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern int msg_get_fd(MsgHandle hMsg);

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
extern const char *msg_get_recv_src(MsgHandle hMsg);

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
extern Int32 msg_set_default_dst(MsgHandle hMsg, const char *dstName);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __MSG_H__ */
