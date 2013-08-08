#include "log.h"
#include "sys_commu.h"
#include <sys/ioctl.h>

#define DEF_BUF_SIZE	(1024 * 1024)
#define DEF_LOOP_CNT	1000

typedef struct _TestParams {
	const char *devName;
	int loopCnt;
	int bufSize;
	Uint32 baseAddr;
	const char *chanName;
}TestParams;

static int chan_open(const char *dev, Uint32 addr, const char *name)
{
	int fd = open(dev, O_RDWR);
	assert(fd > 0);
	if(fd < 0)
		return E_IO;

	struct syslink_attrs attrs;
	attrs.info_base = addr;
	strncpy(attrs.name, name, sizeof(attrs.name));
	DBG("%s, set syslink attrs!", name);
	int err = ioctl(fd, SYSLINK_S_ATTRS, &attrs);
	assert(err == 0);

	return fd;
}

/* test between process */
static void *transfer_thread(void *arg)
{
	Bool ret = FALSE;
	int fd;
	int err = 0;

	TestParams *params = (TestParams *)arg;

	DBG("%s thread start!", params->chanName);

	fd = chan_open(params->devName, params->baseAddr, params->chanName);
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
	uint32_t cmdBase = 0;

	DBG("%s, start read write loop!", params->chanName);
	
	while(1) {

		FD_ZERO(&rdSet);
		FD_SET(fd, &rdSet);
		FD_ZERO(&wrSet);
		FD_SET(fd, &wrSet);
		fdMax = fd + 1;

		/* recv msg */
		memset(buf_in, 0, len);

	#if 1
		err = select(fdMax, &rdSet, NULL, NULL, NULL);
		if(err < 0) {
			ERRSTR("%s, select err", params->chanName);
			usleep(1000);
			continue;
		}

		if(!FD_ISSET(fd, &rdSet)) {
			ERR("%s, fd is not set for rd", params->chanName);
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
			ERR("<%d> %s, read from dsp err: %d", i, params->chanName, err);
			if(err != E_CHECKSUM)
				continue;
		} else {
			DBG("<%d> %s, got msg, cmd: %d, data len: %u", i, params->chanName, 
				msg_in->cmd, msg_in->dataLen);
			if(cmdBase)
				assert(msg_in->cmd == cmdBase + 1);
			cmdBase = msg_in->cmd;
		}

	#if 1
		err = select(fdMax, NULL, &wrSet, NULL, NULL);
		if(err < 0) {
			ERRSTR("%s, select err", params->chanName);
			usleep(1000);
			continue;
		}

		if(!FD_ISSET(fd, &wrSet)) {
			ERR("%s, fd is not set for wr", params->chanName);
			continue;
		}
	#endif

		/* echo msg back */
		//*msg_out = *msg_in;
		
		gettimeofday(&tmStart,NULL);
		err = sys_commu_write(fd, msg_in); //msg_out
		gettimeofday(&tmEnd,NULL);
		timeWr = 1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec; 

		if(err < 0) {
			ERR("%s, write err.", params->chanName);
			usleep(1000);
			continue;
		}
		
		DBG("<%d> %s, rw success, len: %d/%d, w/r time cost: %.2f-%.2f ms", 
				i, params->chanName,  msg_in->dataLen, msg_in->transLen, timeWr/1000, timeRd/1000);
	
		#if 0
		if( msg_in->dataLen != msg_out->dataLen || 
			memcmp(data_in, data_out, msg_in->dataLen) ) {
			ERR("\n<%d> %s, len diff: %d-%d, or mem cmp diff!\n", 
				i, params->chanName, msg_out->dataLen, msg_in->dataLen);
			errCnt++;
		} else
			DBG("<%d> %s, rw success, len: %d/%d, time cost: %.2f-%.2f ms", 
				i, params->chanName,  msg_out->dataLen, msg_in->transLen, timeWr/1000, timeRd/1000);
		msg_out->cmd = i;
		//msg_out->dataLen = RAND(0, len);
		#endif 
		
		usleep(1000);

		++i;
		
		if(params->loopCnt > 0 && i > params->loopCnt)
			break;
	}

	ret = TRUE;

exit:

	close(fd);

	free(buf_in);
	free(buf_out);
	
	DBG("%s, sys_commu, data trans err cnt: %d", params->chanName, errCnt);

	pthread_exit(0);
	
}

#define CHAN_NUM 3

Bool main_loop(TestParams *params)
{
	pthread_t pid[CHAN_NUM] = {0};
	TestParams threadParams[CHAN_NUM];
	Uint32 baseAddr[] = {0xE0000000, 0xE0080000, 0xE0100000};
	const char *chanName[] = {"General", "CapCtrl", "FileSys"};

	int i;
	for(i = 0; i < CHAN_NUM; ++i) {
		threadParams[i] = *params;
		threadParams[i].baseAddr = baseAddr[i];
		threadParams[i].chanName = chanName[i];
		int err = pthread_create(&pid[i], NULL, transfer_thread, &threadParams[i]);
		assert(err == 0);
	}

	for(i = 0; i < CHAN_NUM; ++i) {
		pthread_join(pid[i], NULL);
	}

	DBG("sys commu test success...");
	return TRUE;
	
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

