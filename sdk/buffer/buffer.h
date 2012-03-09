/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : buffer.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/1/10
  Last Modified :
  Description   : buffer.c header file
  Function List :
  History       :
  1.Date        : 2012/1/10
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __BUFFER_H__
#define __BUFFER_H__
	
#include "common.h"
#include "cmem.h"

/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/
#define BUF_TYPE_POOL		CMEM_POOL			//Default
#define BUF_TYPE_HEAP		CMEM_HEAP
#define BUF_TYPE_REF		(BUF_TYPE_HEAP + 1)	//Referenced buffer
#define BUF_FLAG_CACHED		CMEM_CACHED

#define BUF_FLAG_USED		(1 << 0)
#define BUF_FLAG_LCKED		(1 << 1)
#define BUF_FLAG_REF		(1 << 2) //Empty buffer, reference to other ptr

/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/

/*----------------------------------------------*
 * external routine prototypes                  *
 *----------------------------------------------*/
typedef struct _BufAllocAttrs {
	Int32	type;		//ethier BUF_TYPE_HEAP or BUF_TYPE_POOL, default using a Pool
	Int32	flags;		//ethier BUF_FLAG_CACHED or Zero, default is non-cached
	Int32	align;		//only used for heap allocations, must be power of 2
}BufAllocAttrs;

/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/
typedef struct _BufObj {
	void			*userAddr;		//user space addr
	unsigned long	phyAddr;		//physical addr
	Uint32			size;			//buffer size
	Uint32			bytesUsed;		//number of bytes used
	void			*pAllocAttrs;	//alloc params, if any
	void			*bufPool;		//ptr of buffer pool, if any
	Uint32			index;			//index used for buffer pool
	Int32			flag;			//Flag of free or used
}BufObj, *BufHandle;

typedef struct _BufPool {
	Int32			bufNum;			//Total buf num in this pool
	Uint32			bufSize;		//size of every buffer
	BufHandle		*pBufs;			//arrays of buffers
	Uint32			useNum;			//count of used buffer
	pthread_mutex_t	mutex;			//mutex for wait free buf available
	pthread_cond_t	cond;			//conditon for signal wait
}BufPoolObj, *BufPoolHandle;

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
 Prototype    : buffer_init
 Description  : Init buffer module, must be called first
 Input        : None
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 buffer_init();

/*****************************************************************************
 Prototype    : buffer_exit
 Description  : exit module
 Input        : None
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 buffer_exit();

/*****************************************************************************
 Prototype    : buffer_alloc
 Description  : Alloc new buffer from memory pool
 Input        : Uint32 size           
                BufAllocAttrs *attrs, params of alloc, this can be NULL if you want to use default params  
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern BufHandle buffer_alloc(IN Uint32 size, IN BufAllocAttrs *attrs);

/*****************************************************************************
 Prototype    : buffer_free
 Description  : Free buffer to memory pool
 Input        : BufHandle hBuf  
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 buffer_free(IN BufHandle hBuf);

/*****************************************************************************
 Prototype    : buffer_get_phy_addr
 Description  : Get physical address of this buffer
 Input        : BufHandle hBuf  
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern unsigned long buffer_get_phy_addr(IN BufHandle hBuf);

/*****************************************************************************
 Prototype    : buffer_copy
 Description  : Data copy
 Input          : void *src  
                     Uint32 size, len of the data to copy       
 Output       : void *src  
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 buffer_copy(OUT void *dest, IN void *src, IN Uint32 size);


/*****************************************************************************
 Prototype    : buffer_set_user_ptr
 Description  : Set usser addr for a referenced buffer, the buffer should be created with BUF_FLAG_REF set
 Input        : BufHandle hBuf  
 Output       : None
 Return Value : inline
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 buffer_set_user_ptr(IN BufHandle hBuf, IN void *ptr);

/*****************************************************************************
 Prototype    : buffer_get_size
 Description  : Get total size of this buffer
 Input        : BufHandle hBuf  
 Output       : None
 Return Value : inline
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline Uint32 buffer_get_size(IN BufHandle hBuf)
{
	if(!hBuf)
		return 0;
	
	return hBuf->size;
}

/*****************************************************************************
 Prototype    : buffer_get_bytes_used
 Description  : Get number of bytes used in this buffer
 Input        : BufHandle hBuf  
 Output       : None
 Return Value : inline
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline Uint32 buffer_get_bytes_used(IN BufHandle hBuf)
{
	if(!hBuf)
		return 0;
	
	return hBuf->bytesUsed;
}

/*****************************************************************************
 Prototype    : buffer_get_user_addr
 Description  : Get user ptr
 Input        : IN BufHandle hBuf  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/27
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline void *buffer_get_user_addr(IN BufHandle hBuf)
{
	if(!hBuf)
		return NULL;
	
	return hBuf->userAddr;
}


/*****************************************************************************
 Prototype    : buffer_set_bytes_used
 Description  : Set number of bytes used in this buffer
 Input        : BufHandle hBuf  
 Output       : None
 Return Value : inline
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline Uint32 buffer_set_bytes_used(IN BufHandle hBuf, IN Uint32 numBytes)
{
	if(!hBuf || numBytes > hBuf->size)
		return E_INVAL;

	hBuf->bytesUsed = numBytes;
	return E_NO;
}

/*****************************************************************************
 Prototype    : buffer_set_index
 Description  : Set index of this buffer
 Input        : IN BufHandle hBuf  
                IN Int32 index     
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/27
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline Int32 buffer_set_index(IN BufHandle hBuf, IN Int32 index)
{
	if(!hBuf)
		return E_INVAL;

	hBuf->index = index;
	return E_NO;
}

/*****************************************************************************
 Prototype    : buffer_get_index
 Description  : Get index of a buffer handle
 Input        : IN BufHandle hBuf  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/27
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline Int32 buffer_get_index(IN BufHandle hBuf)
{
	if(!hBuf)
		return E_INVAL;

	return hBuf->index;
}

/*****************************************************************************
 Prototype    : buffer_lock
 Description  : lock buffer so others can't free it
 Input        : BufHandle hBuf  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/24
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 buffer_lock(IN BufHandle hBuf);

/*****************************************************************************
 Prototype    : buffer_unlock
 Description  : unlock buf so it can be freeed
 Input        : BufHandle hBuf  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/24
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 buffer_unlock(IN BufHandle hBuf);

/*****************************************************************************
 Prototype    : buffer_is_locked
 Description  : Check if this buf is locked
 Input        : BufHandle hBuf  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/24
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline Bool buffer_is_locked(IN BufHandle hBuf)
{
	if(hBuf->flag & BUF_FLAG_LCKED)
		return TRUE;

	return FALSE;
}

/*****************************************************************************
 Prototype    : buffer_get_pool
 Description  : Get buffer pool handle, if any
 Input        : IN BufHandle hBuf  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/11
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline BufPoolHandle buffer_get_pool(IN BufHandle hBuf)
{
	if(!hBuf)
		return NULL;

	return (BufPoolHandle)(hBuf->bufPool);
}

/*****************************************************************************
 Prototype    : buf_pool_alloc
 Description  : Alloc buffer from a buffer pool
 Input        : BufPoolHandle hPool  
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/11
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern BufHandle buf_pool_alloc(IN BufPoolHandle hPool);

/*****************************************************************************
 Prototype    : buf_pool_alloc_wait
 Description  : Same as buf_pool_alloc, but when there is no free buf, 
 			will wait until timeout
 Input        : BufPoolHandle hPool  
                Uint32 timeoutSec, timeout, unit: mili second    
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/11
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern BufHandle buf_pool_alloc_wait(IN BufPoolHandle hPool, IN Uint32 timeoutMs);

/*****************************************************************************
 Prototype    : buf_pool_create
 Description  : Create a buffer pool
 Input        : Uint32 bufSize        
                Uint32 bufNum         
                BufAllocAttrs *attrs  
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/11
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern BufPoolHandle buf_pool_create(Uint32 bufSize, Uint32 bufNum, BufAllocAttrs *attrs);

/*****************************************************************************
 Prototype    : buf_pool_delete
 Description  : Delete a buffer pool
 Input        : BufPoolHandle hPool  
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/11
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 buf_pool_delete(BufPoolHandle hPool);

/*****************************************************************************
 Prototype    : buf_pool_free
 Description  : Free a buffer to a pool
 Input        : BufHandle hBuf  
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/11
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 buf_pool_free(BufHandle hBuf);

/*****************************************************************************
 Prototype    : buf_pool_get_total_num
 Description  : Get total buffer num in this pool
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
static inline Int32 buf_pool_get_total_num(IN BufPoolHandle hPool)
{
	if(!hPool)
		return E_INVAL;

	return hPool->bufNum;
}

/*****************************************************************************
 Prototype    : buf_pool_get_free_num
 Description  : Get free num of this pool
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
static inline Int32 buf_pool_get_free_num(IN BufPoolHandle hPool)
{
	if(!hPool)
		return E_INVAL;

	return (hPool->bufNum - (Int32)hPool->useNum);
}

/*****************************************************************************
 Prototype    : buf_pool_get_buf_size
 Description  : Get single buffer size
 Input        : IN BufPoolHandle hPool  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/27
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline Int32 buf_pool_get_buf_size(IN BufPoolHandle hPool)
{
	if(!hPool)
		return E_INVAL;

	return hPool->bufSize;
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
extern Int32 buf_pool_free_all(IN BufPoolHandle hPool);

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
extern BufHandle buf_pool_index_alloc(IN BufPoolHandle hPool, IN Int32 index);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __BUFFER_H__ */
