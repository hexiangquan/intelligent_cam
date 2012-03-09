#include "capture.h"
#include "buffer.h"
#include "log.h"
#include <pthread.h>
#include "jpg_enc.h"


#define CAPTURE_DEVICE		"/dev/video0"
#define V4L2VID0_DEVICE		"/dev/video2"
#define V4L2VID1_DEVICE		"/dev/video3"

#define DEF_LOOP_CNT	10
#define DEF_OUT_FILE	"capOut.raw"
#define IMG_MAX_WIDTH	4096
#define IMG_MAX_HEIGHT	4096


typedef struct _TestParams {
	int loopCnt;
	const char *outFileName;
}TestParams;

static Bool check_all_zero(void *buf, Int32 bufSize)
{
	Uint32 *ptr = (Uint32 *)buf;
	Int32 i;

	for(i = 0; i < bufSize; ptr++, i+=4) {
		if(*ptr)
			break;
	}

	if(i >= bufSize) {
		DBG("buf all zero");
		return TRUE;

	}

	return FALSE;
	
}

static Bool main_loop(TestParams *params)
{
	Bool ret = FALSE;
	FILE *fp_out = NULL;

	int err = buffer_init();
	//err = alg_init();
	if(err)
		goto exit;

	CapAttrs attrs;

	attrs.devName = CAPTURE_DEVICE;
	attrs.bufNum = 3;
	attrs.inputType = CAP_INPUT_CAMERA;
	attrs.std = CAP_STD_FULL_FRAME;
	attrs.userAlloc = TRUE;
	
	CapHandle hCapture = capture_create(&attrs);
	assert(hCapture);

	CapInputInfo info;

	err = capture_get_input_info(hCapture, &info);
	DBG("Capture input: %u X %u, fmt: %d", info.width, info.height, info.colorSpace);
	
	FrameBuf frameBuf;
	Uint32 size = 1600 * 1200 * 2;
	BufHandle hBufOut = buffer_alloc(size, NULL);
	assert(hBufOut);
	assert(((Uint32)buffer_get_user_addr(hBufOut) % 32) == 0);

	if(!hCapture)
		goto exit;

	err = capture_start(hCapture);
	if(err)
		goto exit;

	Int32 i = 0;

	while(1) {
		err = capture_get_frame(hCapture, &frameBuf);
		if(err) {
			ERR("wait capture buffer failed...");
			break;
		}

		DBG("<%d> Get frame, index: %d, time: %u.%u", i, frameBuf.index, 
			(unsigned int)frameBuf.timeStamp.tv_sec, (unsigned int)frameBuf.timeStamp.tv_usec);
		

		check_all_zero(frameBuf.dataBuf, frameBuf.bytesUsed);
		
		usleep(1000);
		err = capture_free_frame(hCapture, &frameBuf);
		if(err < 0) {
			ERR("free frame failed");
			break;
		}

		i++;
		if(params->loopCnt > 0 && i > params->loopCnt)
			break;
	}

	err = capture_stop(hCapture);
	if(err)
		goto exit;


	fp_out = fopen(params->outFileName, "wb");
	if(!fp_out) {
		ERRSTR("open %s failed", params->outFileName);
		goto exit;
	}
	
	DBG("writing capture out data(%u bytes) to %s...", 
		frameBuf.bytesUsed, params->outFileName);
	fwrite(frameBuf.dataBuf, frameBuf.bytesUsed, 1, fp_out);
	
	
	ret = TRUE;
exit:

	if(hCapture)
		capture_delete(hCapture);

	if(fp_out)
		fclose(fp_out);

	if(hBufOut)
		buffer_free(hBufOut);


	buffer_exit();

	//alg_exit();
	
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
    INFO("Example:");
    INFO(" use default params: ./captureTest");
    INFO(" use specific params: ./captureTest -n 10");
}

int main(int argc, char **argv)
{
	int c;
    char *options = "n:o:h";
	TestParams params;
	
	params.loopCnt = DEF_LOOP_CNT;
	params.outFileName = DEF_OUT_FILE;

	while ((c = getopt(argc, argv, options)) != -1) {
		switch (c) {
		case 'n':
			params.loopCnt = atoi(optarg);
			break;
		case 'o':
			params.outFileName = optarg;
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

