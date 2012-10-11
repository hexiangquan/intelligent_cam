/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : radar.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/10/10
  Last Modified :
  Description   : radar.c header file
  Function List :
  History       :
  1.Date        : 2012/10/10
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __RADAR_H__
#define __RADAR_H__

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

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/
typedef struct SpeedRadarObj *RadarHandle;

typedef struct {
	Uint16		type;			//Type of radar, see enum define at cam_detector.h 
	Uint16		timeout;		//timeout for recieve radar detection data
}RadarParams;

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*****************************************************************************
 Prototype    : radar_create
 Description  : create radar module
 Input        : const RadarParams *params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/10/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern RadarHandle radar_create(const RadarParams *params);

/*****************************************************************************
 Prototype    : radar_delete
 Description  : delete radar module
 Input        : RadarHandle hRadar  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/10/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 radar_delete(RadarHandle hRadar);

/*****************************************************************************
 Prototype    : radar_set_params
 Description  : set params for radar module
 Input        : RadarHandle hRadar         
                const RadarParams *params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/10/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 radar_set_params(RadarHandle hRadar, const RadarParams *params);

/*****************************************************************************
 Prototype    : radar_detect_speed
 Description  : detect speed for radar
 Input        : RadarHandle hRadar  
                Uint32 *speed       
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/10/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 radar_detect_speed(RadarHandle hRadar, Uint32 *speed, Uint32 modifyRatio);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __RADAR_H__ */
