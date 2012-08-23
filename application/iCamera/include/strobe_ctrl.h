/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : strobe_ctrl.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/8/22
  Last Modified :
  Description   : strobe_ctrl.c header file
  Function List :
  History       :
  1.Date        : 2012/8/22
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __STROBE_CTRL_H__
#define __STROBE_CTRL_H__

#include "common.h"
#include "cam_io.h"
#include "cam_detector.h"

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

typedef struct  {
	const char 			*devName;
	CamStrobeCtrlParam	params;
	Uint32				checkPrd;	// period of time when checking by time, unit: second
	Int32				minSwitchCnt;
}StrobeCtrlAttrs;

typedef struct StrobeObj *StrobeHandle;

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*****************************************************************************
 Prototype    : strobe_ctrl_create
 Description  : create strobe ctrl object
 Input        : const char *devName               
                const CamStrobeCtrlParam *params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/22
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern StrobeHandle strobe_ctrl_create(const StrobeCtrlAttrs *attrs);

/*****************************************************************************
 Prototype    : strobe_ctrl_delete
 Description  : delete this object
 Input        : StrobeHandle hStrobe  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/22
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 strobe_ctrl_delete(StrobeHandle hStrobe);

/*****************************************************************************
 Prototype    : strobe_ctrl_set_cfg
 Description  : cfg strobe ctrl params
 Input        : StrobeHandle hStrobe              
                const CamStrobeCtrlParam *params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/22
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 strobe_ctrl_set_cfg(StrobeHandle hStrobe, const CamStrobeCtrlParam *params);

/*****************************************************************************
 Prototype    : strobe_ctrl_get_cfg
 Description  : get strobe ctrl params
 Input        : StrobeHandle hStrobe        
                CamStrobeCtrlParam *params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/22
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 strobe_ctrl_get_cfg(StrobeHandle hStrobe, CamStrobeCtrlParam *params);

/*****************************************************************************
 Prototype    : strobe_ctrl_set_check_params
 Description  : set check params for switch
 Input        : StrobeHandle hStrobe  
                Int32 minSwitchCnt    
                Uint32 checkPrd       
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/22
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 strobe_ctrl_set_check_params(StrobeHandle hStrobe, Int32 minSwitchCnt, Uint32 checkPrd);

/*****************************************************************************
 Prototype    : strobe_ctrl_auto_switch
 Description  : switch according to cfg
 Input        : StrobeHandle hStrobe     
                const DateTime *curTime  
                Uint16 lumVal            
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/22
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 strobe_ctrl_auto_switch(StrobeHandle hStrobe, const DateTime *curTime, Uint16 lumVal);

/*****************************************************************************
 Prototype    : strobe_ctrl_output_enable
 Description  : check and enable strobe for special cfg flags
 Input        : StrobeHandle hStrobe        
                const CaptureInfo *capInfo  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/23
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 strobe_ctrl_output_enable(StrobeHandle hStrobe, const CaptureInfo *capInfo);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __STROBE_CTRL_H__ */
