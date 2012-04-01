/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : detector.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/30
  Last Modified :
  Description   : detector for trigger
  Function List :
              detector_calc_speed
              detector_close
              detector_control
              detector_create
              detector_delete
              detector_open
              detector_run
              detector_set_params
  History       :
  1.Date        : 2012/3/30
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "detector.h"
#include "log.h"

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

/* tory ep detector fxns */
extern const DetectorFxns TORY_EP_FXNS;

/*****************************************************************************
 Prototype    : detector_close
 Description  : close detector so it will stop workinng
 Input        : DetectorHandle hDetector  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 detector_close(DetectorHandle hDetector)
{		
	Int32 ret = E_NO;
	
	if(!hDetector)
		return E_INVAL;

	/* if not opened, just return */
	if(!(hDetector->status & DETECTOR_STAT_OPENED))
		return E_NO;
	
	if(hDetector->fxns && hDetector->fxns->close) {
		ret = hDetector->fxns->close(hDetector);	
	}

	/* set status to close */
	if(!ret)
		hDetector->status &= ~DETECTOR_STAT_OPENED;

	return ret;
}

/*****************************************************************************
 Prototype    : detector_open
 Description  : open detector to start working
 Input        : DetectorHandle hDetector  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 detector_open(DetectorHandle hDetector)
{
	Int32 ret = E_NO;
	
	if(!hDetector)
		return E_INVAL;

	if(hDetector->status & DETECTOR_STAT_OPENED) {
		/* close first then open */
		detector_close(hDetector);
	}

	if(hDetector->fxns && hDetector->fxns->open) {
		ret = hDetector->fxns->open(hDetector);
	}

	/* set status to open */
	if(!ret)
		hDetector->status |= DETECTOR_STAT_OPENED;
	
	return ret;
}

/*****************************************************************************
 Prototype    : detector_set_params
 Description  : set detector params
 Input        : DetectorHandle hDetector        
                const CamDetectorParam *params  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 detector_set_params(DetectorHandle hDetector, const CamDetectorParam *params)
{
	Int32 ret = E_NO;
	
	if(hDetector->params.detecotorId != params->detecotorId) {
		/* try close opened detector */
		detector_close(hDetector);
		
		/* diff detector Id, change fxns */
		switch(params->detecotorId) {
		case DETECTOR_TORY_EP:
			hDetector->fxns = &TORY_EP_FXNS;
			break;
		case DETECTOR_TORY_CP:
			hDetector->fxns = NULL;
			break;
		default:
			ERR("unsupported detector: %d...", params->detecotorId);
			return E_INVAL;
		}

		hDetector->params = *params;
		/* open detector */
		ret = detector_open(hDetector);
	}else {
		/* same id, just update params */
		if(hDetector->fxns && hDetector->fxns->control)
			ret = hDetector->fxns->control(hDetector, DETECTOR_CMD_SET_PARAMS, (void *)params, sizeof(CamDetectorParam));
		if(!ret)
			hDetector->params = *params;
	}

	return ret;
}

/*****************************************************************************
 Prototype    : detector_create
 Description  : create detector handle
 Input        : const CamDetectorParam *params  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
DetectorHandle detector_create(const CamDetectorParam *params)
{
	DetectorHandle hDetector;
	
	if(!params) {
		ERR("NULL params");
		return NULL;
	}

	hDetector = calloc(1, sizeof(struct DetectorObj));
	if(!hDetector) {
		ERR("alloc memory failed...");
		return NULL;
	}

	if(detector_set_params(hDetector, params) != E_NO) {
		ERR("set detector params failed");
		goto err_quit;
	}
		
	return hDetector;
	
err_quit:
	
	free(hDetector);
	return NULL;
}

/*****************************************************************************
 Prototype    : detector_delete
 Description  : delete detecor module
 Input        : DetectorHandle hDetector  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 detector_delete(DetectorHandle hDetector)
{	
	if(!hDetector)
		return E_INVAL;

	/* try close and ignore error */
	detector_close(hDetector);
	
	free(hDetector);
	
	return E_NO;
}

/*****************************************************************************
 Prototype    : detector_run
 Description  : run detect
 Input        : DetectorHandle hDetector  
                CaptureInfo *capInfo      
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 detector_run(DetectorHandle hDetector, CaptureInfo *capInfo)
{
	if(!hDetector || !capInfo)
		return E_INVAL;

	if(hDetector->fxns && hDetector->fxns->detect)
		return hDetector->fxns->detect(hDetector, capInfo);

	return E_UNSUPT;
}

/*****************************************************************************
 Prototype    : detector_control
 Description  : control fxns
 Input        : DetectorHandle hDetector  
                DetectorCmd cmd           
                void *arg                 
                Int32 len                 
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 detector_control(DetectorHandle hDetector, DetectorCmd cmd, void *arg, Int32 len)
{
	Int32 ret = E_INVAL;
	
	if(!hDetector)
		return E_INVAL;

	switch(cmd) {
	case DETECTOR_CMD_SET_PARAMS:
		if(arg && len == sizeof(CamDetectorParam)) {
			ret = detector_set_params(hDetector, (CamDetectorParam *)arg);
		}
		break;
	case DETECTOR_CMD_GET_ID:
		if(arg && len >= sizeof(Uint16))
			*(Uint16 *)arg = hDetector->params.detecotorId;
		break;
	default:
		if(hDetector->fxns && hDetector->fxns->control)
			ret = hDetector->fxns->control(hDetector, cmd, arg, len);
		else
			ret = E_UNSUPT;
		break;
	}

	return ret;
}

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
Int32 detector_calc_speed(const CamDetectorParam *params, Int32 wayNum, Uint32 timeMs)
{
	Int32 speed = 0;
	
	if(params->loopDist[wayNum - 1] && timeMs) {
		/* Calc speed */
		speed = params->loopDist[wayNum - 1] * CMMS2KMH_FACTOR / timeMs; //unit: km/h
		
		/* Do speed modify if necessary */
		if(params->speedModifyRatio[wayNum - 1]) 
			speed = speed * params->speedModifyRatio[wayNum - 1] / 100;
	}

	return (Uint8)speed;
}

