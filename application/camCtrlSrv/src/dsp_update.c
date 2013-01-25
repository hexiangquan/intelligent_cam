#include "dsp_update.h"
#include "crc16.h"
#include "log.h"
#include "syslink.h"
#include "syslink_proto.h"

/*****************************************************************************
 Prototype    : dsp_update
 Description  : update dsp firmware
 Input        : const void *data  
                size_t len        
                Uint32 checksum   
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/27
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 dsp_update(const void *data, size_t len, Uint32 checksum)
{
	if(!data)
		return E_INVAL;

	Uint32 crc = crc16(data, len);
	if(crc != checksum) {
		ERR("crc failed");
		return E_CHECKSUM;
	}

	int fd = open(SYSLINK_DEV_NAME, O_RDWR);
	if(fd < 0) {
		ERRSTR("open %s failed", SYSLINK_DEV_NAME);
		return E_IO;
	}

	
	size_t bufLen = sizeof(struct syslink_msg) + ROUND_UP(len, SYS_MSG_ALIGN);
	struct syslink_msg *msgBuf = calloc(1, bufLen);
	Int32 ret;
	if(!msgBuf) {
		ret = E_NOMEM;
		goto exit;
	}
	
	msgBuf->magic = SYS_MSG_MAGIC;
	msgBuf->cmd = SYS_CMD_UPDATE;
	msgBuf->dataLen = len;
	msgBuf->checksum = checksum;
	msgBuf->transLen = ROUND_UP(len, SYS_MSG_ALIGN);
	memcpy(msgBuf + 1, data, len);
	
	ret = write(fd, msgBuf, bufLen);
	if(ret < 0) {
		ERRSTR("sys link write failed.");
		ret = E_IO;
		goto exit;
	}

	ret = E_NO;
	DBG("dsp update success...");

exit:
	if(msgBuf)
		free(msgBuf);
	
	close(fd);

	return ret;
}

