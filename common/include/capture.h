/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : capture.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/2/23
  Last Modified :
  Description   : capture.c header file
  Function List :
  History       :
  1.Date        : 2012/2/23
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __CAPTURE_H__
#define __CAPTURE_H__

#include "common.h"

/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/

/*----------------------------------------------*
 * external routine prototypes                  *
 *----------------------------------------------*/

/**
 * @brief       Video capture inputs.
 */
typedef enum {
    /** @brief S-Video video input */
    CAP_INPUT_SVIDEO = 0,

    /** @brief Composite video input */
    CAP_INPUT_COMPOSITE,

    /** @brief Component video input */
    CAP_INPUT_COMPONENT,

    /** @brief Camera/Imager card video input 
      * @remarks default
      */
    CAP_INPUT_CAMERA,

    CAP_INPUT_MAX
} CaptureInput;

/**
 * @brief       Video capture standard inputs.
 */
typedef enum {
	/* Full frame, using the max resolution */
	CAP_STD_FULL_FRAME = 0,
	
	/* High speed mode, using small reoslution */
	CAP_STD_HIGH_SPEED,
	
	CAP_STD_MAX
} CaptureStd;


/**
 * @brief       Video capture modes.
 */
typedef enum {
	/* Continously capture */
	CAP_MODE_CONT = 0,
	
	/* Trigger by cpu */
	CAP_MODE_TRIG,
	
	CAP_MODE_MAX
} CaptureMode;

/**
  * @brief		Capture attributes for create 
  * @remark	  	outAttrs[0].enable must set to TRUE, 
  *				if outAttrs[0].width or outAttrs[0].height set to 0, we will use original input size
 */
typedef struct _CapAttrs {
	const char 		*devName;		/* Capture device name */
	CaptureInput	inputType;		/* See CaptureInput enum */
	CaptureStd		std;			/* Input std */
	CaptureMode		mode;			/* input capture mode */
	Bool			userAlloc;		/* use user ptr or mmap from driver */
	Int32			bufNum;			/* num of buffer for capture */
	Int32			defRefCnt;		/* default frame buf reference cnt */
}CapAttrs;

/**
 * @brief       Video capture frame buf.
 * @remark	This struct will be set by capture_get_frame
 *			after capture
 */
typedef struct _FrameBuf {
	Int32			index;				//Frame number
	struct timeval	timeStamp;			//time stamp 
	void			*dataBuf;			//user addr of capture buffer
	Uint32			bytesUsed;			//bytes used for this buffer
	Uint32			bufSize;			//size of this buffer
	Int32			private;			//private data, used by driver, app should not set this value
	Int32			reserved;			//reserved
}FrameBuf;

/* Capture input info */
typedef ImgDimension CapInputInfo ;

/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/
typedef struct CapObj *CapHandle;
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
 * routines' implementations                    *
 *----------------------------------------------*/

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*****************************************************************************
 Prototype    : capture_create
 Description  : Create capture module and init hardware
 Input        : CapAttrs *attrs  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/23
    Author       : Sun
    Modification : Created function

*****************************************************************************/
CapHandle capture_create(CapAttrs *attrs);

/*****************************************************************************
 Prototype    : capture_delete
 Description  : Delete capture module
 Input        : CapHandle hCapture  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/23
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 capture_delete(CapHandle hCapture);

/*****************************************************************************
 Prototype    : capture_start
 Description  : Start capture
 Input        : CapHandle hCap  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/27
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 capture_start(CapHandle hCapture);

/*****************************************************************************
 Prototype    : capture_stop
 Description  : Stop capture
 Input        : CapHandle hCap  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/27
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 capture_stop(CapHandle hCapture);

/*****************************************************************************
 Prototype    : capture_get_frame
 Description  : Get a new frame
 Input        : CapHandle hCap   
                Int32 flags      
                FrameInfo *info  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/27
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 capture_get_frame(CapHandle hCap, FrameBuf *frameBuf);

/*****************************************************************************
 Prototype    : capture_free_frame
 Description  : Free a frame to buffer
 Input        : CapHandle hCap  
                BufHandle hBuf  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/27
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 capture_free_frame(CapHandle hCap, FrameBuf *frameBuf);

/*****************************************************************************
 Prototype    : capture_get_fd
 Description  : Get fd for select, etc...
 Input        : CapHandle hCap  
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 capture_get_fd(CapHandle hCap);


/*****************************************************************************
 Prototype    : capture_get_input_info
 Description  : Get input dev info
 Input        : CapHandle hCap      
                CapInputInfo *info  
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 capture_get_input_info(CapHandle hCap, CapInputInfo *info);

/*****************************************************************************
 Prototype    : capture_config
 Description  : Change capture attrs at run time, can Only be called when capture is stopped
 Input        : CapHandle hCap    
                CaptureStd std    
                CaptureMode mode  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/2
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 capture_config(CapHandle hCap, CaptureStd std, CaptureMode mode);

/*****************************************************************************
 Prototype    : capture_inc_frame_ref
 Description  : increase frame reference count
 Input        : CapHandle hCap            
                const FrameBuf *frameBuf  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/16
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 capture_inc_frame_ref(CapHandle hCap, const FrameBuf *frameBuf);

/*****************************************************************************
 Prototype    : capture_set_def_frame_ref_cnt
 Description  : set default reference count
 Input        : CapHandle hCap   
                Int32 defRefCnt  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/16
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 capture_set_def_frame_ref_cnt(CapHandle hCap, Int32 defRefCnt);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __CAPTURE_H__ */

