#include "img_convert.h"
#include "log.h"
#include "capture.h"
#include "buffer.h"
#include "log.h"
#include <pthread.h>
#include "jpg_enc.h"
#include "msg.h"


#define CAPTURE_DEVICE		"/dev/video0"
#define V4L2VID0_DEVICE		"/dev/video2"
#define V4L2VID1_DEVICE		"/dev/video3"

#define DEF_LOOP_CNT	10
#define DEF_OUT_FILE	"convOut.jpg"
#define IMG_MAX_WIDTH	4096
#define IMG_MAX_HEIGHT	4096
#define DEF_GAIN		512
#define SWITCH_WIDTH	1280
#define SWITCH_HEIGH	720
#define DEF_BRIGHTNESS	0
#define DEF_CONTRAST	16
#define DEF_IN_WIDTH	2560
#define DEF_IN_HEIGHT	2048

#define THR_MSG_NAME0	"/tmp/convThr0"
#define THR_MSG_NAME1	"/tmp/convThr1"
#define MAIN_MSG		"/tmp/convTest"

#define THR_OUT_FILE0	"rsz0.jpg"
#define THR_OUT_FILE1	"rsz1.jpg"

//#define CONV_IN_CAP_THR


typedef struct {
	MsgHeader 	hdr;
	FrameBuf	frame;
}CapMsg;


typedef struct _TestParams {
	int loopCnt;
	const char *outFileName;
	Uint16	outWidth;
	Uint16	outHeight;
	Uint16	gain;
	Uint32  flags;
	Uint8	brightness;
	Uint8	contrast;
	Uint16	inWidth;
	Uint16	inHeight;
	Bool	fakeInput;
	Uint8	flip;
	Uint8	outFmt;
}TestParams;

typedef struct {
	AlgHandle	hConv;
	AlgHandle	hJpgEnc;
	ImgConvDynParams convDyn;
	pthread_mutex_t *mutex;
	const char		*msgName;
	Bool		*exit;
	const char	*outFileName;
	CapHandle	hCap;
}ThrParams;

/* R   G   B */
#define COLOR_WHITE     {204, 204, 204}
#define COLOR_AMBAR     {208, 208,   0}
#define COLOR_CIAN      {  0, 206, 206}
#define COLOR_GREEN     {  0, 239,   0}
#define COLOR_MAGENTA   {239,   0, 239}
#define COLOR_RED       {205,   0,   0}
#define COLOR_BLUE      {  0,   0, 255}
#define COLOR_BLACK     {  0,   0,   0}
#define CBR	0
#define CBG	1
#define CBB	2
#define CB_RGGB	0
#define CB_BGGR	1
#define CB_GRBG	2
#define CB_GBRG	3

unsigned char bar[8][3]=
{
	COLOR_WHITE,
	COLOR_AMBAR,
	COLOR_CIAN,
	COLOR_GREEN,
	COLOR_MAGENTA,
	COLOR_RED,
	COLOR_BLUE,
	COLOR_BLACK,
};
int sRGB_color_bar(unsigned int fmt, unsigned int owidth, unsigned int oheight, char *obuf)
{
	unsigned int i;
	unsigned int j;
	unsigned int color_pattern[2][2];

	/*
	* CB_RGGB
	* *****
	* *R*G*
	* *****
	* *G*B*
	* *****
	*/
	switch(fmt)
	{
	case CB_RGGB:
	default:
		color_pattern[0][0] = CBR;
		color_pattern[0][1] = CBG;
		color_pattern[1][0] = CBG;
		color_pattern[1][1] = CBB;
		break;
	case CB_BGGR:
		color_pattern[0][0] = CBB;
		color_pattern[0][1] = CBG;
		color_pattern[1][0] = CBG;
		color_pattern[1][1] = CBR;
		break;
	case CB_GRBG:
		color_pattern[0][0] = CBG;
		color_pattern[0][1] = CBR;
		color_pattern[1][0] = CBB;
		color_pattern[1][1] = CBG;
		break;
	case CB_GBRG:
		color_pattern[0][0] = CBG;
		color_pattern[0][1] = CBB;
		color_pattern[1][0] = CBR;
		color_pattern[1][1] = CBG;
		break;
	}
	
	for(i=0; i<oheight; i++)
	{
		for(j=0;j<owidth; j++)
		{
			*obuf++ = bar[j*8/owidth][color_pattern[i&0x1][j&0x1]];
		}
	}

	return 0;
}

static Int32 jpg_save(const void *src, ChromaFormat fmt, Uint16 width, Uint16 height, const char *fname)
{
	AlgHandle hJpgEnc = NULL;
	Int32 ret = E_NO;
	
	hJpgEnc = NULL;
	JpgEncInitParams initParams;
	JpgEncDynParams dynParams;
	
	initParams.maxWidth = IMG_MAX_WIDTH;
	initParams.maxHeight = IMG_MAX_HEIGHT;
	initParams.size = sizeof(initParams);

	dynParams.width = width;
	dynParams.height = height;
	dynParams.inputFormat = fmt;
	dynParams.quality = 90;
	dynParams.rotation = 0;
	dynParams.size = sizeof(dynParams);

	/* create jpg enc handle */
	hJpgEnc = alg_create(&JPGENC_ALG_FXNS, &initParams, &dynParams);
	assert(hJpgEnc);

	JpgEncInArgs inArgs;
	JpgEncOutArgs outArgs;
	BufHandle hJpgOutBuf;
	AlgBuf inBuf, outBuf;
	Uint32 size = width * height;
	
	hJpgOutBuf = buffer_alloc(size, NULL);
	assert(hJpgOutBuf);

	inBuf.buf = (Uint8 *)src;
	inBuf.bufSize = width * height * 2;
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
	Int32 err;
	
	err = alg_process(hJpgEnc, &inBuf, &inArgs, &outBuf, &outArgs);
	//gettimeofday(&tmEnd,NULL); 
	
	if(err) {
		ERR("Jpg enc process failed...");
		ret = E_IO;
		goto exit;
	}

	//write out file
	FILE *fp_out= fopen(fname, "wb");
	if(!fp_out) {
		ERRSTR("open %s failed:", fname);
		goto exit;
	}


	DBG("writing out data(%u bytes) to %s...", 
		outArgs.bytesGenerated, fname);
	fwrite(outBuf.buf, outArgs.bytesGenerated, 1, fp_out);
	fflush(fp_out);
	fclose(fp_out);

exit:

	if(hJpgOutBuf)
		buffer_free(hJpgOutBuf);

	if(hJpgEnc)
		alg_delete(hJpgEnc);

	DBG("save jpg down...");
	return ret;
	
}

#ifdef CONV_IN_CAP_THR

static void *conv_thread(void *arg)
{
	ThrParams *params = (ThrParams *)arg;
	assert(arg);

	DBG("%s start", params->msgName);

	BufHandle hConvOutBuf = NULL;
	Int32	  size;
	ConvOutAttrs	*outAttrs = &params->convDyn.outAttrs[0];
	AlgBuf inBuf, outBuf;
	ImgConvInArgs convInArgs;

	size = outAttrs->width * outAttrs->height * 2;
	hConvOutBuf = buffer_alloc(size, NULL);
	assert(hConvOutBuf);
	assert(((Uint32)buffer_get_user_addr(hConvOutBuf) % 32) == 0);

	MsgHandle hMsg = msg_create(params->msgName, MAIN_MSG, 0);
	assert(hMsg);
	DBG("%s msg create ok", params->msgName);

	struct timeval tmStart,tmEnd; 
	float   timeUse;
	CapMsg msg;

	outBuf.buf = buffer_get_user_addr(hConvOutBuf);
	outBuf.bufSize = buffer_get_size(hConvOutBuf);

	convInArgs.outAttrs[0] = params->convDyn.outAttrs[0];
	convInArgs.outAttrs[1] = params->convDyn.outAttrs[1];
	convInArgs.size = sizeof(convInArgs);

	while(!*(params->exit)) {
		//DBG("%s start recv msg", params->msgName);
		Int32 err = msg_recv(hMsg, (MsgHeader *)&msg, sizeof(msg), 0);

		//DBG("%s recv msg, index: %d", params->msgName, msg.frame.index);
		if(err < 0) {

			ERR("recv msg err: %d", err);	
			break;
		}
		if(err != sizeof(msg)) {
			ERR("invalid size: %d", err);	
			continue;
		}

		if(*(params->exit)) {
			DBG("recv exit ...");
			break;
		}
		
		inBuf.buf = msg.frame.dataBuf;
		inBuf.bufSize = msg.frame.bufSize;
		//DBG("%s start conv", params->msgName);
		gettimeofday(&tmStart,NULL);
		err = alg_process(params->hConv, &inBuf, &convInArgs, &outBuf, NULL);
		//err = 0;
		//pthread_mutex_lock(params->mutex);
		//usleep(30000);
		//pthread_mutex_unlock(params->mutex);
		gettimeofday(&tmEnd,NULL); 

		timeUse = 1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec;
		
		if(err) 
			ERR("img conv failed");
		else
			DBG("%s  img convert cost: %.2f ms", params->msgName, timeUse/1000);

		msg.hdr.cmd = 1;
		err = msg_send(hMsg, NULL, (MsgHeader *)&msg, 0);
		assert(err == E_NO);
	}

	
	
	pthread_mutex_lock(params->mutex);

	jpg_save(outBuf.buf, convInArgs.outAttrs[0].pixFmt, convInArgs.outAttrs[0].width,
		convInArgs.outAttrs[0].height, params->outFileName);
	
	pthread_mutex_unlock(params->mutex);
	
	
exit:

	DBG("thread exit...");
	msg_delete(hMsg);
	pthread_exit(0);
	
}
#endif

static Bool fake_in_test(TestParams *params)
{
	Int32 err;
	Bool ret = FALSE;
	AlgHandle hImgConv;
	ImgConvInitParams convInitParams;
	ImgConvDynParams convDynParams;
	ImgConvInArgs	convInArgs;	
	BufHandle		hConvOutBuf = NULL, hConvInBuf = NULL;

	convInitParams.prevDevName = NULL;
	convInitParams.rszDevName = NULL;
	convInitParams.size = sizeof(convInitParams);

	convDynParams.size = sizeof(convDynParams);
	convDynParams.inputFmt = FMT_BAYER_RGBG;
	convDynParams.inputWidth = params->inWidth;
	convDynParams.inputHeight = params->inHeight;
	convDynParams.ctrlFlags = params->flags;
	convDynParams.digiGain = params->gain;
	convDynParams.brigtness = params->brightness;
	convDynParams.contrast = params->contrast;
	convDynParams.eeTable = NULL;
	convDynParams.eeTabSize = 0;
	
	
	convDynParams.outAttrs[0].enbale = TRUE;
	if(params->outWidth == 0 || params->outHeight == 0) {
		convDynParams.outAttrs[0].width = params->inWidth;
		convDynParams.outAttrs[0].height = params->inHeight;
	} else {
		convDynParams.outAttrs[0].width = ROUND_UP(params->outWidth, 16);
		convDynParams.outAttrs[0].height = params->outHeight;
	}
	convDynParams.outAttrs[0].pixFmt = params->outFmt ? FMT_YUV_420SP: FMT_YUV_422ILE;
	convDynParams.outAttrs[1].enbale = FALSE;

	convDynParams.outAttrs[0].lineLength = 0;
    convDynParams.outAttrs[0].hFlip = params->flip&0x1;
    convDynParams.outAttrs[0].vFlip = (params->flip>>1)&0x1;

	hImgConv = alg_create(&IMGCONV_ALG_FXNS, &convInitParams, &convDynParams);
	if(!hImgConv) {
		ERR("create img conv alg failed");
		goto exit;

	}

	err = alg_control(hImgConv, ALG_CMD_GET_DYN_PARAMS, &convDynParams);
	assert(err == E_NO);

	convInArgs.size = sizeof(convInArgs);
	convInArgs.outAttrs[0] = convDynParams.outAttrs[0];
	convInArgs.outAttrs[1] = convDynParams.outAttrs[1];

	Uint32 size = convDynParams.outAttrs[0].width * convDynParams.outAttrs[0].height * 2;
	hConvOutBuf = buffer_alloc(size, NULL);
	assert(hConvOutBuf);

	size = params->inWidth * params->inHeight;
	hConvInBuf = buffer_alloc(size, NULL);
	assert(hConvInBuf);

	AlgBuf inBuf, outBuf;
	struct timeval tmStart,tmEnd; 
	float   timeUse;

	outBuf.buf = buffer_get_user_addr(hConvOutBuf);
	outBuf.bufSize = buffer_get_size(hConvOutBuf);

	inBuf.buf = buffer_get_user_addr(hConvInBuf);
	inBuf.bufSize = buffer_get_size(hConvInBuf);
	sRGB_color_bar(CB_BGGR, convDynParams.inputWidth, convDynParams.inputHeight, (char *)inBuf.buf);

	Int32 i = 0;

	DBG("conver from %dX%d -> %dX%d", convDynParams.inputWidth, 
		convDynParams.inputWidth, convInArgs.outAttrs[0].width,
		convInArgs.outAttrs[0].height);
	/* start loop */
	while(1) {
		//DBG("%s start conv", params->msgName);
		gettimeofday(&tmStart,NULL);
		err = alg_process(hImgConv, &inBuf, &convInArgs, &outBuf, NULL);
		gettimeofday(&tmEnd,NULL); 

		timeUse = 1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec;
	
		if(err) 
			ERR("img conv failed");
		else
			DBG("<%d> img convert cost: %.2f ms", i, timeUse/1000);
	
		i++;
		if(params->loopCnt > 0 && i > params->loopCnt) {
			DBG("break main loop");
			break;
		}

		
#if 0

		if((i % 10) == 0) {
			if(resSwi) {
				convDynParams.outAttrs[0].width = SWITCH_WIDTH;
				convDynParams.outAttrs[0].height = SWITCH_HEIGH;
				convInArgs.outAttrs[0].width = SWITCH_WIDTH;
				convInArgs.outAttrs[0].height = SWITCH_HEIGH;
				
				resSwi = FALSE;
			} else {
				convDynParams.outAttrs[0].width = outWidth;
				convDynParams.outAttrs[0].height = outHeight;
				convInArgs.outAttrs[0].width = outWidth;
				convInArgs.outAttrs[0].height = outHeight;
				
				resSwi = TRUE;
			}
			DBG("res switched to %u X %u", 
					convInArgs.outAttrs[0].width, convInArgs.outAttrs[0].height);

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
#endif	

	}

	err = jpg_save( outBuf.buf, convInArgs.outAttrs[0].pixFmt, convInArgs.outAttrs[0].width,
					convInArgs.outAttrs[0].height, params->outFileName);

	ret = TRUE;

exit:
	if(hConvInBuf)
		buffer_free(hConvInBuf);

	if(hConvOutBuf)
		buffer_free(hConvOutBuf);

	if(hImgConv)
		alg_delete(hImgConv);

	return ret;
	
}

static Bool main_loop(TestParams *params)
{
	Bool ret = FALSE;
	CapHandle hCapture = NULL;
	Bool		exit = FALSE;
	MsgHandle	hMsg = NULL;
	Int32		capRefCnt = 1;
	BufHandle hConvOutBuf = NULL;
	
	int err = buffer_init();
	err = alg_init();
	if(err)
		goto exit;

	if(params->fakeInput) {
		ret = fake_in_test(params);
		goto exit;
	}
	
	//Uint32 size = info.width * info.height * 2;
	CapAttrs attrs;

	attrs.devName = CAPTURE_DEVICE;
	attrs.bufNum = 4;
	attrs.inputType = CAP_INPUT_CAMERA;
	attrs.std = CAP_STD_FULL_FRAME;
	attrs.userAlloc = TRUE;
	
	hCapture = capture_create(&attrs);
	CapInputInfo info;
	assert(hCapture);
	if(!hCapture)
		goto exit;
	
	capture_get_input_info(hCapture, &info);
	DBG("Capture Input: %d X %d, fmt: %d", (int)info.width, (int)info.height, (int)info.colorSpace);
	
	FrameBuf frameBuf;

	AlgHandle hImgConv;
	ImgConvInitParams convInitParams;
	ImgConvDynParams convDynParams;
	//AlgBuf inBuf, outBuf;
	ImgConvInArgs	convInArgs;

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
	convDynParams.outAttrs[0].pixFmt = params->outFmt ? FMT_YUV_420SP: FMT_YUV_422ILE;
	convDynParams.outAttrs[1].enbale = FALSE;

	convDynParams.outAttrs[0].lineLength = 0;
    convDynParams.outAttrs[0].hFlip = params->flip&0x1;
    convDynParams.outAttrs[0].vFlip = (params->flip>>1)&0x1;

	hImgConv = alg_create(&IMGCONV_ALG_FXNS, &convInitParams, &convDynParams);
	if(!hImgConv) {
		ERR("create img conv alg failed");
		goto exit;

	}


	convInArgs.size = sizeof(convInArgs);
	convInArgs.outAttrs[0] = convDynParams.outAttrs[0];
	convInArgs.outAttrs[1] = convDynParams.outAttrs[1];


#ifdef CONV_IN_CAP_THR
	ThrParams		thrParams[2];
	pthread_mutex_t mutex;	
	pthread_t		pid[2];
	
	/* init mutex */
	pthread_mutex_init(&mutex, NULL);
	
	/* create threads */
	thrParams[0].convDyn = thrParams[1].convDyn = convDynParams;
	thrParams[0].exit = thrParams[1].exit = &exit;
	thrParams[0].hConv = thrParams[1].hConv = hImgConv;
	thrParams[0].hCap = thrParams[1].hCap = hCapture;
	thrParams[0].msgName = THR_MSG_NAME0;
	thrParams[1].msgName = THR_MSG_NAME1;
	thrParams[0].mutex= thrParams[1].mutex = &mutex;
	thrParams[1].convDyn.outAttrs[0].width = SWITCH_WIDTH;
	thrParams[1].convDyn.outAttrs[0].height = SWITCH_HEIGH;
	thrParams[0].outFileName = THR_OUT_FILE0;
	thrParams[1].outFileName = THR_OUT_FILE1;
	
	pthread_create(&pid[0], NULL, conv_thread, &thrParams[0]);
	pthread_create(&pid[1], NULL, conv_thread, &thrParams[1]);

	capRefCnt = 1;
#else
	
	Uint32 size = convDynParams.outAttrs[0].width * convDynParams.outAttrs[0].height * 2;
	hConvOutBuf = buffer_alloc(size, NULL);
	assert(hConvOutBuf);
	assert(((Uint32)buffer_get_user_addr(hConvOutBuf) % 32) == 0);

	AlgBuf inBuf, outBuf;
	struct timeval tmStart,tmEnd; 
	float   timeUse;

	outBuf.buf = buffer_get_user_addr(hConvOutBuf);
	outBuf.bufSize = buffer_get_size(hConvOutBuf);

	convInArgs.outAttrs[0] = convDynParams.outAttrs[0];
	convInArgs.outAttrs[1] = convDynParams.outAttrs[1];
	convInArgs.size = sizeof(convInArgs);
	
#endif

	/* create msg */
	hMsg = msg_create(MAIN_MSG, THR_MSG_NAME0, MSG_FLAG_NONBLK);
	assert(hMsg);
	
	CapMsg msg;
	msg.hdr.cmd = 0;
	msg.hdr.dataLen = 0;
	msg.hdr.index = 0;
	msg.hdr.type = MSG_TYPE_REQU;

	capture_set_def_frame_ref_cnt(hCapture, capRefCnt);
	
	
	err = capture_start(hCapture);
	if(err)
		goto exit;

	//sleep(1);

	Int32 i = 0;
	msg.frame.index = 0;

	
	while(1) {
#ifdef CONV_IN_CAP_THR
		Int32			fdCap, fdMsg, fdMax;
		fd_set			rdSet;

		/* get fd for select */
		fdCap = capture_get_fd(hCapture);
		fdMsg = msg_get_fd(hMsg);
		fdMax = MAX(fdMsg, fdCap) + 1;
	
		/* wait data ready */
		FD_ZERO(&rdSet);
		FD_SET(fdCap, &rdSet);
		FD_SET(fdMsg, &rdSet);
		
		ret = select(fdMax, &rdSet, NULL, NULL, NULL);
		if(ret < 0 && errno != EINTR) {
			ERRSTR("select err");
			break;
		}

		/* no data ready */
		if(!ret)
			continue;
		
		if(FD_ISSET(fdCap, &rdSet)) 
#endif
		{
			err = capture_get_frame(hCapture, &frameBuf);
			if(err) {
				ERR("wait capture buffer failed...");
				break;
			}

			DBG("<%d> Get frame, index: %d, time: %u.%u", i, frameBuf.index, 
				(unsigned int)frameBuf.timeStamp.tv_sec, (unsigned int)frameBuf.timeStamp.tv_usec);
			msg.frame = frameBuf;
	#ifdef CONV_IN_CAP_THR
			ret = msg_send(hMsg, NULL, (MsgHeader *)&msg, 0);
			assert(ret == E_NO);
			if(ret)
				capture_free_frame(hCapture, &frameBuf);
			
			ret = msg_send(hMsg, THR_MSG_NAME1, (MsgHeader *)&msg, 0);
			assert(ret == E_NO);
			if(ret)
				capture_free_frame(hCapture, &frameBuf);
	#else

			inBuf.buf = frameBuf.dataBuf;
			inBuf.bufSize = frameBuf.bufSize;
			//DBG("%s start conv", params->msgName);
			gettimeofday(&tmStart,NULL);
			err = alg_process(hImgConv, &inBuf, &convInArgs, &outBuf, NULL);
			gettimeofday(&tmEnd,NULL); 

			timeUse = 1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec;
		
			if(err) 
				ERR("img conv failed");
			else
				DBG("<%d> img convert cost: %.2f ms", frameBuf.index, timeUse/1000);
			capture_free_frame(hCapture, &frameBuf);
	#endif		
			i++;
			if(params->loopCnt > 0 && i > params->loopCnt) {
				DBG("break main loop");
				break;
			}
		}
#ifdef CONV_IN_CAP_THR

		if(FD_ISSET(fdMsg, &rdSet)) {
			//DBG("%s free frame", params->msgName);
			err = msg_recv(hMsg, (MsgHeader *)&msg, sizeof(msg), 0);
			if(err != sizeof(msg) || msg.hdr.cmd != 1) {
				ERR("recv msg err");
			}else {
				err = capture_free_frame(hCapture, &msg.frame);
				if(err < 0) {
					ERR("free frame failed");
					//break;
				}
			}
		}
#endif

#if 0

		if((i % 10) == 0) {
			if(resSwi) {
				convDynParams.outAttrs[0].width = SWITCH_WIDTH;
				convDynParams.outAttrs[0].height = SWITCH_HEIGH;
				convInArgs.outAttrs[0].width = SWITCH_WIDTH;
				convInArgs.outAttrs[0].height = SWITCH_HEIGH;
				
				resSwi = FALSE;
			} else {
				convDynParams.outAttrs[0].width = outWidth;
				convDynParams.outAttrs[0].height = outHeight;
				convInArgs.outAttrs[0].width = outWidth;
				convInArgs.outAttrs[0].height = outHeight;
				
				resSwi = TRUE;
			}
			DBG("res switched to %u X %u", 
					convInArgs.outAttrs[0].width, convInArgs.outAttrs[0].height);

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
#endif	


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

	exit = TRUE;
#ifdef CONV_IN_CAP_THR

	ret = msg_send(hMsg, NULL, (MsgHeader *)&msg, 0);
	msg_send(hMsg, THR_MSG_NAME1, (MsgHeader *)&msg, 0);
	pthread_join(pid[0], NULL);
	pthread_join(pid[1], NULL);
	
	pthread_mutex_destroy(&mutex);
#else
	err = jpg_save( outBuf.buf, convInArgs.outAttrs[0].pixFmt, convInArgs.outAttrs[0].width,
					convInArgs.outAttrs[0].height, params->outFileName);
	
#endif

	err = capture_stop(hCapture);
	if(err)
		goto exit;

	
	ret = TRUE;
exit:

	if(hCapture)
		capture_delete(hCapture);

	if(hConvOutBuf)
		buffer_free(hConvOutBuf);

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
	INFO(" -i using fake input file instead of capture dev");
	INFO(" -W input width, ignored when using capture dev, default %d", DEF_IN_WIDTH);
	INFO(" -L input height, ignored when using capture dev, default %d", DEF_IN_HEIGHT);
	INFO(" -F flip, 1-hflip,2-vflip,3-hflip and vflip, default: 0");
	INFO(" -f output format, 0-YUV422ILE,1-YUV420SP, default: 1");
	
    INFO("Example:");
    INFO(" use default params: ./captureTest");
    INFO(" use specific params: ./captureTest -w 1920 -l 1080 -n 10");
	INFO(" RAW8 2240x2400 -> YUV422ILE 1280x720 hflip and vflip: ./imgconvTest -W 2240 -H 2400 -w 1280 -l 720 -f 0 -F 3");
}

int main(int argc, char **argv)
{
	int c;
    char *options = "n:o:w:l:g:b:c:hyNeamiW:L:F:f:";
	TestParams params;
	
	params.loopCnt = DEF_LOOP_CNT;
	params.outFileName = DEF_OUT_FILE;
	params.outWidth = params.outHeight = 0;
	params.gain = DEF_GAIN;
	params.flags = 0;
	params.brightness = DEF_BRIGHTNESS;
	params.contrast = DEF_CONTRAST;
	params.flip = 0;
	params.inWidth = DEF_IN_WIDTH;
	params.inHeight = DEF_IN_HEIGHT;
	params.fakeInput = 0;

	while ((c = getopt(argc, argv, options)) != -1) {
		switch (c) {
		case 'i':
			params.fakeInput = 1;
			break;
		case 'f':
			params.outFmt = atoi(optarg);
			break;
		case 'F':
			params.flip = atoi(optarg);
			break;
		case 'W':
			params.inWidth = atoi(optarg);
			break;
		case 'L':
			params.inHeight = atoi(optarg);                
			break;
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

