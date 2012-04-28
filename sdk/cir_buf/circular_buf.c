/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : circular_buf.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/4/27
  Last Modified :
  Description   : circular buffer implementation
  Function List :
              circular_buf_create
              circular_buf_delete
              circular_buf_write
  History       :
  1.Date        : 2012/4/27
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "circular_buf.h"
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

/* Minimum distance between write pos and read pos */
#define CIR_MIN_RW_DST		4	

/* Available write & read buffer size */
#define BUF_SIZE_WR(rdPos, wrPos, bufSize) \
	(((rdPos) > (wrPos)) ? ((rdPos) - (wrPos) - CIR_MIN_RW_DST) : \
	((bufSize) + (rdPos) - (wrPos) - CIR_MIN_RW_DST))

#define BUF_SIZE_RD(rdPos, wrPos, bufSize) \
	(((wrPos) >= (rdPos)) ? ((wrPos) - (rdPos)) : ((bufSize) + (wrPos) - (rdPos)))

#define CIR_SYNC_CODE		0x51BADDAD		//sync code

typedef struct {
	Int32 syncCode;
	Int32 len;
}CirDataHdr;

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

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
CirBufHandle circular_buf_create(Int32 bufSize, Int32 waitCheckPrdMs)
{
	CirBufHandle	hCirBuf;

	if(bufSize <= 0 || waitCheckPrdMs <= 0){
		ERR("invalid argument.");
		return NULL;
	}

	hCirBuf = calloc(1, sizeof(struct CirBufObj));
	if(!hCirBuf) {
		ERR("alloc mem for cir buf obj failed");
		return NULL;
	}

	/* alloc data buffer */
	bufSize = ROUND_UP(bufSize, CIR_LEN_ALIGN);
	hCirBuf->buf = calloc(1, bufSize);
	if(!hCirBuf->buf) {
		ERR("can't alloc cir buf");
		goto exit;
	}

	hCirBuf->bufSize = bufSize;
	hCirBuf->waitTime = waitCheckPrdMs * 1000;
	
	return hCirBuf;
	
exit:
	
	circular_buf_delete(hCirBuf);

	return NULL;
}

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
Int32 circular_buf_delete(CirBufHandle hCirBuf)
{
	if(!hCirBuf)
		return E_INVAL;

	if(hCirBuf->buf)
		free(hCirBuf->buf);

	free(hCirBuf);

	return E_NO;
}

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
Int32 circular_buf_wait_ready(CirBufHandle hCirBuf, Bool isRd, Int32 size, Int32 timeoutMs)
{
	Int32 leftTime = timeoutMs << 10; //covert to us
	Int32 err = E_NO;
	Int32 bufRdy = isRd ? BUF_SIZE_RD(hCirBuf->rdPos, hCirBuf->wrPos, hCirBuf->bufSize) 
					: BUF_SIZE_WR(hCirBuf->rdPos, hCirBuf->wrPos, hCirBuf->bufSize);

	size = ROUND_UP(size, CIR_LEN_ALIGN);

	/* wait enough buffer for write */
	while(size > bufRdy) {
		if(leftTime > 0) {
			leftTime -= hCirBuf->waitTime;
			if(leftTime <= 0){
				err = E_TIMEOUT;
				break;
			}
		}
		usleep(hCirBuf->waitTime);
		bufRdy = isRd ? BUF_SIZE_RD(hCirBuf->rdPos, hCirBuf->wrPos, hCirBuf->bufSize) 
					: BUF_SIZE_WR(hCirBuf->rdPos, hCirBuf->wrPos, hCirBuf->bufSize);
	}

	return err;
}

/*****************************************************************************
 Prototype    : circular_buf_write
 Description  : write buf, you can only have one writer for a buffer
 Input        : CirBufHandle hCirBuf  
                const void *buf       
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
Int32 circular_buf_write(CirBufHandle hCirBuf, const void *buf, Int32 len, Int32 timeoutMs)
{
	if(!hCirBuf || !buf || len < 0 || len > hCirBuf->bufSize) {
		ERR("write len should not bigger than buf len");
		return E_INVAL;
	}

	/* wait enough buffer for write */
	len = ROUND_UP(len, CIR_LEN_ALIGN);
	Int32 err = circular_buf_wait_ready(hCirBuf, FALSE, len, timeoutMs);
	if(err)
		return err;

	/* copy data */
	Int32 sizeToEnd = hCirBuf->bufSize - hCirBuf->wrPos;
	
	if(sizeToEnd >= len) {
		memcpy(hCirBuf->buf + hCirBuf->wrPos, buf, len);
		hCirBuf->wrPos += len;
		if(hCirBuf->wrPos >= hCirBuf->bufSize)
			hCirBuf->wrPos = 0;
	} else {
		/* copy to the end */
		memcpy(hCirBuf->buf + hCirBuf->wrPos, buf, sizeToEnd);
		/* copy left from the beginning */
		memcpy(hCirBuf->buf, (const Int8 *)buf + sizeToEnd, len - sizeToEnd);
		hCirBuf->wrPos = len - sizeToEnd;
	}

	//DBG("wr pos: %d", hCirBuf->wrPos);

	return E_NO;
}

/*****************************************************************************
 Prototype    : circular_buf_read
 Description  : read buf, you can only have one reader for a buffer
 Input        : CirBufHandle hCirBuf  
                void *buf             
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
Int32 circular_buf_read(CirBufHandle hCirBuf, void *buf, Int32 len, Int32 timeoutMs)
{
	if(!hCirBuf || !buf || len > hCirBuf->bufSize) {
		ERR("read len %d should not bigger than buf len %d", len, hCirBuf->bufSize);
		return E_INVAL;
	}

	Int32 err = circular_buf_wait_ready(hCirBuf, TRUE, len, timeoutMs);
	if(err)
		return err;
	
	/* copy data */
	Int32 sizeToEnd = hCirBuf->bufSize - hCirBuf->rdPos;
	
	len = ROUND_UP(len, CIR_LEN_ALIGN);
	if(len <= sizeToEnd) {
		memcpy(buf, hCirBuf->buf + hCirBuf->rdPos, len);
		hCirBuf->rdPos += len;
		if(hCirBuf->rdPos >= hCirBuf->bufSize)
			hCirBuf->rdPos = 0;
	} else {
		/* copy to the end */
		memcpy(buf, hCirBuf->buf + hCirBuf->rdPos, sizeToEnd);
		/* copy left from the beginning */
		memcpy((Int8 *)buf + sizeToEnd, hCirBuf->buf, len - sizeToEnd);
		hCirBuf->rdPos = len - sizeToEnd;
	}

	//DBG("rd pos: %d", hCirBuf->rdPos);

	return E_NO;
}

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
Int32 circular_buf_flush(CirBufHandle hCirBuf)
{
	if(!hCirBuf)
		return E_INVAL;

	/* set rd pos equals to write pos, so the buffer is empty */
	hCirBuf->rdPos = hCirBuf->wrPos;

	return E_NO;
}

/*****************************************************************************
 Prototype    : circular_buf_get_status
 Description  : get current status
 Input        : CirBufHandle hCirBuf  
                Int32 *total          
                Int32 *free           
                Int32 *used           
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/27
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 circular_buf_get_status(CirBufHandle hCirBuf, Int32 *total, Int32 *wrLen, Int32 *rdLen)
{
	if(!hCirBuf)
		return E_INVAL;

	if(total)
		*total = hCirBuf->bufSize;

	if(wrLen)
		*wrLen = BUF_SIZE_WR(hCirBuf->rdPos,hCirBuf->wrPos, hCirBuf->bufSize);

	if(rdLen)
		*rdLen = BUF_SIZE_RD(hCirBuf->rdPos,hCirBuf->wrPos, hCirBuf->bufSize);

	return E_NO;
}

