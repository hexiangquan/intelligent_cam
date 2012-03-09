/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : app_params_cfg.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/5
  Last Modified :
  Description   : app_params_cfg.c header file
  Function List :
  History       :
  1.Date        : 2012/3/5
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __APP_PARAMS_MNG_H__
#define __APP_PARAMS_MNG_H__

#include "cam_params.h"
#include <pthread.h>

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

#define APP_PARAMS_MAGIC		0xC00DBEEF

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/* Params to manage */
typedef struct _AppParams {
	Uint32					crc;				//crc result of following data
	Uint32					magicNum;			//magic used for  validate
	Uint32					dataLen;			//len of valid data
	
	CamNetworkInfo			networkInfo;		//our network info
	CamDeviceInfo			devInfo;			//unique device info, Read Only
	CamOsdParams			osdParams;			//osd params
	CamRoadInfo				roadInfo;			//road info
	CamRtpParams			rtpParams;			//rtp params for h.264 sending
	CamImageUploadProtocol	imgTransType;		//image upload type
	CamTcpImageServerInfo	tcpImgSrvInfo;		//Tcp Image server info
	CamFtpImageServerInfo	ftpSrvInfo;			//Ftp server info
	CamNtpServerInfo		ntpSrvInfo;			//Ntp server info
	CamExprosureParam		exposureParams;		//exposure params, only effective when AE is disabled
	CamRGBGains				rgbGains;			//RGB gains
	CamDRCParam				drcParams;			//Dynamic range compression params 	
	CamTrafficLightRegions	correctRegs;		//traffic light region to correct
	CamImgEnhanceParams		imgAdjParams;		//image adjust params
	CamH264Params			h264EncParams;		//h.264 encode params
	CamWorkMode				workMode;			//work mode params
	CamImgEncParams			imgEncParams;		//image encode params, set at run time
	CamIoCfg				ioCfg;				//IO status
	CamStrobeCtrlParam		strobeParams;		//strobe control params
	CamDetectorParam		detectorParams;		//vehicle detector params
	CamAEParam				aeParams;			//auto exposure params
	CamAWBParam				awbParams;			//auto white balance params
	CamDayNightModeCfg		dayNightCfg;		//day night mode config
}AppParams;

typedef struct ParamsMngObj *ParamsMngHandle;

/* Command ID */
typedef enum {
	PMCMD_S_NETWORKINFO = 0xABCD0000,
	PMCMD_G_NETWORKINFO,
	PMCMD_S_DEVINFO,
	PMCMD_G_DEVINFO,
	PMCMD_S_OSDPARAMS,
	PMCMD_G_OSDPARAMS,
	PMCMD_S_ROADINFO,
	PMCMD_G_ROADINFO,
	PMCMD_S_RTPPARAMS,
	PMCMD_G_RTPPARAMS,
	PMCMD_S_IMGTRANSPROTO,
	PMCMD_G_IMGTRANSPROTO,
	PMCMD_S_TCPSRVINFO,
	PMCMD_G_TCPSRVINFO,
	PMCMD_S_FTPSRVINFO,
	PMCMD_G_FTPSRVINFO,
	PMCMD_S_NTPSRVINFO,
	PMCMD_G_NTPSRVINFO,
	PMCMD_S_EXPPARAMS,
	PMCMD_G_EXPPARAMS,
	PMCMD_S_RGBGAINS,
	PMCMD_G_RGBGAINS,
	PMCMD_S_DRCPARAMS,
	PMCMD_G_DRCPARAMS,
	PMCMD_S_LIGHTCORRECT,
	PMCMD_G_LIGHTCORRECT,
	PMCMD_S_IMGADJPARAMS,
	PMCMD_G_IMGADJPARAMS,
	PMCMD_S_H264ENCPARAMS,
	PMCMD_G_H264ENCPARAMS,
	PMCMD_S_WORKMODE,
	PMCMD_G_WORKMODE,
	PMCMD_S_IMGENCPARAMS,
	PMCMD_G_IMGENCPARAMS,
	PMCMD_S_IOCFG,
	PMCMD_G_IOCFG,
	PMCMD_S_STROBEPARAMS,
	PMCMD_G_STROBEPARAMS,
	PMCMD_S_DETECTORPARAMS,
	PMCMD_G_DETECTORPARAMS,
	PMCMD_S_AEPARAMS,
	PMCMD_G_AEPARAMS,
	PMCMD_S_AWBPARAMS,
	PMCMD_G_AWBPARAMS,
	PMCMD_S_DAYNIGHTCFG,
	PMCMD_G_DAYNIGHTCFG,

	/* thread module cmds */
	PMCMD_G_IMGCONVDYN = 0xBCDA0000,
	PMCMD_G_2NDSTREAMATTRS,
	PMCMD_G_JPGENCDYN,
	PMCMD_S_CAPINFO,	/* This cmd should be set before any other thread cmds */
	PMCMD_G_IMGOSDDYN,
	
}ParamsMngCtrlCmd;


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*****************************************************************************
 Prototype    : params_mng_create
 Description  : Read and validate params from file
 Input        : const char *cfgFile  
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/6
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern ParamsMngHandle params_mng_create(const char *cfgFile);

/*****************************************************************************
 Prototype    : params_mng_delete
 Description  : Delete module
 Input        : ParamsMngHandle hParamsMng  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/6
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 params_mng_delete(ParamsMngHandle hParamsMng);

/*****************************************************************************
 Prototype    : params_mng_control
 Description  : do params control
 Input        : ParamsMngHandle hParamsMng  
                ParamsMngCtrlCmd cmd        
                void *arg                   
                Int32 size, size of arg buf                
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/6
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 params_mng_control(ParamsMngHandle hParamsMng, ParamsMngCtrlCmd cmd, void *arg, Int32 size);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __APP_PARAMS_CFG_H__ */
