/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : tcp_cmd_trans.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/28
  Last Modified :
  Description   : tcp_cmd_trans.c header file
  Function List :
  History       :
  1.Date        : 2012/3/28
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __NET_CMDS_H__
#define __NET_CMDS_H__

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
#define UPDATE_DATA_OFFSET		(128)
#define UPDATE_NAME_OFFSET		(0)
/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/* 
 * TCP commands define 
 */
enum TcpCommand
{
    TC_CMD_UNDEFINED = 0,
    TC_FUN_CONNECTREQUEST = 0x1000,
    TC_FUN_CAMERARESET = 0x1001,
    TC_FUN_HEARTBEATSINGAL = 0x1002,
    TC_FUN_CAPTURECTRL = 0x1010,
    TC_FUN_RESTORETODEFAULT = 0x1020,
    TC_FUN_UPDATE_DSP = 0x1030,
	TC_FUN_UPDATE_FPGA = 0x1031,
    TC_FUN_UPDATE_ARM = 0x1032,
    
    TC_GET_VERINFO = 0x2000,
    TC_GET_TEMPERATURE = 0x2010,
    TC_GET_TIME = 0x2020,
    TC_SET_TIME = 0x2021,
    TC_GET_NETWORKINFO = 0x2030,
    TC_SET_NETWORKINFO = 0x2031,
    TC_GET_OSDINFO = 0x2040,
    TC_SET_OSDINFO = 0x2041,
    TC_GET_IMAGEUPLOADPROTOCOL = 0x2050,
    TC_SET_IMAGEUPLOADPROTOCOL = 0x2051,
    TC_GET_NTPSERVERINFO = 0x2060,
    TC_SET_NTPSERVERINFO = 0x2061,
    TC_GET_TCPIMAGESERVERINFO = 0x2070,
    TC_SET_TCPIMAGESERVERINFO = 0x2071,
    TC_GET_FTPSERVERINFO = 0x2080,
    TC_SET_FTPSERVERINFO = 0x2081,
    TC_GET_DEVICEINFO = 0x2090,
    TC_SET_DEVICEINFO = 0x2091,
    TC_GET_ROADINFO = 0x20A0,
    TC_SET_ROADINFO = 0x20A1,
	TC_GET_RTPPARAMS = 0x20B0,
	TC_SET_RTPPARAMS = 0x20B1,
    
    TC_GET_EXPROSUREPARAM = 0x2100,
    TC_SET_EXPROSUREPARAM = 0x2101,
    TC_GET_RGBGAIN = 0x2110,
    TC_SET_RGBGAIN = 0x2111,
    
    TC_GET_IMGENCPARAMS = 0x2130,
    TC_SET_IMGENCPARAMS = 0x2131,
    
    TC_GET_LUT_DAY = 0x2140,
    TC_SET_LUT_DAY = 0x2141,
    TC_GET_LUT_NIGHT = 0x2150,
    TC_SET_LUT_NIGHT = 0x2151,

	TC_GET_H264_ENC_PARAMS = 0x2160,
	TC_SET_H264_ENC_PARAMS = 0x2161,
	TC_GET_AV_TYPE = 0x2170,
	TC_SET_AV_TYPE = 0x2171,
	
	TC_GET_TRAFFIC_LIGHT_REGS = 0x2190,
	TC_SET_TRAFFIC_LIGHT_REGS = 0x2191,
	TC_GET_IMG_ENHANCE_PARAMS = 0x21A0,
	TC_SET_IMG_ENHANCE_PARAMS = 0x21A1,
	TC_GET_SPEC_CAP_PARAMS = 0x21B0,
	TC_SET_SPEC_CAP_PARAMS = 0x21B1,
    
    TC_GET_WORKSTATUS = 0x2200,
    TC_GET_WORKMODE = 0x2210,
    TC_SET_WORKMODE = 0x2211,
    TC_GET_INPUT_INFO = 0x2230,
    
    TC_GET_IOINFO = 0x2300,
    TC_SET_IOINFO = 0x2301,
    TC_GET_STROBEPARAM = 0x2310,
    TC_SET_STROBEPARAM = 0x2311,
    TC_GET_DETECTORPARAM = 0x2320,
    TC_SET_DETECTORPARAM = 0x2321,
    
    TC_GET_AUTOEXPROSUREPARAM = 0x2400,
    TC_SET_AUTOEXPROSUREPARAM = 0x2401,
    TC_GET_AUTOWHITEBALANCEPARAM = 0x2410,
    TC_SET_AUTOWHITEBALANCEPARAM = 0x2411,
	TC_SET_DAYNIGHTMODEPARAMS = 0x2420,
	TC_GET_DAYNIGHTMODEPARAMS = 0x2421,
	
	TC_GET_SD_ROOT_PATH = 0x2600,
	TC_GET_SD_DIR_INFO = 0x2610,
	TC_SET_RD_SD_DIR = 0x2621,
	TC_SET_RD_SD_FILE = 0x2622,
	TC_SET_DEL_SD_DIR = 0x2631,
	TC_SET_FORMAT_SD = 0x2641,

	TC_GET_DSP_PARAMS = 0x2700,
	TC_SET_DSP_PARAMS = 0x2701,
	
};

/*
 * Request mode for FUN_CONNECTREQUEST
 */
enum ConnectRequestMode
{
    CONNECT_MODE_NORMAL = 0x0000,
	CONNECT_MODE_FORCE = 0x0001,
	CONNECT_MODE_RESET = 0xFFFF
};

/*
 * udp cmds
 */
enum UdpCmds
{
	UC_INVALID_REQUEST = 0,
	UC_GET_CAMERAINFO,
	UC_RESET_CAMERA,	
};

/* structure for UC_GET_CAMERAINFO */
typedef struct {
	Int8	ipString[18];	//IP addr of this camera
	Uint16	listenPort;			//cmd listen port 
}CamListenInfo;

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


#endif /* __TCP_CMD_TRANS_H__ */
