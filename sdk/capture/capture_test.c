#include "capture.h"
#include "buffer.h"
#include "log.h"
#include <pthread.h>
#include "jpg_enc.h"
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/videodev2.h>
#include "cap_info_parse.h"
#include "img_ctrl.h"


#define CAPTURE_DEVICE		"/dev/video0"
#define V4L2VID0_DEVICE		"/dev/video2"
#define V4L2VID1_DEVICE		"/dev/video3"

#define DEF_LOOP_CNT	10
#define DEF_OUT_FILE	"capOut.raw"
#define IMG_MAX_WIDTH	4096
#define IMG_MAX_HEIGHT	4096
#define DEF_CAP_MODE	0


typedef struct _TestParams {
	int loopCnt;
	const char *outFileName;
	int capMode;
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

static void cap_ctrl_test(CapHandle hCapture)
{
	int fd = capture_get_fd(hCapture);
	assert(fd > 0);

	int ret;
	struct v4l2_ext_controls extCtrls;
	struct v4l2_ext_control ctrl;
	char data[16];

	memset(&extCtrls, 0, sizeof(extCtrls));
	extCtrls.count = 1;
	extCtrls.ctrl_class = V4L2_CTRL_CLASS_USER;
	extCtrls.controls = &ctrl;

	ctrl.id = V4L2_CID_AUTOBRIGHTNESS;
	ctrl.size = 16;
	ctrl.string = data;

	while(1) {
		ret = ioctl(fd, VIDIOC_S_EXT_CTRLS, &extCtrls);

		ERRSTR("ext ctrl returns: %d", ret);
		sleep(1);
	}
				

}

static void cap_info_test(FrameBuf *frame, CapInputInfo *inputInfo)
{
	ImgDimension	dim;

	dim.size = inputInfo->size;
	dim.width = inputInfo->width;
	dim.height = inputInfo->height;
	dim.bytesPerLine = inputInfo->bytesPerLine;
	dim.colorSpace = inputInfo->colorSpace;

	RawCapInfo capInfo;

	Int32 err = cap_info_parse((Uint8 *)frame->dataBuf, &dim, &capInfo);
	assert(err == E_NO);
	
	DBG("capInfo: size: %d, index: %d, cap mode: %d, avg Y: %u", 
		dim.size, capInfo.index, capInfo.capMode, capInfo.avgLum);
	DBG("  strobe: 0x%X, exposure: %u, global gain: %u", 
		capInfo.strobeStat, capInfo.exposure, capInfo.globalGain);

	return;
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
	attrs.defRefCnt = 1;
	attrs.mode = CAP_MODE_CONT;
	
	CapHandle hCapture = capture_create(&attrs);
	assert(hCapture);

	int trigCmd = 0;
	if(params->capMode == 0)
		attrs.mode = CAP_MODE_CONT;
	else {
		/* normal trig */
		attrs.mode = CAP_MODE_TRIG;
		if(params->capMode == 1)
			trigCmd = IMGCTRL_TRIGCAP;
		else
			trigCmd = IMGCTRL_SPECTRIG;
	}

	err = capture_config(hCapture, CAP_STD_FULL_FRAME, attrs.mode);
	assert(err == E_NO);
	
	CapInputInfo info;

	err = capture_get_input_info(hCapture, &info);
	DBG("Capture input: %u X %u, fmt: %d, bytes per line: %d", 
		info.width, info.height, info.colorSpace, info.bytesPerLine);
	
	FrameBuf frameBuf;
	Uint32 size = 1600 * 1200 * 2;
	BufHandle hBufOut = buffer_alloc(size, NULL);
	assert(hBufOut);
	assert(((Uint32)buffer_get_user_addr(hBufOut) % 32) == 0);

	if(!hCapture)
		goto exit;

	//cap_ctrl_test(hCapture);

	//capture_set_def_frame_ref_cnt(hCapture, 3);

	err = capture_start(hCapture);
	if(err)
		goto exit;

	//capture_set_def_frame_ref_cnt(hCapture, 3);

	int fdImg = open("/dev/imgctrl", O_RDWR);
	assert(fdImg > 0);
	struct hdcam_ab_cfg cfg;
	bzero(&cfg, sizeof(cfg));
	cfg.flags = HDCAM_AB_FLAG_AE_EN | HDCAM_AB_FLAG_AG_EN;
	cfg.targetValue = 100;
	cfg.minShutterTime = 10;
	cfg.maxShutterTime = 2000;
	cfg.minGainValue = 1;
	cfg.maxGainValue = 100;
	cfg.maxAperture = 10;
	cfg.minAperture = 0;
	cfg.roi[0].endX = 1024;
	cfg.roi[0].endY = 1000;
	
	err = ioctl(fdImg, IMGCTRL_S_ABCFG, &cfg);
	assert(err == 0);

	struct hdcam_spec_cap_cfg specTrig;
	specTrig.exposureTime = 4000;
	specTrig.globalGain = 123;
	specTrig.strobeCtrl = 0x03;
	specTrig.aeMinExpTime = 100;
	specTrig.aeMaxExpTime = 5000;
	specTrig.aeMinGain = 0;
	specTrig.aeMaxGain = 200;
	specTrig.aeTargetVal = 75;
	specTrig.flags = 0;
	err = ioctl(fdImg, IMGCTRL_S_SPECCAP, &specTrig);
	assert(err == 0);

	Int32 i = 0;
	Uint16 frameId;

	while(1) {

		if(trigCmd) {
			//err = ioctl(fdImg, trigCmd, NULL);// &frameId);
			err = ioctl(fdImg, trigCmd, NULL);// &frameId);
			assert(err == 0);
		}

		err = capture_get_frame(hCapture, &frameBuf);
		if(err) {
			ERR("wait capture buffer failed...");
			break;
		}

		DBG("<%d> Get frame, index: %d, time: %u.%u", i, frameBuf.index, 
			(unsigned int)frameBuf.timeStamp.tv_sec, (unsigned int)frameBuf.timeStamp.tv_usec);
		

		//check_all_zero(frameBuf.dataBuf, frameBuf.bytesUsed);
		cap_info_test(&frameBuf, &info);
		
		usleep(1000);
		
		err = capture_free_frame(hCapture, &frameBuf);
		if(err < 0) {
			ERR("free frame failed");
			break;
		}

		//err = capture_inc_frame_ref(hCapture, &frameBuf);
		//assert(err == E_NO);
		
		//err = capture_free_frame(hCapture, &frameBuf);

		i++;
		if(params->loopCnt > 0 && i > params->loopCnt)
			break;

		usleep(70000);
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
	INFO(" -m capture mode, 0-continue, 1-normal trig, 2-special trig, defualt: %d", 
		DEF_CAP_MODE);
    INFO("Example:");
    INFO(" use default params: ./captureTest");
    INFO(" use specific params: ./captureTest -n 10");
}

int main(int argc, char **argv)
{
	int c;
    char *options = "n:o:m:h";
	TestParams params;
	
	params.loopCnt = DEF_LOOP_CNT;
	params.outFileName = DEF_OUT_FILE;
	params.capMode = DEF_CAP_MODE;

	while ((c = getopt(argc, argv, options)) != -1) {
		switch (c) {
		case 'n':
			params.loopCnt = atoi(optarg);
			break;
		case 'o':
			params.outFileName = optarg;
			break;
		case 'm':
			params.capMode = atoi(optarg);
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

