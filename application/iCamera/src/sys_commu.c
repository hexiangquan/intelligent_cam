#include "common.h"
#include "log.h"
#include "sys_commu.h"

#define SYS_MSG_MAGIC	0xC0DECAFEu		
#define SYS_MSG_ALIGN	(256)

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
	msg->trans_len = ROUND_UP(msg->data_len, SYS_MSG_ALIGN);

	len = sizeof(*msg) + msg->trans_len;

	int ret = write(fd, msg, len);

	return ret < 0 ? E_IO : E_NO;
}

/*****************************************************************************
 Prototype    : sys_commu_test
 Description  : test data transfer
 Input        : None
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/7/13
    Author       : Sun
    Modification : Created function

*****************************************************************************/
int sys_commu_test()
{
	int fd;
	int err = 0;

	fd = open(SYSLINK_DEV, O_RDWR);
	assert(fd > 0);

	int i = 0;
	int len = 2 * 1024 * 1024;
	char *buf_in = malloc(len + sizeof(SysMsg));
	char *buf_out = malloc(len + sizeof(SysMsg));

	SysMsg *msg_out = (SysMsg *)buf_out;
	char *data_out = buf_out + sizeof(SysMsg);
	
	SysMsg *msg_in = (SysMsg *)buf_in;
	char *data_in = buf_in + sizeof(SysMsg);

	assert(buf_in && buf_out);
	msg_out->cmd = 0;
	msg_out->data_len = len;

	for(i = 0; i < len; ++i) 
		data_out[i] = i;

	i= 0;
	struct timeval tmStart,tmEnd; 
	float   timeRd, timeWr;
	fd_set	rdSet, wrSet;
	int fdMax;
	int errCnt = 0;
	
	while(++i < 1000) {

		FD_ZERO(&rdSet);
		FD_SET(fd, &rdSet);
		FD_ZERO(&wrSet);
		FD_SET(fd, &wrSet);
		fdMax = fd + 1;

		#if 1
		err = select(fdMax, NULL, &wrSet, NULL, NULL);
		if(err < 0) {
			ERRSTR("select err");
			usleep(1000);
			continue;
		}

		if(!FD_ISSET(fd, &wrSet)) {
			ERR("fd is not set for wr");
			continue;
		}
		#endif
		
		gettimeofday(&tmStart,NULL);
		err = sys_commu_write(fd, msg_out);
		gettimeofday(&tmEnd,NULL);
		timeWr = 1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec; 
		
		memset(buf_in, 0, len);

		#if 1
		err = select(fdMax, &rdSet, NULL, NULL, NULL);
		if(err < 0) {
			ERRSTR("select err");
			usleep(1000);
			continue;
		}

		if(!FD_ISSET(fd, &rdSet)) {
			ERR("fd is not set for rd");
			continue;
		}
		#endif
		
		//usleep(500000);
		gettimeofday(&tmStart,NULL);
		err = sys_commu_read(fd, msg_in, len + sizeof(SysMsg));
		gettimeofday(&tmEnd,NULL);
		timeRd = 1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec;
		
		if( msg_in->data_len != msg_out->data_len || 
			memcmp(data_in, data_out, msg_in->data_len) ) {
			ERR("\n<%d> **len diff: %d-%d, or mem cmp diff!**\n", 
				i, msg_out->data_len, msg_in->data_len);
			errCnt++;
		} else
			DBG("<%d> rw success, len: %d/%d, time cost: %.2f-%.2f ms", 
				i, msg_out->data_len, msg_in->trans_len, timeWr/1000, timeRd/1000);
		msg_out->cmd = i;
		//msg_out->data_len = RAND(0, len);

		usleep(20000);
	}

	close(fd);

	free(buf_in);
	free(buf_out);
	
	DBG("sys_commu, data trans err cnt: %d", errCnt);
	return err;
}

