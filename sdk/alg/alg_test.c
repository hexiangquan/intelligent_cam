#include "common.h"
//#include "ti/xdais/alg.h"
#include "log.h"
#include "alg.h"
#include <ti/sdo/ce/image1/imgenc1.h>
#include "codecs.h"

static void usage(void)
{
    printf("algTest Compiled on %s %s with gcc %s\n", __DATE__, __TIME__, __VERSION__);
    printf("Usage:\n");
    printf("./algTest [options]\n");
    printf("Options:\n");
    printf(" -h get help\n");
    printf("Example:\n");
    printf(" use default params: ./algTest\n");
}

Ptr test_init(Ptr initParams, Ptr dynParams) 
{
	DBG("alg test init");
	//char *h = malloc(512);
	//if(h)
		//memcpy(h, initParams, 128);

	IMGENC1_Handle  hEncode;
	IMGENC1_Params 	encParams = {
	    sizeof(IMGENC1_Params),
	    1920,
	    1080,
	    XDM_DEFAULT,
	    XDM_BYTE,
	    XDM_YUV_422P   
	};

	/* Create imaging encoder */
    hEncode = IMGENC1_create(g_hEngine, JPGENC_NAME, &encParams);

	if(hEncode != NULL)
		DBG("create jpg enc handle ok.");
	else
		DBG("create jpg enc handle failed");

	
	
	return (Ptr)hEncode;
}

Int32 test_free(Ptr handle)
{
	DBG("alg freed");
	//free(handle);

	/* delete */
	IMGENC1_delete((IMGENC1_Handle)handle);
	return E_NO;
}

Int32 test_process(Ptr handle, AlgBuf* inBuf, Ptr inArg, AlgBuf* outBuf, Ptr outArg)
{
	DBG("alg process");
	return E_NO;
}

Int32 test_ctl(Ptr handle, Int32 cmd, Ptr args)
{
	Int32 err = E_NO;
	
	switch(cmd) {
	case ALG_CMD_SET_DYN_PARAMS:
		memcpy(handle, args, 4);
		break;
	default:
		err = E_UNSUPT;
		break;
	}

	return err;
}

int main(int argc, char **argv)
{
	int c;
    char *options = "f:l:m";

	while ((c=getopt(argc, argv, options)) != -1)
	{
		switch (c)
		{
			case 'h':
			default:
				usage();
				return -1;
		}
	}

	int err = alg_init();
	assert(err == E_NO);

	//ALG_Handle hAlg;

	//hAlg = ALG_create(0, NULL, NULL, NULL, ALG_USECACHEDMEM_CACHED);
	//ALG_delete(0, hAlg, ALG_USECACHEDMEM_CACHED);

	AlgFxns fxns = {
		.algInit = test_init,
		.algProcess = test_process,
		.algControl = test_ctl,
		.algExit = test_free
	};

	char buf[128];
	AlgHandle alg = alg_create(&fxns, buf, NULL);
	assert(alg);

	int i;
	
	for(i = 0; i < 10; i++) {
		err = alg_process(alg, (AlgBuf *)buf, NULL, NULL, NULL);
		usleep(1000);
	}

	memset(buf, 0x55, sizeof(buf));
	alg_control(alg, ALG_CMD_SET_DYN_PARAMS, buf);
	assert(*(Uint8 *)alg->handle== 0x55);

	err = alg_delete(alg);
	assert(err == 0);

	alg_exit();

	DBG("Test complete");

	exit(0);
}

