#include "jpg_enc.h"
#include "jpg_encoder.h"
#include "writer.h"
#include "cam_time.h"
#include "log.h"
#include "app_msg.h"
#include "img_trans.h"

#define DEV_SD0		"/dev/mmcblk0"
//#define TEST_SAVE_TIME	
//#define NO_SAVE_FILE

/*****************************************************************************
 Prototype    : add_append_data
 Description  : add append data, only for jpeg
 Input        : ImgMsg *img  
                AlgBuf *buf  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/1
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 add_append_data(ImgMsg *img, AlgBuf *buf)
{
	ImgAppendInfo *info;
	Uint32 offset = ROUND_UP(img->dimension.size, 4);

	if(offset + sizeof(*info) > buf->bufSize)
		return E_NOSPC;
	
	info = (ImgAppendInfo *)(buf->buf + offset);
	img->dimension.size = offset + sizeof(*info);
	info->dimension = img->dimension;
	info->frameId = img->rawInfo.index;
	info->capMode = img->rawInfo.capMode;
	info->strobeStat = img->rawInfo.strobeStat;
	info->exposureTime = img->rawInfo.exposure;
	info->globalGain = img->rawInfo.globalGain;
	info->avgLum = img->rawInfo.avgLum;
	info->rgbGains[0] = img->rawInfo.redGain;
	info->rgbGains[1] = img->rawInfo.greenGain;
	info->rgbGains[2] = img->rawInfo.blueGain;
	info->frameType = img->frameType;
	info->timeStamp = img->timeStamp;
	memcpy(&info->roadInfo,&img->roadInfo, sizeof(ImgRoadInfo));
	memcpy(info->capInfo, &img->capInfo, sizeof(img->capInfo));
	info->offset = sizeof(*info);
	info->magicNum = IMG_APPEND_MAGIC;
	
	return E_NO;
}

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

	/* add append data */
	add_append_data(msg, pOutBuf);

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
	
	/* check whether sd card has been inserted */
	int fd = open(DEV_SD0, O_RDONLY);
	if(fd < 0)
		return E_NOTEXIST;
	close(fd);

	/* only save jpeg and ignore continously frame */
	if(msg->dimension.colorSpace != FMT_JPG || 
		msg->rawInfo.capMode == RCI_CAP_TYPE_CONT) {
		//ERR("not a jpeg file");
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
	snprintf(fileName, sizeof(fileName), "%s/%02u_%02u_%03u.jpg", dirName, 
			capTime->minute, capTime->second, capTime->ms);

#ifdef NO_SAVE_FILE
	return E_NO;
#else
	err = write_file(dirName, fileName, (Int8 *)msg, sizeof(ImgMsg), data, msg->dimension.size);
#endif

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
EncoderHandle jpg_encoder_create(IN EncoderParams *encParams, IN UploadParams *uploadParams, IN pthread_mutex_t *mutex)
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
	encAttrs.poolBufNum = IMG_ENC_POOL_BUF_NUM;
	encAttrs.saveRootPath = SD0_MNT_PATH;
	encAttrs.mutex = mutex;
	encAttrs.encBufSize = IMG_MAX_WIDTH * IMG_MAX_HEIGHT * 8 / 10;

	/* display milisecod for jpeg img */
	encParams->osdInfo.flags |= CAM_OSD_FLAG_MILISEC_EN;

	hJpgEncoder = encoder_create(&encAttrs, encParams, uploadParams);
	if(!hJpgEncoder) {
		ERR("create jpg encoder error...");
		return NULL;
	}

	return hJpgEncoder;
}

