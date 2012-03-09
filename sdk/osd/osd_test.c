#include "osd.h"
#include "jpg_enc.h"
#include "log.h"
#include "alg.h"
#include "cmem.h"
#include <iconv.h> 

#define DEF_SRC_FILE 	"zjz1.yuv"
#define DEF_OUT_FILE 	"zjz1.jpg"
//#define SWITCH_SRC_FILE	"colorful_toys_cif_420SP.yuv"
//#define SWITCH_OUT_FILE	"colorful_toys_cif_420SP.jpg"
#define DEF_OSD_STR		"相机: Hello, word~ 文一西路测试，速度。! @#$& 2012/2/18 (*+_+*)"
//#define DEF_OSD_STR		"你好"


#define DEF_IN_FMT		FMT_YUV_422P
#define DEF_ROTATION	90
#define DEF_LOOP_CNT	10
#define IMG_MAX_WIDTH	4096
#define IMG_MAX_HEIGHT	4096
#define DEF_WIDTH		2816
#define DEF_HEIGHT		2112
#define DEF_QUALITY		95
#define DEF_COLOR		OSD_COLOR_YELLOW
#define DEF_OSD_MODE	OSD_MODE_32x16

typedef struct _TestParams {
	char *srcFileName;
	char *outFileName;
	int inputFormat;
	int rotation;
	int loopCnt;
	int width;
	int height;
	char *osdString;
	OsdColor color;
	OsdMode osdMode;
}TestParams;

static Bool code_convert()
{
	iconv_t cd = iconv_open("GB_2312", "UTF-8"); 
	if(cd == (iconv_t)(-1)) {
		ERRSTR("open code convert failed...");
		return FALSE;
	}

	char *inBuf = "你好";
	char *outBuf = malloc(256);
	size_t inlen = strlen(inBuf);
	size_t outlen = sizeof(outBuf);

	memset(outBuf, 0, 256);

	if(iconv(cd, &inBuf, &inlen, &outBuf, &outlen) < 0) {
		ERRSTR("convert code failed...");
		return E_IO; 
	}

	
	DBG("convert finish, in: %s, out: %X, left: %d", inBuf, outBuf[0], inlen);

	iconv_close(cd);

	return TRUE;
	
	
}

static Bool main_loop(TestParams *params)
{
	Int32 status = alg_init();
	assert(status == E_NO);
	Bool ret = FALSE;

	//code_convert();
	
	//create handle
	AlgHandle hJpgEnc;
	JpgEncInitParams initParams;
	JpgEncDynParams dynParams;

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

	//DBG("inBuf: 0x%X, outBuf: 0x%X", inBuf.buf, outBuf.buf);

	//unsigned long phyaddr = CMEM_getPhys(inBuf.buf + 377);
	//DBG("phyaddr: %u", phyaddr);

	assert(inBuf.buf && outBuf.buf);

	inArgs.appendData = NULL;
	inArgs.appendSize = 0;
	inArgs.endMark = 0;
	inArgs.size = sizeof(inArgs);
	outArgs.size = sizeof(outArgs);
	outArgs.bytesGenerated = 0;
	
	DBG("Jpg encode handle create OK.");

	//create osd handle
	AlgHandle hOsd;
	OsdInitParams osdInitParams;
	OsdDynParams osdDynParams;
	OsdInArgs osdInArgs;
	OsdOutArgs osdOutArgs;

	osdInitParams.asc16Tab = NULL;
	osdInitParams.hzk16Tab = NULL;
	osdInitParams.size = sizeof(osdInitParams);

	osdDynParams.size = sizeof(osdDynParams);
	osdDynParams.imgFormat = params->inputFormat;
	osdDynParams.width = params->width;
	osdDynParams.height = params->height;
	osdDynParams.mode = params->osdMode;
	osdDynParams.color = params->color;
	if(params->rotation == 90)
		osdDynParams.mode = OSD_MODE_32x32_ROTATE_R;
	else if(params->rotation == 270)
		osdDynParams.mode = OSD_MODE_32x32_ROTATE_L;

	osdInArgs.size = sizeof(osdInArgs);
	osdInArgs.startX = 10;
	osdInArgs.startY = 10;
	osdInArgs.strOsd = params->osdString;

	osdOutArgs.size = sizeof(osdOutArgs);
	
	hOsd = alg_create(&OSD_ALG_FXNS, &osdInitParams, &osdDynParams);
	assert(hOsd);

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

	struct timeval tmStart,tmEnd; 
	float   timeUse;
	AlgBuf *pInBuf = &inBuf, *pOutBuf = &outBuf;

	DBG("Start add osd & encode...");

	int cnt = 0;
	int err;
	
	while(1) {
		
		gettimeofday(&tmStart,NULL);
		err = alg_process(hOsd, pInBuf, &osdInArgs, NULL, &osdOutArgs);
		gettimeofday(&tmEnd,NULL); 

		if(err) {
			ERR("Osd add failed...");
			goto exit;
		}
		timeUse = 1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec;
		DBG("<%d> Add osd cost: %.3f ms", cnt, timeUse/1000);

		cnt++;
		if(params->loopCnt > 0 && cnt > params->loopCnt)
			break;

#if 1
		/* change dyn params */
		OsdDynParams *pDynParams;
		if((cnt % 10) == 0) {
			osdDynParams.mode++;
			osdDynParams.mode %= OSD_MODE_MAX;
			osdDynParams.color++;
			osdDynParams.color %= OSD_COLOR_MAX;
			pDynParams = &osdDynParams;
			pInBuf = &inBuf;
			pOutBuf = &outBuf;
			DBG("set osd dyn params, mode: %d, color: %d", osdDynParams.mode, osdDynParams.color);
			err = alg_set_dynamic_params(hOsd, pDynParams);
			if(err) {
				ERR("set dyn params failed!");
			} 
		} 
		//usleep(1000);
		
		
#endif

	}

	
	gettimeofday(&tmStart,NULL);
	err = alg_process(hJpgEnc, pInBuf, &inArgs, pOutBuf, &outArgs);
	gettimeofday(&tmEnd,NULL); 

	if(err) {
		ERR("Jpg enc process failed...");
		goto exit;
	}

	timeUse = 1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec;
	DBG("Jpeg enc cost: %.3f ms", timeUse/1000);

	DBG("encode ok, generate %d bytes.", (int)outArgs.bytesGenerated);

	//write out file
	fp_out= fopen(params->outFileName, "wb");
	if(!fp_out) {
		ERRSTR("open %s failed:", params->outFileName);
		goto exit;
	}
	
	DBG("writing jpg data to %s...", params->outFileName);
	fwrite(outBuf.buf, outArgs.bytesGenerated, 1, fp_out);

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

	if(hJpgEnc)
		alg_delete(hJpgEnc);

	if(hOsd)
		alg_delete(hOsd);

	alg_exit();

	return ret;

	
}

static void usage(void)
{
    INFO("osdTest Compiled on %s %s with gcc %s", __DATE__, __TIME__, __VERSION__);
    INFO("Usage:");
    INFO("./osdTest [options]");
    INFO("Options:");
    INFO(" -h get help");
	INFO(" -i input file name, default: %s", DEF_SRC_FILE);
	INFO(" -o output file name, default: %s", DEF_OUT_FILE);
	INFO(" -c input chroma format, default: %d", DEF_IN_FMT);
	INFO(" -r rotation, default: %d", DEF_ROTATION);
	INFO(" -n loop count, default: %d", DEF_LOOP_CNT);
	INFO(" -w input image width, default: %d", DEF_WIDTH);
	INFO(" -l input image height, default: %d", DEF_HEIGHT);
	INFO(" -s osd string, default: %s", DEF_OSD_STR);
	INFO(" -C osd color, 0-yellow, 1-red, 2-green, 3-blue, default: %d", DEF_COLOR);
	INFO(" -m osd mode, 0-32X16, 1-32X32, 2-32X32 right 90 degree rotation, 4-32X32 left 90 degree rotation, default: %d", DEF_OSD_MODE);
    INFO("Example:");
    INFO(" use default params: ./osdTest");
    INFO(" use specific params: ./osdTest -f ./test/toy.yuv -r 90");
}

int main(int argc, char **argv)
{
	int c;
    char *options = "i:c:o:r:w:s:l:n:C:m:h";
	TestParams params;
	
	params.srcFileName = DEF_SRC_FILE;
	params.inputFormat = DEF_IN_FMT;
	params.rotation = DEF_ROTATION;
	params.loopCnt = DEF_LOOP_CNT;
	params.outFileName = DEF_OUT_FILE;
	params.width = DEF_WIDTH;
	params.height = DEF_HEIGHT;
	params.osdString = DEF_OSD_STR;
	params.color = DEF_COLOR;
	params.osdMode = DEF_OSD_MODE;

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
		case 's':
			params.osdString = optarg;
			break;
		case 'C':
			params.color = atoi(optarg);
			break;
		case 'm':
			params.osdMode = atoi(optarg);
			if(params.osdMode < 0 || params.osdMode >= OSD_MODE_MAX) {
				WARN("invalid osd mode, use default.");
				params.osdMode = DEF_OSD_MODE;
			}
				
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


