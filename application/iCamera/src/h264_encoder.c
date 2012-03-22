#include "h264_enc.h"
#include "h264_encoder.h"
#include "log.h"
#include "app_msg.h"


/*****************************************************************************
 Prototype    : h264_encoder_process
 Description  : do h.264 encode
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
static Int32 h264_encoder_process(IN AlgHandle hEncode, IN AlgBuf *pInBuf, OUT AlgBuf *pOutBuf, INOUT ImgMsg *msg)
{
	H264EncInArgs 	inArgs;
	H264EncOutArgs 	outArgs;
	Int32			ret;

	inArgs.inputID = msg->index;
	inArgs.timeStamp = 0;
	inArgs.size = sizeof(inArgs);

	outArgs.size = sizeof(outArgs);
	outArgs.bytesGenerated = 0;

	ret = alg_process(hEncode, pInBuf, &inArgs, pOutBuf, &outArgs);
	if(ret) {
		ERR("encode err");
		return ret;
	}

	/* fill some filed of msg */
	msg->dimension.colorSpace = FMT_H264;
	msg->dimension.size = outArgs.bytesGenerated;
	msg->frameType = outArgs.frameType;

	return E_NO;
}

/* operation fxns for encoder register */
static const EncoderOps c_h264EncOps = {
	.encProcess = h264_encoder_process,
	.saveFrame = NULL,
};

/*****************************************************************************
 Prototype    : h264_encoder_create
 Description  : create h.264 encoder obj
 Input        : IN ParamsMngHandle hParamsMng  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/20
    Author       : Sun
    Modification : Created function

*****************************************************************************/
EncoderHandle h264_encoder_create(IN ParamsMngHandle hParamsMng, IN pthread_mutex_t *mutex)
{
	EncoderAttrs		encAttrs;
	H264EncInitParams	encInitParams;
	EncoderHandle		hEncoder;

	encInitParams = H264ENC_INIT_DEFAULT;
	encInitParams.maxWidth = 1952;
	encInitParams.maxHeight = 1088;
	encInitParams.maxFrameRate = 30;
	
	encAttrs.name = VID_ENC_NAME;
	encAttrs.msgName = MSG_VID_ENC;
	encAttrs.dstName = NULL;
	encAttrs.encFxns = &H264ENC_ALG_FXNS;
	encAttrs.encInitParams = &encInitParams;
	encAttrs.encOps = &c_h264EncOps;
	encAttrs.hParamsMng = hParamsMng;
	encAttrs.poolBufNum = VID_ENC_POOL_BUF_NUM;
	encAttrs.saveRootPath = SD_MNT_PATH;
	encAttrs.cmdGetEncDyn = PMCMD_G_H264ENCDYN;
	encAttrs.cmdGetOsdDyn = PMCMD_G_VIDOSDDYN;
	encAttrs.cmdGetOsdInfo = PMCMD_G_VIDOSDINFO;
	encAttrs.cmdGetUploadProto = PMCMD_G_VIDUPLOADPROTO;
	encAttrs.encBufSize = encInitParams.maxWidth * encInitParams.maxHeight * 8 / 10;
	encAttrs.mutex = mutex;

	hEncoder = encoder_create(&encAttrs);
	if(!hEncoder) {
		ERR("create h264 encoder error...");
		return NULL;
	}

	return hEncoder;
}


