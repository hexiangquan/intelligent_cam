/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : h264_enc.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/1/31
  Last Modified :
  Description   : h264_enc.c header file
  Function List :
  History       :
  1.Date        : 2012/1/31
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __H264_ENC_H__
#define __H264_ENC_H__

#include "common.h"
#include "alg.h"

/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/

/*----------------------------------------------*
 * external routine prototypes                  *
 *----------------------------------------------*/
/**
 *	@brief		Video frame types.
 *
 *	@remarks	For the various @c VIDEO_xy_FRAME values, this frame type is
 *				interlaced where both top and bottom fields are
 *				provided in a single frame.  The first field is an "x"
 *				frame, the second field is "y" field.
 */
typedef enum {
	VID_NA_FRAME = -1,	/**< Frame type not available. */
	VID_I_FRAME = 0,	/**< Intra coded frame. */
	VID_P_FRAME = 1,	/**< Forward inter coded frame. */
	VID_B_FRAME = 2,	/**< Bi-directional inter coded frame. */
	VID_IDR_FRAME = 3,	/**< Intra coded frame that can be used for
						  *  refreshing video content.
						  */
} VideoFrameType;

typedef enum {
	H264_RC_CBR = 0, 	/**< constant Bitrate rate control */
	H264_RC_VBR,		/**< Variable Bitrate control.*/
	H264_RC_FQP,		/**< Constant quality, fixed QP */
	H264_RC_CVBR,		/**< Constrained variable bitrate */
	H264_RC_FFS,		/**< Fixed  Frame  size  Rate  Control */
	H264_RC_CUSTOM_CBR,	/**< customized version of CBR,  
						  *    reduce the breathing artifacts in encoded videos
						  */
	H264_RC_CUSTOM_VBR,	/**< customized version of VBR, targeted for sequences 
						  *	 with varying complexity distribution 
						  */
}H264RateControlMode;

typedef struct _H264EncInitParams {
	Int32	size;          	    /**< size of this struct */
	Int32	maxWidth;			/**< max width of image */
	Int32	maxHeight;			/**< max height of image */
	Int32	maxFrameRate;		/**< max frame rate in fps */
	Int32	maxBitRate;      	/**< Maximum bit rate, bits per second. */
    Int32 	inputChromaFormat;	/**< Chroma format for the input buffer, 
    								  *    ONLY FMT_YUV420SP is supported
 			                                   *   @see enum ChromaFormat in common.h
 			                                   */
	Int32	sliceFormat;		/* 1 -> encoded stream in NAL unit format, */
                                /* 0 -> encoded stream in bytes stream format */
}H264EncInitParams;

typedef struct _H264EncDynParams {
	Int32	size;           	/**< size of this struct */
	Int32	width;				/**< input image width */
	Int32 	height;				/**< input image height */
	Int32	frameRate;			/**< input frame rate in fps  */
	Int32 	targetBitRate;   	/**< Target bit rate in bits per second. */
	Int32	intraFrameInterval;	/**< The number of frames between two I
				                            *    frames.  For example, 30.
				                            */
	Int32 	interFrameInterval;/**< Number of B frames between two reference
			                                 *   frames; that is, the number of B frames
			                                 *   between two P frames or I/P frames.
			                                 *   DEFAULT(30).
			                                 *
			                                 *   @remarks   For example, this field will be:
			                                 *     - 0 - to use maxInterFrameInterval.
			                                 *     - 1 - 0 B frames between two reference
			                                 *       frames.
			                                 *     - 2 - 1 B frame between two reference
			                                 *       frames.
			                                 *     - 3 - 2 B frames between two reference
			                                 *       frames.
			                                 *     - and so on...
			                                 */
	Int32	forceFrame;        /**< Force the current (immediate) frame to be
			                                 *   encoded as a specific frame type.
			                                 *
			                                 *   @remarks   For example, this field will be:
			                                 *     - VID_NA_FRAME - No forcing of any
			                                 *       specific frame type for the frame.
			                                 *     - VID_I_FRAME - Force the frame to be
			                                 *       encoded as I frame.
			                                 *     - VID_IDR_FRAME - Force the frame to
			                                 *       be encoded as an IDR frame (specific
			                                 *       to H.264 codecs).
			                                 *     - VID_P_FRAME - Force the frame to be
			                                 *       encoded as a P frame.
			                                 *     - VID_B_FRAME - Force the frame to be
			                                 *       encoded as a B frame.
			                                 *
			                                 *   @sa VideoFrameType.
			                                 */	
	Int32	intraFrameQP;	    /*< Quant. param for I Slices (0-51), used when no-rate control */
	Int32	interFrameQP;		/*< Quant. Param for non - I Slices, used when no-rate control  */
	Int32 	initQP; 		    /*< Initial QP for Rate Control, default: 28 */
	Int32 	maxQP;		  		/*< Maximum QP to be used  Range[0,51] */
	Int32 	minQP;		  		/*< Minimum QP to be used  Range[0,51] */
	Int32 	rateCtrlMode;		/*< Algorithm to be used by Rate Ctrl Scheme, see H264RateControlMode */
							   	/*< => CBR, 1 => VBR, 2 => Fixed QP, 3=> CVBR,*/ 
							   	/*< 4=> FIXED_RC 5=> CBR custom1 6=> VBR custom1*/
	Int32 	maxDelay;		   	/*< max delay for rate control interms of ms,*/
							   	/*< set it to 1000 for 1 second delay  */
	Int32 	enablePicTimSEI;  	/*< Enable Picture Timing SEI */
	Int32 	idrFrameInterval;	/* IDR Frame Interval, shuld be bigger than I frame interval */
	Int32 	maxBitrateCVBR;  	/* Specifies the max Bitrate for CVBR Rate Control mode */
	Int32 	maxHighCmpxIntCVBR;	/* Specifies the maximum duration of increased complexity in minitues, 0~60 */		                                 
}H264EncDynParams;

typedef struct _H264EncInArgs {
	Int32	size;           	/* size of this struct */
	Int32	inputID; 			/* Identifier to attach with the corresponding encoded bit stream frames. */
	Int32	timeStamp;			/* Time stamp value of the frame to be placed in bit stream, 
								  * used only when enablePicTimSEI = 1 
								  */

}H264EncInArgs;

typedef struct _H264EncOutArgs {
	Int32	size;			/* size of this struct, should set by app */
	Int32	bytesGenerated;	/* number of bytes genearted during process */
	Int32	frameType;		/* encoded frame type, see VideoFrameType */
	Int32	outputID;		/* Output ID corresponding to the encoder buffer */
}H264EncOutArgs;

typedef AlgHandle	H264EncHandle;
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
/* Default init params */
extern const H264EncInitParams H264ENC_INIT_DEFAULT;
/* Default dynamic params */
extern const H264EncDynParams H264ENC_DYN_DEFAULT;
/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/
 
/* Only support ALG_CMD_SET_DYN_PARAMS command */
enum H264EncCmd {
	H264ENC_CMD_SET_DYN_PARAMS = ALG_CMD_SET_DYN_PARAMS,
};

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/* Functions of this module */
extern const AlgFxns H264ENC_ALG_FXNS;

/* Macros for easy use of h.264 encode alg */
#define h264_enc_create(initParams, dynParams)	\
	alg_create(&H264ENC_ALG_FXNS, initParams, dynParams)

#define h264_enc_delete(handle) \
	alg_delete(handle)

#define h264_enc_process(handle, inBuf, inArgs, outBuf, outArgs) \
	alg_process(handle, inBuf, inArgs, outBuf, outArgs)

#define h264_enc_control(handle, cmd, args) \
	alg_control(handle, cmd, args)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __H264_ENC_H__ */
