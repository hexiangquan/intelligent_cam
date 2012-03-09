/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : osd.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/2/17
  Last Modified :
  Description   : osd.c header file
  Function List :
  History       :
  1.Date        : 2012/2/17
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __OSD_H__
#define __OSD_H__

#include "common.h"
#include "alg.h"

/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/

/*----------------------------------------------*
 * external routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/
typedef enum _OsdMode {
	OSD_MODE_32x16 = 0,
	OSD_MODE_32x32,
	OSD_MODE_32x32_ROTATE_R,
	OSD_MODE_32x32_ROTATE_L,
	OSD_MODE_MAX
}OsdMode; 
 
typedef enum _OsdColor {
	OSD_COLOR_YELLOW = 0,
	OSD_COLOR_RED,
	OSD_COLOR_GREEN,
	OSD_COLOR_BLUE,
	OSD_COLOR_GRAY,
	OSD_COLOR_MAX
}OsdColor; 

typedef struct _OsdInitParams {
	Int32		size;			/* size of this struct */
	const Uint8 *hzk16Tab;		/* ptr to hzk table lib, unused, set to NULL */
	const Uint8 *asc16Tab;		/* ptr to ascII table lib, unused, set to NULL */
}OsdInitParams;

typedef struct _OsdDynParams {
	Int32			size;		/* size of this struct */
	ChromaFormat	imgFormat;	/* input image chroma format, only support YUV422P, YUV420P, YUV420SP */	
	Uint16			width;		/* image width */
	Uint16			height;		/* image height */
	OsdMode			mode;		/* osd mode, see OsdMode */
	OsdColor		color;		/* color of osd */
	
}OsdDynParams;

typedef struct _OsdInArgs {
	Int32			size;		/* size of this struct */
	Uint16			startX;		/* start horizontal position for osd */
	Uint16			startY;		/* start vertical position for osd */
	const char 		*strOsd;	/* string to be added */
}OsdInArgs;

/* Can set to NULL when call alg process */
typedef struct _OsdOutArgs {
	Int32			size;		/* size of this struct */
}OsdOutArgs;

typedef AlgHandle OsdHandle;

/*----------------------------------------------*
 * project-wide global variables                *
 *----------------------------------------------*/

/*----------------------------------------------*
 * module-wide global variables                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * constants                                    *
 *----------------------------------------------*/
/* Default init params */
extern const OsdInitParams OSD_INIT_DEFAULT;

/* Default dynamic params */
extern const const OsdDynParams OSD_DYN_DEFAULT;

/* Control cmds supported by this alg */
enum OsdCmd {
	OSD_CMD_SET_DYN_PARAMS = ALG_CMD_SET_DYN_PARAMS,
};
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

/* Functions of this module */
extern const AlgFxns OSD_ALG_FXNS;

/* Macros for easy use of img convert alg */
#define osd_create(initParams, dynParams)	\
	alg_create(&OSD_ALG_FXNS, initParams, dynParams)

#define osd_delete(handle) \
	alg_delete(handle)

#define osd_process(handle, inBuf, inArgs, outBuf, outArgs) \
	alg_process(handle, inBuf, inArgs, outBuf, outArgs)

#define osd_control(handle, cmd, args) \
	alg_control(handle, cmd, args)


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __OSD_H__ */
