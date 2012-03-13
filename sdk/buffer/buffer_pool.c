/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : buffer_pool.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/1/11
  Last Modified :
  Description   : Buffer pool module
  Function List :
              buf_pool_alloc
              buf_pool_alloc_wait
              buf_pool_create
              buf_pool_delete
              buf_pool_find_free
              buf_pool_free
  History       :
  1.Date        : 2012/1/11
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "buffer.h"
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
/* Uncomment this to print debug info */
// #define BUF_DBG

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/


/*****************************************************************************
 Prototype    : buf_pool_create
 Description  : create buffer pool
 Input        : Uint32 bufSize        
                Uint32 bufNum         
                BufAllocAttrs *attrs  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/11
    Author       : Sun
    Modification : Created function

*****************************************************************************/
BufPoolHandle buf_pool_create(Uint32 bufSize, Uint32 bufNum, BufAllocAttrs *attrs)
{
	if(!bufSize || !bufNum)
		return NULL;

	BufPoolHandle hPool;

	/* Alloc memory for pool handle */
	hPool = calloc(1, sizeof(BufPoolObj));
	if(!hPool) {
		ERRSTR("Calloc buf pool obj failed.");
		return NULL;
	}

	/* Alloc memory for buf handle array */
	hPool->pBufs = calloc(bufNum, sizeof(BufHandle));
	if(!hPool->pBufs) {
		ERRSTR("Calloc buf handle failed.");
		goto exit;
	}

	/* Alloc buffers */
	Int32 i;
	for(i = 0; i < bufNum; i++) {
		BufHandle hBuf;
		hBuf = buffer_alloc(bufSize, attrs);
		if(!hBuf) {
			ERR("Alloc buffer failed, allocated buf num: %d", hPool->bufNum);
			break;
		}

		hBuf->bufPool = (void *)hPool;
		hBuf->index = i;
		hBuf->flag = 0;
		hPool->bufNum++;
		hPool->pBufs[i] = hBuf;
	}

	if(hPool->bufNum <= 0) {
		ERR("Can't alloc any buffer...");
		goto exit;
	}

	/* Init mutex and condition */
	if(pthread_mutex_init(&hPool->mutex, NULL))
		goto exit;

	if(pthread_cond_init(&hPool->cond, NULL))
		goto exit;
	
	hPool->bufSize = bufSize;

	return hPool;

exit:
	if(hPool && hPool->pBufs)
		free(hPool->pBufs);
	if(hPool)
		free(hPool);

	return NULL;
}

/*****************************************************************************
 Prototype    : buf_pool_delete
 Description  : Delete buffer pool
 Input        : BufPoolHandle hPool  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/11
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 buf_pool_delete(BufPoolHandle hPool)
{
	if(!hPool || !hPool->pBufs)
		return E_INVAL;

	/* Free all buffers */
	if(hPool->useNum > 0)
		WARN("warning: there still buffer used in this pool...");
	
	Int32 i;

	pthread_mutex_lock(&hPool->mutex);
	for(i = 0; i < hPool->bufNum; i++) {
		buffer_free(hPool->pBufs[i]);
	}
	hPool->bufNum = 0;
	pthread_mutex_unlock(&hPool->mutex);

	/* Free blocking threads, if any */
	pthread_cond_broadcast(&hPool->cond);

	/* Destroy mutex lock and condition */
	pthread_mutex_destroy(&hPool->mutex);
	pthread_cond_destroy(&hPool->cond);

	free(hPool->pBufs);
	free(hPool);

	return E_NO;
}

/*****************************************************************************
 Prototype    : buf_pool_find_free
 Description  : Find a free buffer
 Input        : BufPoolHandle hPool  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/11
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline BufHandle buf_pool_find_free(BufPoolHandle hPool)
{
	/* Get a free one */
	Int32 i;
	BufHandle hBuf = NULL;
	
	for(i = 0; i < hPool->bufNum; i++) {
		hBuf = hPool->pBufs[i];
		if(!(hBuf->flag & BUF_FLAG_USED)) {
			hBuf->flag |= BUF_FLAG_USED;
			hPool->useNum++;
			//DBG("buf pool use: %d, total: %d", hPool->useNum, hPool->bufNum);
			break;
		}
	}

	return hBuf;
}

/*****************************************************************************
 Prototype    : buf_pool_alloc
 Description  : Alloc one free buffer from pool, 
                	if no free buf, this function will block
 Input        : BufPoolHandle hPool  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/11
    Author       : Sun
    Modification : Created function

*****************************************************************************/
BufHandle buf_pool_alloc(BufPoolHandle hPool)
{
	if(!hPool || !hPool->pBufs || hPool->bufNum <= 0)
		return NULL;

	pthread_mutex_lock(&hPool->mutex);

	/* Wait condition if no free buffer available */
	if(hPool->useNum >= hPool->bufNum) {
		#ifdef BUF_DBG
		DBG("wait buf available for this pool.");
		#endif
		pthread_cond_wait(&hPool->cond, &hPool->mutex);
		#ifdef BUF_DBG
		DBG("got signal.");
		#endif
	}

	/* Find a free buffer */
	BufHandle hBuf = buf_pool_find_free(hPool);
	
	pthread_mutex_unlock(&hPool->mutex);

	return hBuf;
}

/*****************************************************************************
 Prototype    : buf_pool_alloc_wait
 Description  : Alloc free buffer in pool with a timeout
 Input        : BufPoolHandle hPool  
                Uint32 timeoutSec    
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/11
    Author       : Sun
    Modification : Created function

*****************************************************************************/
BufHandle buf_pool_alloc_wait(BufPoolHandle hPool, Uint32 timeoutMs)
{
	if(!hPool || !hPool->pBufs || hPool->bufNum <= 0)
		return NULL;

	pthread_mutex_lock(&hPool->mutex);

	/* Wait condition if no free buffer available */
	if(hPool->useNum >= hPool->bufNum) {
		#ifdef BUF_DBG
		DBG("start condition timeout wait...");
		#endif
		
		struct timespec tm;
		tm.tv_sec = time(0) + timeoutMs / 1000;
		tm.tv_nsec = timeoutMs % 1000 * 1000;
		int err = pthread_cond_timedwait(&hPool->cond, &hPool->mutex, &tm);

		#ifdef BUF_DBG
		DBG("condition timeout wait return: %d", err);
		#endif
		
		if(hPool->useNum >= hPool->bufNum) {
			pthread_mutex_unlock(&hPool->mutex);
			return NULL;
		}
	}

	/* Find a free buffer */
	BufHandle hBuf = buf_pool_find_free(hPool);
	
	pthread_mutex_unlock(&hPool->mutex);

	return hBuf;
}

/*****************************************************************************
 Prototype    : buf_pool_index_alloc
 Description  : Alloc by index in pool
 Input        : BufPoolHandle hPool  
                Int32 index          
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/24
    Author       : Sun
    Modification : Created function

*****************************************************************************/
BufHandle buf_pool_index_alloc(BufPoolHandle hPool, Int32 index)
{
	if(!hPool || !hPool->pBufs || hPool->bufNum <= 0 || index >= hPool->bufNum)
		return NULL;

	//pthread_mutex_lock(&hPool->mutex);

	BufHandle hBuf = hPool->pBufs[index];
	if(hBuf->flag & (BUF_FLAG_USED | BUF_FLAG_LCKED)) {
		ERR("buffer already used...");
		return NULL;
	}

	hBuf->flag |= BUF_FLAG_USED;
	hPool->useNum++;
	
	//pthread_mutex_unlock(&hPool->mutex);
	return hBuf;
}


/*****************************************************************************
 Prototype    : buf_pool_free
 Description  : Free buffer from pool
 Input        : BufHandle hBuf  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/11
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 buf_pool_free(BufHandle hBuf)
{
	if(!hBuf)
		return E_INVAL;
	
	BufPoolHandle hPool = hBuf->bufPool;
	if(!hPool) {
		ERR("this buffer doesn't belong to any pool.");
		return E_INVAL;
	}
	
	pthread_mutex_lock(&hPool->mutex);

	Uint32 	index = hBuf->index;
	Int32 	err = E_NO;

	/* Check the index is valid or not*/
	if(index >= hPool->bufNum || hPool->pBufs[index] != hBuf) {
		#ifdef BUF_DBG
		DBG("Invaid index of buf, find from the beginning....");
		#endif
		for(index = 0; index < hPool->bufNum; index++) {
			if(hBuf == hPool->pBufs[index])
				break;
		}

		/* Not belong to this pool */
		if(index >= hPool->bufNum) {
			err = E_INVAL;
			ERR("this buffer doesn't belong to this pool.");
			hBuf->bufPool = NULL;
			goto exit;
		}
	}

	/* Checke if this buf has been locked */
	if(hBuf->flag & BUF_FLAG_LCKED) {
		#ifdef BUF_DBG
		DBG("can't free buf, buf is locked.");
		#endif
		err = E_BUSY;
		goto exit;
	}

	/* Set flag to free */
	hBuf->flag &= ~(BUF_FLAG_USED);
	if(hPool->useNum == hPool->bufNum) {
		hPool->useNum--;
		#ifdef BUF_DBG
		DBG("free buf, Send signal....");
		#endif
		pthread_mutex_unlock(&hPool->mutex);
		//pthread_cond_signal(&hPool->cond);
		pthread_cond_broadcast(&hPool->cond);
		return err;
	} else {
		if(hPool->useNum)
			hPool->useNum--;
	}
	
exit:
	pthread_mutex_unlock(&hPool->mutex);
	return err;
}

/*****************************************************************************
 Prototype    : buf_pool_free_all
 Description  : Force all buffers as free, may be uesful for error recovery
 Input        : IN BufPoolHandle hPool  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/11
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 buf_pool_free_all(IN BufPoolHandle hPool)
{
	if(!hPool || !hPool->pBufs)
		return E_INVAL;

	Int32 i;

	pthread_mutex_lock(&hPool->mutex);
	for(i = 0; i < hPool->bufNum; i++) {
		if(hPool->pBufs[i])
			hPool->pBufs[i]->flag = 0;
	}

	if(hPool->useNum == hPool->bufNum) 
	{
		hPool->useNum = 0;
		pthread_mutex_unlock(&hPool->mutex);
		//pthread_cond_signal(&hPool->cond);
		pthread_cond_broadcast(&hPool->cond);
	} else {
		hPool->useNum = 0;
		pthread_mutex_unlock(&hPool->mutex);
	}

	return E_NO;
}


