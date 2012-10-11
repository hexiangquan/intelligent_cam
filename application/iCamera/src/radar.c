/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : radar.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/10/10
  Last Modified :
  Description   : radar speed detection module
  Function List :
              csr_radar_data_parse
              csr_radar_init
              csr_radar_speed_detect
              radar_create
              radar_delete
              radar_detect_speed
              radar_set_params
  History       :
  1.Date        : 2012/10/10
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "radar.h"
#include "cam_detector.h"
#include "log.h"
#include "uart.h"

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

/* Sub functions for detail implemention of specific radar */
typedef Int32 (*FxnRadarInit)(RadarHandle hRadar);
typedef Int32 (*FxnRadarDetect)(RadarHandle hRadar, Uint32 *speed);

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/* Object for speed radar */
struct SpeedRadarObj {
	RadarParams 	params;
	int 			fd;
	FxnRadarInit 	initFxn;
	FxnRadarDetect 	detectFxn;
};

/* Macros for CSR radar */
#define CSR_RESP_TIME		200  	// time of radar response 
#define CSR_MIN_VAL			0x01
#define CSR_MAX_VAL			0xF0

/*****************************************************************************
 Prototype    : csr_radar_init
 Description  : csr radar initialize
 Input        : RadarHandle hRadar  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/10/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 csr_radar_init(RadarHandle hRadar)
{
	hRadar->fd = uart_open(UART_RS232, UART_B9600, UART_D8, UART_S1, UART_POFF);
	if(hRadar->fd < 0) {
		ERRSTR("open dev %s failed", UART_RS232);
		return E_IO;
	}

	uart_set_timeout(hRadar->fd, 1, CSR_RESP_TIME/100);
	uart_set_trans_delay(0, UART_B9600);

	return E_NO;
}

/*****************************************************************************
 Prototype    : csr_radar_data_parse
 Description  : csr radar data parse
 Input        : const Uint8 *data  
                Int32 len          
                Uint32 *speed      
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/10/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 csr_radar_data_parse(const Uint8 *data, Int32 len, Uint32 *speed)
{
	Int32 i;
	Int32 ret = E_CHECKSUM;

	for(i = 0; i < len; ++i) {
		if(data[i] >= CSR_MIN_VAL && data[i] <= CSR_MAX_VAL) {
			*speed = data[i];
			ret = E_NO;
			break;
		}
	}

	return ret;
}

/*****************************************************************************
 Prototype    : csr_radar_speed_detect
 Description  : csr radar speed detection
 Input        : RadarHandle hRadar  
                Uint32 *speed       
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/10/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 csr_radar_speed_detect(RadarHandle hRadar, Uint32 *speed)
{
	Int32 	timeLeft = hRadar->params.timeout;
	Uint8	buf[8];
	Int32	ret;

	/* clear data recieved before, invalid */
	UART_FLUSH(hRadar->fd);
	
	while(1) {
		ret = read(hRadar->fd, buf, sizeof(buf));
		if(ret > 0) {
			ret = csr_radar_data_parse(buf, ret, speed);
			if(!ret) /* parse data correct, return  */
				break;
		}

		timeLeft -= CSR_RESP_TIME;
		if(timeLeft < 0) {
			ret = E_TIMEOUT;
			break;
		}
	}

	return ret;
}

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
RadarHandle radar_create(const RadarParams *params)
{
	RadarHandle hRadar;

	hRadar = calloc(1, sizeof(struct SpeedRadarObj));
	if(!hRadar) {
		ERR("alloc memory for radar obj failed!");
		return NULL;
	}

	if(radar_set_params(hRadar, params) != E_NO) {
		radar_delete(hRadar);
		return NULL;
	}
	
	return hRadar;
}

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
Int32 radar_delete(RadarHandle hRadar)
{
	if(!hRadar)
		return E_INVAL;

	if(hRadar->fd > 0)
		close(hRadar->fd);

	free(hRadar);
	return E_NO;
}

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
Int32 radar_set_params(RadarHandle hRadar, const RadarParams *params)
{
	/* close fd if opened before */
	if(hRadar->fd > 0) {
		close(hRadar->fd);
		hRadar->fd = -1;
	}

	Int32 err = E_NO;
	
	/* select radar type */
	switch(params->type) {
		case RADAR_CSR:
			hRadar->initFxn = csr_radar_init;
			hRadar->detectFxn = csr_radar_speed_detect;
			break;
		default:
			ERR("unsupported radar type: %u", params->type);
			err = E_UNSUPT;
			break;
	}

	if(!err) {
		/* Run init fxn for init special radar */
		hRadar->params = *params;
		err = hRadar->initFxn(hRadar);
	}
	
	return err;
}

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
Int32 radar_detect_speed(RadarHandle hRadar, Uint32 *speed, Uint32 modifyRatio)
{
	if(!hRadar || !speed)
		return E_INVAL;

	if(!hRadar->detectFxn)
		return E_MODE;

	Int32 err = hRadar->detectFxn(hRadar, speed);
	if(!err && modifyRatio) {
		/* modify speed for error correct */
		*speed = (*speed) * modifyRatio / 100;
	}

	return err;
}

/* test function for this module */
Int32 radar_test()
{
	RadarHandle hRadar;
	RadarParams params;

	params.timeout = 0;
	params.type = RADAR_CSR;

	hRadar = radar_create(&params);
	assert(hRadar);

	Int32 err;

	params.timeout = 1000;
	err = radar_set_params(hRadar, &params);
	assert(err == E_NO);

	while(1) {
		Uint32 speed = 0;
		err = radar_detect_speed(hRadar, &speed, 0);
		if(!err) {
			DBG("radar detect speed: %u", speed);
			break;
		} else {
			//DBG("radar detect speed err: %d", err);
		}
	}
	err = radar_delete(hRadar);

	DBG("radar test done...");

	return err;
}

