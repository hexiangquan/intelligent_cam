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
              jpg_append_data
              jpg_enc_control
              jpg_enc_exit
              jpg_enc_init
              jpg_enc_process
              jpg_enc_set_dyn_params
              rman_exit
              rman_init
  History       :
  1.Date        : 2012/2/1
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "ijpegenc.h"
#include "jpg_enc.h"
#include "codecs.h"
#include "log.h"
#include "cmem.h"
#include <ti/sdo/ce/image1/imgenc1.h>
#include <ti/sdo/ce/visa.h>


/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/

/*----------------------------------------------*
 * external routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/
typedef struct _JpgEncObj {
	IMGENC1_Handle			algHandle;	/* Internal Alg handle */
	IMGENC1_Status 			status; 	/* Alg status */
	IJPEGENC_DynamicParams	dynParams;	/* Dynamic params */
	XDM1_BufDesc			inBufDesc;	/* Input/Output Buffer Descriptor */ 
	XDM1_BufDesc			outBufDesc; 
}JpgEncObj, *JpgEncodeHandle;

/*----------------------------------------------*
 * project-wide global variables                *
 *----------------------------------------------*/

/*----------------------------------------------*
 * module-wide global variables                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * constants                                    *
 *----------------------------------------------*/

/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/
#define MAX_QUALITY			97
#define MAX_WIDTH			16384
#define MAX_HEIGHT			16384

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/
#if 0
static XDAS_Int32 _IMGENC1_control(IMGENC1_Handle handle, IIMGENC1_Cmd id,
    IMGENC1_DynamicParams *dynParams, IMGENC1_Status *status)
{
    XDAS_Int32 retVal = IMGENC1_EFAIL;

	assert(handle);
  
    if (handle) {
        IIMGENC1_Fxns *fxns =
            (IIMGENC1_Fxns *)VISA_getAlgFxns((VISA_Handle)handle);
        IIMGENC1_Handle alg = VISA_getAlgHandle((VISA_Handle)handle);

		if(fxns == &JPEGENC_TI_IJPEGENC)
			DBG("Using jpeg codecs");

        if ((fxns != NULL) && (alg != NULL) && (fxns->control != NULL)) {
            VISA_enter((VISA_Handle)handle);
            retVal = fxns->control(alg, id, dynParams, status);
			DBG("retVal: %d, err: %d", retVal, status->extendedError);
            VISA_exit((VISA_Handle)handle);
        }
    }

    return (retVal);
}
#endif

/*****************************************************************************
 Prototype    : jpg_enc_set_dyn_params
 Description  : Set dynamic params for jpg encode
 Input        : JpgEncodeHandle hJpgEnc        
                JpgEncDynParams *dynParams  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/1
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 jpg_enc_set_dyn_params(JpgEncodeHandle hJpgEnc, JpgEncDynParams *dynParams)
{		
	/** Set up dynamic parameters (can be changed before each call to jpeg processing)
	  * Parameters in structure dynamicParams are default image encode parameters required by XDM
	  */
	if(!dynParams || dynParams->size != sizeof(JpgEncDynParams))
		return E_INVAL;
	
	IJPEGENC_DynamicParams *jpgEncDynParams = &hJpgEnc->dynParams;

	jpgEncDynParams->imgencDynamicParams.inputChromaFormat = dynParams->inputFormat;
	jpgEncDynParams->imgencDynamicParams.qValue = dynParams->quality;
	jpgEncDynParams->imgencDynamicParams.inputWidth = dynParams->width;
	jpgEncDynParams->imgencDynamicParams.inputHeight = dynParams->height;
	jpgEncDynParams->rotation = dynParams->rotation;
	jpgEncDynParams->imgencDynamicParams.size = sizeof(IJPEGENC_DynamicParams);

	Int32 status;

	status = IMGENC1_control(hJpgEnc->algHandle, 
							XDM_SETPARAMS, 
							(IIMGENC1_DynamicParams *)jpgEncDynParams,
							(IIMGENC1_Status *)&hJpgEnc->status);

	if(status == XDM_EFAIL) {
		ERR("set dynamic params failed: %d", (int)hJpgEnc->status.extendedError);
		return E_IO;
	}

	/* Get buffer info */
	status = IMGENC1_control(hJpgEnc->algHandle,
                         	XDM_GETBUFINFO,
                         	(IIMGENC1_DynamicParams *)jpgEncDynParams,
                         	(IIMGENC1_Status *)&hJpgEnc->status);

	if(status == XDM_EFAIL) {
		ERR("get buf info failed: %d", (int)status);
		return E_IO;
	}

	/* Init buffer descriptor */
	hJpgEnc->inBufDesc.numBufs = hJpgEnc->status.bufInfo.minNumInBufs;
	hJpgEnc->outBufDesc.numBufs = hJpgEnc->status.bufInfo.minNumOutBufs;

	//DBG("jpg enc, min in buf num: %d, min out buf num: %d", 
		//hJpgEnc->inBufDesc.numBufs, hJpgEnc->outBufDesc.numBufs);

	/* Set up buffer size, so we need not set them when process */
	int i;
	for(i = 0; i < hJpgEnc->inBufDesc.numBufs; i++) {
		hJpgEnc->inBufDesc.descs[i].bufSize = hJpgEnc->status.bufInfo.minInBufSize[i];
	}

	//for(i = 0; i < hJpgEnc->outBufDesc.numBufs; i++) {
		//hJpgEnc->outBufDesc.descs[i].bufSize = hJpgEnc->status.bufInfo.minOutBufSize[i];
	//}
	hJpgEnc->outBufDesc.descs[0].bufSize = hJpgEnc->status.bufInfo.minOutBufSize[0];

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
static Ptr jpg_encode_init(const Ptr init, const Ptr dyn)
{
	JpgEncInitParams *initParams = (JpgEncInitParams *)init;

	if(!init || initParams->size != sizeof(JpgEncInitParams))
		return NULL;

	/* Init params */
	IIMGENC1_Params imgEncParams;
	IMGENC1_Handle 	handle = NULL;
	JpgEncodeHandle    hJpgEnc = calloc(1, sizeof(JpgEncObj));
	if(!hJpgEnc)
		goto exit;
	
	imgEncParams.maxHeight = initParams->maxHeight;
	imgEncParams.maxWidth = initParams->maxWidth;
	imgEncParams.maxScans = 0;
	imgEncParams.dataEndianness = XDM_BYTE;
	imgEncParams.forceChromaFormat = XDM_YUV_420P;
	imgEncParams.size = sizeof(imgEncParams);

	/* Create  alg instance */
	handle = IMGENC1_create(g_hEngine, 
							JPGENC_NAME,
                       		&imgEncParams);
	if(!handle) {
        ERR("Failed to Create Instance...");
        goto exit;
    }

	/* Set fields of the handle */
	hJpgEnc->algHandle = handle;
	hJpgEnc->status.size = sizeof(IJPEGENC_Status);

	/* Set dynamic params */
	int err = jpg_enc_set_dyn_params(hJpgEnc, (JpgEncDynParams *)dyn);
	if(err) {
		ERR("Failed to set dyn params.");
		goto exit;
	}

	return hJpgEnc;
    
exit:

	if(handle)
		IMGENC1_delete(handle);

	if(hJpgEnc)
		free(hJpgEnc);
	return NULL;
}

/*****************************************************************************
 Prototype    : jpg_append_data
 Description  : Append user data
 Input        : AlgBuf *outBuf        
                Int32 bytesGenerated  
                JpgEncInArgs *inArgs  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/1
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline Int32 jpg_append_data(AlgBuf *outBuf, Int32 bytesGenerated, JpgEncInArgs *inArgs)
{
	/* Round up to 4 bytes */
	Int32 dataLen = ROUND_UP(bytesGenerated, sizeof(Int32));

	/* Check buf size */
	inArgs->appendSize = ROUND_UP(inArgs->appendSize, sizeof(Int32));
	if(outBuf->bufSize < dataLen + inArgs->appendSize + 8)
		return bytesGenerated;

	/* Copy data */
	memcpy(outBuf->buf + dataLen, inArgs->appendData, inArgs->appendSize);

	/* Set size of append data */
	dataLen += inArgs->appendSize;
	*(Int32 *)(outBuf->buf + dataLen) = inArgs->appendSize;

	/* Copy end mark */
	dataLen += sizeof(Int32);
	*(Uint32 *)(outBuf->buf + dataLen) = inArgs->endMark;

	/* Return data size after data appending */
	return dataLen + sizeof(Uint32);
}

/*****************************************************************************
 Prototype    : jpg_enc_process
 Description  : Run alg for jpeg encode
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
static Int32 jpg_encode_process(Ptr algHandle, AlgBuf *inBuf, Ptr inArgsPtr, AlgBuf *outBuf, Ptr outArgsPtr)
{
	JpgEncodeHandle hJpgEnc = (JpgEncodeHandle)algHandle;
	
	if(!algHandle || !hJpgEnc->algHandle || !inBuf || !outBuf || !inArgsPtr || !outArgsPtr)
		return E_INVAL;

	JpgEncInArgs 	*inArgs = (JpgEncInArgs *)inArgsPtr;
	JpgEncOutArgs	*outArgs =(JpgEncOutArgs *)outArgsPtr;
	
	if(inArgs->size != sizeof(JpgEncInArgs) || outArgs->size != sizeof(JpgEncOutArgs)) {
		ERR("Invaild in/out args size.");
		return E_INVAL;
	}

	/* Setup input and output args */
	IIMGENC1_InArgs		inEncArgs;
	IIMGENC1_OutArgs 	outEncArgs;

	inEncArgs.size = sizeof(IIMGENC1_InArgs);
	outEncArgs.size = sizeof(IIMGENC1_OutArgs);
	outEncArgs.bytesGenerated = 0;
	outEncArgs.currentAU = 0;
	outEncArgs.extendedError = 0;

	/* Setup buffer descriptors */
	Int32 i, offset = 0;
	
	for(i = 0; i < hJpgEnc->inBufDesc.numBufs; i++) {
		hJpgEnc->inBufDesc.descs[i].buf = (XDAS_Int8 *)(inBuf->buf + offset);
		offset +=  hJpgEnc->status.bufInfo.minInBufSize[i];
	}

	hJpgEnc->outBufDesc.descs[0].buf = (XDAS_Int8 *)(outBuf->buf);

	Int32 status;
	status = IMGENC1_process(hJpgEnc->algHandle, 
							&hJpgEnc->inBufDesc, 
							&hJpgEnc->outBufDesc,
							&inEncArgs, &outEncArgs);

	if(status != IIMGENC1_EOK) {
		ERR("process error: %d", (int)status);
		return E_IO;
	}

	/* Fill out args */
	if(inArgs->appendData) {
		/* Append data to the end of jpeg image */
		outArgs->bytesGenerated = jpg_append_data(outBuf, outEncArgs.bytesGenerated, inArgs);
	} else {
		outArgs->bytesGenerated = outEncArgs.bytesGenerated;
	}

	return E_NO;
	
}

/*****************************************************************************
 Prototype    : jpg_enc_control
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
static Int32 jpg_encode_control(Ptr algHandle, Int32 cmd, Ptr args)
{
	JpgEncodeHandle hJpgEnc = (JpgEncodeHandle)algHandle;
	
	if(!algHandle || !hJpgEnc->algHandle)
		return E_INVAL;

	Int32 err;
	
	switch(cmd) {
	case JPGENC_CMD_SET_DYN_PARAMS:
		err = jpg_enc_set_dyn_params(hJpgEnc, (JpgEncDynParams *)args);
		break;
	default:
		err = E_UNSUPT;
		break;
	}

	return err;
}

/*****************************************************************************
 Prototype    : jpg_enc_exit
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
static Int32 jpg_encode_exit(Ptr algHandle)
{
	JpgEncodeHandle hJpgEnc = (JpgEncodeHandle)algHandle;

	if(!hJpgEnc || !hJpgEnc->algHandle)
		return E_INVAL;

	IMGENC1_delete(hJpgEnc->algHandle);
	free(hJpgEnc);

	return E_NO;
}

/* structure for alg functions */
const AlgFxns JPGENC_ALG_FXNS = {
	.algInit = jpg_encode_init,
	.algProcess = jpg_encode_process,
	.algControl = jpg_encode_control,
	.algExit = jpg_encode_exit,
};

