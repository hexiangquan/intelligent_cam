/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : dsp_update.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/8/27
  Last Modified :
  Description   : dsp_update.c header file
  Function List :
  History       :
  1.Date        : 2012/8/27
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __DSP_UPDATE_H__
#define __DSP_UPDATE_H__
	
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

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern Int32 dsp_update(const void *data, size_t len, Uint32 checksum);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __DSP_UPDATE_H__ */
