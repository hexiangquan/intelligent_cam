/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : uart.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/29
  Last Modified :
  Description   : uart.c header file
  Function List :
  History       :
  1.Date        : 2012/3/29
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __UART_H__
#define __UART_H__

#include "common.h"
#include <termios.h>

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
/* name of uart device */
#define UART_RS232		"/dev/ttyS0"
#define UART_RS485		"/dev/ttyS1"

#define UART_FLUSH(fd)	(tcflush(fd, TCIOFLUSH))

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/* baudrate for uart */
typedef enum {
	UART_B300 = 0,
	UART_B1200,
	UART_B2400,
	UART_B4800,
	UART_B9600,
	UART_B19200,
	UART_B38400,
	UART_B57600,
	UART_B115200,
	UART_B230400,
	UART_B460800,
	UART_B_MAX,
}UartBaudrate;

/* data bits per character */
typedef enum {
	UART_D5 = 0,
	UART_D6,
	UART_D7,
	UART_D8,
	UART_D_MAX,
}UartDataBits;

/* stop bits per character */
typedef enum {
	UART_S1 = 0,	// 1 stop bit
	UART_S2,		// 2 stop bits
	UART_S_MAX
}UartStopBits;

typedef enum {
	UART_POFF = 0,	//no parity
	UART_PODD,		//odd parity
	UART_PEVEN,		//even parity
	UART_P_MAX
}UartParity;

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


/*****************************************************************************
 Prototype    : uart_open
 Description  : open uart dev and set attributes
 Input        : const char *name       
                UartBaudrate baudrate  
                UartDataBits dataBits  
                UartStopBits stopBits  
                UartParity parity      
 Output       : None
 Return Value : return fd if sucess, error code if failed
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 uart_open(const char *name, UartBaudrate baudrate, UartDataBits dataBits, UartStopBits stopBits, UartParity parity);

/*****************************************************************************
 Prototype    : uart_set_attrs
 Description  : set uart attributes
 Input        : int fd                 
                UartBaudrate baudrate  
                UartDataBits dataBits  
                UartStopBits stopBits  
                UartParity parity      
 Output       : None
 Return Value : error code
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 uart_set_attrs(int fd, UartBaudrate baudrate, UartDataBits dataBits, UartStopBits stopBits, UartParity parity);

/*****************************************************************************
 Prototype    : uart_set_timeout
 Description  : set recieve timeout
 Input        : Uint8 minData, minimal counts of data recieved before timeout     
                Uint16 timeout, timeout for reieve, unit: tenths of second  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 uart_set_timeout(Int32 fd, Uint8 minCnt, Uint8 timeout);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __UART_H__ */
