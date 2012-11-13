#include "log.h"
#include "sys_commu.h"

#define DEF_BUF_SIZE	(2 * 1024 * 1024)
#define DEF_LOOP_CNT	1000

typedef struct _TestParams {
	const char *devName;
	int loopCnt;
	int bufSize;
}TestParams;


/* test between process */
static Bool main_loop(TestParams *params)
{
	Bool ret = FALSE;
	int fd;
	int err = 0;

	fd = open(params->devName, O_RDWR);
	assert(fd > 0);
	if(fd < 0)
		goto exit;

	int i = 0;
	int len = params->bufSize;
	char *buf_in = malloc(len + sizeof(SysMsg));
	char *buf_out = malloc(len + sizeof(SysMsg));

	SysMsg *msg_out = (SysMsg *)buf_out;
	char *data_out = buf_out + sizeof(SysMsg);
	
	SysMsg *msg_in = (SysMsg *)buf_in;
	char *data_in = buf_in + sizeof(SysMsg);

	assert(buf_in && buf_out);
	msg_out->cmd = 0;
	msg_out->dataLen = len;

	for(i = 0; i < len; ++i) 
		data_out[i] = i;

	i= 0;
	struct timeval tmStart,tmEnd; 
	float	timeRd, timeWr;
	fd_set	rdSet, wrSet;
	int fdMax;
	int errCnt = 0;
	
	while(++i < params->loopCnt) {

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
		bzero(data_in, len);
		gettimeofday(&tmStart,NULL);
		err = sys_commu_read(fd, msg_in, len + sizeof(SysMsg));
		gettimeofday(&tmEnd,NULL);
		timeRd = 1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec;

		if(err) {
			ERR("<%d> read from dsp err: %d", i, err);
			if(err != E_CHECKSUM)
				continue;
		}
		
		if( msg_in->dataLen != msg_out->dataLen || 
			memcmp(data_in, data_out, msg_in->dataLen) ) {
			ERR("\n<%d> **len diff: %d-%d, or mem cmp diff!**\n", 
				i, msg_out->dataLen, msg_in->dataLen);
			errCnt++;
		} else
			DBG("<%d> rw success, len: %d/%d, time cost: %.2f-%.2f ms", 
				i, msg_out->dataLen, msg_in->transLen, timeWr/1000, timeRd/1000);
		msg_out->cmd = i;
		//msg_out->dataLen = RAND(0, len);

		usleep(5000);
	}

	ret = TRUE;

exit:

	close(fd);

	free(buf_in);
	free(buf_out);
	
	DBG("sys_commu, data trans err cnt: %d", errCnt);

	return ret;
	
}

static void usage(void)
{
    INFO("sysCommuTest Compiled on %s %s with gcc %s", __DATE__, __TIME__, __VERSION__);
    INFO("Usage:");
    INFO("./sysCommuTest [options]");
    INFO("Options:");
    INFO(" -h get help");
	INFO(" -n loop count, default: %d", DEF_LOOP_CNT);
	INFO(" -d sys link dev name, default: %s", SYSLINK_DEV);
	INFO(" -s commu buf size, default: %d", DEF_BUF_SIZE);
    INFO("Example:");
    INFO(" use default params: ./sysCommuTest");
    INFO(" use specific params: ./sysCommuTest -d /dev/syslink -s 2048");
}

int main(int argc, char **argv)
{
	int c;
    char *options = "s:n:d:h";
	TestParams params;
	
	params.devName = SYSLINK_DEV;
	params.loopCnt = DEF_LOOP_CNT;	
	params.bufSize = DEF_BUF_SIZE;

	while ((c = getopt(argc, argv, options)) != -1) {
		switch (c) {
		case 'd':
			params.devName = optarg;
			break;
		case 's':
			params.bufSize = atoi(optarg);
			break;
		case 'n':
			params.loopCnt = atoi(optarg);
			break;
		case 'h':
		default:
			usage();
			return -1;
		}
	}

	Bool ret = main_loop(&params);
	if(ret)
		INFO("test success!");
	else
		INFO("test failed!");

	exit(0);
}

