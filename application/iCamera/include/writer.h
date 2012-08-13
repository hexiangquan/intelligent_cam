/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : writer.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/20
  Last Modified :
  Description   : writer.c header file
  Function List :
  History       :
  1.Date        : 2012/3/20
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __WRITER_H__
#define __WRITER_H__

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

extern Int32 mkdir_if_need(const char *dir);
extern Int32 write_file(const char *dirName, const char *fileName, const Int8 *header, Int32 hdrLen, const Int8 *data, Int32 len);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __WRITER_H__ */
