/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : img_convert.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/2/29
  Last Modified :
  Description   : image format convert and resize
  Function List :
              img_convert_control
              img_convert_exit
              img_convert_init
              img_convert_process
              img_convert_set_dyn_params
  History       :
  1.Date        : 2012/2/29
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "img_convert.h"
#include "previewer.h"
#include "resize.h"
#include "log.h"
#include <pthread.h>

/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/

/*----------------------------------------------*
 * external routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/
typedef struct _ImgConvObj {
	int 				fdPrev;
	int 				fdRsz;
	int 				outChanNum;
	ImgConvDynParams	dynParams;
	pthread_mutex_t		mutex;
	RszAttrs 			rszAttrs;
	Bool				largeFrame;
}ImgConvObj, *ImgConvHandle;

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
//#define RSZ_B_FOR_LARGE_IMG		0
/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/*****************************************************************************
 Prototype    : convert_previewer_attrs
 Description  : convert from dyn params to previewer attrs
 Input        : ImgConvDynParams *dynParams  
                PreviewAttrs *prevAttrs      
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/1
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static void convert_previewer_attrs(ImgConvDynParams *dynParams, PreviewAttrs *prevAttrs)
{
	prevAttrs->inputFmt = dynParams->inputFmt;
	prevAttrs->inputWidth = dynParams->inputWidth;
	prevAttrs->inputHeight = dynParams->inputHeight;
    prevAttrs->inputPitch = prevAttrs->inputWidth;
    prevAttrs->inputHStart = 0;
    prevAttrs->inputVStart = 0;
	prevAttrs->setRgbColorPallet = FALSE;
	prevAttrs->ctrlFlags = dynParams->ctrlFlags;
	prevAttrs->digiGain = dynParams->digiGain;
	prevAttrs->brightness = dynParams->brigtness;
	prevAttrs->contrast = dynParams->contrast;
	prevAttrs->gamma = dynParams->gamma;
	prevAttrs->sharpness = dynParams->sharpness;
	prevAttrs->saturation = dynParams->saturation;
}

/*****************************************************************************
 Prototype    : convert_rsz_attrs
 Description  : convert dyn params to resizer attrs
 Input        : ImgConvDynParams *dynParams  
                RszAttrs *rszAttrs           
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/5/15
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static void convert_rsz_attrs(ImgConvDynParams *dynParams, RszAttrs *rszAttrs)
{
	rszAttrs->inPitch = dynParams->inputWidth;
	rszAttrs->inHStart = 0;
    rszAttrs->inVStart = 1;
	rszAttrs->inFmt = dynParams->inputFmt;
	rszAttrs->inWidth = dynParams->inputWidth;
	rszAttrs->inHeight = dynParams->inputHeight;
	rszAttrs->isChained = TRUE;
	rszAttrs->outAttrs[0] = dynParams->outAttrs[0];
	rszAttrs->outAttrs[1] = dynParams->outAttrs[1];
}

/*****************************************************************************
 Prototype    : img_convert_set_dyn_params
 Description  : Set dyn params
 Input        : ImgConvHandle hConv          
                ImgConvDynParams *dynParams  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 img_convert_set_dyn_params(ImgConvHandle hConv, ImgConvDynParams *dynParams)
{
	if(!dynParams || dynParams->size != sizeof(ImgConvDynParams))
		return E_INVAL;

	if(hConv->fdRsz <= 0)
		return E_MODE;

	if( dynParams->inputFmt != FMT_BAYER_RGBG && 
		dynParams->inputFmt != FMT_YUV_422ILE && 
		dynParams->inputFmt != FMT_YUV_420SP) {
		ERR("unsupported input pixel format");
		return E_UNSUPT;
	}

	/* validate width alignment for input and output */
	if( dynParams->inputWidth > IPIPE_MAX_OUTPUT1_WIDTH_NORMAL || 
		dynParams->outAttrs[0].width > IPIPE_MAX_OUTPUT1_WIDTH_NORMAL ) {
		if( !ALIGNED(dynParams->inputWidth, CONV_LARGE_IN_WIDTH_ALIGN) || 
			!ALIGNED(dynParams->outAttrs[0].width, CONV_LARGE_OUT_WIDTH_ALIGN) ) {
			ERR("for large frame, input width must aligned to %d, out width must aligned to %d", 
				CONV_LARGE_IN_WIDTH_ALIGN, CONV_LARGE_OUT_WIDTH_ALIGN);
			return E_INVAL;
		}
		
		hConv->largeFrame = TRUE;
	} else {
		if( !ALIGNED(dynParams->inputWidth, CONV_IN_WIDTH_ALIGN) || 
			!ALIGNED(dynParams->outAttrs[0].width, CONV_OUT_WIDTH_ALIGN) ) {
			ERR("for normal frame, input width must aligned to %d, out width must aligned to %d", 
				CONV_IN_WIDTH_ALIGN, CONV_OUT_WIDTH_ALIGN);
			return E_INVAL;
		}
		
		hConv->largeFrame = FALSE;
	}

	if(dynParams->outAttrs[1].enbale && 
		(!ALIGNED(dynParams->outAttrs[1].width, 16))) {
		ERR("Out1 width must be multiply of 16");
		return E_INVAL;
	}

	Int32 		ret;
	RszAttrs 	rszAttrs;

	convert_rsz_attrs(dynParams, &rszAttrs);
	ret = resize_ss_config(hConv->fdRsz, &rszAttrs);
	if(ret)
		return ret;

	PreviewAttrs prevAttrs;
	convert_previewer_attrs(dynParams, &prevAttrs);
	ret = previewer_ss_config(hConv->fdPrev, &prevAttrs);
	ret |= previewer_cap_update(hConv->fdPrev, &prevAttrs);
	
	if(ret)
		return ret;
	
	hConv->dynParams = *dynParams;
	hConv->rszAttrs = rszAttrs;

	return E_NO;
}

/*****************************************************************************
 Prototype    : img_convert_set_rsz_out_attrs
 Description  : set resize output attrs
 Input        : ImgConvHandle hConv  
                ImgConvInArgs *args  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/17
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 img_convert_set_out_attrs(ImgConvHandle hConv, ImgConvInArgs *args)
{
	RszAttrs 	*rszAttrs = &hConv->rszAttrs;
	Int32		ret;

	if(memcmp(rszAttrs->outAttrs, args->outAttrs, sizeof(rszAttrs->outAttrs)) == 0) {
		return E_NO; //same params, need not config
	}

	if(!ALIGNED(args->outAttrs[0].width, 16) ) {
		ERR("Out0 width must be multiply of 16");
		return E_INVAL;
	}

	if(args->outAttrs[1].enbale && 
		(!ALIGNED(args->outAttrs[1].width, 16))) {
		ERR("Out1 width must be multiply of 16");
		return E_INVAL;
	}

	rszAttrs->outAttrs[0] = args->outAttrs[0];
	rszAttrs->outAttrs[1] = args->outAttrs[1];
	
	ret = resize_ss_config(hConv->fdRsz, rszAttrs);
	if(ret)
		return ret;

	
	/* record params */
	hConv->dynParams.outAttrs[0] = args->outAttrs[0];
	hConv->dynParams.outAttrs[1] = args->outAttrs[1];

	PreviewAttrs prevAttrs;
	convert_previewer_attrs(&hConv->dynParams, &prevAttrs);
	ret = previewer_ss_config(hConv->fdPrev, &prevAttrs);
	
	return ret;

}

/*****************************************************************************
 Prototype    : img_convert_init
 Description  : Init convert
 Input        : const Ptr init  
                const Ptr dyn   
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Ptr img_convert_init(const Ptr init, const Ptr dyn)
{
	ImgConvInitParams   *initParams = (ImgConvInitParams *)init;
	ImgConvDynParams	*dynParams = (ImgConvDynParams *)dyn;

	if(!init || !dynParams || dynParams->size != sizeof(ImgConvDynParams)) {
		ERR("Inv dyn params.");
		return NULL;
	}
	
	ImgConvHandle 	hConv;
	Int32 			ret;

	hConv = calloc(1, sizeof(ImgConvObj));
	if(!hConv) {
		ERR("Alloc mem failed.");
		return NULL;
	}

	hConv->dynParams = *dynParams;

	/* Init resizer */
	if(!initParams->rszDevName)
		initParams->rszDevName = RESIZER_DEVICE;
	hConv->fdRsz = resize_init(initParams->rszDevName, FALSE, dynParams->outAttrs);
	if(hConv->fdRsz < 0)
		goto err_quit;

	hConv->outChanNum = 1;
	if(dynParams->outAttrs[1].enbale)
		hConv->outChanNum++;

	/* Init previewer */
	PreviewAttrs prevAttrs;

	if(!initParams->prevDevName)
		initParams->prevDevName = PREVIEWER_DEVICE;
	
	convert_previewer_attrs(dynParams, &prevAttrs);
	
	hConv->fdPrev = previewer_init(initParams->prevDevName, FALSE, &prevAttrs);
	if(hConv->fdPrev < 0)
		goto err_quit;

	/* Set dynamic params */
	ret = img_convert_set_dyn_params(hConv, dynParams);
	if(ret < 0)
		goto err_quit;

	if(pthread_mutex_init(&hConv->mutex, NULL) < 0) {
		ERRSTR("init mutex failed");
		goto err_quit;
	}

	return hConv;

err_quit:

	if(hConv->fdRsz >= 0)
		close(hConv->fdRsz);

	if(hConv->fdPrev)
		close(hConv->fdPrev);

	free(hConv);

	return NULL;
	
}

/*****************************************************************************
 Prototype    : img_convert_exit
 Description  : exit module
 Input        : Ptr handle  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 img_convert_exit(Ptr handle)
{
	ImgConvHandle 	hConv = (ImgConvHandle)handle; 

	if(!hConv)
		return E_INVAL;
	
	if(hConv->fdRsz >= 0)
		close(hConv->fdRsz);

	if(hConv->fdPrev)
		close(hConv->fdPrev);

	pthread_mutex_destroy(&hConv->mutex);

	free(hConv);

	return E_NO;
}

/*****************************************************************************
 Prototype    : img_convert_process2
 Description  : run process twice for large images
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
  1.Date         : 2012/5/7
    Author       : Zhang
    Modification : Created function

*****************************************************************************/
static Int32 img_convert_large_frame(ImgConvHandle hConv, AlgBuf *inBuf, ImgConvInArgs *inArgs, AlgBuf *outBuf, ImgConvOutArgs *outArgs)
{
    Int32 				ret;
    ImgConvDynParams	*dynParams = &hConv->dynParams;
	
	if(FMT_YUV_422ILE == inArgs->outAttrs[0].pixFmt) {
		if(inArgs->outAttrs[0].width&0x1f) 
			ERR("output width not 32 aligned");
	} else {
		if(inArgs->outAttrs[0].width&0x3f) 
			ERR("output width not 64 aligned");
	}

#ifdef CONV_DBG
	DBG("img conv process2 %dx%d -> %dx%d", dynParams->inputWidth, dynParams->inputHeight,
		inArgs->outAttrs[0].width, inArgs->outAttrs[0].height);
#endif

#ifndef RSZ_B_FOR_LARGE_IMG
	if(inArgs->outAttrs[1].enbale) {
		ERR("can't resize using channel B.");
		return E_UNSUPT;
	}
#endif

	pthread_mutex_lock(&hConv->mutex);

	/* skip half of line for convert twice */	
	Uint32 	lineLength;
	if(FMT_YUV_422ILE == inArgs->outAttrs[0].pixFmt) {
		lineLength = ROUND_UP(dynParams->outAttrs[0].width * 2, 32);
	} else {
		lineLength = ROUND_UP(dynParams->outAttrs[0].width, 32);
	}

	RszAttrs rszAttrs;
	convert_rsz_attrs(dynParams, &rszAttrs);
	rszAttrs.inWidth = dynParams->inputWidth >> 1;
	rszAttrs.inVStart = 0;
	rszAttrs.outAttrs[0].lineLength = lineLength;
	rszAttrs.outAttrs[0].width /= 2;

	Bool enChanB = FALSE;
#ifdef RSZ_B_FOR_LARGE_IMG
	enChanB = inArgs->outAttrs[1].enbale;
	/* update params for channel 2 */
	if(enChanB) {
		/* skip half of line for convert twice */
		if(FMT_YUV_422ILE == inArgs->outAttrs[1].pixFmt) {
			rszAttrs.outAttrs[1].lineLength = ROUND_UP(dynParams->outAttrs[1].width * 2, 32);
		} else {
			rszAttrs.outAttrs[1].lineLength = ROUND_UP(dynParams->outAttrs[1].width, 32);
		}
		rszAttrs.outAttrs[1].width /= 2;
	}
#endif

    PreviewAttrs prevAttrs;
	convert_previewer_attrs(&hConv->dynParams, &prevAttrs);
	prevAttrs.inputPitch = dynParams->inputWidth;
	prevAttrs.inputHStart = 0;
	prevAttrs.inputVStart = 0;
	prevAttrs.inputWidth = dynParams->inputWidth >> 1;

	/* update params for single shot mode */
    ret = resize_ss_config(hConv->fdRsz, &rszAttrs);
	ret = previewer_ss_config(hConv->fdPrev, &prevAttrs);

	AlgBuf 	src;
    AlgBuf	dst[2];
	memcpy(&src, inBuf, sizeof(src));
	memcpy(&dst[0], &outBuf[0], sizeof(dst[0]));
#ifdef RSZ_B_FOR_LARGE_IMG
	if(enChanB)
		memcpy(&dst[1], &outBuf[1], sizeof(dst[1]));
#endif

	/* skip */
	if(rszAttrs.outAttrs[0].hFlip) {
		src.buf += dynParams->inputWidth/2;
	}
	

	/* convert for the left half of the image */	
	ret = previewer_convert(hConv->fdPrev, &src, dst, enChanB);
	if(ret) {
		goto exit;
	}

	memcpy(&src, inBuf, sizeof(src));
	memcpy(&dst[0], &outBuf[0], sizeof(dst[0]));
#ifdef RSZ_B_FOR_LARGE_IMG
	if(enChanB)
		memcpy(&dst[1], &outBuf[1], sizeof(dst[1]));
#endif
	if(!rszAttrs.outAttrs[0].hFlip) {
		src.buf += dynParams->inputWidth/2;
	}

	/* start from next half frame */
	dst[0].buf += lineLength/2;
#ifdef RSZ_B_FOR_LARGE_IMG
	if(enChanB)
		dst[1].buf += rszAttrs.outAttrs[1].lineLength/2;
#endif
	
	/* convert the right half of the image */
	ret = previewer_convert(hConv->fdPrev, &src, dst, enChanB);

exit:
	
	pthread_mutex_unlock(&hConv->mutex);
	return ret;
}

/*****************************************************************************
 Prototype    : img_convert_normal_frame
 Description  : Process normal frame
 Input        : ImgConvHandle hConv      
                ImgConvInArgs inArgs     
                ImgConvOutArgs *outArgs  
                AlgBuf *inBuf            
                AlgBuf *outBuf           
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 img_convert_normal_frame(ImgConvHandle hConv, AlgBuf *inBuf, ImgConvInArgs *inArgs, AlgBuf *outBuf, ImgConvOutArgs *outArgs)
{
	/* only convert once for normal frame */
	pthread_mutex_lock(&hConv->mutex);
	
	Int32 ret = img_convert_set_out_attrs(hConv, inArgs);
	if(ret) {
		goto exit;
	}

	Bool enChanB = inArgs->outAttrs[1].enbale;

	ret = previewer_convert(hConv->fdPrev, inBuf, outBuf, enChanB);

exit:
	
	pthread_mutex_unlock(&hConv->mutex);
	return ret;
}
/*****************************************************************************
 Prototype    : img_convert_process
 Description  : run process
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
  1.Date         : 2012/2/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 img_convert_process(Ptr algHandle, AlgBuf *inBuf, Ptr inArgsPtr, AlgBuf *outBuf, Ptr outArgsPtr)
{
	ImgConvHandle 	hConv = (ImgConvHandle)algHandle; 
	ImgConvInArgs	*inArgs = (ImgConvInArgs *)inArgsPtr;
	ImgConvOutArgs	*outArgs = (ImgConvOutArgs *)outArgsPtr; 

	if(!hConv || !inBuf || !outBuf || !inArgs || inArgs->size != sizeof(ImgConvInArgs))
		return E_INVAL;
/*
	if(!ALIGNED(inBuf->buf, 32) || !ALIGNED(outBuf->buf, 32)) {
		ERR("buf not aligned");
		return E_INVAL;
	}
*/
	Int32 ret;

	if( hConv->largeFrame ||
		inArgs->outAttrs[0].width > IPIPE_MAX_OUTPUT1_WIDTH_NORMAL) {
		/* convert large frames */
		ret = img_convert_large_frame(hConv, inBuf, inArgs, outBuf, outArgs);
	} else {
		/* convert normal frames */
		ret = img_convert_normal_frame(hConv, inBuf, inArgs, outBuf, outArgs);
	}
	
	return ret;
}


/*****************************************************************************
 Prototype    : img_convert_control
 Description  : do control
 Input        : Ptr algHandle  
                Int32 cmd      
                Ptr args       
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 img_convert_control(Ptr algHandle, Int32 cmd, Ptr args)
{
	ImgConvHandle 	hConv = (ImgConvHandle)algHandle; 
	Int32 			ret;

	if(!hConv)
		return E_INVAL;
	
	switch(cmd) {
	case CONV_CMD_SET_DYN_PARAMS:
		ret = img_convert_set_dyn_params(hConv, (ImgConvDynParams *)args);
		break;
	case CONV_CMD_GET_DYN_PARAMS:
		if(args) {
			memcpy(args, &hConv->dynParams, sizeof(ImgConvDynParams));
			ret = E_NO;
		} else 
			ret = E_INVAL;
		break;
	default:
		ret = E_UNSUPT;
		break;
	}

	return ret;
}

/* structure for alg functions */
const AlgFxns IMGCONV_ALG_FXNS = {
	.algInit = img_convert_init,
	.algProcess = img_convert_process,
	.algControl = img_convert_control,
	.algExit = img_convert_exit,
};


