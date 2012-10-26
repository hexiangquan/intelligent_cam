#include "common.h"
#include "log.h"
#include "sys_commu.h"

/*****************************************************************************
 Prototype    : sys_commu_read
 Description  : read from target cpu
 Input        : int fd       
                SysMsg *buf  
                int len      
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/7/13
    Author       : Sun
    Modification : Created function
    Note: msg header and data will put contiously

*****************************************************************************/
int sys_commu_read(int fd, SysMsg *buf, int len)
{
	if(fd < 0 || !buf || len <= 0)
		return E_INVAL;

	int ret;
	ret = read(fd, buf, len);
	//DBG("read len: %d", ret);
	
	if(ret < 0)
		return E_IO;

	if(buf->magic != SYS_MSG_MAGIC)
		return E_CHECKSUM;

	return E_NO;
}

/*****************************************************************************
 Prototype    : sys_commu_write
 Description  : write data to target
 Input        : int fd       
                SysMsg *msg  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/7/13
    Author       : Sun
    Modification : Created function

*****************************************************************************/
int sys_commu_write(int fd, SysMsg *msg)
{
	if(fd < 0 || !msg )
		return E_INVAL;

	int len;
	
	msg->magic = SYS_MSG_MAGIC;
	msg->transLen = ROUND_UP(msg->dataLen, SYS_MSG_ALIGN);

	len = sizeof(*msg) + msg->transLen;

	int ret = write(fd, msg, len);

	return ret < 0 ? E_IO : E_NO;
}


