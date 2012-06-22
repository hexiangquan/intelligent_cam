#include "h264_enc.h"
#include "log.h"
#include "alg.h"
#include "cmem.h"

//#define DYN_SET_TEST

#define DEF_SRC_FILE 	"colorful_toys_cif_5frms_420SP.yuv"
#define DEF_OUT_FILE 	"colorful_toys_cif_5frms_420SP.h264"
#define SWITCH_SRC_FILE	"1080p.yuv"
#define SWITCH_OUT_FILE	"1080p.h264"

#define DEF_IN_FMT		FMT_YUV_420SP
#define DEF_RC_MODE		H264_RC_VBR
#define DEF_LOOP_CNT	10
#define IMG_MAX_WIDTH	2048
#define IMG_MAX_HEIGHT	2048
#define DEF_WIDTH		352
#define DEF_HEIGHT		288
#define DEF_BIT_RATE	2000000
#define DEF_FRAME_NUM	5
#define DEF_FRAME_RATE	30
#define SWITCH_HEIGH	1080
#define SWITCH_WIDTH	1920
#define SWITCH_RC_MODE	H264_RC_CBR
#define SWITCH_FRAME_NUM	1

#define TIMECLOCK_FRQ	90 //KHz

typedef struct _TestParams {
	char *srcFileName;
	char *outFileName;
	int inputFormat;
	int frameNum;
	int bitRate;
	int frameRate;
	int rateCtrlMode;
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
	AlgHandle hH264Enc;
	H264EncInitParams initParams;
	H264EncDynParams dynParams;

	FILE *fp_in = NULL, *fp_out = NULL;

	initParams.maxWidth = IMG_MAX_WIDTH;
	initParams.maxHeight = IMG_MAX_HEIGHT;
	initParams.inputChromaFormat = params->inputFormat;
	initParams.maxBitRate = 5000000;
	initParams.maxFrameRate = 60;
	initParams.sliceFormat = 1;
	initParams.size = sizeof(initParams);

	dynParams = H264ENC_DYN_DEFAULT;

	dynParams.width = params->width;
	dynParams.height = params->height;
	dynParams.frameRate = params->frameRate;
	dynParams.targetBitRate = params->bitRate;
	dynParams.intraFrameInterval = 30;
	dynParams.forceFrame = VID_NA_FRAME;
	dynParams.rateCtrlMode = params->rateCtrlMode;
	dynParams.size = sizeof(dynParams);

#ifdef DYN_SET_TEST
	H264EncDynParams dynParamsSwitch;
	dynParamsSwitch = dynParams;
	dynParamsSwitch.width = SWITCH_WIDTH;
	dynParamsSwitch.height = SWITCH_HEIGH;
	dynParamsSwitch.rateCtrlMode = SWITCH_RC_MODE;
#endif

	DBG("Create fxns.");
	hH264Enc = alg_create(&H264ENC_ALG_FXNS, &initParams, &dynParams);
	assert(hH264Enc);

	AlgBuf inBuf, outBuf;
	H264EncInArgs inArgs;
	H264EncOutArgs outArgs;

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

	DBG("H264 encode handle create OK.");

	assert(inBuf.buf && outBuf.buf);
	int timeStampStep = TIMECLOCK_FRQ * 1000 / params->frameRate;
	inArgs.inputID = 1;
	inArgs.timeStamp = 0;
	inArgs.size = sizeof(inArgs);
	
	outArgs.size = sizeof(outArgs);
	outArgs.bytesGenerated = 0;
	outArgs.frameType = VID_NA_FRAME;
	outArgs.outputID = 0;
	
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

	DBG("input format: %d, yuv size: %d", params->inputFormat, size);
		
	int len;

#ifdef DYN_SET_TEST
	//read switch file
	AlgBuf inBuf2, outBuf2;

	if(params->inputFormat == FMT_YUV_420P ||params->inputFormat == FMT_YUV_420SP)
		size = dynParamsSwitch.width * dynParamsSwitch.height * 3 /2;
	else
		size = dynParamsSwitch.width * dynParamsSwitch.height * 2;

	inBuf2.buf = CMEM_alloc(size, &memParams);
	inBuf2.bufSize = size;
	outBuf2.buf = CMEM_alloc(size/2, &memParams);
	outBuf2.bufSize = size/2;
	assert(inBuf2.buf && outBuf2.buf);
	
    FILE *fp_in2 = fopen(SWITCH_SRC_FILE, "rb");
	if(!fp_in2) {
		ERRSTR("open %s failed:", SWITCH_SRC_FILE);
		goto exit;
	}

	len = fread(inBuf2.buf, size, 1, fp_in2);
	if(len != 1) {
		ERR("<2>Read %d, needed: %d", len, size);
		goto exit;
	}

	FILE *fp_out2 = fopen(SWITCH_OUT_FILE, "wb");
	if(!fp_out2) {
		ERRSTR("open %s failed:", SWITCH_OUT_FILE);
		goto exit;
	}

	H264EncInArgs inArgs2;
	inArgs2.inputID = 1;
	inArgs2.timeStamp = 0;
	inArgs2.size = sizeof(inArgs2);
	
#endif

	//write out file
	DBG("writing h264 data to %s...", params->outFileName);
	fp_out= fopen(params->outFileName, "wb");
	if(!fp_out) {
		ERRSTR("open %s failed:", params->outFileName);
		goto exit;
	}

	struct timeval tmStart,tmEnd; 
	float   timeUse;
	AlgBuf *pInBuf = &inBuf, *pOutBuf = &outBuf;
	FILE *fp_rd = fp_in, *fp_wr = fp_out;
	H264EncInArgs *pInArgs = &inArgs;
	size = params->width * params->height * 3 /2;
	//size = dynParamsSwitch.width * dynParamsSwitch.height * 3 /2;
	
	DBG("Start encode...");

	int cnt = 0;
	int frameNum = params->frameNum;
	DBG("frame num in file: %d", frameNum);
	while(1) {

		len = fread(pInBuf->buf, size, 1, fp_rd);
		if(len != 1) {
			ERR("Read %d, needed: %d", len, size);
			goto exit;
		}
		
		gettimeofday(&tmStart,NULL);
		int err = alg_process(hH264Enc, pInBuf, pInArgs, pOutBuf, &outArgs);
		gettimeofday(&tmEnd,NULL); 

		if(err) {
			ERR("H264 enc process failed...");
			goto exit;
		}

		timeUse = 1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec;
		DBG("<%d> H264 enc cost: %.2f ms, bytes generated: %d, type: %d", 
			cnt, timeUse/1000, outArgs.bytesGenerated, outArgs.frameType);

		cnt++;
		if(params->loopCnt > 0 && cnt >= params->loopCnt)
			break;

		if((cnt % frameNum) == 0) {
			//At the end of file
			DBG("restart from the beginning of file");
			fseek(fp_rd, 0, SEEK_SET);
		}

		// Increase ID and timestamp
		pInArgs->inputID++;
		pInArgs->timeStamp += timeStampStep;
		err = 0;
#if 1
		/* change dyn params */
		H264EncDynParams *pDynParams;
		if((cnt % 20) == 0) {
			dynParams.rateCtrlMode += 1;
			dynParams.rateCtrlMode %= (H264_RC_CUSTOM_VBR + 1);
			dynParams.targetBitRate += 100;
			if(dynParams.targetBitRate < 5 || dynParams.targetBitRate > 5000000)
				dynParams.targetBitRate = 100;
			pDynParams = &dynParams;
			pInBuf = &inBuf;
			pOutBuf = &outBuf;
			pInArgs = &inArgs;
			fp_rd = fp_in;
			fp_wr = fp_out;
			frameNum = params->frameNum;
			DBG("encode for rc mode: %d, bit rate: %d", 
				dynParams.rateCtrlMode, dynParams.targetBitRate);
			err = alg_set_dynamic_params(hH264Enc, pDynParams);
			fseek(fp_rd, 0, SEEK_SET);
			size = params->width * params->height * 3 /2;
		} else if((cnt % 10) == 0) {
	#ifdef DYN_SET_TEST
			pDynParams = &dynParamsSwitch;
			pInBuf = &inBuf2;
			pOutBuf = &outBuf2;
			pInArgs = &inArgs2;
			fp_rd = fp_in2;
			fp_wr = fp_out2;
			frameNum = SWITCH_FRAME_NUM;
			DBG("change to another file.");
			err = alg_set_dynamic_params(hH264Enc, pDynParams);
			fseek(fp_rd, 0, SEEK_SET);
			size = dynParamsSwitch.width * dynParamsSwitch.height * 3 /2;
	#endif
		}
		//usleep(1000);

		if(err) {
			ERR("set dyn params failed!");
		} 
#endif
		// write encoded data
		//fwrite(pOutBuf->buf,  outArgs.bytesGenerated, 1, fp_wr);

		usleep(33000);

	}

	//DBG("encode ok, generate %d bytes.", (int)outArgs.bytesGenerated);

	ret = TRUE;
exit:

	if(fp_in)
		fclose(fp_in);

	if(fp_out)
		fclose(fp_out);

	if(inBuf.buf)
		CMEM_free(inBuf.buf, &memParams);

	if(outBuf.buf)
		CMEM_free(outBuf.buf, &memParams);
	
#ifdef DYN_SET_TEST
	if(fp_in2)
		fclose(fp_in2);

	if(fp_out2)
		fclose(fp_out2);

	if(inBuf2.buf)
		CMEM_free(inBuf2.buf, &memParams);

	if(outBuf2.buf)
		CMEM_free(outBuf2.buf, &memParams);
#endif

	if(hH264Enc)
		alg_delete(hH264Enc);

	alg_exit();

	return ret;

	
}

static void usage(void)
{
    INFO("h264EncTest Compiled on %s %s with gcc %s", __DATE__, __TIME__, __VERSION__);
    INFO("Usage:");
    INFO("./h264EncTest [options]");
    INFO("Options:");
    INFO(" -h get help");
	INFO(" -i input file name, default: %s", DEF_SRC_FILE);
	INFO(" -o output file name, default: %s", DEF_OUT_FILE);
	INFO(" -c input chroma format, default: %d, only support YUV420SP", DEF_IN_FMT);
	INFO(" -r rate control mode, 0-CBR, 1-VBR, 2-Fixed QP, 3-CVBR, default: %d", DEF_RC_MODE);
	INFO(" -n loop count, default: %d", DEF_LOOP_CNT);
	INFO(" -w input image width, default: %d", DEF_WIDTH);
	INFO(" -l input image height, default: %d", DEF_HEIGHT);
	INFO(" -b bit rate, default: %d", DEF_BIT_RATE);
	INFO(" -f frame num in input file, default: %d", DEF_FRAME_NUM);
    INFO("Example:");
    INFO(" use default params: ./h264EncTest");
    INFO(" use specific params: ./h264EncTest -i ./test/toy.yuv -r 2 -f 5");
}

int main(int argc, char **argv)
{
	int c;
    char *options = "i:c:o:r:w:l:n:f:b:h";
	TestParams params;
	
	params.srcFileName = DEF_SRC_FILE;
	params.outFileName = DEF_OUT_FILE;
	params.inputFormat = DEF_IN_FMT;
	params.rateCtrlMode = DEF_RC_MODE;
	params.frameRate = DEF_FRAME_RATE;
	params.frameNum = DEF_FRAME_NUM;
	params.bitRate = DEF_BIT_RATE;
	params.loopCnt = DEF_LOOP_CNT;	
	params.width = DEF_WIDTH;
	params.height = DEF_HEIGHT;

	while ((c = getopt(argc, argv, options)) != -1) {
		switch (c) {
		case 'i':
			params.srcFileName = optarg;
			break;
		case 'o':
			params.outFileName = optarg;
			break;
		case 'c':
			params.inputFormat = atoi(optarg);
			if(params.inputFormat != FMT_YUV_420SP) {
				ERR("invalid input format, use default");
				params.inputFormat = DEF_IN_FMT;
			}	
			break;
		case 'r':
			params.rateCtrlMode = atoi(optarg);
			break;
		case 'b':
			params.bitRate = atoi(optarg);
			break;
		case 'f':
			params.frameNum = atoi(optarg);
			//DBG("set frame num: %d", params.frameNum);
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

