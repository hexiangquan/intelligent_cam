/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : circular_buf.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/4/27
  Last Modified :
  Description   : circular_buf.c header file
  Function List :
  History       :
  1.Date        : 2012/4/27
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __CIRCULAR_BUF_H__
#define __CIRCULAR_BUF_H__

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

/* all read and write len must align to this value */
#define CIR_LEN_ALIGN		4

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/* struct for circular buffer operations */
struct CirBufObj{
	Int8 			*buf;
	Int32			bufSize;
	Int32			rdPos;
	Int32			wrPos;
	Int32			waitTime;
};

typedef struct CirBufObj *CirBufHandle;

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


/*****************************************************************************
 Prototype    : circular_buf_create
 Description  : create a circular buffer
 Input        : Int32 bufSize  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/27
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern CirBufHandle circular_buf_create(Int32 bufSize, Int32 waitCheckPrdMs);

/*****************************************************************************
 Prototype    : circular_buf_delete
 Description  : delete a circular buffer
 Input        : CirBufHandle hCirBuf  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/27
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 circular_buf_delete(CirBufHandle hCirBuf);

/*****************************************************************************
 Prototype    : circular_buf_write
 Description  : write buf, you can only have one writer for a buffer
 Input        : CirBufHandle hCirBuf  
                const void *buf       
                Int32 len             
                Int32 timeoutMs,  timeoutMs == 0, returns immediately if not enough buffer availabe
                if timeoutMs > 0, wait until timeout
                if timeoutMS <0, wati forever untils enough buffer is available
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/27
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 circular_buf_write(CirBufHandle hCirBuf, const void *buf, Int32 len, Int32 timeoutMs);

/*****************************************************************************
 Prototype    : circular_buf_read
 Description  : read buf, you can only have one reader for a buffer
 Input        : CirBufHandle hCirBuf  
                void *buf             
                Int32 len             
                Int32 timeoutMs, same as write, Unit: Milisecond       
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/27
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 circular_buf_read(CirBufHandle hCirBuf, void *buf, Int32 len, Int32 timeoutMs);

/*****************************************************************************
 Prototype    : circular_buf_flush
 Description  : flush buffer to empty for write
 Input        : CirBufHandle hCirBuf  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/27
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 circular_buf_flush(CirBufHandle hCirBuf);

/*****************************************************************************
 Prototype    : circular_buf_get_status
 Description  : get current status
 Input        : CirBufHandle hCirBuf  
               
 Output       :  Int32 *total   -- total buf size       
                	Int32 *wrLen -- available size for write          
                	Int32 *rdLen  -- available size for read         
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/27
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 circular_buf_get_status(CirBufHandle hCirBuf, Int32 *total, Int32 *wrLen, Int32 *rdLen);

/*****************************************************************************
 Prototype    : circular_buf_wait_ready
 Description  : wait buffer available for read or write
 Input        : CirBufHandle hCirBuf  
                Bool isRd             
                Int32 len             
                Int32 timeoutMs       
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/27
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 circular_buf_wait_ready(CirBufHandle hCirBuf, Bool isRd, Int32 size, Int32 timeoutMs);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __CIRCULAR_BUF_H__ */
