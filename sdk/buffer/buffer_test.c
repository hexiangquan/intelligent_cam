#include "buffer.h"
#include "log.h"
#include <pthread.h>

#define DEF_SIZE 	(256 * 1024)
#define MAX_SIZE	(16 * 1024 * 1024)
#define BUF_NUM		4
#define POOL_TEST
#define BUFS_NUM	(BUF_NUM * 2)

static void *thr_alloc_buf(void *arg)
{
	int i;
	BufHandle hBuf[BUF_NUM];
	int size = (int)arg;
	BufAllocAttrs allocAttrs;
	struct timeval tmStart,tmEnd; 
	float   timeUse;

	memset(hBuf, 0, sizeof(hBuf));
	allocAttrs.type = BUF_TYPE_POOL;
	allocAttrs.flags = 0;//BUF_FLAG_CACHED;
	allocAttrs.align = 256;
	
	for(i = 0; i < BUF_NUM; i++)
	{
		gettimeofday(&tmStart,NULL);
		hBuf[i] = buffer_alloc(size, &allocAttrs);
		gettimeofday(&tmEnd,NULL); 
		
		if(!hBuf[i]) {
			INFO("Totol alloc cnt: %d", i);
			break;
		}

		timeUse = 1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec; 
		DBG("Thread %u Buffer info:", pthread_self());
		DBG("  User addr: 0x%X", (Uint32)buffer_get_user_addr(hBuf[i]));
		DBG("  Phys addr: 0x%X", (Uint32)buffer_get_phy_addr(hBuf[i]));
		DBG("  Size: %u", buffer_get_size(hBuf[i]));
		DBG("  Alloc buf cost: %f us", timeUse);
		//size = size * 2;
		if(size > MAX_SIZE)
			size = MAX_SIZE;

		char *buf = buffer_get_user_addr(hBuf[i]);
		memset(buf, i, buffer_get_size(hBuf[i]));
		assert(buf[i] == (i & 0xFF));

		usleep(100);
	}

	for(i = 0; i < BUF_NUM; i++) {
		
		int err;
		if(hBuf[i]) {
			gettimeofday(&tmStart,NULL);
			err = buffer_free(hBuf[i]);
			gettimeofday(&tmEnd,NULL); 
			assert(err == E_NO);

			timeUse = 1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec; 
			DBG("  Free buf cost: %f us", timeUse);
		}
	}

	DBG("Thread %u exit...", pthread_self());
	pthread_exit(0);

}

//static BufHandle bufs[8] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
//static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//static Bool g_exit = 0;
//static int head = 0, tail = 0;
//#define CIR_LEN ARRAY_SIZE(bufs)

typedef struct _AllocInfo {
	int head;
	int tail;
	int exit;
	BufHandle bufs[BUFS_NUM];
	BufPoolHandle hPool;
	
}AllocInfo;

static void *thr_pool_alloc(void *arg)
{
	AllocInfo* info = (AllocInfo *)arg;
	BufPoolHandle hBufPool = info->hPool;
	assert(info && hBufPool);

	int i;
	struct timeval tmStart,tmEnd; 
	float   timeUse;
	int cirLen = ARRAY_SIZE(info->bufs);

	int totalNum = buf_pool_get_total_num(hBufPool);
	int freeNum = buf_pool_get_free_num(hBufPool);
	DBG("Pool buf num, total: %d, free: %d", totalNum, freeNum);
	
	for(i = 0; i < totalNum * 4; i++) {

		//pthread_mutex_lock(&mutex);
		gettimeofday(&tmStart,NULL);
		BufHandle hBuf = buf_pool_alloc(hBufPool);
		//BufHandle hBuf = buf_pool_alloc_wait(hBufPool, 1);
		gettimeofday(&tmEnd,NULL); 
		if(!hBuf) {
			DBG("Alloc buf failed...");
			continue;
		}

		timeUse = 1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec; 
		DBG("Thread %u Buffer info:", pthread_self());
		DBG("  User addr: 0x%X", (Uint32)buffer_get_user_addr(hBuf));
		DBG("  Phys addr: 0x%X", (Uint32)buffer_get_phy_addr(hBuf));
		DBG("  Size: %u", buffer_get_size(hBuf));
		DBG("  Alloc buf cost: %f us", timeUse);

		while(((info->head + 1)%cirLen) == info->tail)
			sleep(1);

		DBG("Head = %d", info->head);
		info->bufs[info->head] = hBuf;
		info->head = (info->head + 1)%cirLen;
		//pthread_mutex_unlock(&mutex);
		
		usleep(1000);

		/*
		gettimeofday(&tmStart,NULL);
		buf_pool_free(hBuf);
		gettimeofday(&tmEnd,NULL); 
		timeUse = 1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec;
		DBG("  Free buf cost: %f us", timeUse);
		*/
	}

	DBG("Pool alloc Thread %u exit...", pthread_self());
	info->exit = 1;
	pthread_exit(0);
}

static void *thr_pool_free(void *arg)
{
	//BufPoolHandle hBufPool = arg;
	struct timeval tmStart,tmEnd; 
	float   timeUse;
	AllocInfo *info = (AllocInfo *)arg;
	int cirLen = ARRAY_SIZE(info->bufs);

	//int i;
	DBG("info->exit = %d", info->exit);
	while(!info->exit) {
		if(info->tail != info->head){
			if(info->bufs[info->tail]) {
				gettimeofday(&tmStart,NULL);
				buf_pool_free(info->bufs[info->tail]);
				gettimeofday(&tmEnd,NULL); 
				info->bufs[info->tail] = NULL;
				info->tail = (info->tail + 1)%cirLen;
				timeUse = 1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec;
				DBG("  Free buf cost: %f us", timeUse);
			}
		}
		usleep(4000);
	}

	/* Free all left buf */
	while(info->tail != info->head){
		if(info->bufs[info->tail]) {
			gettimeofday(&tmStart,NULL);
			buf_pool_free(info->bufs[info->tail]);
			gettimeofday(&tmEnd,NULL); 
			info->bufs[info->tail] = NULL;
			info->tail = (info->tail + 1)%cirLen;
			timeUse = 1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec;
			DBG("  Free buf cost: %f us", timeUse);
		}
	}

	DBG("Pool Free Thread %u exit...", pthread_self());
	pthread_exit(0);
}

static void main_loop(int size)
{
	if(buffer_init() < 0) {
		ERR("init buffer failed!!");
		return;
	}

	int i, err;
	pthread_t pid[16];
	BufPoolHandle hBufPool;
	static AllocInfo allocInfo[3];

	hBufPool = buf_pool_create(size, BUF_NUM, NULL);
	assert(hBufPool);

#if 0	
	for(i = 0; i < 2; i++) {
		err = pthread_create(&pid[i], NULL, thr_alloc_buf, (void *)size);
		if(err < 0) {
			ERRSTR("create thread");
			break;
		}
	}
#endif
	

	memset(allocInfo, sizeof(allocInfo), 0);
	allocInfo[0].hPool = allocInfo[1].hPool = allocInfo[2].hPool = hBufPool;

	pthread_create(&pid[0], NULL, thr_pool_alloc, &allocInfo[0]);
	pthread_create(&pid[1], NULL, thr_pool_free, &allocInfo[0]);
	pthread_create(&pid[2], NULL, thr_pool_alloc, &allocInfo[1]);
	pthread_create(&pid[3], NULL, thr_pool_free, &allocInfo[1]);
	pthread_create(&pid[4], NULL, thr_pool_alloc, &allocInfo[2]);
	pthread_create(&pid[5], NULL, thr_pool_free, &allocInfo[2]);
	i = 6;

	int j;
	for(j = 0; j < i; j++) {
		pthread_join(pid[j], NULL);
	}

	DBG("Test complete");
	buf_pool_free_all(hBufPool);
//exit:
	err = buf_pool_delete(hBufPool);
	assert(err == E_NO);
	
	buffer_exit();
}

static void usage(void)
{
    printf("testBuf Compiled on %s %s with gcc %s\n", __DATE__, __TIME__, __VERSION__);
    printf("Usage:\n");
    printf("./testBuf [options]\n");
    printf("Options:\n");
    printf(" -h get help\n");
	printf(" -s size for alloc, default: %d\n", DEF_SIZE);
    printf("Example:\n");
    printf(" use default params: ./testBuf\n");
    printf(" use specified params: ./testBuf -s 512000\n");
}

int main(int argc, char **argv)
{
	int c;
    char *options = "s:h";
	int size = DEF_SIZE;

	while ((c=getopt(argc, argv, options)) != -1)
	{
		switch (c)
		{
			case 's':
				size = atoi(optarg);
				break;
			case 'h':
			default:
				usage();
				return -1;
		}
	}

	main_loop(size);

	exit(0);
}

