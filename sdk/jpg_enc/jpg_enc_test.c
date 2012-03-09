#include "jpg_enc.h"
#include "log.h"
#include "alg.h"
#include "cmem.h"
#include <ti/sdo/ce/image1/imgdec1.h>


#define DEF_SRC_FILE 	"zjz1.yuv"
#define DEF_OUT_FILE 	"zjz1.jpg"
#define SWITCH_SRC_FILE	"colorful_toys_cif_420SP.yuv"
#define SWITCH_OUT_FILE	"colorful_toys_cif_420SP.jpg"

#define DEF_IN_FMT		FMT_YUV_422P
#define DEF_ROTATION	0
#define DEF_LOOP_CNT	10
#define IMG_MAX_WIDTH	4096
#define IMG_MAX_HEIGHT	4096
#define DEF_WIDTH		2816
#define DEF_HEIGHT		2112
#define DEF_QUALITY		95
#define SWITCH_HEIGH	352
#define SWITCH_WIDTH	288
#define SWITCH_FMT		FMT_YUV_420SP

typedef struct _TestParams {
	char *srcFileName;
	char *outFileName;
	int inputFormat;
	int rotation;
	int loopCnt;
	int width;
	int height;
}TestParams;

static Bool main_loop(TestParams *params)
{
	Int32 status = alg_init();
	assert(status == E_NO);
	Bool ret = FALSE;
	
	//create handle
	AlgHandle hJpgEnc;
	JpgEncInitParams initParams;
	JpgEncDynParams dynParams, dynParamsSwitch;

	FILE *fp_in = NULL, *fp_out = NULL;

	initParams.maxWidth = IMG_MAX_WIDTH;
	initParams.maxHeight = IMG_MAX_HEIGHT;
	initParams.size = sizeof(initParams);

	dynParams.width = params->width;
	dynParams.height = params->height;
	dynParams.inputFormat = params->inputFormat;
	dynParams.quality = DEF_QUALITY;
	dynParams.rotation = params->rotation;
	dynParams.size = sizeof(dynParams);

	dynParamsSwitch = dynParams;
	dynParamsSwitch.width = SWITCH_WIDTH;
	dynParamsSwitch.height = SWITCH_HEIGH;
	dynParamsSwitch.inputFormat = SWITCH_FMT;

	DBG("Create fxns.");
	hJpgEnc = alg_create(&JPGENC_ALG_FXNS, &initParams, &dynParams);
	assert(hJpgEnc);

	AlgBuf inBuf, outBuf;
	JpgEncInArgs inArgs;
	JpgEncOutArgs outArgs;

	int size = dynParams.width * dynParams.height * 2;

	CMEM_AllocParams memParams;
    memParams.type = CMEM_POOL;
    memParams.flags = CMEM_NONCACHED;
    memParams.alignment = 256;
	
	inBuf.buf = CMEM_alloc(size, &memParams);
	inBuf.bufSize = size;
	outBuf.buf = CMEM_alloc(size/2, &memParams);
	outBuf.bufSize = size/2;

	DBG("inBuf: 0x%X, outBuf: 0x%X", inBuf.buf, outBuf.buf);

	//unsigned long phyaddr = CMEM_getPhys(inBuf.buf + 377);
	//DBG("phyaddr: %u", phyaddr);

	assert(inBuf.buf && outBuf.buf);

	char *comment = "Hi, there! Jpeg encode test~";
	inArgs.appendData = comment;
	inArgs.appendSize = strlen(comment)+ 1;
	inArgs.endMark = 0xDEADBEEF;
	inArgs.size = sizeof(inArgs);
	outArgs.size = sizeof(outArgs);
	outArgs.bytesGenerated = 0;
	
	DBG("Jpg encode handle create OK.");

	//read file
    fp_in = fopen(params->srcFileName, "rb");
	if(!fp_in) {
		ERRSTR("open %s failed:", params->srcFileName);
		goto exit;
	}

	if(params->inputFormat == FMT_YUV_420P || params->inputFormat == FMT_YUV_420SP)
		size = dynParams.width * dynParams.height * 3 /2;
	else
		size = dynParams.width * dynParams.height * 2;
		
	int len = fread(inBuf.buf, size, 1, fp_in);
	if(len != 1) {
		ERR("Read %d, needed: %d", len, size);
		goto exit;
	}

	//read switch file
	AlgBuf inBuf2, outBuf2;

	if(dynParamsSwitch.inputFormat == FMT_YUV_420P || dynParamsSwitch.inputFormat == FMT_YUV_420SP)
		size = dynParamsSwitch.width * dynParamsSwitch.height * 3 /2;
	else
		size = dynParamsSwitch.width * dynParamsSwitch.height * 2;

	inBuf2.buf = CMEM_alloc(size, &memParams);
	inBuf2.bufSize = size;
	outBuf2.buf = CMEM_alloc(size/2, &memParams);
	outBuf2.bufSize = size/2;
	assert(inBuf2.buf && outBuf2.buf);
	
    FILE *fp_switch = fopen(SWITCH_SRC_FILE, "rb");
	if(!fp_switch) {
		ERRSTR("open %s failed:", SWITCH_SRC_FILE);
		goto exit;
	}

	len = fread(inBuf2.buf, size, 1, fp_switch);
	if(len != 1) {
		ERR("<2>Read %d, needed: %d", len, size);
		goto exit;
	}

	struct timeval tmStart,tmEnd; 
	float   timeUse;
	AlgBuf *pInBuf = &inBuf, *pOutBuf = &outBuf;

	DBG("Start encode...");

	int cnt = 0;
	while(1) {
		gettimeofday(&tmStart,NULL);
		int err = alg_process(hJpgEnc, pInBuf, &inArgs, pOutBuf, &outArgs);
		gettimeofday(&tmEnd,NULL); 

		if(err) {
			ERR("Jpg enc process failed...");
			goto exit;
		}

		timeUse = 1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec;
		DBG("<%d> Jpeg enc cost: %.0f ms", cnt, timeUse/1000);

		cnt++;
		if(params->loopCnt > 0 && cnt > params->loopCnt)
			break;

		/* change dyn params */
		JpgEncDynParams *pDynParams;
		if(cnt % 10) {
			dynParams.rotation += 90;
			dynParams.rotation %= 360;
			dynParams.quality -= 10;
			if(dynParams.quality < 5 || dynParams.quality > 97)
				dynParams.quality = DEF_QUALITY;
			pDynParams = &dynParams;
			pInBuf = &inBuf;
			pOutBuf = &outBuf;
			DBG("encode for rotation: %d, quality: %d", dynParams.rotation, dynParams.quality);
		} else {
			pDynParams = &dynParamsSwitch;
			pInBuf = &inBuf2;
			pOutBuf = &outBuf2;
			DBG("change to another file.");
		}
		//usleep(1000);
		
		err = alg_set_dynamic_params(hJpgEnc, pDynParams);
		if(err) {
			ERR("set dyn params failed!");
		} 

	}

	DBG("encode ok, generate %d bytes.", (int)outArgs.bytesGenerated);

	//write out file
	DBG("writing jpg data to %s...", params->outFileName);
	fp_out= fopen(params->outFileName, "wb");
	if(!fp_out) {
		ERRSTR("open %s failed:", params->outFileName);
		goto exit;
	}
	
	fwrite(outBuf.buf, outArgs.bytesGenerated, 1, fp_out);

	ret = TRUE;
exit:

	if(fp_in)
		fclose(fp_in);

	if(fp_out)
		fclose(fp_out);

	if(fp_switch)
		fclose(fp_switch);

	if(inBuf.buf)
		CMEM_free(inBuf.buf, &memParams);

	if(outBuf.buf)
		CMEM_free(outBuf.buf, &memParams);

	if(inBuf2.buf)
		CMEM_free(inBuf2.buf, &memParams);

	if(outBuf2.buf)
		CMEM_free(outBuf2.buf, &memParams);

	if(hJpgEnc)
		alg_delete(hJpgEnc);

	alg_exit();

	return ret;

	
}

static void usage(void)
{
    INFO("jpgEncTest Compiled on %s %s with gcc %s", __DATE__, __TIME__, __VERSION__);
    INFO("Usage:");
    INFO("./jpgEncTest [options]");
    INFO("Options:");
    INFO(" -h get help");
	INFO(" -f input file name, default: %s", DEF_SRC_FILE);
	INFO(" -o output file name, default: %s", DEF_OUT_FILE);
	INFO(" -c input chroma format, default: %d", DEF_IN_FMT);
	INFO(" -r rotation, default: %d", DEF_ROTATION);
	INFO(" -n loop count, default: %d", DEF_LOOP_CNT);
	INFO(" -w input image width, default: %d", DEF_WIDTH);
	INFO(" -l input image height, default: %d", DEF_HEIGHT);
    INFO("Example:");
    INFO(" use default params: ./jpgEncTest");
    INFO(" use specific params: ./jpgEncTest -f ./test/toy.yuv -r 90");
}

int main(int argc, char **argv)
{
	int c;
    char *options = "f:c:o:r:w:l:n:h";
	TestParams params;
	
	params.srcFileName = DEF_SRC_FILE;
	params.inputFormat = DEF_IN_FMT;
	params.rotation = DEF_ROTATION;
	params.loopCnt = DEF_LOOP_CNT;
	params.outFileName = DEF_OUT_FILE;
	params.width = DEF_WIDTH;
	params.height = DEF_HEIGHT;

	while ((c = getopt(argc, argv, options)) != -1) {
		switch (c) {
		case 'f':
			params.srcFileName = optarg;
			break;
		case 'o':
			params.outFileName = optarg;
			break;
		case 'c':
			params.inputFormat = atoi(optarg);
			if(params.inputFormat < FMT_YUV_420P || 
				params.inputFormat > FMT_ARGB8888) {
				ERR("invalid input format, use default");
				params.inputFormat = DEF_IN_FMT;
			}	
			break;
		case 'r':
			params.rotation = atoi(optarg);
			break;
		case 'w':
			params.width = atoi(optarg);
			break;
		case 'l':
			params.height = atoi(optarg);
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

