/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : day_night_switch.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/8/16
  Last Modified :
  Description   : day_night_switch.c header file
  Function List :
  History       :
  1.Date        : 2012/8/16
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __DAY_NIGHT_SWITCH_H__
#define __DAY_NIGHT_SWITCH_H__

#include "cam_status.h"
#include "msg.h"

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

typedef struct _DayNightSwitchObj *DayNightHandle;

typedef struct _DayNightAttrs {
	CamDayNightModeCfg	cfg;
	Int32				minSwitchCnt;
	const char			*dstMsg;
	Int32				cmd;
}DayNightAttrs;

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*****************************************************************************
 Prototype    : day_night_create
 Description  : create module
 Input        : const DayNightAttrs *attrs  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/16
    Author       : Sun
    Modification : Created function

*****************************************************************************/
DayNightHandle day_night_create(const DayNightAttrs *attrs);

/*****************************************************************************
 Prototype    : day_night_delete
 Description  : delete module
 Input        : DayNightHandle hDayNight  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/16
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 day_night_delete(DayNightHandle hDayNight);

/*****************************************************************************
 Prototype    : day_night_cfg_params
 Description  : cfg switch mode params
 Input        : DayNightHandle hDayNight       
                const CamDayNightModeCfg *cfg  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/16
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 day_night_cfg_params(DayNightHandle hDayNight, const CamDayNightModeCfg *cfg);

/*****************************************************************************
 Prototype    : day_night_cfg_min_switch_cnt
 Description  : cfg min switch count
 Input        : DayNightHandle hDayNight  
                Int32 minCnt              
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/16
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 day_night_cfg_min_switch_cnt(DayNightHandle hDayNight, Int32 minCnt);

/*****************************************************************************
 Prototype    : day_night_cfg_dst_msg
 Description  : cfg dest msg for switch notification
 Input        : DayNightHandle hDayNight  
                const char *msgName       
                Int32 cmd                 
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/16
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 day_night_cfg_dst_msg(DayNightHandle hDayNight, const char *msgName, Int32 cmd);

/*****************************************************************************
 Prototype    : day_night_check_by_time
 Description  : check by time
 Input        : DayNightHandle hDayNight  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/16
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 day_night_check_by_time(DayNightHandle hDayNight, MsgHandle hCurMsg);

/*****************************************************************************
 Prototype    : day_night_check_by_lum
 Description  : check day/night mode change by avg lum value
 Input        : DayNightHandle hDayNight  
                Uint16 lumVal             
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/16
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 day_night_check_by_lum(DayNightHandle hDayNight, Uint16 lumVal, MsgHandle hCurMsg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __DAY_NIGHT_SWITCH_H__ */
