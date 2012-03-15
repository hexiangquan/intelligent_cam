#include "icam_ctrl.h"
#include "log.h"
#include <linux/types.h>

#define DEF_LOOP_CNT 10
#define DEF_PATH_NAME "/tmp/icamTestMsg"

typedef struct {
	const char *pathName;
	Int32 loopCnt;
}TestParams;

static void main_loop(TestParams *params)
{
	ICamCtrlHandle hCtrl = NULL;

	hCtrl = icam_ctrl_create(params->pathName, 0);
	assert(hCtrl);
	
	Int32 err;
	CamVersionInfo version;

	err = icam_get_version(hCtrl, &version);
	if(err)
		goto exit;

	DBG("cam version:");
	DBG(" arm: 0x%X", (__u32)version.armVer);
	DBG(" dsp: 0x%X", (__u32)version.dspVer);
	DBG(" fpga: 0x%X", (__u32)version.fpgaVer);
	
exit:

	if(hCtrl)
		icam_ctrl_delete(hCtrl);
	
	DBG("ftp test complete.");
	
}


static void usage(void)
{
    printf("testICamCtrl Compiled on %s %s with gcc %s\n", __DATE__, __TIME__, __VERSION__);
    printf("Usage:\n");
    printf("./testICamCtrl [options]\n");
    printf("Options:\n");
    printf(" -h get help\n");
	printf(" -n loop count, default: %d\n", DEF_LOOP_CNT);
	printf(" -p path name for IPC, default: %s\n", DEF_PATH_NAME);
    printf("Example:\n");
    printf(" use default params: ./testICamCtrl\n");
    printf(" use specified params: ./testICamCtrl -n 100\n");
}

int main(int argc, char **argv)
{
	int c;
    char *options = "n:h";
	TestParams params;

	params.loopCnt = DEF_LOOP_CNT;
	params.pathName = DEF_PATH_NAME;

	while ((c=getopt(argc, argv, options)) != -1) {
		switch (c) {
		case 'n':
			params.loopCnt = atoi(optarg);
			break;
		case 'p':
			params.pathName = optarg;
			break;
		case 'h':
		default:
			usage();
			return -1;
		}
	}

	main_loop(&params);

	exit(0);
}

