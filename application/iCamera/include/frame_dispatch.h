/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : frame_dispatch.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/8
  Last Modified :
  Description   : frame_dispatch.c header file
  Function List :
  History       :
  1.Date        : 2012/3/8
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __FRAME_DISPATCH_H__
#define __FRAME_DISPATCH_H__

#include "app_msg.h"
#include "params_mng.h"

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
#define FD_FLAG_SAVE_ONLY		(1 << 0)	//directly save to local file system
#define FD_FLAG_NOT_FREE_BUF	(1 << 1)	//do not free buffer after dispatch


/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/* server status */
typedef enum {
	FT_NET_UNAVAILABLE = -1,	/* net is not ready */
	FT_SRV_UNCONNECTED = 0,		/* server has not been connected */
	FT_SRV_CONNECTED,			/* connect server ok */	
}FrameTxStatus;

typedef enum {
	FRAME_ENC_OFF = 0,
	FRAME_ENC_ON,
}FrameEncMode;

typedef struct FrameDispObj *FrameDispHandle;

typedef struct {
	const char 		*savePath;		/* path to save file */
	ParamsMngHandle	hParamsMng;		/* for params update */
}FrameDispInfo;

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*****************************************************************************
 Prototype    : frame_disp_create
 Description  : create file dispatch handle
 Input        : FrameTxStatus curStat  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/8
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern FrameDispHandle frame_disp_create(FrameTxStatus curStat, FrameEncMode encMode, FrameDispInfo *info);

/*****************************************************************************
 Prototype    : frame_disp_delete
 Description  : free frame dispatch handle
 Input        : FrameDispHandle hFrameDisp  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/8
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 frame_disp_delete(FrameDispHandle hFrameDisp);

/*****************************************************************************
 Prototype    : frame_disp_run
 Description  : run frame dispatch
 Input        : FrameDispHandle hFrameDisp  
                ImgMsg *frameBuf            
                const char *dstMsgName      
                Int32 flags                 
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/8
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 frame_disp_run(FrameDispHandle hFrameDisp, MsgHandle hMsg, ImgMsg *frameBuf, const char *dstMsgName, Int32 flags);

/*****************************************************************************
 Prototype    : frame_disp_set_tx_status
 Description  : set current send status
 Input        : FrameDispHandle hFrameDisp  
                FrameTxStatus status        
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/9
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 frame_disp_set_tx_status(FrameDispHandle hFrameDisp, FrameTxStatus status);

/*****************************************************************************
 Prototype    : frame_disp_set_encode_mode
 Description  : set encode mode
 Input        : FrameDispHandle hFrameDisp  
                FrameEncMode mode           
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/9
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 frame_disp_set_encode_mode(FrameDispHandle hFrameDisp, FrameEncMode mode);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __FRAME_DISPATCH_H__ */
