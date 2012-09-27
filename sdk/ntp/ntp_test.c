#include "common.h"
#include "ntp.h"
#include "log.h"

#define DEF_NTP_SRV		"192.168.0.11"
#define DEF_NTP_PORT	0
#define DEF_LOOP_CNT	5

typedef struct _TestParams {
	int loopCnt;
	const char *serverIP;
	Uint16 port;
}TestParams;


static Bool main_loop(TestParams *params)
{
	NtpHandle hNtp = ntp_create(params->serverIP, params->port);
	assert(hNtp);

	int cnt = 0;
	Bool ret = FALSE;
	int err;

	while(1) {
		DateTime time;
		err = ntp_get_time(hNtp, &time);
		if(err < 0)
			goto exit;

		DBG("ntp get time: %04u.%02u.%02u %02u:%02u:%02u",
			time.year, time.month, time.day, time.hour, time.minute, time.second);

		sleep(1);
		cnt++;
		if(params->loopCnt > 0 && cnt > params->loopCnt)
			break;
	}

	err = ntp_set_server_info(hNtp, params->serverIP, params->port);
	assert(err == E_NO);
	
	ret = TRUE;
	
exit:
	err = ntp_delete(hNtp);
	assert(err == E_NO);

	return ret;
	
}

static void usage(void)
{
    INFO("ntpTest Compiled on %s %s with gcc %s", __DATE__, __TIME__, __VERSION__);
    INFO("Usage:");
    INFO("./ntpTest [options]");
    INFO("Options:");
    INFO(" -h get help");
	INFO(" -n loop count, default: %d", DEF_LOOP_CNT);
	INFO(" -s server ip, default: %s", DEF_NTP_SRV);
	INFO(" -p server port, default: %u", DEF_NTP_PORT);
    INFO("Example:");
    INFO(" use default params: ./ntpTest");
    INFO(" use specified params: ./ntpTest -s 192.168.0.15");
}

int main(int argc, char **argv)
{
	int c;
    char *options = "s:p:n:h";
	TestParams params;

	params.serverIP = DEF_NTP_SRV;
	params.port = DEF_NTP_PORT;
	params.loopCnt = DEF_LOOP_CNT;
	
	while ((c = getopt(argc, argv, options)) != -1) {
		switch (c) {
		case 's':
			params.serverIP = optarg;
			break;
		case 'p':
			params.port = atoi(optarg);
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

	Bool result = main_loop(&params);
	if(result == TRUE)
		INFO("test success...");
	else
		INFO("test failed...");

	exit(0);
}


