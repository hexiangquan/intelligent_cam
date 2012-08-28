/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : display.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/8/27
  Last Modified :
  Description   : display.c header file
  Function List :
  History       :
  1.Date        : 2012/8/27
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __DISPLAY_H__
#define __DISPLAY_H__

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

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/
typedef struct DisplayObj *DisplayHanlde;

/**
 * @brief       Video output mode.
 */
typedef enum {
    /** @brief PAL output mode */
    DISPLAY_MODE_PAL = 0,

    /** @brief NTSC output mode */
    DISPLAY_MODE_NTSC,

    DISPLAY_MODE_MAX
} DisplayMode;

/* Width and height define for different output mode */
#define PAL_WIDTH	720
#define PAL_HEIGHT	576

#define NTSC_WIDTH	720
#define NTSC_HEIGHT	480

#define DISPLAY_MAX_CHAN_ID		1

typedef struct {
	Uint32 			chanId;
	DisplayMode 	mode;
	ChromaFormat	outputFmt;	/* only support UYVY and NV12 */
}DisplayAttrs;

/* Capture buffer allocated */
typedef struct {
	void			*userAddr;
	unsigned long 	phyAddr;	/* not used */
	Int32			index;
	Uint32			bufSize;
}DisplayBuf;


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*****************************************************************************
 Prototype    : display_create
 Description  : create display module
 Input        : const DisplayAttrs *attrs  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern DisplayHanlde display_create(const DisplayAttrs *attrs);

/*****************************************************************************
 Prototype    : display_delete
 Description  : delete display module
 Input        : DisplayHanlde hDisplay  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 display_delete(DisplayHanlde hDisplay);

/*****************************************************************************
 Prototype    : display_config
 Description  : config attrs for display module
 Input        : DisplayHanlde hDisplay     
                const DisplayAttrs *attrs  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 display_config(DisplayHanlde hDisplay, const DisplayAttrs *attrs);

/*****************************************************************************
 Prototype    : display_start
 Description  : start display stream
 Input        : DisplayHanlde hDisplay  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 display_start(DisplayHanlde hDisplay);

/*****************************************************************************
 Prototype    : display_stop
 Description  : stop display stream
 Input        : DisplayHanlde hDisplay  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 display_stop(DisplayHanlde hDisplay);

/*****************************************************************************
 Prototype    : display_get_free_buf
 Description  : alloc a free buffer from driver for filling with new image 
 Input        : DisplayHanlde hDisplay  
                DisplayBuf *buf         
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 display_get_free_buf(DisplayHanlde hDisplay, DisplayBuf *buf);

/*****************************************************************************
 Prototype    : display_put_buf
 Description  : put buffer to display
 Input        : DisplayHanlde hDisplay  
                const DisplayBuf *buf   
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 display_put_buf(DisplayHanlde hDisplay, const DisplayBuf *buf);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __DISPLAY_H__ */
