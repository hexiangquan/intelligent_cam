/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : broadcast.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/8/30
  Last Modified :
  Description   : broadcast.c header file
  Function List :
  History       :
  1.Date        : 2012/8/30
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __BROADCAST_H__
#define __BROADCAST_H__

#include "common.h"

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
#define CAM_BROADCAST_MAGIC		0xBAD2CAFE
/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

typedef struct {
	Uint32		magicNum;
	Uint32		devSN;				// device serical number
	Int8		description[32];	// device description
	Int8		location[32];		// location of device
	Uint32		version;			// version of main program
	Uint32		bootTime;			// sytem boot time, in seconds since 1970.1.1 00:00
	Uint32		runTime;			// run time, in seconds
	Uint16		tcpSrvPort;			// tcp port for cmd ctrl
	Uint16		reserved;
}CamBroadcastInfo;



#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __BROADCAST_H__ */
