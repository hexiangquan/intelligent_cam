/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : converter.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/19
  Last Modified :
  Description   : image format convert and adjust module
  Function List :
              converter_create
              converter_delete
              converter_params_update
              converter_workmode_update
  History       :
  1.Date        : 2012/3/19
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "converter.h"
#include "img_convert.h"
#include "log.h"
#include "app_msg.h"
#include "buffer.h"
#include "display.h"
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
#define CONV_MAX_STREAM_NUM		2
#define CONVERTER_FLAG_EN		(1 << 16)

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/* private data for converter */
struct ConverterObj {
	AlgHandle			hImgConv;
	ImgConvInArgs		convInArgs[CONV_MAX_STREAM_NUM];
	ImgDimension		outDim[CONV_MAX_STREAM_NUM];
	BufPoolHandle		hBufPool;
	Int32				flags;
	DisplayHanlde		hDisplay;
	CamAVParam			avParams;
};

/*****************************************************************************
 Prototype    : converter_display_cfg
 Description  : cfg display module
 Input        : ConverterHandle hConverter  
                CamAVParam *params          
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 converter_display_cfg(ConverterHandle hConverter, CamAVParam *params)
{
	//DBG("converter display cfg, type: %d", params->avType);
	
	if(params->avType == AV_TYPE_NONE) {
		/* stop display, delete this object */
		display_delete(hConverter->hDisplay);
		hConverter->hDisplay = NULL;	
		/* disable rsz B out */
		hConverter->convInArgs[0].outAttrs[1].enbale = FALSE;
		hConverter->convInArgs[1].outAttrs[1].enbale = FALSE;
		//DBG("delete display handle");
		return E_NO;
	}
	
	DisplayAttrs attrs;
	attrs.chanId = 0;
	attrs.mode = (params->avType == AV_TYPE_PAL) ? DISPLAY_MODE_PAL : DISPLAY_MODE_NTSC;
	attrs.outputFmt = FMT_YUV_422ILE;

	Int32 err = E_NO;
		
	if(!hConverter->hDisplay) {
		/* first time used, create display object */
		hConverter->hDisplay = display_create(&attrs);
		if(!hConverter->hDisplay) {
			ERR("create display handle failed.");
			err = E_IO;
		}
	} else {
		display_stop(hConverter->hDisplay);
		/* cfg params of display obj */
		err = display_config(hConverter->hDisplay, &attrs);
	}

	/* start display */
	if(!err)
		err = display_start(hConverter->hDisplay);

	hConverter->avParams = *params;

	return err;
}

/*****************************************************************************
 Prototype    : converter_params_update
 Description  : update params
 Input        : ConverterHandle hConverter  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/19
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 converter_params_update(ConverterHandle hConverter, ConverterParams *params)
{
	Int32				ret = E_NO;
	ImgConvDynParams	*convDynParams = &params->convDyn[0];

	/* valid img convert dynamic params */
	if(!params) {
		ERR("invalid img conv params");
		return E_INVAL;
	}

	if(!convDynParams->outAttrs[0].enbale) {
		/* need not convert, just return */
		hConverter->flags &= ~CONVERTER_FLAG_EN;
		return E_NO;
	}

	/* set convert enable flag */
	hConverter->flags |= CONVERTER_FLAG_EN;	

	/* if convert handle not exit, create one */
	if(!hConverter->hImgConv) {
		ImgConvInitParams convInitParams;

		convInitParams.size = sizeof(convInitParams);
		convInitParams.prevDevName = NULL;	//use default name
		convInitParams.rszDevName = NULL;
		
		hConverter->hImgConv = img_conv_create(&convInitParams, convDynParams);
		if(!hConverter->hImgConv) {
			ERR("create convert handle failed!");
			return E_INVAL;
		}
	} else {
		/* update dyn params */
		ret = alg_set_dynamic_params(hConverter->hImgConv, convDynParams);
		if(ret) {
			ERR("set dynamic params failed");
			return ret;
		}
	}

	/* use outattrs in dyn params  for stream1 convert */
	ImgConvInArgs *inArgs = &hConverter->convInArgs[0];
	inArgs->outAttrs[0] = convDynParams->outAttrs[0];
	inArgs->outAttrs[1] = convDynParams->outAttrs[1];
	inArgs->size = sizeof(ImgConvInArgs);

	/* set convert out dimisions for stream1 */
	hConverter->outDim[0].width = convDynParams->outAttrs[0].width;
	hConverter->outDim[0].height = convDynParams->outAttrs[0].height;
	hConverter->outDim[0].size = convDynParams->outAttrs[0].width * convDynParams->outAttrs[0].height * 3 / 2;
	hConverter->outDim[0].bytesPerLine = hConverter->outDim[0].width;
	hConverter->outDim[0].colorSpace = convDynParams->outAttrs[0].pixFmt;

	/* get stream2 out params */
	hConverter->convInArgs[1].outAttrs[0] = params->convDyn[1].outAttrs[0];
	ConvOutAttrs *pConvOut = &hConverter->convInArgs[1].outAttrs[0];

	inArgs = &hConverter->convInArgs[1];
	convDynParams = &params->convDyn[1];
	inArgs->outAttrs[0] = convDynParams->outAttrs[0];
	inArgs->outAttrs[1] = convDynParams->outAttrs[1];
	inArgs->size = sizeof(ImgConvInArgs);

	/* set convert out dimisions  for stream2 */
	hConverter->outDim[1].width = pConvOut->width;
	hConverter->outDim[1].height = pConvOut->height;
	hConverter->outDim[1].size = pConvOut->width * pConvOut->height * 3 / 2;
	hConverter->outDim[1].bytesPerLine = hConverter->outDim[1].width;
	hConverter->outDim[1].colorSpace = pConvOut->pixFmt;

	/* cfg display */
	ret = converter_display_cfg(hConverter, &params->avParams);

	DBG("\nstream1 size: %u X %u", hConverter->outDim[0].width, hConverter->outDim[0].height);
	DBG("stream2 size: %u X %u\n", hConverter->outDim[1].width, hConverter->outDim[1].height);

	if(!hConverter->hDisplay)
		assert(hConverter->convInArgs[0].outAttrs[1].enbale == FALSE);
	return ret;
}


/*****************************************************************************
 Prototype    : converter_run
 Description  : do convert
 Input        : ConverterHandle hConverter  
                Int32 streamId              
                ImgMsg *sndMsg              
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/19
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 converter_run(ConverterHandle hConverter, FrameBuf *rawBuf, Int32 streamId, ImgMsg *imgMsg)
{
	AlgBuf 			inBuf, outBuf[2];
	ImgConvInArgs 	*inArgs;
	BufHandle		hBufOut = NULL;
	Int32			err = E_NO;

	if(streamId >= CONV_MAX_STREAM_NUM || !hConverter || !rawBuf || !imgMsg)
		return E_INVAL;
	
	/* Alloc buffer for convert out */
	hBufOut = buf_pool_alloc(hConverter->hBufPool);
	if(!hBufOut)
		goto free_buf;
	
	/* do image convert */
	inBuf.buf = rawBuf->dataBuf;
	inBuf.bufSize = rawBuf->bufSize;
	outBuf[0].buf = buffer_get_user_addr(hBufOut);
	outBuf[0].bufSize = buffer_get_size(hBufOut);
	
	if(hConverter->flags & CONVERTER_FLAG_EN) {
		/* convert for stream */
		inArgs = &hConverter->convInArgs[streamId];

		DisplayBuf dispBuf;
		Bool needDisplay;
		if(streamId == 0 && hConverter->hDisplay) 
			needDisplay = TRUE;
		else {
			needDisplay = FALSE;
		}

		/* get buf for analog display */
		if(needDisplay) {		
			err = display_get_free_buf(hConverter->hDisplay, &dispBuf);
			if(!err) {
				outBuf[1].buf = dispBuf.userAddr;
				outBuf[1].bufSize = dispBuf.bufSize;
			} else
				needDisplay = FALSE; // need not display because get buffer failed
		}

		/* process convert */
		err = img_conv_process(hConverter->hImgConv, &inBuf, inArgs, outBuf, NULL);

		/* put for display */
		if(needDisplay)
			display_put_buf(hConverter->hDisplay, &dispBuf);

		/* check convert error */
		if(err) {
			ERR("imgConv err: %s.", str_err(err));
			goto free_buf;
		}
		
		imgMsg->dimension = hConverter->outDim[streamId];		
		
	} else {
		/* no convert, just copy */
		buffer_copy(outBuf[0].buf, inBuf.buf, inBuf.bufSize);
	}
	
	/* send out buffer to next module */
	imgMsg->hBuf = hBufOut;
	imgMsg->index = rawBuf->index;
	imgMsg->header.cmd = APPCMD_NEW_DATA;
	imgMsg->header.type = MSG_TYPE_REQU;
	imgMsg->header.index = 0;
	imgMsg->header.dataLen = sizeof(ImgMsg) - sizeof(MsgHeader);

	return E_NO;

free_buf:

	if(hBufOut && err)
		buf_pool_free(hBufOut);

	return err;
}


/*****************************************************************************
 Prototype    : converter_create
 Description  : create converter module
 Input        : ConverterAttrs *attrs  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/19
    Author       : Sun
    Modification : Created function

*****************************************************************************/
ConverterHandle converter_create(ConverterAttrs *attrs, const ConverterParams *params)
{
	assert(attrs && attrs->bufNum > 0);

	ConverterHandle hConverter;
	Int32			ret;

	hConverter = calloc(1, sizeof(struct ConverterObj));
	if(!hConverter) {
		ERR("alloc meme failed");
		return NULL;
	}

	/* create buffer pool */
	Uint32 size = attrs->maxImgWidth * attrs->maxImgHeight * 3 / 2;

	size = ROUND_UP(size, 256);
	BufAllocAttrs allocAttrs;
	allocAttrs.align = 256;
	allocAttrs.type = BUF_TYPE_POOL;
	allocAttrs.flags = 0;
	hConverter->hBufPool = buf_pool_create(size, attrs->bufNum, &allocAttrs);
	if(!hConverter->hBufPool)
		goto exit;

	/* set params */
	hConverter->flags = attrs->flags;
	
	ret = converter_params_update(hConverter, (ConverterParams *)params);
	if(ret) {
		goto exit;
	}
	
	return hConverter;

exit:
	
	converter_delete(hConverter);
	return NULL;
}

/*****************************************************************************
 Prototype    : converter_delete
 Description  : delete converter module
 Input        : ConverterHandle hConverter  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/19
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 converter_delete(ConverterHandle hConverter)
{
	if(!hConverter)
		return E_INVAL;

	/* free resources used */
	if(hConverter->hImgConv) {
		img_conv_delete(hConverter->hImgConv);
	}

	if(hConverter->hDisplay)
		display_delete(hConverter->hDisplay);

	if(hConverter->hBufPool)
		buf_pool_delete(hConverter->hBufPool);

	return E_NO;
}

