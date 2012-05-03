/******************************************************************************

  Copyright (C), 2001-2011, Vixtend Co., Ltd.

 ******************************************************************************
  File Name     : crc16.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2011/5/27
  Last Modified :
  Description   : CRC16 checksum alg
  Function List :
              crc16
  History       :
  1.Date        : 2011/5/27
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "crc16.h"
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


/* Table used for CRC16 algorithm */
static const unsigned short c_usCrcTableA[] =
{
    0x0000,
    0x1081,
    0x2102,
    0x3183,
    0x4204,
    0x5285,
    0x6306,
    0x7387,
    0x8408,
    0x9489,
    0xA50A,
    0xB58B,
    0xC60C,
    0xD68D,
    0xE70E,
    0xF78F
};

/* See comments above */
static const unsigned short c_usCrcTableB[] =
{
    0x0000,
    0x1189,
    0x2312,
    0x329B,
    0x4624,
    0x57AD,
    0x6536,
    0x74BF,
    0x8C48,
    0x9DC1,
    0xAF5A,
    0xBED3,
    0xCA6C,
    0xDBE5,
    0xE97E,
    0xF8F7
};

/*****************************************************************************
 Prototype    : crc16
 Description  : CRC16 checksun algorithm
 Input        : const Uint8 *restrict pData  
                Uint32 unSize                
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2011/5/27
    Author       : Sun
    Modification : Created function

*****************************************************************************/
unsigned short crc16(const void *data, int size)
{
	unsigned short	usCrc = 0;
	unsigned char	ucByte;
	const unsigned char *pData = data;
	
	
    for (; (size > 0); size--)
    {
        /* byte loop */
        ucByte = *pData++;  /* fetch the next data byte */

        ucByte ^= usCrc;    /* EOR data with current CRC value */
        usCrc = ((c_usCrcTableA[(ucByte & 0xF0) >> 4] ^ c_usCrcTableB[ucByte & 0x0F]) ^
                (usCrc >> 8));
    }

    return (usCrc);
}          


