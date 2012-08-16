/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : cam_file.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/4/12
  Last Modified :
  Description   : define for file save and read 
  Function List :
  History       :
  1.Date        : 2012/4/12
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __CAM_FILE_H__
#define __CAM_FILE_H__
	
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
#define CAM_SD_NUM	1
	
/* path for file save */
#define FILE_SAVE_PATH			"/media/mmc"

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/*
 * Cam local file params 
 */

typedef struct _CamFileInfoHdr
{
	Uint32	type;				//Type of this file, 0--file, 1--dir
	Uint32	size;				//Size of this file in bytes
}CamFileInfoHdr;

enum CamFileType
{
	FILE_TYPE_NORMAL = 0,		//Normal file
	FILE_TYPE_DIR = 1,			//Directory
	FILE_TYPE_OTHER,			//Other type
};

/*
 * firmware type
 */
typedef enum {
	CAM_UPDATE_ARM = 0,
	CAM_UPDATE_DSP,
	CAM_UPDATE_FPGA,
}CamUpdateType;

#endif

