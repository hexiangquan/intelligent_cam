/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : self_test.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/5/21
  Last Modified :
  Description   : self_test.c header file
  Function List :
  History       :
  1.Date        : 2012/5/21
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __SELF_TEST_H__
#define __SELF_TEST_H__
	
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
#define SELF_TEST_DETECTOR		(1 << 0)
#define SELF_TEST_PATHNAME		(1 << 1)
#define SELF_TEST_FTPUPLOAD		(1 << 2)
#define SELF_TEST_LOCALSND		(1 << 3)

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern Int32 self_test(Int32 flags);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __SELF_TEST_H__ */
