#include "jpg_enc.h"
#include "jpg_encoder.h"
#include "writer.h"
#include "cam_time.h"
#include "log.h"
#include "app_msg.h"

#define TEST_SAVE_TIME	

/*****************************************************************************
 Prototype    : jpg_encoder_process
 Description  : do jpeg encode
 Input        : IN AlgHandle hEncode  
                IN AlgBuf *pInBuf     
                OUT AlgBuf *pOutBuf   
                INOUT ImgMsg *msg     
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/20
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 jpg_encoder_process(IN AlgHandle hEncode, IN AlgBuf *pInBuf, OUT AlgBuf *pOutBuf, INOUT ImgMsg *msg)
{
	JpgEncInArgs 	inArgs;
	JpgEncOutArgs 	outArgs;
	Int32			ret;

	inArgs.appendData = NULL;
	inArgs.appendSize = 0;
	inArgs.endMark = 0;
	inArgs.size = sizeof(inArgs);

	outArgs.size = sizeof(outArgs);
	outArgs.bytesGenerated = 0;

	ret = alg_process(hEncode, pInBuf, &inArgs, pOutBuf, &outArgs);
	if(ret) {
		ERR("encode err");
		return ret;
	}

	/* fill some filed of msg */
	msg->dimension.colorSpace = FMT_JPG;
	msg->dimension.size = outArgs.bytesGenerated;

	return E_NO;
}


/*****************************************************************************
 Prototype    : jpg_encoder_save_frame
 Description  : save one frame to local file system
 Input        : IN ImgMsg *msg       
                IN const char *path  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/20
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 jpg_encoder_save_frame(IN const ImgMsg *msg, IN const char *path)
{
	char 		fileName[128];
	char		dirName[128];
	Int8		*data = buffer_get_user_addr(msg->hBuf);
	const CamDateTime	*capTime = &msg->timeStamp;
	Int32		err;
	
	if(!path || !msg)
		return E_INVAL;

	/* only save jpeg */
	if(msg->dimension.colorSpace != FMT_JPG) {
		ERR("not a jpeg file");
		return E_UNSUPT;
	}

#ifdef TEST_SAVE_TIME
	struct timeval tmStart,tmEnd; 
	float   timeUse;

	gettimeofday(&tmStart,NULL);
#endif

	/* generate dir file name */
	snprintf(dirName, sizeof(dirName), "%s/%04u_%02u_%02u_%02u", path,
             capTime->year, capTime->month, capTime->day, capTime->hour);
	snprintf(fileName, sizeof(fileName), "%s/%u_%u_%u.jpg", dirName, 
			capTime->minute, capTime->second, capTime->ms);

	err = write_file(dirName, fileName, data, msg->dimension.size);

#ifdef TEST_SAVE_TIME
	gettimeofday(&tmEnd,NULL); 
	timeUse = 1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec;
	DBG("<%s> saved, size: %d, cost: %.2f ms", fileName, msg->dimension.size, timeUse/1000);
#endif

	return err;
}

/* operation fxns for encoder register */
static const EncoderOps c_jpgEncOps = {
	.encProcess = jpg_encoder_process,
	.saveFrame = jpg_encoder_save_frame,
};

/*****************************************************************************
 Prototype    : jpg_encoder_create
 Description  : create jpeg encoder
 Input        : ParamsMngHandle hParamsMng  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/20
    Author       : Sun
    Modification : Created function

*****************************************************************************/
EncoderHandle jpg_encoder_create(IN ParamsMngHandle hParamsMng, IN pthread_mutex_t *mutex)
{
	
	EncoderAttrs		encAttrs;
	JpgEncInitParams	jpgEncInitParams;
	EncoderHandle		hJpgEncoder;

	jpgEncInitParams.maxWidth = IMG_MAX_WIDTH;
	jpgEncInitParams.maxHeight = IMG_MAX_HEIGHT;
	jpgEncInitParams.size = sizeof(JpgEncInitParams);
	
	encAttrs.name = IMG_ENC_NAME;
	encAttrs.msgName = MSG_IMG_ENC;
	encAttrs.dstName = NULL;
	encAttrs.encFxns = &JPGENC_ALG_FXNS;
	encAttrs.encInitParams = &jpgEncInitParams;
	encAttrs.encOps = &c_jpgEncOps;
	encAttrs.hParamsMng = hParamsMng;
	encAttrs.poolBufNum = IMG_ENC_POOL_BUF_NUM;
	encAttrs.saveRootPath = SD_MNT_PATH;
	encAttrs.cmdGetEncDyn = PMCMD_G_JPGENCDYN;
	encAttrs.cmdGetOsdDyn = PMCMD_G_IMGOSDDYN;
	encAttrs.cmdGetOsdInfo = PMCMD_G_IMGOSDINFO;
	encAttrs.cmdGetUploadProto = PMCMD_G_IMGTRANSPROTO;
	encAttrs.mutex = mutex;
	
	encAttrs.encBufSize = IMG_MAX_WIDTH * IMG_MAX_HEIGHT * 8 / 10;

	hJpgEncoder = encoder_create(&encAttrs);
	if(!hJpgEncoder) {
		ERR("create jpg encoder error...");
		return NULL;
	}

	return hJpgEncoder;
}

