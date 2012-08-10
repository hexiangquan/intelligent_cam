#include "common.h"
#include "img_ctrl.h"
#include "log.h"
#include <sys/ioctl.h>
#include <stropts.h>


#define DEV_NAME		"/dev/imgctrl"
#define SPEC_TRIG_CNT	10

typedef struct _TestParams {
	const char *devName;
	int abEn;
	int awbEn;
	int hwTestEn;
	int loopCnt;
}TestParams;

static void hw_test(int fd)
{
	int err = 0;
	
	while(!err) {
		err = ioctl(fd, IMGCTRL_HW_TEST, NULL);
		usleep(100000);
	}
}

static int spec_cap_test(int fd, int cnt)
{
	int err;
	struct hdcam_spec_cap_cfg cfg;

	cfg.exposureTime = 4000;
	cfg.globalGain = 100;
	cfg.strobeCtrl = 0x03;
	cfg.aeMinExpTime = 100;
	cfg.aeMaxExpTime = 4000;
	cfg.aeMinGain = 0;
	cfg.aeMaxGain = 200;
	cfg.aeTargetVal = 70;
	cfg.flags = HDCAM_SPEC_CAP_AE_EN;

	err = ioctl(fd, IMGCTRL_S_SPECCAP, &cfg);
	if(err) {
		ERRSTR("set spec cap failed");
		return E_IO;
	}

	int i;
	__u16 id;

	for(i = 0; i < cnt; ++i) {
		err = ioctl(fd, IMGCTRL_SPECTRIG, &id);
		if(err)
			break;
		DBG("<%d> spec trig next id: %u", i, (unsigned)id);
		usleep(100000);
	}

	if(!err)
		DBG("spec cap test success.");
	
	return err;
	
}

static Bool main_loop(TestParams *params)
{
	Bool ret = FALSE;
	int fd;
	
	fd = open(params->devName, O_RDWR);
	if(fd < 0) {
		ERR("open %s failed", params->devName);
		goto exit;
	}

	if(params->hwTestEn)
		hw_test(fd);
	
	/* test lum info */
	int err;
	struct hdcam_lum_info lumInfo;

	lumInfo.exposureTime = 2000;
	lumInfo.globalGain = 321;
	lumInfo.apture = 0;
	lumInfo.reserved = 0;

	err = ioctl(fd, IMGCTRL_S_LUM, &lumInfo);
	if(err) {
		ERR("set lum failed");
		goto exit;
	}

	err = ioctl(fd, IMGCTRL_G_LUM, &lumInfo);
	if(err) {
		ERR("get lum failed");
		goto exit;
	}

	usleep(10000);

	DBG("get lum info, exposure: %u, gain: %u", (__u32)lumInfo.exposureTime, (__u32)lumInfo.globalGain);

	struct hdcam_chroma_info chromaInfo;
	chromaInfo.redGain = 27;
	chromaInfo.greenGain = 59;
	chromaInfo.blueGain = 73;
	chromaInfo.reserved = 0;

	err = ioctl(fd, IMGCTRL_S_CHROMA, &chromaInfo);
	if(err) {
		ERR("set chroma failed");
		goto exit;
	}

	err = ioctl(fd, IMGCTRL_G_CHROMA, &chromaInfo);
	if(err) {
		ERR("get chroma failed");
		goto exit;
	}

	usleep(10000);
	
	DBG("get chroma, rgb: %u, %u, %u", 
		(__u32)chromaInfo.redGain, (__u32)chromaInfo.greenGain, (__u32)chromaInfo.blueGain);
	
	struct hdcam_ab_cfg abCfg;

	memset(&abCfg, 0, sizeof(abCfg));
	abCfg.flags = 0x03;
	abCfg.minShutterTime = 22;
	abCfg.maxShutterTime = 4567;
	abCfg.minGainValue = 0;
	abCfg.maxGainValue = 345;
	abCfg.targetValue = 75;
	
	err = ioctl(fd, IMGCTRL_S_ABCFG, &abCfg);
	if(err) {
		ERR("set ab failed");
		goto exit;
	}

	struct hdcam_awb_cfg awbCfg;

	memset(&awbCfg, 0, sizeof(awbCfg));
	awbCfg.flags = 0x01;
	awbCfg.maxRedGain = 122;
	awbCfg.maxBlueGain = 157;
	awbCfg.maxGreenGain = 99;
	awbCfg.initRedGain[0] = 73;
	awbCfg.initRedGain[1] = 83;
	awbCfg.initBlueGain[0] = 23;
	awbCfg.initBlueGain[1] = 46;
	
	err = ioctl(fd, IMGCTRL_S_AWBCFG, &awbCfg);
	if(err) {
		ERR("set awb failed");
		goto exit;
	}

	struct hdcam_img_enhance_cfg enhanceCfg;

	memset(&enhanceCfg, 0, sizeof(enhanceCfg));
	enhanceCfg.flags = 
		HDCAM_ENH_FLAG_CONTRAST_EN | HDCAM_ENH_FLAG_SHARP_EN | HDCAM_ENH_FLAG_DRC_EN;
	enhanceCfg.contrast = 16;
	enhanceCfg.sharpness = 24;
	enhanceCfg.drcStrength = 80;
	
	err = ioctl(fd, IMGCTRL_S_ENHCFG, &enhanceCfg);
	if(err) {
		ERR("set enhance cfg failed");
		goto exit;
	}

	err = ioctl(fd, IMGCTRL_TRIGCAP, NULL);
	if(err) {
		ERR("trig cap failed!");
		goto exit;
	}

	err = spec_cap_test(fd, params->loopCnt);
	if(err) {
		ERR("spec cap test failed");
		goto exit;
	}

	ret = TRUE;
exit:

	if(fd > 0)
		close(fd);

	return ret;
}

static void usage(void)
{
    INFO("imgctrlTest Compiled on %s %s with gcc %s", __DATE__, __TIME__, __VERSION__);
    INFO("Usage:");
    INFO("./osdTest [options]");
    INFO("Options:");
    INFO(" -h get help");
	INFO(" -d dev name, default: %s", DEV_NAME);
	INFO(" -b ab ctrl, 0-enable, 1-disable, default: enable");
	INFO(" -w awb ctrl, 0-enable, 1-disable, default: enable");
	INFO(" -t fpga rw test, 0-enable, 1-disable, default: disable");
	INFO(" -n special trigger loop cnt, default: %d", SPEC_TRIG_CNT);
    INFO("Example:");
    INFO(" use default params: ./imgctrlTest");
    INFO(" use specific params: ./imgctrlTest -s /dev/imgctrl");
}

int main(int argc, char **argv)
{
	int c;
    char *options = "d:b:t:w:h";
	TestParams params;
	
	params.devName = DEV_NAME;
	params.abEn = 1;
	params.awbEn = 1;
	params.hwTestEn = 0;
	params.loopCnt = SPEC_TRIG_CNT;

	while ((c = getopt(argc, argv, options)) != -1) {
		switch (c) {
		case 'd':
			params.devName = optarg;
			break;
		case 'b':
			params.abEn = atoi(optarg);
			break;
		case 'w':
			params.awbEn = atoi(optarg);
			break;
		case 't':
			params.hwTestEn = atoi(optarg);
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

