/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : cap_info_parse.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/7/27
  Last Modified :
  Description   : cap_info_parse.c header file
  Function List :
  History       :
  1.Date        : 2012/7/27
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __CAP_INFO_PARSE_H__
#define __CAP_INFO_PARSE_H__

#include "common.h"

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

/* Bits define for strobe status  */
#define RCI_STROBE0_EN		(1 << 0)		// Strobe0 enabled
#define RCI_STROBE1_EN		(1 << 1)		// Strobe1 enabled

/* Cap mode */
#define RCI_CAP_TYPE_CONT	(0)				// Continous cap frame
#define RCI_CAP_TYPE_TRIG	(1)				// Normal trigger frame
/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/* Info from hardware */
typedef struct _RawCapInfo {
	Uint16		index;		/* frame index */
	Uint8		capMode;	/* capture mode of this frame, 3--continue, 1--normal trig, 2--spec trig */
	Uint8		strobeStat;	/* strobe status, bit[0:3]--strobe[0:3] */
	Uint32		exposure;	/* exposure time of this frame, us */
	Uint16		globalGain;	/* global gain of this frame */
	Uint16		avgLum;		/* average lum value of this frame */
	Uint16		redGain;
	Uint16		greenGain;
	Uint16		blueGain;
	Uint16		reserved;
}RawCapInfo;

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*****************************************************************************
 Prototype    : cap_info_parse
 Description  : parse hw cap info
 Input        : IN const Uint8 *imgBuf      
                IN const ImgDimension *dim  
                OUT RawCapInfo *info        
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/7/27
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 cap_info_parse(IN const Uint8 *imgBuf, IN const ImgDimension *dim, 
								OUT RawCapInfo *info);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __CAP_INFO_PARSE_H__ */
