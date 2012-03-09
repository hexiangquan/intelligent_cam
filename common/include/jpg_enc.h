/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : jpg_enc.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/1/31
  Last Modified :
  Description   : jpg_enc.c header file
  Function List :
  History       :
  1.Date        : 2012/1/31
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __JPG_ENC_H__
#define __JPG_ENC_H__

#include "common.h"
#include "alg.h"

/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/

/*----------------------------------------------*
 * external routine prototypes                  *
 *----------------------------------------------*/
typedef struct _JpgEncInitParams {
	Int32	size;           /* size of this struct, must equal to sizeof(JpgEncParams) */
	Int32	maxWidth;		/* max width of image */
	Int32	maxHeight;		/* max height of image */
}JpgEncInitParams;

typedef struct _JpgEncDynParams {
	Int32	size;           /* size of this struct */
	Int32	width;			/* input image width */
	Int32 	height;			/* input image height */
	Int32	inputFormat;	/* input format, enum type of ChromaFormat in common.h 
							  * only support FMT_YUV420P, FMT_YUV422P, 
							  * FMT_YUV422ILE(default), and FMT_YUV420SP
							  */
	Uint16	quality;		/* encode quality, must between 2~97 */
	Uint16	rotation;		/* degree of anticlockwise roation, must be 0(default), 90, 180, 270*/
}JpgEncDynParams;

typedef struct _JpgEncInArgs {
	Int32	size;           /* size of this struct */
	void	*appendData; 	/* addtional data, if no data to append, set to NULL */
	Int32	appendSize;		/* size of append data */
	Uint32	endMark;		/* end mark for append data, last 4 bytes of encoded data */
}JpgEncInArgs;

typedef struct _JpgEncOutArgs {
	Int32	size;			/* size of this struct, should set by app */
	Int32	bytesGenerated;	/* number of bytes genearted during process */
}JpgEncOutArgs;

enum JpgEncCmd {
	JPGENC_CMD_SET_DYN_PARAMS = ALG_CMD_SET_DYN_PARAMS,
};

typedef AlgHandle JpgEncHandle;
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
 
/* Only support ALG_CMD_SET_DYN_PARAMS command */

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/* Functions of this module */
extern const AlgFxns JPGENC_ALG_FXNS;

/* Macros for easy use of img convert alg */
#define jpg_enc_create(initParams, dynParams)	\
	alg_create(&JPGENC_ALG_FXNS, initParams, dynParams)

#define jpg_enc_delete(handle) \
	alg_delete(handle)

#define jpg_enc_process(handle, inBuf, inArgs, outBuf, outArgs) \
	alg_process(handle, inBuf, inArgs, outBuf, outArgs)

#define jpg_enc_control(handle, cmd, args) \
	alg_control(handle, cmd, args)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __JPG_ENC_H__ */
