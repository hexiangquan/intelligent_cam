/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : strobe_ctrl.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/8/23
  Last Modified :
  Description   : strobe ctrl module
  Function List :
              strobe_ctrl_auto_switch
              strobe_ctrl_create
              strobe_ctrl_delete
              strobe_ctrl_get_cfg
              strobe_ctrl_output_enable
              strobe_ctrl_set_cfg
              strobe_ctrl_set_check_params
              strobe_ctrl_sync_hw
              strobe_ctrl_test
              strobe_switch_by_lum
              strobe_switch_by_time
  History       :
  1.Date        : 2012/8/23
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "strobe_ctrl.h"
#include "log.h"
#include "ext_io.h"
#include "cam_time.h"
#include <sys/ioctl.h>

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
struct StrobeObj {
	CamStrobeCtrlParam	params;
	int					fdExtIo;
	time_t				lastCheckTime;
	time_t				checkPrd;
	Bool				outputEnable;
	Uint16				autoSwitchMask;
	int					lumKeepCnt;
	int					minSwitchCnt;
	Uint16				curEnIndex;		// current enable index for enable in turn
	Uint16				enableCnt;		// count of strobes enabled
};

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
StrobeHandle strobe_ctrl_create(const StrobeCtrlAttrs *attrs)
{
	assert(attrs);
	assert(attrs->devName);

	StrobeHandle hStrobe;

	/* alloc mem */
	hStrobe = calloc(1, sizeof(struct StrobeObj));
	if(!hStrobe) {
		ERR("alloc mem failed.");
		return NULL;
	}

	/* open device for hardware ctrl */
	hStrobe->fdExtIo = open(attrs->devName, O_RDWR);
	if(hStrobe->fdExtIo < 0) {
		ERRSTR("can't open %s", attrs->devName);
		goto exit;
	}

	/* config params */
	Int32 err = strobe_ctrl_set_cfg(hStrobe, &attrs->params);
	if(err)
		goto exit;

	strobe_ctrl_set_check_params(hStrobe, attrs->minSwitchCnt, attrs->checkPrd);
	return hStrobe;

exit:

	strobe_ctrl_delete(hStrobe);
	return NULL;
	
}

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
Int32 strobe_ctrl_delete(StrobeHandle hStrobe)
{
	if(!hStrobe)
		return E_INVAL;

	if(hStrobe->fdExtIo > 0)
		close(hStrobe->fdExtIo);

	free(hStrobe);

	return E_NO;
}

/*****************************************************************************
 Prototype    : strobe_ctrl_sync_hw
 Description  : cfg hw for strobe status
 Input        : StrobeHandle hStrobe  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/22
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 strobe_ctrl_sync_hw(StrobeHandle hStrobe)
{
	if(hStrobe->fdExtIo > 0) {
		/* sync with hardware */
		struct hdcam_strobe_info info;
		CamStrobeCtrlParam *params = &hStrobe->params;

		info.status = params->status & 0x0F;
		info.sigVal = params->sigVal & 0x0F;
		info.mode = params->outMode & 0x0F;
		info.syncAC = params->acSyncEn & 0x0F;
		info.offset = params->offset;
		
		if(ioctl(hStrobe->fdExtIo, EXTIO_S_STROBE, &info) < 0) {
			return E_IO;
		}
	}

	return E_NO;
}

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
Int32 strobe_ctrl_set_cfg(StrobeHandle hStrobe, const CamStrobeCtrlParam *params)
{
	if(!hStrobe)
		return E_INVAL;
	
	/* validate data */
	if( params->switchMode >= CAM_STROBE_SWITCH_MAX || 
		params->enStartHour > 23 || params->enStartMin > 59 ||
		params->enEndHour > 23 || params->enEndMin > 59) {
		ERR("invalid strobe params, time must be valid, hour[0-23], min[0-59]");
		return E_INVAL;
	}

	hStrobe->params = *params;
	/* record which strobes need to auto switch */
	hStrobe->autoSwitchMask = (params->ctrlFlags & 0x0F);
	if(!hStrobe->autoSwitchMask) {
		/* all strobe auto switch are disabled, need not switch */
		hStrobe->params.switchMode = CAM_STROBE_SWITCH_OFF;
	}
	
	/* disable if need auto switch */
	if(params->switchMode != CAM_STROBE_SWITCH_OFF) {
		hStrobe->params.status &= ~(hStrobe->autoSwitchMask);	
		hStrobe->outputEnable = FALSE;
	} else
		hStrobe->outputEnable = TRUE;
	
	/* check auto switch enable num */
	hStrobe->enableCnt = 0;
	if(params->ctrlFlags & 0x01)
		hStrobe->enableCnt++;
	if(params->ctrlFlags & 0x02)
		hStrobe->enableCnt++;
	if(params->ctrlFlags & 0x04)
		hStrobe->enableCnt++;

	/* convert orginal threshold from 8 bits to 12 bits so we can compare with raw info */
	hStrobe->params.thresholdOn <<= 4;
	hStrobe->params.thresholdOff <<= 4;
	hStrobe->lumKeepCnt = 0;
	DBG("strobe set switch mode: %d, enable flag: 0x%x, cur status: 0x%x",
		params->switchMode, params->ctrlFlags, params->status);

	return strobe_ctrl_sync_hw(hStrobe);
}

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
Int32 strobe_ctrl_get_cfg(StrobeHandle hStrobe, CamStrobeCtrlParam *params)
{
	if(!hStrobe || !params)
		return E_INVAL;

	if(hStrobe->fdExtIo > 0) {
		/* read from hardware */
		struct hdcam_strobe_info info;

		if(ioctl(hStrobe->fdExtIo, EXTIO_G_STROBE, &info) < 0) {
			return E_IO;
		}

		hStrobe->params.status = info.status;
		hStrobe->params.offset = info.offset;
		hStrobe->params.sigVal = info.sigVal;
		hStrobe->params.acSyncEn = info.syncAC;
		hStrobe->params.outMode = info.mode;
	}

	*params = hStrobe->params;
	/* convert back to 8 bits -- original value */
	params->thresholdOn >>= 4;
	params->thresholdOff >>= 4;
	DBG("strobe get cfg: time: %u:%u - %u:%u", params->enStartHour, params->enStartMin,
		params->enEndHour, params->enEndMin);
	return E_NO;
}

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
Int32 strobe_ctrl_set_check_params(StrobeHandle hStrobe, Int32 minSwitchCnt, Uint32 checkPrd)
{
	if(!hStrobe)
		return E_INVAL;

	hStrobe->minSwitchCnt = minSwitchCnt;
	hStrobe->checkPrd = checkPrd;

	return E_NO;
}

/*****************************************************************************
 Prototype    : strobe_switch_by_time
 Description  : switch strobe by time
 Input        : StrobeHandle hStrobe     
                const DateTime *curTime  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/22
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 strobe_switch_by_time(StrobeHandle hStrobe, const DateTime *curTime)
{
	if(!curTime)
		return E_INVAL;

	if(time(NULL) < hStrobe->lastCheckTime + hStrobe->checkPrd) {
		/* not time to check */
		return E_AGAIN;
	}

	/* Convert to minitues */
	Uint32 curMin = HOUR_TO_MIN(curTime->hour, curTime->minute);
	Uint32 enableTime = HOUR_TO_MIN(hStrobe->params.enStartHour, hStrobe->params.enStartMin);
	Uint32 disableTime = HOUR_TO_MIN(hStrobe->params.enEndHour, hStrobe->params.enEndMin);
	
	hStrobe->lastCheckTime = time(NULL);
	
	/* Check current time */
	if(curMin >= disableTime && curMin < enableTime) {
		if(hStrobe->outputEnable) {
			/* need turn off */
			hStrobe->outputEnable = FALSE;
			return E_NO;
		}
	} else {
		/* enable output */
		if(!hStrobe->outputEnable) {
			hStrobe->outputEnable = TRUE;
			return E_NO;
		}
	}

	return E_AGAIN;
}

/*****************************************************************************
 Prototype    : strobe_switch_by_lum
 Description  : switch strobe by env lum value
 Input        : StrobeHandle hStrobe  
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
Int32 strobe_switch_by_lum(StrobeHandle hStrobe, Uint16 lumVal)
{
	if(hStrobe->outputEnable) {
		if(lumVal <= hStrobe->params.thresholdOff) 
			hStrobe->lumKeepCnt = 0;
		else
			hStrobe->lumKeepCnt++;
	} else {
		if(lumVal > hStrobe->params.thresholdOn) 
			hStrobe->lumKeepCnt = 0;
		else
			hStrobe->lumKeepCnt++;
	}

	if(hStrobe->lumKeepCnt > hStrobe->minSwitchCnt) {
		if(hStrobe->outputEnable) {
			hStrobe->outputEnable = FALSE;
		} else
			hStrobe->outputEnable = TRUE;

		hStrobe->lumKeepCnt = 0;
		return E_NO;
	}

	return E_AGAIN;
}

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
Int32 strobe_ctrl_auto_switch(StrobeHandle hStrobe, const DateTime *curTime, Uint16 lumVal)
{
	if(!hStrobe)
		return E_INVAL;

	Int32 err = E_MODE;
	
	if(hStrobe->params.switchMode == CAM_STROBE_SWITCH_TIME)
		err = strobe_switch_by_time(hStrobe, curTime);
	else if(hStrobe->params.switchMode == CAM_STROBE_SWITCH_AUTO)
		err = strobe_switch_by_lum(hStrobe, lumVal);

	if(err)
		return err;
	
	if(hStrobe->outputEnable) {
		if(!(hStrobe->params.status & hStrobe->autoSwitchMask)) {
			/* enable output */
			DBG("enable strobe output, status: 0x%x, mask: 0x%x.", hStrobe->params.status, 
				hStrobe->autoSwitchMask);
			hStrobe->params.status |= hStrobe->autoSwitchMask;
			err = strobe_ctrl_sync_hw(hStrobe);
		}
	} else {
		if(hStrobe->params.status & hStrobe->autoSwitchMask) {
			/* disable output */
			DBG("disable strobe output.");
			hStrobe->params.status &= ~(hStrobe->autoSwitchMask);
			err = strobe_ctrl_sync_hw(hStrobe);
		} 
	}

	return err;
}


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
Int32 strobe_ctrl_output_enable(StrobeHandle hStrobe, const CaptureInfo *capInfo)
{
	if(!hStrobe || !capInfo)
		return E_INVAL;

	/* output has already been disabled or no strobe is enabled for auto switch */
	if(!hStrobe->outputEnable || !hStrobe->enableCnt)
		return E_NO;

	/* check if we need enbale according to way num */
	Uint16 status = 0;
	if(hStrobe->params.ctrlFlags & CAM_STROBE_FLAG_EN_BY_WAY) {
		if(!capInfo->capCnt)
			return E_AGAIN;
		/* only enable the strobe of the way */
		Uint16 shift = (capInfo->triggerInfo[0].wayNum - 1) % hStrobe->enableCnt;
		status = (1 << shift);
	} else if(hStrobe->params.ctrlFlags & CAM_STROBE_FLAG_EN_IN_TURN) {
		/* turn on each strobe in turn */
		status = (1 << hStrobe->curEnIndex);
		hStrobe->curEnIndex++;
		hStrobe->curEnIndex %= hStrobe->enableCnt;
	} else 
		return E_NO;	// need not special enable

	/* only enable strobe that needs auto switch */
	hStrobe->params.status = (status & hStrobe->autoSwitchMask);
	
	return strobe_ctrl_sync_hw(hStrobe);
}

/* self test function */
Int32 strobe_ctrl_test()
{
	StrobeCtrlAttrs attrs;

	bzero(&attrs, sizeof(attrs));
	attrs.devName = "/dev/extio";
	attrs.checkPrd = 10;
	attrs.minSwitchCnt = 30;
	attrs.params.status = 0x03;
	attrs.params.switchMode = CAM_STROBE_SWITCH_OFF;
	attrs.params.ctrlFlags = CAM_STROBE_FLAG_AUTO0 | CAM_STROBE_FLAG_AUTO1 | CAM_STROBE_FLAG_AUTO2;
	attrs.params.thresholdOn = 200;
	attrs.params.thresholdOff = 230;
	attrs.params.offset = 500;
	attrs.params.enStartHour = 18;
	attrs.params.enStartMin = 10;
	attrs.params.enEndHour = 5;
	attrs.params.enEndMin = 30;
	attrs.params.sigVal = CAM_STROBE_SIG_HIGH << 1;
	attrs.params.outMode = 0;
	attrs.params.acSyncEn = CAM_STROBE_AC_SYNC_EN;

	StrobeHandle hStrobe;
	Int32 err;

	hStrobe = strobe_ctrl_create(&attrs);
	assert(hStrobe);

	/* test params cfg */
	CamStrobeCtrlParam params;
	err = strobe_ctrl_set_cfg(hStrobe, &attrs.params);
	assert(err == E_NO);
	assert(hStrobe->enableCnt == 3);
	err = strobe_ctrl_get_cfg(hStrobe, &params);
	assert(err == E_NO);
	err = memcmp(&attrs.params, &params, sizeof(params));
	assert(err == 0);
	

	/* test check set */
	Int32 minSwitchCnt = 3;
	err = strobe_ctrl_set_check_params(hStrobe, minSwitchCnt, 1);
	assert(err == E_NO);

	DateTime curTime;
	bzero(&curTime, sizeof(curTime));
	curTime.year = 2012;
	curTime.month = 8;
	curTime.day = 23;
	curTime.hour = 18;
	curTime.minute = 0;
	curTime.second = 0;

	err = strobe_ctrl_auto_switch(hStrobe, &curTime, 1023);
	assert(err == E_MODE);
	
	/* test auto switch by time */
	DBG("testing strobe switch by time...");
	params.switchMode = CAM_STROBE_SWITCH_TIME;
	params.ctrlFlags |= CAM_STROBE_FLAG_EN_IN_TURN;
	err = strobe_ctrl_set_cfg(hStrobe, &params);
	assert(err == E_NO);

	err = strobe_ctrl_auto_switch(hStrobe, &curTime, 1023);
	assert(err == E_NO);
	err = strobe_ctrl_get_cfg(hStrobe, &params);
	assert(err == E_NO);
	assert(params.status == 0);

	sleep(2);
	curTime.hour = 19;
	err = strobe_ctrl_auto_switch(hStrobe, &curTime, 1023);
	assert(err == E_NO);
	err = strobe_ctrl_get_cfg(hStrobe, &params);
	assert(err == E_NO);
	DBG("strobe status: 0x%X", params.status);
	assert(params.status == (CAM_STROBE_FLAG_AUTO0|CAM_STROBE_FLAG_AUTO1|CAM_STROBE_FLAG_AUTO2));
	
	/* test enable strobe in turn */
	DBG("testing enable output in turn...");
	int i;
	CaptureInfo capInfo;
	bzero(&capInfo, sizeof(capInfo));
	capInfo.capCnt = 1;
	capInfo.triggerInfo[0].wayNum = 1;
	
	for(i = 0; i < 6; ++i) {
		err = strobe_ctrl_output_enable(hStrobe, &capInfo);
		assert(err == E_NO);
		err = strobe_ctrl_get_cfg(hStrobe, &params);
		assert(err == E_NO);
		// make sure only strobe0 is enabled
		DBG("<%d> strobe status: 0x%x", i, params.status);
		ASSERT(params.status == 1 << (i % hStrobe->enableCnt), "i = %d", i);
	}
	
	/* test auto switch */
	DBG("testing auto switch by lum...");
	params.switchMode = CAM_STROBE_SWITCH_AUTO;
	
	// enable strobe by way num
	params.ctrlFlags |= CAM_STROBE_FLAG_EN_BY_WAY;
	err = strobe_ctrl_set_cfg(hStrobe, &params);
	assert(err == E_NO);

	for(i = 0; i <= minSwitchCnt; ++i) {
		err = strobe_ctrl_auto_switch(hStrobe, &curTime, params.thresholdOn - i - 1);
		if(i < minSwitchCnt)
			assert(err == E_AGAIN);
		else
			ASSERT(err == E_NO, "i = %d", i);
	}

	/* test output enable */
	DBG("testing enable output by way num...");
	err = strobe_ctrl_output_enable(hStrobe, &capInfo);
	assert(err == E_NO);
	
	for(i = 0; i < 6; i++) {
		capInfo.triggerInfo[0].wayNum = i + 1;
		
		err = strobe_ctrl_output_enable(hStrobe, &capInfo);
		assert(err == E_NO);
		err = strobe_ctrl_get_cfg(hStrobe, &params);
		assert(err == E_NO);
		// make sure only strobe0 is enabled
		DBG("<%d> strobe status: 0x%x", i, params.status);
		ASSERT(params.status == 1 << (i % hStrobe->enableCnt), "i = %d", i);
	}
	
	
	err = strobe_ctrl_delete(hStrobe);
	assert(err == E_NO);

	return err;
}

