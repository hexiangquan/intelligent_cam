/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : detector_uart_generic.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/4/6
  Last Modified :
  Description   : detector_uart_generic.c header file
  Function List :
  History       :
  1.Date        : 2012/4/6
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __DETECTOR_UART_GENERIC_H__
#define __DETECTOR_UART_GENERIC_H__

#include "detector.h"
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
#define CAP_TABLE_SIZE			4

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/
typedef struct DetectorUartObj *DetectorUartHandle; 

typedef struct {
	/* init fxns, detect id*/
	Int32 (*init)(IN DetectorUartHandle hUart, IN Uint16 detectorId);
	/* pre parse capture params */
	void (*capPreParse)(IN DetectorUartHandle hUart, IN const CamDetectorParam *params);
	/* compare start code, can be NULL */
	Bool (*startCodeSync)(IN DetectorUartHandle hUart, IN Uint8 data);
	/* parse signle packet */
	Int32 (*singleTrigParse)(IN DetectorUartHandle hUart, IN const CamDetectorParam *params, IN Uint8 *data, OUT TriggerInfo *info);
	/* exit fxns, can be NULL */
	void (*exit)(IN DetectorUartHandle hUart);
}DetectorUartFxns;

enum TrigIndex{
	TRIG_EP = 0,
	TRIG_CP,
	TRIG_RE,
	TRIG_MAX,
};

/* obj for uart detector */
typedef struct DetectorUartObj {
	Int32					fd;									//Uart fd
	const char 				*devName;							//uart attributes
	UartBaudrate			baudRate;							
	UartDataBits			dataBits;
	UartParity				parity;
	UartStopBits			stopBits;
	Int32					packetLen;							//len of an uart packet, include start code
	const DetectorUartFxns 	*opts;								//operations for specific device
	Uint16					groupId[APP_MAX_CAP_CNT];			//GroupNum for each way
	Uint16					redlightTime[APP_MAX_CAP_CNT];		//Redlight time record for each way
	Uint8					epCapTable[CAP_TABLE_SIZE];			//Epolice trigger code to frame id table
	Uint8					cpCapTable[CAP_TABLE_SIZE];			//Checkpost trigger code frame id table
	Uint8					reCapTable[CAP_TABLE_SIZE];			//Retrograde trigger code to frame id table
	Uint8					lastFrameCode[TRIG_MAX];			// Trigger code for the last frame 
	Uint8					timeout;							//recv timeout: 100ms
	void					*private;							//private data
}DetectorUart;

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/* fxns for uart detector */
extern const DetectorFxns DETECTOR_UART_FXNS;


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __DETECTOR_UART_GENERIC_H__ */
