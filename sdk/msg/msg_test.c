#include "msg.h"
#include "log.h"
#include <pthread.h>

#define BASE_NAME		"/tmp/msgtest"
#define DEF_MSG_SIZE	1024
#define DEF_LOOP_CNT	100
#define DEF_THR_NUM		10
#define PARENT_MSG		"/tmp/msgParent"
#define CHILD_MSG		"/tmp/msgChild"
#define ICAM_MSG_NAME	"/tmp/iCamCtrl"



//#define DEF_USE_THR		1
#define PARENT_ONLY

typedef struct _TestParams {
	char *baseName;
	int loopCnt;
	int msgSize;
	int thrNum;
}TestParams;

typedef struct _TaskEnv {
	char name[256];
	char dest[256];
	int	 id;
	TestParams *params;
}TaskEnv;

typedef struct _MsgData {
	MsgHeader	header;
	char		buf[512];
}MsgData;

#ifdef DEF_USE_THR
static void *thr_msg(void *arg)
{
	assert(arg);
	TaskEnv *env = (TaskEnv *)arg;
	MsgHandle hMsg;
	MsgData	msgData;	

	hMsg = msg_create(env->name, env->dest, 0);
	if(!hMsg) {
		ERR("create msg handle failed...");
		goto exit;
	}

	DBG("create msg ok, our name: %s, dst name: %s.", env->name, env->dest);
	sleep(1);
	
	int cnt = 0, dataLen = sizeof(msgData.header);
	int err;
	
	memset(&msgData, 0, sizeof(msgData));
	msgData.header.cmd = cnt;
	msgData.header.index = cnt;
	msgData.header.magicNum = MSG_MAGIC_SEND;
	msgData.header.dataLen = sprintf(msgData.buf, "%s start", env->name);
	dataLen = msgData.header.dataLen + sizeof(msgData.header);	
	//msgData.header.dataLen += 16;

	if(env->id) {
		DBG("send msg to %s", env->dest);
		err = msg_send(hMsg, env->dest, &msgData, dataLen);
		if(err < 0)
			DBG("send msg returns %d", err);
	}

	DBG("%s, start recv msg", env->name);
	struct timeval tmStart,tmEnd; 
	float   timeUse;
	err = msg_set_recv_timeout(hMsg, 1);
	err |= msg_set_send_timeout(hMsg, 2);
	assert(err == E_NO);
	
	while(1) {
		/* Do echo loop */
		gettimeofday(&tmStart,NULL);
		err = msg_recv(hMsg, &msgData, sizeof(msgData));
		gettimeofday(&tmEnd,NULL); 
		
		if(err > 0) {
			DBG("<%u> recv msg from %s, len: %d", 
				(unsigned int)pthread_self(), msg_get_recv_src(hMsg), err);
			DBG("  cmd: %d, index: %d", 
				msgData.header.cmd, msgData.header.index);
			if(msgData.header.dataLen > 0)
				DBG("  append data: %s", msgData.buf);
			timeUse = 1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec; 
			DBG("  recv cost: %.0f us", timeUse);
		}
		/* Send back data */
		sprintf(msgData.buf, "<%d> %s send msg", cnt, env->name);
		msgData.header.dataLen = sizeof(msgData.buf);
		msgData.header.magicNum = MSG_MAGIC_RESP;
		msgData.header.index = cnt;
		dataLen = msgData.header.dataLen + sizeof(msgData.header);

		gettimeofday(&tmStart,NULL);
		err = msg_send(hMsg, NULL, &msgData, dataLen);
		gettimeofday(&tmEnd,NULL); 
		if(err)
			ERR("Send msg failed");

		timeUse = 1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec; 
		DBG("<%u> send %d bytes msg to %s cost: %.0f us", 
			(unsigned int)pthread_self(), dataLen, msg_get_recv_src(hMsg), timeUse);

		cnt++;
		if( env->params->loopCnt > 0 &&
			cnt > env->params->loopCnt)
			break;
	}


exit:
	if(env)
		free(env);
	if(hMsg)
		msg_delete(hMsg);
	pthread_exit(0);
}


static Bool main_loop(TestParams *params)
{
	Bool ret = FALSE;
	pthread_t *pid = calloc(params->thrNum, sizeof(pthread_t));

	int i;
	for(i = 0; i < params->thrNum; i++) {
		TaskEnv *env = malloc(sizeof(TaskEnv));
		if(!env) {
			ERR("alloc thr env failed.");
			break;
		}

		env->params = params;
		env->id = i;
		sprintf(env->name, "%s%d", params->baseName, i);

		if(i == params->thrNum - 1)
			sprintf(env->dest, "%s0", params->baseName);
		else
			sprintf(env->dest, "%s%d", params->baseName, i+1);
		
		if(pthread_create(&pid[i], NULL, thr_msg, env) < 0) {
			free(env);
			ERRSTR("create thread");
			break;
		}
		usleep(1000);
	}

	
	int j;
	for(j = 0; j < i; j++) {
		pthread_join(pid[j], NULL);
	}

	ret = TRUE;
//exit:
	if(pid)
		free(pid);

	return ret;
	
}
#else
/* test between process */
static Bool main_loop(TestParams *params)
{
	Bool ret = FALSE;

#ifndef PARENT_ONLY
	pid_t	pid;

	if((pid = fork()) < 0) {
		ERRSTR("fork err");
		goto exit;
	} else if(pid == 0) {
		/* child */
		MsgHandle hMsg0 = msg_create(CHILD_MSG, PARENT_MSG, 0);
		if(!hMsg0) {
			ERR("create child msg failed");
			goto exit;
		}
		sleep(1);

		/* we just send msg */
		Int32 cnt0;
		MsgData	msgData0;
		Int32 err0, dataLen0;

		DBG("msg test client start...");
		
		for(cnt0 = 0; cnt0 < params->loopCnt; cnt0++) {
			memset(&msgData0.buf, 0, sizeof(msgData0.buf));
			msgData0.header.cmd = cnt0;
			msgData0.header.index = cnt0;
			msgData0.header.magicNum = MSG_MAGIC_SEND;
			msgData0.header.dataLen = sprintf(msgData0.buf, "child msg [%d]", cnt0);
			dataLen0 = msgData0.header.dataLen + sizeof(msgData0.header);	

			err0 = msg_send(hMsg0, NULL, &msgData0, dataLen0);
			if(err0)
				ERR("<%d> send msg err", cnt0);
			else
				DBG("<%d> send msg ok...", cnt0);

			err0 = msg_recv(hMsg0, &msgData0, sizeof(msgData0));
			if(err0 < 0) {
				ERR("child wait reply err");
			} else 
				DBG("<%d> child recv reply: %s", cnt0, msgData0.buf);

			usleep(100000);
		}
		
		DBG("child exit");
	} 
	else 
#endif
	{
		/* parent */
		MsgHandle hMsg1 = msg_create(PARENT_MSG, CHILD_MSG, 0);
		if(!hMsg1) {
			ERR("create parent msg failed");
			goto exit;
		}
		sleep(1);

		/* we just send msg */
		Int32 cnt1;
		MsgData	msgData1;
		Int32 err1, dataLen1;

		DBG("msg test server start...");
		
		for(cnt1 = 0; cnt1 < params->loopCnt; cnt1++) {
			memset(&msgData1.buf, 0, sizeof(msgData1.buf));

			err1 = msg_recv(hMsg1, &msgData1, sizeof(msgData1));
			if(err1 < 0) {
				ERR("<%d> recv msg err", cnt1);
				continue;
			}
			else
				DBG("<%d> parent recv msg: %s...", cnt1, msgData1.buf);

			usleep(100000);

			/* reply msg */
			msgData1.header.magicNum = MSG_MAGIC_RESP;
			err1 = msg_send(hMsg1, NULL, &msgData1, sizeof(msgData1));
			if(err1 < 0)
				ERR("<%d> send reply msg err", cnt1);
		}

		DBG("parent exit");

	}

	ret = TRUE;

exit:
	

	return ret;
	
}


#endif

static void usage(void)
{
    INFO("msgTest Compiled on %s %s with gcc %s", __DATE__, __TIME__, __VERSION__);
    INFO("Usage:");
    INFO("./msgTest [options]");
    INFO("Options:");
    INFO(" -h get help");
	INFO(" -n loop count, default: %d", DEF_LOOP_CNT);
	INFO(" -b base name for msg create, default: %s", BASE_NAME);
	INFO(" -s msg size, default: %d", DEF_MSG_SIZE);
	INFO(" -c num of threads to create, default: %d", DEF_THR_NUM);
    INFO("Example:");
    INFO(" use default params: ./msgTest");
    INFO(" use specific params: ./msgTest -b /tmp/msg -s 512");
}

int main(int argc, char **argv)
{
	int c;
    char *options = "s:n:c:b:h";
	TestParams params;
	
	params.baseName = BASE_NAME;
	params.loopCnt = DEF_LOOP_CNT;	
	params.msgSize = DEF_MSG_SIZE;
	params.thrNum = DEF_THR_NUM;

	while ((c = getopt(argc, argv, options)) != -1) {
		switch (c) {
		case 'b':
			params.baseName = optarg;
			break;
		case 'c':
			params.thrNum = atoi(optarg);
			//DBG("set frame num: %d", params.frameNum);
			break;
		case 's':
			params.msgSize = atoi(optarg);
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

