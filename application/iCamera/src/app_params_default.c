#include "params_mng.h"
#include "version.h"

#define NET_SEG			"192.168.1."
#define LOCAL_IP		NET_SEG"200"
#define IP_MASK			"255.255.255.0"
#define GATE_WAY		NET_SEG"1"
#define DOMAIN_NAME		"WORKGROUP"
#define HOST_NAME		"dm36x-ipnc"
#define DNS_SERVER		NET_SEG"1"
#define SERVER_IP		NET_SEG"102"
#define NTP_SRV_IP		NET_SEG"102"
#define TCP_SRV_PORT	9300
#define OSD_STRING		"HDCAM"
#define ROAD_NAME		OSD_STRING
#define RTSP_PORT		554
#define RTP_PAYLOAD		21
#define FTP_USER_NAME	"test"
#define FTP_PASS_WORD	"123456"
#define MANUFACTURE		"ShineVision"
#define MODEL			"ITC2-200"

#define DEF_DETECTOR	2

const AppParams c_appParamsDef = {
	.crc = 0,
	.magicNum = APP_PARAMS_MAGIC,
	.dataLen = 0,
	.networkInfo = {
		.ipAddr = {LOCAL_IP},
		.ipMask = {IP_MASK},
		.gatewayIP = {GATE_WAY},
		.domainName = {DOMAIN_NAME},
		.hostName = {HOST_NAME},
		.dnsServer = {DNS_SERVER},
	},
	.devInfo = {
		.macAddr = {0x0C, 0x00, 0x20, 0x12, 0x03, 0x26},
		.deviceSN = 20130104,
		.name = {OSD_STRING},
		.location = {__DATE__" "__TIME__},
		.manufacture = {MANUFACTURE},
		.model = {MODEL},
		.firmware = {__DATE__},
		.hardware = {HARDWARE_VER},
	},
	.osdParams = {
		.imgOsd = {
			.osdString = {OSD_STRING},
			.flags = CAM_OSD_FLAG_EN | CAM_OSD_FLAG_TEXT_EN | CAM_OSD_FLAG_TIME_EN,
			.color = CAM_OSD_COLOR_YELLOW,
			.postion = CAM_OSD_POS_UP_LEFT, 
			.size = CAM_OSD_SIZE_32x32,
		},
		.vidOsd = {
			.osdString = {OSD_STRING},
			.flags = CAM_OSD_FLAG_EN | CAM_OSD_FLAG_TEXT_EN | CAM_OSD_FLAG_TIME_EN,
			.color = CAM_OSD_COLOR_YELLOW,
			.postion = CAM_OSD_POS_UP_LEFT, 
			.size = CAM_OSD_SIZE_32x32,
		},
	},
	.roadInfo = {
		.roadName = {ROAD_NAME},
		.roadNum = 0,
		.directionNum = 0,
		.devSN = 0,
		.reserved = 0,
	},
	.rtpParams = {
		.streamName = {"h264"},
		.rtspSrvPort = RTSP_PORT,
		.flags = CAM_RTP_FLAG_EN | CAM_RTP_SAVE_EN,
	},
	.uploadCfg = {
		.protocol = CAM_UPLOAD_PROTO_TCP,
		.flags = CAM_AUTO_UPLOAD_EN | CAM_AUTO_DEL_EN,
	},
	.tcpImgSrvInfo = {
		.serverIP = {SERVER_IP},
		.serverPort = TCP_SRV_PORT,
		.flag = 0,
	},
	.ftpSrvInfo = {
		.serverIP = {SERVER_IP},
		.serverPort = 21,
		.flag = 0,
		.userName = {FTP_USER_NAME},
		.password = {FTP_PASS_WORD},
		.rootDir = {0,},
		.pathNamePattern = {0},
	},
	.ntpSrvInfo = {
		.serverIP = {NTP_SRV_IP},
		.serverPort = 0,
		.syncPrd = 24,
	},
	.exposureParams = {
		.shutterTime = 2000,
		.globalGain = 100,
	},
	.rgbGains = {
		.redGain = 75,
		.greenGain = {0, 0},
		.blueGain = 30,
	},
	.imgAdjParams = {
		.dayCfg = {
			.flags = CAM_IMG_SHARP_EN | CAM_IMG_GAMMA_EN,
			.contrast = 128,
			.sharpness = 100,
			.brightness = 0,
			.saturation = 128,
			.digiGain = 32,
			.drcStrength = 16,
			.gamma = 200,
		},
		.nightCfg = {
			.flags = CAM_IMG_SHARP_EN | CAM_IMG_GAMMA_EN,
			.contrast = 8,
			.sharpness = 16,
			.brightness = 0,
			.saturation = 128,
			.digiGain = 80,
			.drcStrength = 16,
			.gamma = 220,
		},
	},
	.h264EncParams = {
		.resolution = H264_RES_1920X1080,
		.frameRate = 15,
		.rateControl = CAM_H264_RC_CVBR,
		.forceIFrame = 0,
		.bitRate = 4000,
		.IPRatio = 30,
		.QPInit = 24,
		.QPMax = 51,
		.QPMin = 1,
		.encPreset = 0,
		.packetSize = 1280,
		.videoLen = 0,
		.flags = 0,
	},
	.workMode = {
		.format = CAM_FMT_JPEG_H264,
		.resType = CAM_RES_FULL_FRAME,
		.capMode = CAM_CAP_MODE_CONTINUE,
		.flags = 0,
		.reserved = {0},
	},
	.imgEncParams = {
		.width = 0,
		.height = 0,
		.encQuality = 80,
		.rotation = 0,
	},
	.ioCfg = {
		.direction = 0,
		.status = 0,
	},
	.strobeParams = {
		.status = 0,
		.switchMode = CAM_STROBE_SWITCH_AUTO,
		.ctrlFlags = 
			CAM_STROBE_FLAG_AUTO0 | CAM_STROBE_FLAG_AUTO1 | CAM_STROBE_FLAG_AUTO2 | CAM_STROBE_FLAG_EN_BY_WAY,
		.thresholdOn = 10,
		.thresholdOff = 25,
		.offset = 100,
	},
	.detectorParams = {
	#if 0
		.detecotorId = DETECTOR_TORY_EP,
		.redLightCapFlag = 
			DETECTOR_FLAG_LOOP1_POS_CAP | DETECTOR_FLAG_LOOP1_NEG_CAP | DETECTOR_FLAG_LOOP2_NEG_CAP,
		.greenLightCapFlag = DETECTOR_FLAG_LOOP1_NEG_CAP,
		.retrogradeCapFlag = 
			DETECTOR_FLAG_LOOP1_POS_CAP | DETECTOR_FLAG_LOOP2_POS_CAP | DETECTOR_FLAG_LOOP2_NEG_CAP,
	#else 
		.detecotorId = DETECTOR_TORY_EP_V2,
		.redLightCapFlag = 
			DETECTOR_FLAG_LOOP1_POS_CAP | DETECTOR_FLAG_LOOP1_NEG_CAP | DETECTOR_FLAG_LOOP2_NEG_CAP | DETECTOR_FLAG_LOOP2_POS_CAP,
		.greenLightCapFlag = DETECTOR_FLAG_LOOP1_NEG_CAP,
		.retrogradeCapFlag = DETECTOR_FLAG_LOOP1_POS_CAP | DETECTOR_FLAG_LOOP2_POS_CAP | DETECTOR_FLAG_LOOP2_NEG_CAP,
	#endif
		.loopDist = { 300, 300, 300, 300, },
		.limitSpeed = 80,
		.calcSpeed = 88,
		.radarId = RADAR_NONE,
	},
	.aeParams = {
		.flags = CAM_AE_FLAG_AE_EN | CAM_AE_FLAG_AG_EN,
		.targetValue = 75, 
		.minShutterTime = 10,
		.maxShutterTime = 4000,
		.minGainValue = 10,
		.maxGainValue = 200,
		.roi = {{0, 0, 800, 600}, {0},},
	},
	.awbParams = {
		.flags = 0,
	},
	.dayNightCfg = {
		.mode = CAM_DAY_MODE,
		.switchMethod = CAM_DN_SWT_OFF,
	},
	.avParams = {
		.avType = AV_TYPE_NONE,
		.flags = 0,
	},
	.specCapParams = {
		.expTime = 4000,
		.globalGain = 100,
		.strobeEn = 0x03,
		.aeMinExpTime = 100,
		.aeMaxExpTime = 4000,
		.aeMinGain = 0,
		.aeMaxGain = 300,
		.aeTargetVal = 75,
		.flags = CAM_SPEC_CAP_FLAG_AE_EN,
	},
};

