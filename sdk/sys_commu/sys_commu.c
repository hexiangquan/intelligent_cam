#include "common.h"
#include "log.h"
#include "sys_commu.h"
#include "crc16.h"
#include "syslink.h"

#define SYS_COMMU_CRC_EN	1

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
int sys_commu_read(int fd, SysMsg *msg, int len)
{
	if(fd < 0 || !msg || len < sizeof(SysMsg))
		return E_INVAL;

	int ret;

	/* read msg header */
	do {
		ret = read(fd, msg, sizeof(SysMsg));
		//DBG("read len: %d", ret);
		
		if(ret != sizeof(SysMsg))
			return E_IO;
	} while(msg->magic != SYS_MSG_MAGIC);

	if(msg->dataLen) {
		/* Read additional data */
		if(len - sizeof(SysMsg) < msg->transLen) {
			ERR("no enough space for recieve append data, translen: %u, buf available: %u",
				msg->transLen, len - sizeof(SysMsg));
			/* clear such length */
			ioctl(fd, SYSLINK_FLUSH_RD, &msg->transLen);
			return E_NOSPC;
		}

		Uint8 *data = (Uint8 *)msg + sizeof(SysMsg);
		ret = read(fd, data, msg->transLen);
		if(ret < 0)
			return E_IO;
		
	#ifdef SYS_COMMU_CRC_EN
		Uint32 crc = crc16(data, msg->dataLen);
		if(crc != msg->checksum) {
			ERR("crc check failed(%x-%x), cmd: %x, data len: %u!",
				msg->checksum, crc, msg->cmd, msg->dataLen);
			return E_CHECKSUM;
		}
	#endif
	}

	//DBG("recv len: %u, data len: %u", ret, msg->transLen + sizeof(SysMsg));
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

#ifdef SYS_COMMU_CRC_EN
	if(msg->dataLen) {
		Uint8 *data = (Uint8 *)msg + sizeof(SysMsg);
		msg->checksum = crc16(data, msg->dataLen);
	}
#endif

	int ret = write(fd, msg, len);

	return ret < 0 ? E_IO : E_NO;
}


