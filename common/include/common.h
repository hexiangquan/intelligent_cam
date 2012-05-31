/******************************************************************************

  Copyright (C), 2001-2011,  Co., Ltd.

 ******************************************************************************
  File Name     : sysdef.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/1/7
  Last Modified :
  Description   : This file defines common error and types for system wide
  Function List :
  History       :
  1.Date        : 2012/1/7
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __COMMON_H__
#define __COMMON_H__

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include "ti/xdc/std.h"

/* Common error define */
#define E_NO			(0)		// No error
#define E_INVAL			(-1)	// Invalid argument
#define E_NOMEM			(-2)	// Alloc memory failed or static memory is not big enough
#define E_IO			(-3)	// Hardware error
#define E_BUSY			(-4)	// Device or resource busy
#define E_TIMEOUT		(-5)	// Operation timeout
#define E_NOSPC			(-6)	// No space left on device
#define E_CHECKSUM		(-7)	// Checksum error
#define E_CONNECT		(-8)	// Connect error
#define E_TRANS			(-9)	// Transfer error
#define E_REFUSED		(-10)	// Operation was refused
#define E_INVPATH		(-11)	// Invalid path
#define E_INVNAME		(-12)	// Invalid file name
#define E_MODE			(-13)	// Mode transfer error
#define E_NOTEXIST		(-14)	// File not exist
#define E_INVUSER		(-15)	// Invalid user name
#define E_INVPASSWD		(-16)	// Invalid password
#define E_AGAIN			(-17)	// Unkwon error, May retry
#define E_UNSUPT		(-18)	// Unsupported operation
#define E_REBOOT		(-19)	//system need reboot

/* Some common defines */
#if 0
#ifndef TRUE
#define TRUE 		1
#endif

#ifndef FALSE 
#define FALSE		0
#endif


typedef void			*Ptr;
typedef unsigned int  	Bool;
typedef char			Int8;
typedef short			Int16;
typedef int				Int32;
typedef unsigned int	Uint32;
typedef unsigned short	Uint16;
typedef unsigned char	Uint8;
#endif

/* Indicators of input/out arguments */
#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef INOUT
#define INOUT
#endif

#define SYS_FOREVER				(Uint32)(-1)

/* Macro for array size calc */
#define ARRAY_SIZE(x)			(sizeof(x)/sizeof(x[0]))

/* Alignment */
#define ROUND_UP(x, align) 		(((Int32) (x) + (align - 1)) & ~(align - 1))
#define ROUND_DOWN(x, align)	((Int32)(x) & ~(align - 1))  
#define ALIGNED(x, align) 		(((Int32)(x) & (align - 1)) == 0)

/* Compare */
#define MAX(a, b) 				((a) > (b) ? (a) : (b))
#define MIN(a, b) 				((a) < (b) ? (a) : (b))

/* Chroma Format define */
typedef enum _ChromaFormat {
	FMT_NA = -1,			/* Chroma format not applicable. */
	FMT_YUV_420P = 1,		/* YUV 4:2:0 planer. */
	FMT_YUV_422P = 2,		/* YUV 4:2:2 planer. */
	FMT_YUV_422IBE = 3,		/* YUV 4:2:2 interleaved (big endian). */
	FMT_YUV_422ILE = 4,		/* YUV 4:2:2 interleaved (little endian). */
	FMT_YUV_444P = 5,		/* YUV 4:4:4 planer. */
	FMT_YUV_411P = 6,		/* YUV 4:1:1 planer. */
	FMT_GRAY = 7,			/* Gray format. */
	FMT_RGB = 8,				/* RGB color format. */
	FMT_YUV_420SP = 9,		/* YUV 420 semi_planar format.(Luma 1st plane,
							  * CbCr interleaved 2nd plane)
							  */
	FMT_ARGB8888 = 10,		/* Alpha plane. */
	FMT_RGB555 = 11,		/* RGB 555 color format. */
	FMT_RGB565 = 12,		/* RGB 565 color format. */
	FMT_YUV_444ILE = 13,	/* YUV 4:4:4 interleaved (little endian). */
	FMT_BAYER_RGBG = 14,	/* Bayer, for 4 near pixels: Red(Top Left) Green(Top Right) 
							  * Blue(Down left) Green(Down Right) 
							  */	
	FMT_JPG = 30,			/* Format after jpeg encode */
	FMT_H264,				/* Format after h.264 encode */
	FMT_MPEG4,				/* Format after Mpeg4 encode */
	
	/** Default setting. */
	FMT_DEFAULT = FMT_YUV_420P
}ChromaFormat;

/* rectangle position */
typedef struct _Rectanagle {
	Uint16	left;
	Uint16	top;
	Uint16	width;
	Uint16	height;
}Rectanagle;

/* Image dimention */
typedef struct _ImgDimension {
	ChromaFormat	colorSpace;
	Uint32			size;			//size of image
	Uint16			width;
	Uint16			height;
	Uint16			bytesPerLine;
	Uint16			reserved;
}ImgDimension;

/* Common APIs */
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

const char *str_err(IN int err);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif
