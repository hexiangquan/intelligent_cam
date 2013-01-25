/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : cam_info.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2011/10/11
  Last Modified :
  Description   : cam info structure define
  Function List :
  History       :
  1.Date        : 2011/10/11
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __CAM_INFO_H__
#define __CAM_INFO_H__

#include "common.h"

/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/

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
 * routines' implementations                    *
 *----------------------------------------------*/

/*
  * Firmware version info 
  */
typedef struct {
	Uint32	dspVer;
	Uint32	fpgaVer;
	Uint32	armVer;
} CamVersionInfo;


/* 
  * Network info 
  */
typedef struct {
	Int8	ipAddr[16];  	// Set to "0.0.0.0" for DHCP    
	Int8	ipMask[16];  	// Not used when using DHCP
	Int8	gatewayIP[16];  // Not used when using DHCP
	Int8	domainName[20];	// Not used when using DHCP
	Int8	hostName[20];	// Host name
	Int8	dnsServer[16];	// Used when set to anything but zero
} CamNetworkInfo;

/* 
  * Device info 
  */
typedef struct {
	Uint8	macAddr[8];		//Mac Addr
	Uint32	deviceSN;		//Device Serial Number
	Int8	name[32];		//Name of this device
	Int8	location[32];	//Location of this device
	Int8	manufacture[32];//Name of manufacture
	Int8	model[32];		//Device Model 
	Int8	hardware[16];	//Hardware Version
	Int8	firmware[16];	//Firmware Version
} CamDeviceInfo;

/* 
  * Road info for this device
  */
#define MAX_ROAD_NAME_SIZE 		64

typedef struct {
	Int8	roadName[MAX_ROAD_NAME_SIZE];	//road info
	Uint16	roadNum;						//Road Number
	Uint16	directionNum;					//Direction Number
	Uint16	devSN;							//Divice Serial Num
	Uint16	reserved;
} CamRoadInfo;

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


#endif /* __CAMCTL_H__ */
