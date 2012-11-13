/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : day_night_switch.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/8/16
  Last Modified :
  Description   : check day/night mode switch 
  Function List :
              day_night_cfg_min_switch_cnt
              day_night_cfg_params
              day_night_check_by_lum
              day_night_check_by_time
              day_night_create
              day_night_delete
  History       :
  1.Date        : 2012/8/16
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "day_night_switch.h"
#include "log.h"
#include "cam_time.h"
#include <pthread.h>
#include "target_ctrl.h"

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
#define DAY_NIGHT_MIN_SWITCH 	1000

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/
struct _DayNightSwitchObj {
	CamDayNightModeCfg	cfg;
	Int32				lumKeepCnt;
	Int32				minSwitchCnt;
	const char			*dstMsg;
	Int32				msgCmd;
	pthread_mutex_t		mutex;
};

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
Int32 day_night_cfg_params(DayNightHandle hDayNight, const CamDayNightModeCfg *cfg)
{
	assert(hDayNight && cfg);

	pthread_mutex_lock(&hDayNight->mutex);
	hDayNight->cfg = *cfg;
	pthread_mutex_unlock(&hDayNight->mutex);

	if(cfg->switchMethod == CAM_DN_SWT_TIME)
		DBG("day night cfg params, day start %u:%u, night start %u:%u", cfg->dayModeStartHour, cfg->dayModeStartMin,
			cfg->nightModeStartHour, cfg->nightModeStartMin);
	else if(cfg->switchMethod == CAM_DN_SWT_AUTO)
		DBG("day night cfg params, threshold: %u, switch cnt: %u", 
			cfg->threshold, hDayNight->minSwitchCnt);
	else
		DBG("day night cfg params, none-switch.");

	/* tell target to set current day night mode */
	target_day_night_cfg(-1, hDayNight->cfg.mode);
	
	return E_NO;
}

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
Int32 day_night_cfg_min_switch_cnt(DayNightHandle hDayNight, Int32 minCnt)
{
	assert(hDayNight);

	if(minCnt <= 0)
		return E_INVAL;

	pthread_mutex_lock(&hDayNight->mutex);
	hDayNight->minSwitchCnt = minCnt;
	pthread_mutex_unlock(&hDayNight->mutex);
	
	return E_NO;
}

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
Int32 day_night_cfg_dst_msg(DayNightHandle hDayNight, const char *msgName, Int32 cmd)
{
	assert(hDayNight);
	if(!msgName)
		return E_INVAL;

	pthread_mutex_lock(&hDayNight->mutex);
	hDayNight->dstMsg = msgName;
	hDayNight->msgCmd = cmd;
	pthread_mutex_unlock(&hDayNight->mutex);

	return E_NO;
}

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
DayNightHandle day_night_create(const DayNightAttrs *attrs)
{
	DayNightHandle hDayNight = NULL;

	hDayNight = calloc(1, sizeof(struct _DayNightSwitchObj));
	if(!hDayNight) {
		ERR("alloc mem failed");
		return hDayNight;
	}

	pthread_mutex_init(&hDayNight->mutex, NULL);

	if(attrs) {
		Int32 err;
		err = day_night_cfg_params(hDayNight, &attrs->cfg);
		err |= day_night_cfg_min_switch_cnt(hDayNight, attrs->minSwitchCnt);
		err |= day_night_cfg_dst_msg(hDayNight, attrs->dstMsg, attrs->cmd);
		if(err) {
			day_night_delete(hDayNight);
			hDayNight = NULL;
		}
	} else {
		hDayNight->minSwitchCnt = DAY_NIGHT_MIN_SWITCH;
	}
	
	return hDayNight;
}

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
Int32 day_night_delete(DayNightHandle hDayNight)
{
	assert(hDayNight);

	pthread_mutex_destroy(&hDayNight->mutex);
	free(hDayNight);
	
	return E_NO;
}

/*****************************************************************************
 Prototype    : day_night_notify_switch
 Description  : notify day night switch to other modules
 Input        : DayNightHandle hDayNight  
                MsgHandle hCurMsg         
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/16
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 day_night_notify_switch(DayNightHandle hDayNight, MsgHandle hCurMsg)
{
	/* notify target to switch, using invalid fd because it will be opened in the function */
	int fd = -1;
	target_day_night_cfg(fd, hDayNight->cfg.mode);

	if(!hCurMsg || !hDayNight->dstMsg)
		return E_MODE;

	MsgHeader msg;

	bzero(&msg, sizeof(msg));
	msg.cmd = hDayNight->msgCmd;
	msg.type = MSG_TYPE_REQU;
	msg.param[0] = (Int32)hDayNight->cfg.mode;

	return msg_send(hCurMsg, hDayNight->dstMsg, &msg, 0);
}


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
Int32 day_night_check_by_time(DayNightHandle hDayNight, MsgHandle hCurMsg)
{
	assert(hDayNight);
	CamDayNightModeCfg *cfg = &hDayNight->cfg;
	
	if(cfg->switchMethod != CAM_DN_SWT_TIME)
		return E_MODE; /* only calc timed switch */

	CamDateTime curTime;
	Int32 err = cam_get_time(&curTime);
	if(err)
		return err;
	
	/* Convert to minitues */
	Uint32 curMin = HOUR_TO_MIN(curTime.hour, curTime.minute);
	Uint32 dayStart = HOUR_TO_MIN(cfg->dayModeStartHour, cfg->dayModeStartMin);
	Uint32 nightStart = HOUR_TO_MIN(cfg->nightModeStartHour, cfg->nightModeStartMin);

	/* Check current time */
	CamDayNightMode mode;
	if(curMin >= dayStart && curMin < nightStart)
		mode = CAM_DAY_MODE;
	else
		mode = CAM_NIGHT_MODE;

	if(mode != cfg->mode) {
		/* Update params */
		DBG("day night mode switched by time: %d", mode);
		cfg->mode = (Uint16)mode;
		err = day_night_notify_switch(hDayNight, hCurMsg);
		return err;
	}

	/* Need not switch mode, try again */
	return E_AGAIN;
}

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
Int32 day_night_check_by_lum(DayNightHandle hDayNight, Uint16 lumVal, MsgHandle hCurMsg)
{
	assert(hDayNight);

	CamDayNightModeCfg *cfg = &hDayNight->cfg;

	if(cfg->switchMethod != CAM_DN_SWT_AUTO)
		return E_MODE;

	if(cfg->mode == CAM_DAY_MODE) {
		if(lumVal >= cfg->threshold) 
			hDayNight->lumKeepCnt = 0;
		else
			hDayNight->lumKeepCnt++;
	} else {
		if(lumVal <= cfg->threshold) 
			hDayNight->lumKeepCnt = 0;
		else
			hDayNight->lumKeepCnt++;
	}

	//DBG("day night switch by lum, lum: %u, cnt: %u, threshold: %u", 
		//lumVal, hDayNight->lumKeepCnt, hDayNight->cfg.threshold);

	if(hDayNight->lumKeepCnt > hDayNight->minSwitchCnt) {
		DBG("switch day/night mode by lum val...");
		Uint8 oldMode = cfg->mode;
		if(oldMode == CAM_DAY_MODE)
			cfg->mode = CAM_NIGHT_MODE;
		else
			cfg->mode = CAM_DAY_MODE;

		/* If switch mode failed, we need to keep previous mode */
		if(day_night_notify_switch(hDayNight, hCurMsg) != E_NO)
			cfg->mode = oldMode;
	}

	return E_AGAIN;	
}

/* test day/night switch module */
Int32 day_night_test(void)
{
	DayNightAttrs attrs;
	const char *msgName = "/tmp/dnTest";

	attrs.cfg.mode = CAM_DAY_MODE;
	attrs.cfg.switchMethod = CAM_DN_SWT_TIME;
	attrs.cfg.threshold = 255;
	attrs.cfg.dayModeStartHour = 6;
	attrs.cfg.dayModeStartMin = 0;
	attrs.cfg.nightModeStartHour = 17;
	attrs.cfg.nightModeStartMin = 15;
	attrs.cmd = 1;
	attrs.dstMsg = msgName;
	attrs.minSwitchCnt = 150;
	
	DayNightHandle hDayNight = day_night_create(&attrs);
	assert(hDayNight);

	MsgHandle hMsg = msg_create(msgName, msgName, 0);
	assert(hMsg);

	Int32 err;

	/* test check by time */
	CamDateTime time;

	time.year = 2012;
	time.month = 8;
	time.day = 17;
	time.hour = 10;
	time.minute = 58;
	time.second = 0;
	err = cam_set_time(&time);
	assert(err == E_NO);

	/* should not change */
	err = day_night_check_by_time(hDayNight, hMsg);
	assert(err == E_AGAIN);

	time.hour = 18;
	err = cam_set_time(&time);
	assert(err == E_NO);
	/* should change to night mode */
	err = day_night_check_by_time(hDayNight, hMsg);
	assert(err == E_NO);

	err = day_night_check_by_lum(hDayNight, 100, hMsg);
	assert(err == E_MODE);


	/* change to auto switch mode */
	attrs.cfg.switchMethod = CAM_DN_SWT_AUTO;
	err = day_night_cfg_params(hDayNight, &attrs.cfg);
	assert(err == E_NO);
	err = day_night_cfg_dst_msg(hDayNight, msgName, 1);
	assert(err == E_NO);
	attrs.minSwitchCnt = 10;
	err = day_night_cfg_min_switch_cnt(hDayNight, attrs.minSwitchCnt);
	assert(err == E_NO);

	/* run check by lum value */
	err = day_night_check_by_time(hDayNight, hMsg);
	assert(err == E_MODE);
	err = day_night_check_by_lum(hDayNight, 230, hMsg);
	assert(err == E_AGAIN);

	int i;
	for(i = 0; i <= attrs.minSwitchCnt + 1; ++i) {
		err = day_night_check_by_lum(hDayNight, attrs.cfg.threshold - i, hMsg);
		if(i <= attrs.minSwitchCnt)
			ASSERT(err == E_AGAIN, "i = %d", i);
		else
			ASSERT(err == E_NO, "i = %d", i);
	}

	err = day_night_delete(hDayNight);
	assert(err == E_NO);

	DBG("day night test success....");
	return E_NO;
	
}

