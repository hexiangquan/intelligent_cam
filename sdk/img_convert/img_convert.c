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
	prevAttrs->setRgbColorPallet = FALSE;
	prevAttrs->ctrlFlags = dynParams->ctrlFlags;
	prevAttrs->digiGain = dynParams->digiGain;
	prevAttrs->brightness = dynParams->brigtness;
	prevAttrs->contrast = dynParams->contrast;
	prevAttrs->eeTable = dynParams->eeTable;
	prevAttrs->eeTabSize = dynParams->eeTabSize;
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

	if(dynParams->inputFmt != FMT_BAYER_RGBG && 
		dynParams->inputFmt != FMT_YUV_422ILE && 
		dynParams->inputFmt != FMT_YUV_420SP) {
		ERR("unsupported input pixel format");
		return E_UNSUPT;
	}

	Int32 ret;
	RszAttrs rszAttrs;
	
	rszAttrs.inFmt = dynParams->inputFmt;
	rszAttrs.inWidth = dynParams->inputWidth;
	rszAttrs.inHeight = dynParams->inputHeight;
	rszAttrs.isChained = TRUE;
	rszAttrs.outAttrs[0] = dynParams->outAttrs[0];
	rszAttrs.outAttrs[1] = dynParams->outAttrs[1];

	if(!ALIGNED(dynParams->outAttrs[0].width, 16) ) {
		ERR("Out0 width must be multiply of 16");
		return E_INVAL;
	}

	if(dynParams->outAttrs[1].enbale && 
		(!ALIGNED(dynParams->outAttrs[1].width, 16))) {
		ERR("Out1 width must be multiply of 16");
		return E_INVAL;
	}

	ret = resize_ss_config(hConv->fdRsz, &rszAttrs);
	if(ret)
		return ret;

	PreviewAttrs prevAttrs;
	convert_previewer_attrs(dynParams, &prevAttrs);
	ret = previewer_ss_config(hConv->fdPrev, &prevAttrs);
	if(ret)
		return ret;

	hConv->dynParams = *dynParams;

	return E_NO;
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

	free(hConv);

	return E_NO;
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

	if(!hConv || !inBuf || !outBuf)
		return E_INVAL;

	if(!ALIGNED(inBuf->buf, 32) || !ALIGNED(outBuf->buf, 32)) {
		ERR("buf not aligned");
		return E_INVAL;
	}
	
	return previewer_convert(hConv->fdPrev, inBuf, outBuf);
}

/*****************************************************************************
 Prototype    : img_convert_set_out_attrs
 Description  : set output attrs
 Input        : ImgConvHandle hConv     
                ConvOutAttrs *outAttrs  
                Int32 chanNum           
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/7
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 img_convert_set_out_attrs(ImgConvHandle hConv, ConvOutAttrs *outAttrs, Int32 chanNum)
{
	if(!outAttrs || chanNum >= CONV_MAX_OUT_CHAN_NUM)
		return E_INVAL;

	if(chanNum == 0 && !outAttrs->enbale) {
		ERR("out chan0 must enable");
		return E_INVAL;
	}

	if(outAttrs->enbale && !ALIGNED(outAttrs->width, 16)) {
		ERR("width must align to 16");
		return E_INVAL;
	}

	hConv->dynParams.outAttrs[chanNum] = *outAttrs;
	return img_convert_set_dyn_params(hConv, &hConv->dynParams);
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
	case CONV_CMD_SET_OUT0_ATTRS:
		ret = img_convert_set_out_attrs(hConv, (ConvOutAttrs *)args, 0);
		break;
	case CONV_CMD_SET_OUT1_ATTRS:
		ret = img_convert_set_out_attrs(hConv, (ConvOutAttrs *)args, 1);
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


