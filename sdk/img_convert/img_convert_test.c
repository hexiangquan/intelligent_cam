#include "img_convert.h"
#include "log.h"
#include "capture.h"
#include "buffer.h"
#include "log.h"
#include <pthread.h>
#include "jpg_enc.h"


#define CAPTURE_DEVICE		"/dev/video0"
#define V4L2VID0_DEVICE		"/dev/video2"
#define V4L2VID1_DEVICE		"/dev/video3"

#define DEF_LOOP_CNT	10
#define DEF_OUT_FILE	"capOut.jpg"
#define IMG_MAX_WIDTH	2048
#define IMG_MAX_HEIGHT	2048
#define DEF_GAIN		512
#define SWITCH_WIDTH	1280
#define SWITCH_HEIGH	720
#define DEF_BRIGHTNESS	0
#define DEF_CONTRAST	16


typedef struct _TestParams {
	int loopCnt;
	const char *outFileName;
	Uint16	outWidth;
	Uint16	outHeight;
	Uint16	gain;
	Uint32  flags;
	Uint8	brightness;
	Uint8	contrast;
}TestParams;

static Bool main_loop(TestParams *params)
{
	Bool ret = FALSE;
	FILE *fp_out = NULL;

	int err = buffer_init();
	err = alg_init();
	if(err)
		goto exit;

	CapAttrs attrs;

	attrs.devName = CAPTURE_DEVICE;
	attrs.bufNum = 3;
	attrs.inputType = CAP_INPUT_CAMERA;
	attrs.std = CAP_STD_FULL_FRAME;
	attrs.userAlloc = TRUE;
	
	CapHandle hCapture = capture_create(&attrs);
	CapInputInfo info;
	assert(hCapture);
	if(!hCapture)
		goto exit;
	
	capture_get_input_info(hCapture, &info);
	DBG("Capture Input: %d X %d, fmt: %d", (int)info.width, (int)info.height, (int)info.colorSpace);
	
	FrameBuf frameBuf;
	
	Uint32 size = info.width * info.height * 2;
	

	AlgHandle hImgConv;
	ImgConvInitParams convInitParams;
	ImgConvDynParams convDynParams;
	AlgBuf inBuf, outBuf;

	convInitParams.prevDevName = NULL;
	convInitParams.rszDevName = NULL;
	convInitParams.size = sizeof(convInitParams);

	convDynParams.size = sizeof(convDynParams);
	convDynParams.inputFmt = FMT_BAYER_RGBG;
	convDynParams.inputWidth = info.width;
	convDynParams.inputHeight = info.height;
	convDynParams.ctrlFlags = params->flags;
	convDynParams.digiGain = params->gain;
	convDynParams.brigtness = params->brightness;
	convDynParams.contrast = params->contrast;
	convDynParams.eeTable = NULL;
	convDynParams.eeTabSize = 0;
	
	convDynParams.outAttrs[0].enbale = TRUE;
	if(params->outWidth == 0 || params->outHeight == 0) {
		convDynParams.outAttrs[0].width = info.width;
		convDynParams.outAttrs[0].height = info.height;
	} else {
		convDynParams.outAttrs[0].width = ROUND_UP(params->outWidth, 16);
		convDynParams.outAttrs[0].height = params->outHeight;
	}
	convDynParams.outAttrs[0].pixFmt = FMT_YUV_420SP;
	convDynParams.outAttrs[1].enbale = FALSE;

	hImgConv = alg_create(&IMGCONV_ALG_FXNS, &convInitParams, &convDynParams);
	if(!hImgConv) {
		ERR("create img conv alg failed");
		goto exit;

	}
	
	size = IMG_MAX_WIDTH * IMG_MAX_HEIGHT * 2;
	BufHandle hBufOut = buffer_alloc(size, NULL);
	assert(hBufOut);
	assert(((Uint32)buffer_get_user_addr(hBufOut) % 32) == 0);
	
	err = capture_start(hCapture);
	if(err)
		goto exit;

	Int32 i = 0;
	Bool resSwi = TRUE;
	Uint16 outWidth = convDynParams.outAttrs[0].width, outHeight = convDynParams.outAttrs[0].height;
	outBuf.buf = buffer_get_user_addr(hBufOut);
	outBuf.bufSize = buffer_get_size(hBufOut);

	struct timeval tmStart,tmEnd; 
	float   timeUse;

	while(1) {
		err = capture_get_frame(hCapture, &frameBuf);
		if(err) {
			ERR("wait capture buffer failed...");
			break;
		}

		DBG("<%d> Get frame, index: %d, time: %u.%u", i, frameBuf.index, 
			(unsigned int)frameBuf.timeStamp.tv_sec, (unsigned int)frameBuf.timeStamp.tv_usec);

		inBuf.buf = frameBuf.dataBuf;
		inBuf.bufSize = frameBuf.bufSize;
		gettimeofday(&tmStart,NULL);
		err = alg_process(hImgConv, &inBuf, NULL, &outBuf, NULL);
		gettimeofday(&tmEnd,NULL); 

		timeUse = 1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec;
		DBG("  img convert cost: %.3f ms", timeUse/1000);
		
		err = capture_free_frame(hCapture, &frameBuf);
		if(err < 0) {
			ERR("free frame failed");
			break;
		}

		i++;
		if(params->loopCnt > 0 && i > params->loopCnt)
			break;

		if((i % 10) == 0) {
			if(resSwi) {
				convDynParams.outAttrs[0].width = SWITCH_WIDTH;
				convDynParams.outAttrs[0].height = SWITCH_HEIGH;
				resSwi = FALSE;
			} else {
				convDynParams.outAttrs[0].width = outWidth;
				convDynParams.outAttrs[0].height = outHeight;
				resSwi = TRUE;
			}

			if(convDynParams.ctrlFlags & CONV_FLAG_NF_EN)
				convDynParams.ctrlFlags &= ~CONV_FLAG_NF_EN;
			else
				convDynParams.ctrlFlags |= CONV_FLAG_NF_EN;

			gettimeofday(&tmStart,NULL);
			err = alg_set_dynamic_params(hImgConv, &convDynParams);
			gettimeofday(&tmEnd,NULL); 
			if(err) {
				ERR("set dyn params failed....");
				goto exit;
			} else {
				DBG("res switched to %u X %u", 
					convDynParams.outAttrs[0].width, convDynParams.outAttrs[0].height);
			}

			timeUse = 1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec;
			DBG("  set dyn params cost: %.3f ms", timeUse/1000);
			
		}

		if((i % 30) == 0) {
			err = capture_stop(hCapture);
			if(err)
				goto exit;

			err = capture_config(hCapture, CAP_STD_FULL_FRAME, CAP_MODE_CONT);
			if(err)
				goto exit;

			err =  capture_start(hCapture);
			if(err)
				goto exit;

			DBG("\nCapture restarted....");
		}
	}

	err = capture_stop(hCapture);
	if(err)
		goto exit;

	AlgHandle hJpgEnc;
	JpgEncInitParams initParams;
	JpgEncDynParams dynParams;
	
	initParams.maxWidth = IMG_MAX_WIDTH;
	initParams.maxHeight = IMG_MAX_HEIGHT;
	initParams.size = sizeof(initParams);

	dynParams.width = convDynParams.outAttrs[0].width;
	dynParams.height = convDynParams.outAttrs[0].height;
	dynParams.inputFormat = convDynParams.outAttrs[0].pixFmt;
	dynParams.quality = 90;
	dynParams.rotation = 0;
	dynParams.size = sizeof(dynParams);

	DBG("Create fxns.");
	hJpgEnc = alg_create(&JPGENC_ALG_FXNS, &initParams, &dynParams);
	assert(hJpgEnc);

	JpgEncInArgs inArgs;
	JpgEncOutArgs outArgs;

	BufHandle hJpgOutBuf = buffer_alloc(size/2, NULL);
	if(!hJpgOutBuf)
		goto exit;

	inBuf.buf = buffer_get_user_addr(hBufOut);
	inBuf.bufSize = buffer_get_size(hBufOut);
	outBuf.buf = buffer_get_user_addr(hJpgOutBuf);
	outBuf.bufSize = buffer_get_size(hJpgOutBuf);
	inArgs.appendData = NULL;
	inArgs.appendSize = 0;
	inArgs.endMark = 0;
	inArgs.size = sizeof(inArgs);
	outArgs.size = sizeof(outArgs);
	outArgs.bytesGenerated = 0;

	//gettimeofday(&tmStart,NULL);
	DBG("begin encode");
	err = alg_process(hJpgEnc, &inBuf, &inArgs, &outBuf, &outArgs);
	//gettimeofday(&tmEnd,NULL); 

	if(err) {
		ERR("Jpg enc process failed...");
		goto exit;
	}

	//write out file
	fp_out= fopen(params->outFileName, "wb");
	if(!fp_out) {
		ERRSTR("open %s failed:", params->outFileName);
		goto exit;
	}


	DBG("writing capture out data(%u bytes) to %s...", 
		outArgs.bytesGenerated, params->outFileName);
	fwrite(buffer_get_user_addr(hJpgOutBuf), outArgs.bytesGenerated, 1, fp_out);
	
	ret = TRUE;
exit:

	if(hCapture)
		capture_delete(hCapture);

	if(fp_out)
		fclose(fp_out);

	if(hBufOut)
		buffer_free(hBufOut);

	if(hJpgOutBuf)
		buffer_free(hJpgOutBuf);

	if(hJpgEnc)
		alg_delete(hJpgEnc);

	buffer_exit();

	alg_exit();
	
	return ret;
	
}

static void usage(void)
{
    INFO("captureTest Compiled on %s %s with gcc %s", __DATE__, __TIME__, __VERSION__);
    INFO("Usage:");
    INFO("./captureTest [options]");
    INFO("Options:");
    INFO(" -h get help");
	INFO(" -n loop count, default: %d", DEF_LOOP_CNT);
	INFO(" -o out file name, default: %s", DEF_OUT_FILE);
	INFO(" -w out width, default 0, using input width, width should be multiple of 16");
	INFO(" -l out height, default 0, using input height");
	INFO(" -g digital gains, default %d", DEF_GAIN);
	INFO(" -b brightness for luma adjust, default %d", DEF_BRIGHTNESS);
	INFO(" -c contrast for luma adjust, default %d", DEF_CONTRAST);
	INFO(" -y enable luma adjust");
	INFO(" -N enable 2D noise filter");
	INFO(" -e enable edge enhance");
	INFO(" -a enable average filter.");
	INFO(" -m enable gamma");
    INFO("Example:");
    INFO(" use default params: ./captureTest");
    INFO(" use specific params: ./captureTest -w 1920 -l 1080 -n 10");
}

int main(int argc, char **argv)
{
	int c;
    char *options = "n:o:w:l:g:b:c:hyNeam";
	TestParams params;
	
	params.loopCnt = DEF_LOOP_CNT;
	params.outFileName = DEF_OUT_FILE;
	params.outWidth = params.outHeight = 0;
	params.gain = DEF_GAIN;
	params.flags = 0;
	params.brightness = DEF_BRIGHTNESS;
	params.contrast = DEF_CONTRAST;

	while ((c = getopt(argc, argv, options)) != -1) {
		switch (c) {
		case 'n':
			params.loopCnt = atoi(optarg);
			break;
		case 'w':
			params.outWidth = atoi(optarg);
			break;
		case 'l':
			params.outHeight = atoi(optarg);
			break;
		case 'g':
			params.gain = atoi(optarg);
			break;
		case 'b':
			params.brightness = atoi(optarg);
			break;
		case 'c':
			params.contrast = atoi(optarg);
			break;
		case 'o':
			params.outFileName = optarg;
			break;
		case 'N':
			params.flags |= CONV_FLAG_NF_EN;
			break;
		case 'm':
			params.flags |= CONV_FLAG_GAMMA_EN;
			break;
		case 'e':
			params.flags |= CONV_FLAG_EE_EN;
			break;
		case 'a':
			params.flags |= CONV_FLAG_AVG_EN;
			break;
		case 'y':
			params.flags |= CONV_FLAG_LUMA_EN;
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

