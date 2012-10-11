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
#include "detector_uart_generic.h"

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

	if(hDetector->fd > 0) {
		close(hDetector->fd);
		hDetector->fd = -1;
	}

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
		case DETECTOR_IO:
		case DETECTOR_VIDEO_TRIG:
			ret = E_UNSUPT;
			break;
		default:
			hDetector->fxns = &DETECTOR_UART_FXNS;
			break;
		}

		hDetector->params = *params;
		/* open detector */
		if(!ret)
			ret = detector_open(hDetector);
	}else {
		/* same id, just update params */
		if(hDetector->fxns && hDetector->fxns->control)
			ret = hDetector->fxns->control(hDetector, DETECTOR_CMD_SET_PARAMS, (void *)params, sizeof(CamDetectorParam));
		if(!ret)
			hDetector->params = *params;
	}

	if(params->radarId != RADAR_NONE) {
		RadarParams radarCfg;

		radarCfg.timeout = params->radarTimeout;
		radarCfg.type = params->radarId;
		if(!hDetector->hRadar) {
			/* create radar handle */
			hDetector->hRadar = radar_create(&radarCfg);
			if(!hDetector->hRadar) {
				ERR("create radar handle failed!");
				ret = E_INVAL;
			}
		} else {
			/* already created, set params */
			ret = radar_set_params(hDetector->hRadar, &radarCfg);
		}
	} else {
		/* don't use radar */
		if(hDetector->hRadar) {
			ret = radar_delete(hDetector->hRadar);
			hDetector->hRadar = NULL;
		}
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

	if(hDetector->hRadar)
		radar_delete(hDetector->hRadar);
	
	free(hDetector);
	
	return E_NO;
}

/*****************************************************************************
 Prototype    : detector_radar_speed_detect
 Description  : detect speed using radar
 Input        : DetectorHandle hDetector  
                CaptureInfo *capInfo      
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/10/11
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 detector_radar_speed_detect(DetectorHandle hDetector, CaptureInfo *capInfo)
{
	/* calc index for speed modify ratio */
	Int32  index = capInfo->triggerInfo[0].wayNum - 1;
	CamDetectorParam *params = &hDetector->params;
	
	Uint32 speed;		
	Int32  ret = radar_detect_speed(hDetector->hRadar, &speed, params->speedModifyRatio[index]);

	if(!ret) {
		capInfo->triggerInfo[0].speed = (Uint8)speed;
		if(speed > params->calcSpeed) {
			capInfo->triggerInfo[0].flags |= TRIG_INFO_OVERSPEED;
		} else {
			if(params->greenLightCapFlag & DETECTOR_FLAG_OVERSPEED_CAP)
				return E_AGAIN; /* capture only when over speed */
		}
	}
	
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

	Int32 ret = E_UNSUPT;
	
	if(hDetector->fxns && hDetector->fxns->detect)
		ret = hDetector->fxns->detect(hDetector, capInfo);

	if(!ret) {
		/* need delay for capture next frame */
		if(capInfo->flags & CAPINFO_FLAG_DELAY_CAP)
			usleep(hDetector->params.capDelayTime * 1000);
		/* set limit speed */
		capInfo->limitSpeed = hDetector->params.limitSpeed;
		if(hDetector->hRadar)
			ret = detector_radar_speed_detect(hDetector, capInfo);
	}

	return ret;
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
		if(arg && len >= sizeof(Uint16)) {
			*(Uint16 *)arg = hDetector->params.detecotorId;
			ret = E_NO;
		}
		break;
	case DETECTOR_CMD_GET_FD:
		if(len >= sizeof(Int32)) {
			*(Int32 *)arg = hDetector->fd;
			ret = E_NO;
		}
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

	return speed;
}

extern Int32 tory_cp_test();
extern Int32 tory_ep_test();
extern Int32 tory_ep2_test();
/* test for radar */
extern Int32 radar_test();


/*****************************************************************************
 Prototype    : detector_test
 Description  : test detector module
 Input        : None
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/5/21
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 detector_test()
{
	CamDetectorParam params;
	Int32 err;

	bzero(&params, sizeof(params));
	params.detecotorId = DETECTOR_TORY_EP;
	params.capDelayTime = 0;
	params.redLightCapFlag = DETECTOR_FLAG_LOOP1_POS_CAP | DETECTOR_FLAG_LOOP1_NEG_CAP | DETECTOR_FLAG_LOOP2_NEG_CAP;
	params.greenLightCapFlag = DETECTOR_FLAG_LOOP1_NEG_CAP;
	params.retrogradeCapFlag = DETECTOR_FLAG_LOOP1_POS_CAP | DETECTOR_FLAG_LOOP2_POS_CAP | DETECTOR_FLAG_LOOP2_NEG_CAP;
	params.loopDist[0] = 500;
	params.loopDist[1] = 450;
	params.loopDist[2] = 300;
	params.loopDist[3] = 250;
	params.limitSpeed = 80;
	params.calcSpeed = 88;
	params.speedModifyRatio[0] = 100;
	params.speedModifyRatio[1] = 110;
	params.speedModifyRatio[2] = 100;
	params.speedModifyRatio[3] = 90;
	
	/* create detector */
	DetectorHandle hDetector = detector_create(&params);
	assert(hDetector);

	/* test control */
	err = detector_control(hDetector, DETECTOR_CMD_START, NULL, 0);
	assert(err == E_NO);

	Int32 fd;
	err = detector_control(hDetector, DETECTOR_CMD_GET_FD, &fd, sizeof(fd));
	assert(err == E_NO);
	assert(fd > 0);

	params.calcSpeed = 90;
	err = detector_control(hDetector, DETECTOR_CMD_SET_PARAMS, &params, sizeof(params));
	assert(err == E_NO);

	Int32 timeout = 5000;
	err = detector_control(hDetector, DETECTOR_CMD_SET_TIMEOUT, &timeout, sizeof(timeout));
	assert(err == E_NO);

	DBG("\nStart testing tory ep detector...");
	err = tory_ep_test();
	assert(err == E_NO);

	DBG("\nStart testing tory ep2 detector...");
	err = tory_ep2_test();
	assert(err == E_NO);

	DBG("\nStart testing tory cp detector...");
	err = tory_cp_test();
	assert(err == E_NO);

	err = detector_control(hDetector, DETECTOR_CMD_STOP, NULL, 0);
	assert(err == E_NO);

	err = detector_delete(hDetector);
	assert(err == E_NO);

	err = radar_test();
	assert(err == E_NO);

	DBG("%s done", __func__);

	return E_NO;
	
}

