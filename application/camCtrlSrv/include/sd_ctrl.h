/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : sd_ctrl.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/5/7
  Last Modified :
  Description   : sd_ctrl.c header file
  Function List :
  History       :
  1.Date        : 2012/5/7
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __SD_CTRL_H__
#define __SD_CTRL_H__

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
#define SD_MNT_FMT		"/media/mmcblk%dp1"		//mount format for sd card
#define SD_MAX_ID		1					//max number of sd id

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/




#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*****************************************************************************
 Prototype    : sd_get_root_path
 Description  : get sd root path name
 Input        : Int32 id    
                void *buf   
                Int32 size  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/5/7
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 sd_get_root_path(Int32 id, void *buf, Int32 size);

/*****************************************************************************
 Prototype    : sd_get_dir_info
 Description  : get sd dir info
 Input        : const char *pathName  
                Int8 *buf             
                Int32 size            
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/5/7
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 sd_get_dir_info(const char *pathName, Int8 *buf, Int32 size);

/*****************************************************************************
 Prototype    : sd_del_dir
 Description  : delete a dir and all the files in it
 Input        : const char *pathName  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/5/7
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 sd_del_dir(const char *pathName);

/*****************************************************************************
 Prototype    : sd_format
 Description  : format sd card
 Input        : Int32 id  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/5/7
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 sd_format(Int32 id);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __SD_CTRL_H__ */
