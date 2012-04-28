#include "circular_buf.h"
#include "log.h"
#include <pthread.h>

#define PROG_NAME		"cirBufTest"

#define DEF_OPT_LEN		(200 * 1024)
#define DEF_BUF_LEN		(2 * 1024 * 1024)
#define DEF_LOOP_CNT	100

typedef struct _TestParams {
	Int32	optLen;
	Int32	bufLen;
	Int32	loopCnt;
	CirBufHandle hCirBuf;
	Int32	exit;
}TestParams;

void *cir_buf_rd_thr(void *arg)
{
	assert(arg);
	TestParams *params = (TestParams *)arg;
	char *buf = malloc(params->optLen);
	assert(buf);

	/* read buf */
	Int32 i = 0, err;
	while(!params->exit) {
		memset(buf, 0, params->optLen);
		
		err = circular_buf_read(params->hCirBuf, buf, params->optLen, 500);
		if(err) {
			ERR("<%d> read buf err: %d", i, err);
			continue;
		}

		Int32 j;
		for(j = 0; j < params->optLen; j++){
			if(buf[j] != i) {
				ERR("<%d> rd buf diff from write: %d", i, (int)buf[j]);
				break;
			}
		}
		
		Uint32 wait = random() % 2000000;

		usleep(wait);

		i++;
	}


	err = circular_buf_flush(params->hCirBuf);
	DBG("flush cir buf: %d", err);
	Int32 total, wrLen, rdLen;
	circular_buf_get_status(params->hCirBuf, &total, &wrLen, &rdLen);
	DBG("after flush, total: %d, available for write: %d, for read: %d", total, wrLen, rdLen);
	
	free(buf);
	pthread_exit(0);
	
}


static Bool main_loop(TestParams *params)
{
	Bool ret = FALSE;
	Int32 err;
	CirBufHandle hCirBuf;
	pthread_t	pid;
	char *buf = malloc(params->optLen);
	
	assert(buf);
	assert(params);

	hCirBuf = circular_buf_create(params->bufLen, 5);
	if(!hCirBuf) {
		ERR("create buf failed");
		goto exit;
	}
	params->hCirBuf = hCirBuf;
	params->exit = 0;

	/* create thread for read */
	pthread_create(&pid, NULL, cir_buf_rd_thr, params);

	Int32 i = 0;

	/* write buf */
	while(1) {
		memset(buf, i & 0xFF, params->optLen);
		
		err = circular_buf_write(hCirBuf, buf, params->optLen, 500);
		if(err) {
			ERR("<%d> write buf err: %d", i, err);
		}

		DBG("<%d> write success.", i);
		
		Uint32 wait = random() % 2000000;

		usleep(wait);
		i++;

		if(params->loopCnt > 0 && i > params->loopCnt)
			break;
	}

	ret = TRUE;
exit:

	params->exit = 1;
	pthread_join(pid, NULL);

	free(buf);

	if(hCirBuf)
		circular_buf_delete(hCirBuf);
		
	return ret;
}

static void usage(void)
{
    INFO("Compiled at %s %s with gcc %s", __DATE__, __TIME__, __VERSION__);
    INFO("Usage:");
    INFO("./%s [options]", PROG_NAME);
    INFO("Options:");
    INFO(" -h get help");
	INFO(" -l circular buf len, default: %d", DEF_BUF_LEN);
	INFO(" -p len for one time read/write, default: %d", DEF_OPT_LEN);
	INFO(" -n loop cnt, default: %d", DEF_LOOP_CNT);
    INFO("Example:");
    INFO(" use default params: ./%s", PROG_NAME);
    INFO(" use specific params: ./%s -p 1024", PROG_NAME);
}

int main(int argc, char **argv)
{
	int c;
    char *options = "l:p:n:h";
	TestParams params;
	
	params.bufLen = DEF_BUF_LEN;
	params.optLen = DEF_OPT_LEN;
	params.loopCnt = DEF_LOOP_CNT;

	while ((c = getopt(argc, argv, options)) != -1) {
		switch (c) {
		case 'l':
			params.bufLen = atoi(optarg);
			break;
		case 'p':
			params.optLen = atoi(optarg);
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



