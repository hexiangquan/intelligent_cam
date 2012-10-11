/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : detector.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/29
  Last Modified :
  Description   : detector.c header file
  Function List :
  History       :
  1.Date        : 2012/3/29
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __DETECTOR_H__
#define __DETECTOR_H__

#include "cam_detector.h"
#include "radar.h"

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
#define DETECTOR_STAT_OPENED		(1 << 0)
#define CMMS2KMH_FACTOR				36


/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

typedef struct DetectorObj *DetectorHandle;

/* cmd list */
typedef enum 
{
	/* set params, use CamDetectorParam for arg */
	DETECTOR_CMD_SET_PARAMS = 0xA500,
	/* get current detector ID */
	DETECTOR_CMD_GET_ID,
	/* get drvier fd for select, poll, etc. */
	DETECTOR_CMD_GET_FD,
	/* set recv timeout */
	DETECTOR_CMD_SET_TIMEOUT,
	/* start work */
	DETECTOR_CMD_START,
	/* stop work */
	DETECTOR_CMD_STOP,
	/* cmd for low level detector */
	DETECTOR_CMD_DEV = 0xB500	// cmd used for internal function
}DetectorCmd;

/* fxns for specific detector operations */
typedef struct {
	/* open detector */
	Int32 (*open)(IN DetectorHandle hDetector);
	/* close detector */
	Int32 (*close)(IN DetectorHandle hDetector);
	/* run detect */
	Int32 (*detect)(IN DetectorHandle hDetector, OUT CaptureInfo *capInfo);
	/* config params */
	Int32 (*control)(IN DetectorHandle hDetector, IN DetectorCmd cmd, IN void *arg, IN Int32 len);
}DetectorFxns;

/* macros for getting data from handle */
#define DETECTOR_GET_PARAMS(hDetector)	\
	(const CamDetectorParam *)(&hDetector->params)

#define DETECTOR_GET_PRIVATE(hDetector)	(hDetector->private)

#define DETECTOR_SET_PRIVATE(hDetector, data) \
	(hDetector->private = (void *)data)

#define DETECTOR_SET_FD(hDetector, fd) \
	((hDetector->fd) = (fd))

/* object of this module */
struct DetectorObj {
	Int32				fd;			//fd for driver
	const DetectorFxns	*fxns;
	CamDetectorParam	params;
	void				*private;
	Int32				status;
	RadarHandle			hRadar;
};

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*****************************************************************************
 Prototype    : detector_create
 Description  : create detector
 Input        : CamDetectorParam params  
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern DetectorHandle detector_create(const CamDetectorParam *params);

/*****************************************************************************
 Prototype    : detector_delete
 Description  : delete detector module
 Input        : DetectorHandle hDetector  
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 detector_delete(DetectorHandle hDetector);

/*****************************************************************************
 Prototype    : detector_run
 Description  : run detector 
 Input        : DetectorHandle hDetector  
                CaptureInfo *capInfo      
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 detector_run(DetectorHandle hDetector, CaptureInfo *capInfo);

/*****************************************************************************
 Prototype    : detector_control
 Description  : set cmds
 Input        : DetectorHandle hDetector  
                DetectorCmd cmd           
                void *arg                 
                Int32 len                 
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 detector_control(DetectorHandle hDetector, DetectorCmd cmd, void *arg, Int32 len);

/*****************************************************************************
 Prototype    : detector_calc_speed
 Description  : calc speed
 Input        : const CamDetectorParam *params  
                Int32 wayNum                    
                Uint32 timeMs                   
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/30
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 detector_calc_speed(const CamDetectorParam *params, Int32 wayNum, Uint32 timeMs);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __DETECTOR_H__ */
