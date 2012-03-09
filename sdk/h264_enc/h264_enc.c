/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : jpg_enc.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/2/1
  Last Modified :
  Description   : Jpeg encode module
  Function List :
              h264_enc_control
              h264_enc_exit
              h264_enc_init
              h264_enc_process
              h264_enc_set_dyn_params
  History       :
  1.Date        : 2012/2/1
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "h264_enc.h"
#include "codecs.h"
#include "log.h"
#include "cmem.h"
#include <ti/sdo/ce/video1/videnc1.h>
#include <ti/sdo/ce/visa.h>
#include "ih264venc.h"


/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/

/*----------------------------------------------*
 * external routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/
typedef struct _H264EncObj {
	VIDENC1_Handle			algHandle;		/* Internal Alg handle */
	IVIDENC1_Status 		status; 		/* Alg status */
	IH264VENC_DynamicParams	dynParams;		/* Dynamic params */
	IVIDEO1_BufDescIn		inBufDesc;		/* Input/Output Buffer Descriptor */ 
	//XDM_BufDesc				outBufDesc; 
	Int32					inBufOffset[3];	/* Offset for input buffer */
	Int32					inputChromaFmt;	/* Input chroma format */
}H264EncObj, *H264EncHandle;

/*----------------------------------------------*
 * project-wide global variables                *
 *----------------------------------------------*/

/*----------------------------------------------*
 * module-wide global variables                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * constants                                    *
 *----------------------------------------------*/
/* Default init params */
const H264EncInitParams H264ENC_INIT_DEFAULT = {
	.size = sizeof(H264EncInitParams),
	.maxWidth = 2048,
	.maxHeight = 2048,
	.maxFrameRate = 60,
	.maxBitRate = 5000000,
	.inputChromaFormat = FMT_YUV_420SP,
	.sliceFormat = 1,
};

/* Default dynamic params */
const H264EncDynParams H264ENC_DYN_DEFAULT = {
	.size = sizeof(H264EncDynParams),
	.width = 1280,
	.height = 720,
	.frameRate = 25,
	.targetBitRate = 2000000,
	.intraFrameInterval = 30,
	.interFrameInterval = 30,
	.forceFrame = VID_NA_FRAME,
	.intraFrameQP = 28,
	.interFrameQP = 28,
	.initQP = 28,
	.maxQP = 50,
	.minQP = 1,
	.rateCtrlMode = H264_RC_CVBR,
	.maxDelay = 2000,
	.enablePicTimSEI = 0,
	.idrFrameInterval = 300,
	.maxBitrateCVBR = 4000000,
	.maxHighCmpxIntCVBR = 10,
};

/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/
#define FRAME_SIZE_ALIGN	16

//#define H264ENC_DBG

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/*****************************************************************************
 Prototype    : h264_enc_set_dyn_params
 Description  : Set dynamic params for h264 encode
 Input        : H264EncHandle hH264Enc        
                H264EncDynParams *dynParams  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/1
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 h264_enc_set_dyn_params(H264EncHandle hH264Enc, H264EncDynParams *dynParams)
{		
	/** Set up dynamic parameters (can be changed before each call to jpeg processing)
	  * Parameters in structure dynamicParams are default image encode parameters required by XDM
	  */
	if(!dynParams || dynParams->size != sizeof(H264EncDynParams))
		return E_INVAL;
	
	IH264VENC_DynamicParams *pH264EncDynParams = &hH264Enc->dynParams;

	pH264EncDynParams->videncDynamicParams.inputWidth 			= dynParams->width;
	pH264EncDynParams->videncDynamicParams.inputHeight 			= dynParams->height;
	pH264EncDynParams->videncDynamicParams.refFrameRate 		= dynParams->frameRate * 1000;
	pH264EncDynParams->videncDynamicParams.targetFrameRate 		= 
		pH264EncDynParams->videncDynamicParams.refFrameRate;
	pH264EncDynParams->videncDynamicParams.targetBitRate 		= dynParams->targetBitRate;
	pH264EncDynParams->videncDynamicParams.intraFrameInterval 	= dynParams->intraFrameInterval;
	//pH264EncDynParams->videncDynamicParams.interFrameInterval 	= 0;
	pH264EncDynParams->videncDynamicParams.forceFrame 			= dynParams->forceFrame;

	pH264EncDynParams->intraFrameQP								= dynParams->intraFrameQP;
	pH264EncDynParams->interPFrameQP							= dynParams->interFrameQP;
	pH264EncDynParams->initQ									= dynParams->initQP;
	pH264EncDynParams->rcQMax									= dynParams->maxQP;
	pH264EncDynParams->rcQMin									= dynParams->minQP;
	pH264EncDynParams->rcAlgo									= dynParams->rateCtrlMode;
	pH264EncDynParams->maxDelay									= dynParams->maxDelay;
	pH264EncDynParams->enablePicTimSEI							= dynParams->enablePicTimSEI;
	pH264EncDynParams->enableBufSEI								= dynParams->enablePicTimSEI;
	pH264EncDynParams->idrFrameInterval							= dynParams->idrFrameInterval;
	pH264EncDynParams->maxBitrateCVBR							= dynParams->maxBitrateCVBR;
	pH264EncDynParams->maxHighCmpxIntCVBR						= dynParams->maxHighCmpxIntCVBR;
	
	Int32 status;

	status = VIDENC1_control(hH264Enc->algHandle, 
							XDM_SETPARAMS, 
							(VIDENC1_DynamicParams *)pH264EncDynParams,
							&hH264Enc->status);

	if(status == XDM_EFAIL) {
		ERR("set dynamic params failed: 0x%X", (unsigned int)hH264Enc->status.extendedError);
		return E_IO;
	}

	/* Get buffer info */
	status = VIDENC1_control(hH264Enc->algHandle,
                         	XDM_GETBUFINFO,
                         	(VIDENC1_DynamicParams *)pH264EncDynParams,
                         	&hH264Enc->status);

	if(status == XDM_EFAIL) {
		ERR("get buf info failed: %d", (int)status);
		return E_IO;
	}

	/* Init buffer descriptor */
	if(!ALIGNED(dynParams->width, FRAME_SIZE_ALIGN) || 
		!ALIGNED(dynParams->height, FRAME_SIZE_ALIGN))
		WARN("frame width %d, height: %d not multiple of %d", 
				(int)dynParams->width, (int)dynParams->height, FRAME_SIZE_ALIGN);
	
	hH264Enc->inBufDesc.numBufs = hH264Enc->status.bufInfo.minNumInBufs;
	hH264Enc->inBufDesc.frameWidth = ROUND_UP(dynParams->width, FRAME_SIZE_ALIGN);
	hH264Enc->inBufDesc.frameHeight = ROUND_UP(dynParams->height, FRAME_SIZE_ALIGN);
	hH264Enc->inBufDesc.framePitch = hH264Enc->inBufDesc.frameWidth;
	//hH264Enc->outBufDesc.numBufs = hH264Enc->status.bufInfo.minNumOutBufs;
#ifdef H264ENC_DBG
	DBG("frame width: %d, frame height: %d, pitch: %d", 
			hH264Enc->inBufDesc.frameWidth, hH264Enc->inBufDesc.frameHeight, 
			hH264Enc->inBufDesc.framePitch);
	DBG("buf num: %d, min size: %d, %d", hH264Enc->inBufDesc.numBufs,
			hH264Enc->status.bufInfo.minInBufSize[0], 
			hH264Enc->status.bufInfo.minInBufSize[1]);
#endif

	/* Set up buffer size, so we need not set them when process */
	int i;
	for(i = 0; i < hH264Enc->inBufDesc.numBufs; i++) {
		hH264Enc->inBufDesc.bufDesc[i].bufSize = hH264Enc->status.bufInfo.minInBufSize[i];
	}

	/* Set in buffer offset */
	hH264Enc->inBufOffset[0] = 0;
	hH264Enc->inBufOffset[1] = dynParams->height * dynParams->width;
	if(hH264Enc->inputChromaFmt == FMT_YUV_420P) {
		hH264Enc->inBufOffset[2] = dynParams->height * dynParams->width >> 2;
	} else if(hH264Enc->inputChromaFmt == FMT_YUV_422P) {
		hH264Enc->inBufOffset[2] = dynParams->height * dynParams->width >> 1;
	}

	return E_NO;
}


/*****************************************************************************
 Prototype    : jpg_enc_init
 Description  : init alg module
 Input        : Ptr *init  
                Ptr *dyn   
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/1
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Ptr h264_enc_init(const Ptr init, const Ptr dyn)
{
	H264EncInitParams *initParams = (H264EncInitParams *)init;

	if(!init || initParams->size != sizeof(H264EncInitParams))
		return NULL;

	/* Alloc mem for handle */
	VIDENC1_Handle 	handle = NULL;
	H264EncHandle   hH264Enc = calloc(1, sizeof(H264EncObj));
	if(!hH264Enc)
		goto exit;
	
	/* Init params */
	IH264VENC_Params 	h264EncParams = IH264VENC_PARAMS;

	h264EncParams.videncParams.encodingPreset = XDM_USER_DEFINED;
	h264EncParams.videncParams.rateControlPreset = IVIDEO_STORAGE;
	h264EncParams.videncParams.maxHeight = initParams->maxHeight;
	h264EncParams.videncParams.maxWidth = initParams->maxWidth;
	h264EncParams.videncParams.maxFrameRate = initParams->maxFrameRate * 1000;
	h264EncParams.videncParams.maxBitRate = initParams->maxBitRate;
	h264EncParams.videncParams.maxInterFrameInterval = 1;
	h264EncParams.videncParams.dataEndianness = XDM_BYTE;
	h264EncParams.videncParams.inputChromaFormat = initParams->inputChromaFormat;
	h264EncParams.videncParams.inputContentType = IVIDEO_PROGRESSIVE;
	h264EncParams.videncParams.reconChromaFormat = XDM_YUV_420SP;
	h264EncParams.sliceFormat = initParams->sliceFormat;

	/* Create  alg instance */
	handle = VIDENC1_create(g_hEngine, 
							H264ENC_NAME,
                       		(IVIDENC1_Params *)&h264EncParams);
	if(!handle) {
        ERR("Failed to Create Instance...");
        goto exit;
    }

	/* Set fields of the handle */
	hH264Enc->algHandle = handle;
	hH264Enc->status.size = sizeof(IVIDENC1_Status);
	hH264Enc->status.data.buf = NULL;
	hH264Enc->dynParams = H264VENC_TI_IH264VENC_DYNAMICPARAMS;
	hH264Enc->inputChromaFmt = initParams->inputChromaFormat;

	/* Set dynamic params */
	int err = h264_enc_set_dyn_params(hH264Enc, (H264EncDynParams *)dyn);
	if(err) {
		ERR("Failed to set dyn params.");
		goto exit;
	}

	return hH264Enc;
    
exit:

	if(handle)
		VIDENC1_delete(handle);

	if(hH264Enc)
		free(hH264Enc);
	return NULL;
}


/*****************************************************************************
 Prototype    : h264_enc_process
 Description  : Run alg for h264 encode
 Input        : Ptr algHandle   
                AlgBuf *inBuf   
                Ptr inArgsPtr   
                AlgBuf *outBuf  
                Ptr outArgsPtr  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/1
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 h264_enc_process(Ptr algHandle, AlgBuf *inBuf, Ptr inArgsPtr, AlgBuf *outBuf, Ptr outArgsPtr)
{
	H264EncHandle hH264Enc = (H264EncHandle)algHandle;
	
	if(!algHandle || !hH264Enc->algHandle || !inBuf || !outBuf || !inArgsPtr || !outArgsPtr)
		return E_INVAL;

	H264EncInArgs 	*inArgs = (H264EncInArgs *)inArgsPtr;
	H264EncOutArgs	*outArgs =(H264EncOutArgs *)outArgsPtr;
	
	if(inArgs->size != sizeof(H264EncInArgs) || outArgs->size != sizeof(H264EncOutArgs)) {
		ERR("Invaild in/out args size.");
		return E_INVAL;
	}

	/* Setup input and output args */
	IH264VENC_InArgs	inEncArgs;
	VIDENC1_OutArgs 	outEncArgs;

	memset(&inEncArgs, 0, sizeof(inEncArgs));
	inEncArgs.videncInArgs.size = sizeof(inEncArgs);
	inEncArgs.videncInArgs.inputID = inArgs->inputID;
	inEncArgs.videncInArgs.topFieldFirstFlag = 1;
	inEncArgs.timeStamp = inArgs->timeStamp;

	memset(&outEncArgs, 0, sizeof(outEncArgs));
	outEncArgs.size = sizeof(outEncArgs);

	/* Setup buffer descriptors */
	XDM_BufDesc	outBufDesc;
	XDAS_Int32  outBufSizeArray[1];
	XDAS_Int8	*outBufArray[1];
	
	hH264Enc->inBufDesc.bufDesc[0].buf = (XDAS_Int8 *)(inBuf->buf);
	if(hH264Enc->inputChromaFmt == FMT_YUV_420SP) {
		hH264Enc->inBufDesc.bufDesc[1].buf = 
			(XDAS_Int8 *)(inBuf->buf + hH264Enc->inBufOffset[1]);
	} else {
		ERR("unsupported input chroma format: %d", (int)hH264Enc->inputChromaFmt);
		return E_INVAL;
	}

	outBufSizeArray[0] = outBuf->bufSize;
	outBufArray[0] = (XDAS_Int8 *)outBuf->buf;
	outBufDesc.numBufs = 1;
	outBufDesc.bufs = outBufArray;
	outBufDesc.bufSizes = outBufSizeArray;
	
	Int32 status;
	status = VIDENC1_process(hH264Enc->algHandle, 
							&hH264Enc->inBufDesc, 
							&outBufDesc,
							(VIDENC1_InArgs *)&inEncArgs, 
							&outEncArgs);

	if(status != VIDENC1_EOK) {
		ERR("process error: %d", (int)status);
		return E_IO;
	}

	/* Fill out args */
	outArgs->bytesGenerated = outEncArgs.bytesGenerated;
	outArgs->frameType = outEncArgs.encodedFrameType;
	outArgs->outputID = outEncArgs.outputID;
	
	return E_NO;
}

/*****************************************************************************
 Prototype    : h264_enc_control
 Description  : set and get params for alg
 Input        : Ptr algHandle  
                Int32 cmd      
                Ptr args       
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/1
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 h264_enc_control(Ptr algHandle, Int32 cmd, Ptr args)
{
	H264EncHandle hH264Enc = (H264EncHandle)algHandle;
	
	if(!algHandle || !hH264Enc->algHandle)
		return E_INVAL;

	Int32 err;
	
	switch(cmd) {
	case ALG_CMD_SET_DYN_PARAMS:
		err = h264_enc_set_dyn_params(hH264Enc, (H264EncDynParams *)args);
		break;
	default:
		err = E_UNSUPT;
		break;
	}

	return err;
}

/*****************************************************************************
 Prototype    : h264_enc_exit
 Description  : free resources of this alg
 Input        : Ptr algHandle  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/1
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 h264_enc_exit(Ptr algHandle)
{
	H264EncHandle hH264Enc = (H264EncHandle)algHandle;

	if(!hH264Enc || !hH264Enc->algHandle)
		return E_INVAL;

	VIDENC1_delete(hH264Enc->algHandle);
	free(hH264Enc);

	return E_NO;
}

/* structure for alg functions */
const AlgFxns H264ENC_ALG_FXNS = {
	.algInit = h264_enc_init,
	.algProcess = h264_enc_process,
	.algControl = h264_enc_control,
	.algExit = h264_enc_exit,
};

