/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     :path_name.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/12
  Last Modified :
  Description   : path_name.c header file
  Function List :
  History       :
  1.Date        : 2012/3/12
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __PATH_NAME_H__
#define __PATH_NAME_H__

#include "common.h"
#include "app_msg.h"

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

/* buffer size */
#define PATHNAME_MAX_LINE_SIZE		500
#define PATHNAME_MAX_CNT_NUM		4

/* default path name pattern */
#define PATHNAME_PATTERN_DEFAULT	"<IP>/<Y><M>/<D>/<Y><M><D><H><m><S><s>"

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/* module handle */
typedef struct PathNameObj *PathNameHandle;

/* output info */
typedef struct 
{
	Uint32		fileNum;							//how many file name generated
	const Int8 	*path[PATHNAME_MAX_CNT_NUM];		//path string for every file
	const Int8 	*fileName[PATHNAME_MAX_CNT_NUM];	//name string for every file
}PathNameInfo; //output info

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*****************************************************************************
 Prototype    : ftp_file_name_create
 Description  : create this module
 Input        : const Int8 *pattern  
                const Int8 *text     
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern PathNameHandle path_name_create(IN const Int8 *pattern, IN const Int8 *text);

/*****************************************************************************
 Prototype    : path_name_generate
 Description  : generate file path name
 Input        : PathNameHandle hPathName  
                const ImgMsg *imgBuf      
                PathNameInfo * info       
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 path_name_generate(IN PathNameHandle hPathName, IN const ImgMsg *imgBuf, OUT PathNameInfo *info);

/*****************************************************************************
 Prototype    : path_name_config
 Description  : config patter & text 
 Input        : PathNameHandle hPathName  
                const Int8 *pattern       
                const Int8 *text          
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 path_name_config(IN PathNameHandle hPathName, IN const Int8 *pattern, IN const Int8 *text);

/*****************************************************************************
 Prototype    : path_name_delete
 Description  : delete module
 Input        : PathNameHandle hPathName  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 path_name_delete(IN PathNameHandle hPathName);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __FTP_FILE_NAME_H__ */
