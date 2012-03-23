#include "icam_ctrl.h"
#include "log.h"
#include <linux/types.h>
#include "msg.h"

#define DEF_LOOP_CNT 	10
#define DEF_PATH_NAME 	"/tmp/msgCtrlTest"
#define PARENT_MSG		"/tmp/msgParent"
//#define CHILD_MSG		"/tmp/msgChild"
#define CHILD_MSG		DEF_PATH_NAME

typedef struct {
	const char *pathName;
	Int32 loopCnt;
}TestParams;

typedef struct _MsgData {
	MsgHeader	header;
	char		buf[512];
}MsgData;


extern Int32 icamCtrl_cmd_test(ICamCtrlHandle hCtrl,const char * str);

static void main_loop(TestParams *params)
{
	ICamCtrlHandle hCtrl = NULL;

	hCtrl = icam_ctrl_create(params->pathName, 0, 5);
	DBG("create msg: %s", params->pathName);
	assert(hCtrl);
	
	Int32 err, cnt = 0;
	CamVersionInfo version;

	while(1) {

#if 1
		err = icam_get_version(hCtrl, &version);
		//if(err)
			//break;
		if(!err) {
			DBG("cam version:");
			DBG(" arm: 0x%X", (__u32)version.armVer);
			DBG(" dsp: 0x%X", (__u32)version.dspVer);
			DBG(" fpga: 0x%X", (__u32)version.fpgaVer);
		} else {
			DBG("get ver err");
		}
#endif

		//err = icamCtrl_cmd_test(hCtrl, "test");

		if(cnt++ > params->loopCnt) {
			break;
		}
	}


	if(hCtrl)
		icam_ctrl_delete(hCtrl);

exit:
	DBG("icam ctrl test complete.");
	
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
    char *options = "n:p:h";
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

