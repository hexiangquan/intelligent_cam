#include "capture.h"
#include "img_convert.h"
#include "display.h"
#include "buffer.h"
#include "log.h"
#include <pthread.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/videodev2.h>

#define DEF_LOOP_CNT	10
#define PROG_NAME		"displayTest"
#define DEF_DISP_MODE	DISPLAY_MODE_PAL
#define DEF_OUT_FILE	"displayOut.yuv"


typedef struct _TestParams {
	int loopCnt;
	int chanId;
	int mode;
	const char *fname;
	Bool autoSwitch;
}TestParams;

static CapHandle capture_init(CapInputInfo *info)
{
	CapAttrs attrs;

	attrs.devName = "/dev/video0";
	attrs.bufNum = 3;
	attrs.inputType = CAP_INPUT_CAMERA;
	attrs.std = CAP_STD_FULL_FRAME;
	attrs.userAlloc = TRUE;
	attrs.defRefCnt = 1;
	attrs.mode = CAP_MODE_CONT;
	
	CapHandle hCapture = capture_create(&attrs);
	assert(hCapture);

	Int32 err = capture_get_input_info(hCapture, info);
	DBG("Capture input: %u X %u, fmt: %d, bytes per line: %d", 
		info->width, info->height, info->colorSpace, info->bytesPerLine);	

	assert(err == E_NO);
	
	err = capture_start(hCapture);
	assert(err == E_NO);

	return hCapture;
}

static AlgHandle img_conv_init(const CapInputInfo *info,  DisplayMode mode, ImgConvInArgs *convInArgs)
{	
	ImgConvInitParams convInitParams;
	ImgConvDynParams convDynParams;

	convInitParams.prevDevName = NULL;
	convInitParams.rszDevName = NULL;
	convInitParams.size = sizeof(convInitParams);

	convDynParams.size = sizeof(convDynParams);
	convDynParams.inputFmt = FMT_BAYER_RGBG;
	convDynParams.inputWidth = info->width;
	convDynParams.inputHeight = info->height;
	convDynParams.ctrlFlags = 0;
	convDynParams.digiGain = 512;
	convDynParams.brigtness = 0;
	convDynParams.contrast = 128;
	convDynParams.eeTable = NULL;
	convDynParams.eeTabSize = 0;
	convDynParams.gamma = 0;
	
	/* Chan A, out original size */
	convDynParams.outAttrs[0].enbale = TRUE;
	convDynParams.outAttrs[0].width = info->width;
	convDynParams.outAttrs[0].height = info->height;
	convDynParams.outAttrs[0].pixFmt = FMT_YUV_420SP;
	convDynParams.outAttrs[0].lineLength = 0;
	convDynParams.outAttrs[0].hFlip = 0;
	convDynParams.outAttrs[0].vFlip = 0;

	/* Chan B, out display size */
	convDynParams.outAttrs[1] = convDynParams.outAttrs[0];
	convDynParams.outAttrs[1].pixFmt = FMT_YUV_422ILE;
	if(mode == DISPLAY_MODE_PAL) {
		convDynParams.outAttrs[1].width = PAL_WIDTH;
		convDynParams.outAttrs[1].height = PAL_HEIGHT;
	} else {
		convDynParams.outAttrs[1].width = NTSC_WIDTH;
		convDynParams.outAttrs[1].height = NTSC_HEIGHT;
	}

	AlgHandle hImgConv = alg_create(&IMGCONV_ALG_FXNS, &convInitParams, &convDynParams);
	assert(hImgConv);

	DBG("img conv create done.");

	convInArgs->size = sizeof(*convInArgs);
	convInArgs->outAttrs[0] = convDynParams.outAttrs[0];
	convInArgs->outAttrs[1] = convDynParams.outAttrs[1];
	
	return hImgConv;
}

static Bool main_loop(TestParams *params)
{
	Bool ret = FALSE;

	int err = buffer_init();
	err |= alg_init();
	if(err)
		goto exit;

	CapInputInfo capInfo;
	CapHandle hCapture = capture_init(&capInfo);
	assert(hCapture);

	ImgConvInArgs convInArgs;
	AlgHandle hImgConv = img_conv_init(&capInfo, params->mode, &convInArgs);

	DisplayAttrs attrs;
	attrs.chanId = params->chanId;
	attrs.mode = params->mode;
	attrs.outputFmt = FMT_YUV_422ILE;
	
	DisplayHanlde hDisplay = display_create(&attrs);
	assert(hDisplay);
	err = display_start(hDisplay);
	assert(err == E_NO);
	
	FrameBuf frameBuf;
	Uint32 size = capInfo.width * capInfo.height * 2;
	BufHandle hBufOut = buffer_alloc(size, NULL);
	assert(hBufOut);

	Int32 i = 0;
	DisplayBuf dispBuf;

	struct timeval tmStart,tmEnd; 
	float   getBufTime, putBufTime, convTime;
	
	while(1) {

		err = capture_get_frame(hCapture, &frameBuf);
		if(err) {
			ERR("wait capture buffer failed...");
			break;
		}

		/* alloc buf for display */
		gettimeofday(&tmStart, NULL);
		err = display_get_free_buf(hDisplay, &dispBuf);
		assert(err == E_NO);
		gettimeofday(&tmEnd, NULL);
		getBufTime =  1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec;

		/* do resize */
		AlgBuf inBuf, outBuf[2];
		inBuf.buf = frameBuf.dataBuf;
		inBuf.bufSize = frameBuf.bufSize;
		outBuf[0].buf = buffer_get_user_addr(hBufOut);
		outBuf[0].bufSize = buffer_get_size(hBufOut);
		outBuf[1].buf = dispBuf.userAddr;
		outBuf[1].bufSize = dispBuf.bufSize;

		gettimeofday(&tmStart, NULL);
		err = img_conv_process(hImgConv, &inBuf, &convInArgs, outBuf, NULL);
		assert(err == E_NO);
		gettimeofday(&tmEnd, NULL);
		convTime = 1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec;
		
		err = capture_free_frame(hCapture, &frameBuf);
		if(err < 0) {
			ERR("free frame failed");
			break;
		}

		/* put to display */
		gettimeofday(&tmStart, NULL);
		err = display_put_buf(hDisplay, &dispBuf);
		assert(err == E_NO);
		gettimeofday(&tmEnd, NULL);
		putBufTime =  1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec;

		DBG("<%d> display time, get buf: %.3f us, put buf: %.3f us, conv: %.3f ms", 
			i, getBufTime, putBufTime, convTime/1000.0);

		if((i%10) == 0 && params->autoSwitch) {
			// change cfg 
			err = display_stop(hDisplay);
			assert(err == E_NO);
			attrs.mode = rand()%DISPLAY_MODE_MAX;
			err = display_config(hDisplay, &attrs);
			assert(err == E_NO);
			err = display_start(hDisplay);
			assert(err == E_NO);
		}
		
		i++;
		if(params->loopCnt > 0 && i > params->loopCnt)
			break;


		usleep(70000);
	}

	err = capture_stop(hCapture);
	if(err)
		goto exit;

	err = display_stop(hDisplay);
	assert(err == E_NO);

	/* writing display image to file */
	FILE *fpSave = fopen(params->fname, "wb");
	if(!fpSave) {
		ERRSTR("open %s failed", params->fname);
		goto exit;
	}
	
	DBG("writing display out data(%u bytes) to %s...", 
		dispBuf.bufSize, params->fname);
	fwrite(dispBuf.userAddr, dispBuf.bufSize, 1, fpSave);
	fclose(fpSave);
	
	
	ret = TRUE;
exit:

	display_delete(hDisplay);

	capture_delete(hCapture);
	
	if(hBufOut)
		buffer_free(hBufOut);


	buffer_exit();

	alg_exit();
	
	return ret;
	
}

static void usage(void)
{
    INFO("%s Compiled on %s %s with gcc %s", PROG_NAME, __DATE__, __TIME__, __VERSION__);
    INFO("Usage:");
    INFO("./%s [options]", PROG_NAME);
    INFO("Options:");
    INFO(" -h get help");
	INFO(" -n loop count, default: %d", DEF_LOOP_CNT);
	INFO(" -c display chan id, 0-1, defualt: 0");
	INFO(" -m display mode, 0-PAL, 1-NTSC, default: %d", DEF_DISP_MODE);
	INFO(" -f save file name for output image, defualt: %s", DEF_OUT_FILE);
	INFO(" -s enable output mode auto switch");
    INFO("Example:");
    INFO(" use default params: ./%s", PROG_NAME);
    INFO(" use specific params: ./%s -n 10 -f 1", PROG_NAME);
}

int main(int argc, char **argv)
{
	int c;
    char *options = "n:m:c:f:sh";
	TestParams params;
	
	params.loopCnt = DEF_LOOP_CNT;
	params.chanId = 0;
	params.mode = DEF_DISP_MODE;
	params.fname = DEF_OUT_FILE;
	params.autoSwitch = FALSE;

	while ((c = getopt(argc, argv, options)) != -1) {
		switch (c) {
		case 'n':
			params.loopCnt = atoi(optarg);
			break;
		case 'm':
			params.mode = atoi(optarg);
			break;
		case 'c':
			params.chanId = atoi(optarg);
			break;
		case 'f':
			params.fname = optarg;
			break;
		case 's':
			params.autoSwitch = TRUE;
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


