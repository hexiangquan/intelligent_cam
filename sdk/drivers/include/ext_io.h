/* *
 * Copyright (C) 2012 S.K. Sun
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _EXT_IO_H_
#define _EXT_IO_H_

#include <linux/ioctl.h>
#include <linux/types.h>  

#ifdef __KERNEL__
#include <linux/kernel.h>  
#include <linux/wait.h>
#include <linux/mutex.h>
#include <asm/io.h>
#endif		/* end of #ifdef __KERNEL__ */

#define EXTIO_DEV_NAME		"/dev/extio"

/* IO direction define */
#define EXTIO_DIR_INPUT		0u
#define EXTIO_DIR_OUTPUT	1u

/* IO status define */
#define EXTIO_STAT_HIGH		1u
#define EXTIO_STAT_LOW		0u

/* List of ioctls */
#pragma pack(1)
#define EXTIO_MAGIC_NO		'o'
#define EXTIO_S_STROBE		_IOW(EXTIO_MAGIC_NO, 1, struct hdcam_strobe_info *)
#define EXTIO_G_STROBE		_IOR(EXTIO_MAGIC_NO, 2, struct hdcam_strobe_info *)
#define EXTIO_S_GPIO		_IOW(EXTIO_MAGIC_NO, 3, struct hdcam_io_info *)
#define EXTIO_G_GPIO		_IOR(EXTIO_MAGIC_NO, 4, struct hdcam_io_info *)
#define EXTIO_S_RTC			_IOW(EXTIO_MAGIC_NO, 5, struct hdcam_rtc_time *)
#define EXTIO_G_RTC			_IOR(EXTIO_MAGIC_NO, 6, struct hdcam_rtc_time *)
#define EXTIO_G_TMP			_IOR(EXTIO_MAGIC_NO, 7, int *)
#define EXTIO_S_FIRMWARE	_IOW(EXTIO_MAGIC_NO, 8, struct hdcam_firmware *)
#define EXTIO_S_REG			_IOW(EXTIO_MAGIC_NO, 9, struct hdcam_reg *)
#define EXTIO_G_REG			_IOWR(EXTIO_MAGIC_NO, 10, struct hdcam_reg *)
#define EXTIO_S_RESET		_IO(EXTIO_MAGIC_NO, 11)
#define EXTIO_S_UART		_IOW(EXTIO_MAGIC_NO, 12, struct hdcam_uart_cfg *)
#define EXTIO_G_BACKIO		_IOR(EXTIO_MAGIC_NO, 13, int *)
#define EXTIO_G_ENCRYPT		_IOR(EXTIO_MAGIC_NO, 14, int *)
#pragma  pack()

/* 
 * Strobe info
 */
struct hdcam_strobe_info {
	__u32	offset;			//offset for enable strobe before exposure, unit: us
	__u8	status;			//enable status, bit[0:2]~strobe[0:2]
	__u8	sigVal;			//trigger signal value, 0--low edge, 1--high edge, bit[0:2]~strobe[0:2]
	__u8	mode;			//trigger mode, 0--normal trig, 1--frequency trigger
	__u8	syncAC;			//sync with AC signals
};

/* 
 * GPIO info
 * Note: pin must be set to input before the interrupt to be effective
 */
struct hdcam_io_info {
	__u32	status;			//status of high/low, bit[0:7] ~ IO[0:7]
	__u32	direction;		//direction for IO, bit[0:7] ~ IO[0:7]
	__u32	pos_intr;		//indicates interrupt enable when set, or pos intr status when get, for positive edge, bit[0:7] ~ IO[0:7] 
	__u32	neg_intr;		//indicates interrupt enable when set, or neg intr status when get, for negative edge, bit[0:7] ~ IO[0:7] 
};

/* 
 * Rtc time info
 */
struct hdcam_rtc_time {
	__u16	year;			//year, >= 2012
	__u8	month;			//month, 1~12
	__u8	day;			//month-day, 1~31
	__u8	weekday;		//weekday, 1~7
	__u8	hour;			//hour, 0~23
	__u8	minute;			//minute, 0~59
	__u8	second;			//second, 0~59
	__u16	millisec;		//mili second, 0~999
	__u16	flags;			//flags for future usage
};

/*
 * Reg raw rw
 */
struct hdcam_reg {
	__u16	addr;
	__u16	data;
};

/*
 * Firmware load 
 */
struct hdcam_firmware {
	__u32	magic;		//magic num
	__u8	*data;		//data
	__u32	len;		//len of data in bytes
	__u32	check_sum;  //crc check sum
};

/*
 * Uart TX/RX transition delay
 */
#define HDCAM_UART_MAX_ID	1

enum hdcam_uart_trans_delay {
	HDCAM_UART_BAUD_4800 = 0,
	HDCAM_UART_BAUD_9600,
	HDCAM_UART_BAUD_19200,
	HDCAM_UART_BAUD_38400,
	HDCAM_UART_BAUD_57600,
	HDCAM_UART_BAUD_115200,
	HDCAM_UART_BAUD_MAX
};

struct hdcam_uart_cfg {
	__u16	id;				// channel id
	__u16	baudrate;		// baudrate, see enum above 
	__u32	reserved;
};

#endif /* end of #ifdef _EXT_IO_H_ */

