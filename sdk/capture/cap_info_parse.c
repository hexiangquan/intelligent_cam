/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : cap_info_parse.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/7/27
  Last Modified :
  Description   : parse capture info from hardware in raw image
  Function List :
              bytes_convert
              cap_info_parse
  History       :
  1.Date        : 2012/7/27
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "cap_info_parse.h"
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

typedef struct _RawInfoBytes {
	Uint8	globalGain[10];
	Uint8	exposure[32];		
	Uint8	avgLum[12];
	Uint8	strobe[8];
	Uint8	frameCnt[16];
	Uint8	capMode[2];
}RawInfoBytes;


/*****************************************************************************
 Prototype    : bytes_convert
 Description  : convert from raw bytes to bits info
 Input        : const Uint8 *data  
                Int32 bitsNum      
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/7/27
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Uint32 bytes_convert(const Uint8 *data, Int32 bitsNum)
{
	Uint32 	result = 0;
	
	while(--bitsNum >= 0) {
		if(*data)
			result |= (1 << bitsNum);
		++data;
	}

	return result;
}

/*****************************************************************************
 Prototype    : cap_info_parse
 Description  : parse cap info from raw data
 Input        : const void *imgBuf       
                const ImgDimension *dim  
                RawCapInfo *info         
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/7/27
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 cap_info_parse(const Uint8 *imgBuf, const ImgDimension *dim, RawCapInfo *info)
{
	if(!imgBuf || !dim || !info)
		return E_INVAL;

	const RawInfoBytes *rawInfo = (RawInfoBytes *)(imgBuf + dim->size - dim->bytesPerLine - sizeof(RawInfoBytes));

	//DBG("raw info offset: %d", dim->size - dim->bytesPerLine - sizeof(RawInfoBytes));

	info->globalGain = (Uint16)bytes_convert(rawInfo->globalGain, sizeof(rawInfo->globalGain));
	info->exposure = bytes_convert(rawInfo->exposure, sizeof(rawInfo->exposure));
	info->avgLum = (Uint16)bytes_convert(rawInfo->avgLum, sizeof(rawInfo->avgLum));
	info->strobeStat = (Uint8)bytes_convert(rawInfo->strobe, sizeof(rawInfo->strobe));
	info->index = (Uint16)bytes_convert(rawInfo->frameCnt, sizeof(rawInfo->frameCnt));
	info->capMode = (Uint8)bytes_convert(rawInfo->capMode, sizeof(rawInfo->capMode));
	info->redGain = 0;
	info->greenGain = 0;
	info->blueGain = 0;

	return E_NO;
}


