/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : buffer.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/1/10
  Last Modified :
  Description   : buffer manage module
  Function List :
              buffer_alloc
              buffer_create
              buffer_delete
              buffer_free
  History       :
  1.Date        : 2012/1/10
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "common.h"
#include "buffer.h"
#include "cmem.h"
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
static Bool s_bIsCMEMInited = FALSE;

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
 Prototype    : buffer_int
 Description  : Init buffer module
 Input        : None
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 buffer_init() {

	if(s_bIsCMEMInited)
		return E_NO;

	Int32 err = CMEM_init();
	assert(err == 0);

	if(err < 0) {
		ERR("Buffer init failed!!!");
		return E_IO;
	}

	s_bIsCMEMInited = TRUE;
	return E_NO;
}

/*****************************************************************************
 Prototype    : buffer_exit
 Description  : Exit buffer module
 Input        : None
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 buffer_exit() {
	if(s_bIsCMEMInited)
		CMEM_exit();

	s_bIsCMEMInited = FALSE;
	return E_NO;
}

/*****************************************************************************
 Prototype    : buffer_alloc
 Description  : Alloc buffer from memory pool
 Input        : Uint32 size           
                BufAllocAttrs *attrs  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
BufHandle buffer_alloc(Uint32 size, BufAllocAttrs *attrs) {
	BufHandle hBuf;

	/* Alloc memory for buffer handle */
	hBuf = calloc(1, sizeof(BufObj));
	if(!hBuf) {
		ERRSTR("alloc buf obj failed");
		return NULL;
	}
	
	if(attrs) {
		if(attrs->type != BUF_TYPE_REF) {
			hBuf->pAllocAttrs = malloc(sizeof(CMEM_AllocParams));
			if(!hBuf->pAllocAttrs) {
				ERRSTR("alloc buf for alloAttrs failed");
				goto exit;
			}

			/* Alloc using CMEM */
			CMEM_AllocParams *params = hBuf->pAllocAttrs;
			params->type = attrs->type;
			params->flags = attrs->flags;
			params->alignment = attrs->align;
			hBuf->userAddr = CMEM_alloc(size, params);
		} else {
			hBuf->flag |= BUF_FLAG_REF;
			hBuf->userAddr = (void *)(-1);	//Point to an invalid addr
		}
	} else {
		/* Using default params to alloc */
		hBuf->userAddr = CMEM_alloc(size, NULL);
	}
	
	if(!hBuf->userAddr) {
		ERR("Alloc actual buffer failed...");
		goto exit;
	}
	
	hBuf->size = size;
	return hBuf;

exit:
	if(hBuf && hBuf->pAllocAttrs)
		free(hBuf->pAllocAttrs);
	if(hBuf)
		free(hBuf);
	return NULL;
}

/*****************************************************************************
 Prototype    : buffer_free
 Description  : Free buffer to memory pool
 Input        : BufHandle hBuf  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 buffer_free(BufHandle hBuf) {
	if(!hBuf)
		return E_INVAL;

	/* Checke if this buf has been locked */
	if(hBuf->flag & BUF_FLAG_LCKED) {
		ERR("buffer has been locked.");
		return E_BUSY;
	}

	if(hBuf->userAddr && !(hBuf->flag & BUF_FLAG_REF)) {
		CMEM_free(hBuf->userAddr, (CMEM_AllocParams *)hBuf->pAllocAttrs);
	}

	if(hBuf->pAllocAttrs)
		free(hBuf->pAllocAttrs);
	
	free(hBuf);
	return E_NO;
}

/*****************************************************************************
 Prototype    : buffer_get_phy_addr
 Description  : Get physcal address of this buffer
 Input        : BufHandle hBuf  
 Output       : None
 Return Value : void
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
unsigned long buffer_get_phy_addr(BufHandle hBuf) {
	if(!hBuf || !hBuf->userAddr)
		return 0;

	if(hBuf->phyAddr)
		return hBuf->phyAddr;

	hBuf->phyAddr = CMEM_getPhys(hBuf->userAddr);
	return hBuf->phyAddr;
}

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
Int32 buffer_set_user_ptr(IN BufHandle hBuf, IN void *ptr) 
{
	if(!hBuf || !ptr)
		return E_INVAL;

	if(hBuf->flag & BUF_FLAG_REF) {
		hBuf->userAddr = ptr;
		return E_NO;
	}

	/*This is not a reference buffer */
	return E_MODE;
}

/*****************************************************************************
 Prototype    : buffer_copy
 Description  : Buffer copy, may use DMA in the future
 Input        : BufHandle hDstBuf  
                BufHandle hSrcBuf  
                Uint32 size        
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/1/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 buffer_copy(void *dest, void *src, Uint32 size)
{
	if(!dest || !src)
		return E_INVAL;

	memcpy(dest, src, size);
	return E_NO;
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
Int32 buffer_lock(BufHandle hBuf)
{
	if(hBuf)
		hBuf->flag |= BUF_FLAG_LCKED;

	return E_NO;
}

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
Int32 buffer_unlock(BufHandle hBuf)
{
	if(!hBuf)
		return E_INVAL;
	
	hBuf->flag &= ~BUF_FLAG_LCKED;
	return E_NO;
}

