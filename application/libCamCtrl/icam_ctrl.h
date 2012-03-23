/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : icam_ctrl.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/14
  Last Modified :
  Description   : icam_ctrl.c header file
  Function List :
  History       :
  1.Date        : 2012/3/14
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __ICAM_CTRL_H__
#define __ICAM_CTRL_H__
	
#include "cam_params.h"

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
#define ICAM_MSG_NAME		"/tmp/iCamCtrl"
#define ICAM_CLIENT_NAME	"/tmp/iCamClient"
#define ICAM_FLAG_NONBLOCK	(1 << 0)
#define ICAM_CMD_BASE		0x8000

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

typedef struct ICamCtrlObj *ICamCtrlHandle;

/* cmds for iCam ctrl */
typedef enum {
	ICAMCMD_G_VERSION = ICAM_CMD_BASE,
	ICAMCMD_G_WORKSTATUS,
	ICAMCMD_S_DATETIME,
	ICAMCMD_G_DATETIME,
	ICAMCMD_S_NETWORKINFO,
	ICAMCMD_G_NETWORKINFO,
	ICAMCMD_S_DEVINFO,
	ICAMCMD_G_DEVINFO,
	ICAMCMD_S_OSDPARAMS,
	ICAMCMD_G_OSDPARAMS,
	ICAMCMD_S_RODAINFO,
	ICAMCMD_G_RODAINFO,
	ICAMCMD_S_RTPPARAMS,
	ICAMCMD_G_RTPPARAMS,
	ICAMCMD_S_UPLOADPROTO,
	ICAMCMD_G_UPLOADPROTO,
	ICAMCMD_S_IMGSRVINFO,
	ICAMCMD_G_IMGSRVINFO,
	ICAMCMD_S_FTPSRVINFO,
	ICAMCMD_G_FTPSRVINFO,
	ICAMCMD_S_NTPSRVINFO,
	ICAMCMD_G_NTPSRVINFO,
	ICAMCMD_S_EXPPARAMS,
	ICAMCMD_G_EXPPARAMS,
	ICAMCMD_S_RGBGAINS,
	ICAMCMD_G_RGBGAINS,
	ICAMCMD_S_DRCPARAMS,
	ICAMCMD_G_DRCPARAMS,
	ICAMCMD_S_TRAFLIGHTREG,
	ICAMCMD_G_TRAFLIGHTREG,
	ICAMCMD_S_IMGENHANCE,
	ICAMCMD_G_IMGENHANCE,
	ICAMCMD_S_H264PARAMS,
	ICAMCMD_G_H264PARAMS,
	ICAMCMD_S_AVPARAMS,
	ICAMCMD_G_AVPARAMS,
	ICAMCMD_S_WORKMODE,
	ICAMCMD_G_WORKMODE,
	ICAMCMD_S_IMGENCPARAMS,
	ICAMCMD_G_IMGENCPARAMS,
	ICAMCMD_S_SPECCAPPARAMS,
	ICAMCMD_G_SPECCAPPARAMS,
	ICAMCMD_S_IOCFG,
	ICAMCMD_G_IOCFG,
	ICAMCMD_S_STROBEPARAMS,
	ICAMCMD_G_STROBEPARAMS,
	ICAMCMD_S_DETECTORPARAMS,
	ICAMCMD_G_DETECTORPARAMS,
	ICAMCMD_S_AEPARAMS,
	ICAMCMD_G_AEPARAMS,
	ICAMCMD_S_AWBPARAMS,
	ICAMCMD_G_AWBPARAMS,
	ICAMCMD_S_DAYNIGHTMODE,
	ICAMCMD_G_DAYNIGHTMODE,
	ICAMCMD_S_CAPEN,
	ICAMCMD_G_SDROOTPATH,
	ICAMCMD_G_SDDIRINFO,
	ICAMCMD_S_SNDDIR,
}ICamCtrlCmds;

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*
 *	all functions except icam_ctrl_create, use error code defined at "common.h" as return code
 */

/* create icam control handle */
extern ICamCtrlHandle icam_ctrl_create(IN const char *pathName, IN Int32 flags, IN Int32 transTimeout);

/* delete icam control handle */
extern Int32 icam_ctrl_delete(IN ICamCtrlHandle hCtrl);

/* control capture, i.e. start, stop, restart */
extern Int32 icam_capture_ctrl(IN ICamCtrlHandle hCtrl, IN CamCapCtrl ctrl);

/* get auto exposure params */
extern Int32 icam_get_ae_params(IN ICamCtrlHandle hCtrl, OUT CamAEParam *params);

/* get analog video params */
extern Int32 icam_get_av_params(IN ICamCtrlHandle hCtrl, OUT CamAVParam *params);

/* get auto white balance params */
extern Int32 icam_get_awb_params(IN ICamCtrlHandle hCtrl, OUT CamAWBParam *params);

/* get day night switch mode params */
extern Int32 icam_get_day_night_mode_params(ICamCtrlHandle hCtrl, CamDayNightModeCfg *params);

/* get detector params  */
extern Int32 icam_get_detector_params(IN ICamCtrlHandle hCtrl, OUT CamDetectorParam *params);

/* get device info */
extern Int32 icam_get_dev_info(IN ICamCtrlHandle hCtrl, OUT CamDeviceInfo *buf);

/* get dynamic range compression params */
extern Int32 icam_get_drc_params(IN ICamCtrlHandle hCtrl, OUT CamDRCParam *params);

/* get exposure params */
extern Int32 icam_get_exposure_params(IN ICamCtrlHandle hCtrl, OUT CamExprosureParam *params);

/* get ftp server info */
extern Int32 icam_get_ftp_srv_info(IN ICamCtrlHandle hCtrl, OUT CamFtpImageServerInfo *srvInfo);

/* get h.264 params */
extern Int32 icam_get_h264_params(IN ICamCtrlHandle hCtrl, OUT CamH264Params *params);

/* get image encode params */
extern Int32 icam_get_img_enc_params(IN ICamCtrlHandle hCtrl, OUT CamImgEncParams *params);

/* get image enhance params */
extern Int32 icam_get_img_enhance_params(IN ICamCtrlHandle hCtrl, OUT CamImgEnhanceParams *params);

/* get image server info */
extern Int32 icam_get_img_srv_info(IN ICamCtrlHandle hCtrl, OUT CamTcpImageServerInfo *srvInfo);

/* get io config params */
extern Int32 icam_get_io_config(IN ICamCtrlHandle hCtrl, OUT CamIoCfg *params);

/* get network info */
extern Int32 icam_get_network_info(IN ICamCtrlHandle hCtrl, OUT CamNetworkInfo *info);

/* get ntp server info */
extern Int32 icam_get_ntp_srv_info(IN ICamCtrlHandle hCtrl, OUT CamNtpServerInfo *srvInfo);

/* get osd params */
extern Int32 icam_get_osd_params(IN ICamCtrlHandle hCtrl, OUT CamOsdParams *params);

/* get rgb gains */
extern Int32 icam_get_rgb_gains(IN ICamCtrlHandle hCtrl, OUT CamRGBGains *params);

/* get road info */
extern Int32 icam_get_road_info(IN ICamCtrlHandle hCtrl, OUT CamRoadInfo *buf);

/* get rtp params */
extern Int32 icam_get_rtp_params(IN ICamCtrlHandle hCtrl, OUT CamRtpParams *buf);

/* get sd directory info */
extern Int32 icam_get_sd_dir_info(IN ICamCtrlHandle hCtrl, IN const char *dirPath, OUT void *buf, IN Int32 bufLen);

/* get sd root path */
extern Int32 icam_get_sd_root_path(IN ICamCtrlHandle hCtrl, OUT void *buf, IN Int32 bufLen);

/* get special capture params */
extern Int32 icam_get_spec_cap_params(IN ICamCtrlHandle hCtrl, OUT CamSpecCapParams *params);

/* get strobe control params */
extern Int32 icam_get_strobe_params(IN ICamCtrlHandle hCtrl, OUT CamStrobeCtrlParam *params);

/* get date time of cam */
extern Int32 icam_get_time(IN ICamCtrlHandle hCtrl, OUT CamDateTime *buf);

/* get traffic light region params */
extern Int32 icam_get_traffic_light_regions(IN ICamCtrlHandle hCtrl, OUT CamTrafficLightRegions *params);

/* get image upload protocol */
extern Int32 icam_get_upload_proto(IN ICamCtrlHandle hCtrl, OUT CamImgUploadProto *buf);

/* get firmware version */
extern Int32 icam_get_version(IN ICamCtrlHandle hCtrl, OUT CamVersionInfo *buf);

/* get work mode */
extern Int32 icam_get_work_mode(IN ICamCtrlHandle hCtrl, OUT CamWorkMode *params);

/* get work status */
extern Int32 icam_get_work_status(ICamCtrlHandle hCtrl, CamWorkStatus *buf);

/* set auto exposure params */
extern Int32 icam_set_ae_params(IN ICamCtrlHandle hCtrl, IN const CamAEParam *params);

/* set auto analog video params */
extern Int32 icam_set_av_params(IN ICamCtrlHandle hCtrl, IN const CamAVParam *params);

/* set auto auto white balance params */
extern Int32 icam_set_awb_params(IN ICamCtrlHandle hCtrl, IN const CamAWBParam *params);

/* set day night switch mode */
extern Int32 icam_set_day_night_mode_params(IN ICamCtrlHandle hCtrl, IN const CamDayNightModeCfg *params);

/* set vehicle detector params */
extern Int32 icam_set_detector_params(IN ICamCtrlHandle hCtrl, IN const CamDetectorParam *params);

/* set device info */
extern Int32 icam_set_dev_info(IN ICamCtrlHandle hCtrl, IN const CamDeviceInfo *info);

/* set dynamic range compression params */
extern Int32 icam_set_drc_params(IN ICamCtrlHandle hCtrl, IN const CamDRCParam *params);

/* set exposure params, only useful when auto exposure is off */
extern Int32 icam_set_exposure_params(IN ICamCtrlHandle hCtrl, IN const CamExprosureParam *params);

/* set ftp server info */
extern Int32 icam_set_ftp_srv_info(IN ICamCtrlHandle hCtrl, IN const CamFtpImageServerInfo *srvInfo);

/* set h.264 params */
extern Int32 icam_set_h264_params(IN ICamCtrlHandle hCtrl, IN const CamH264Params *params);

/* set img encode params */
extern Int32 icam_set_img_enc_params(IN ICamCtrlHandle hCtrl, IN const CamImgEncParams *params);

/* set img ehance params */
extern Int32 icam_set_img_enhance_params(IN ICamCtrlHandle hCtrl, IN const CamImgEnhanceParams *params);

/* set img tcp server info */
extern Int32 icam_set_img_srv_info(IN ICamCtrlHandle hCtrl, IN const CamTcpImageServerInfo *srvInfo);

/* set io config params */
extern Int32 icam_set_io_config(IN ICamCtrlHandle hCtrl, IN const CamIoCfg *params);

/* set network params */
extern Int32 icam_set_network_info(IN ICamCtrlHandle hCtrl, IN const CamNetworkInfo *info);

/* set ntp server info */
extern Int32 icam_set_ntp_srv_info(IN ICamCtrlHandle hCtrl, IN const CamNtpServerInfo *srvInfo);

/* set osd params */
extern Int32 icam_set_osd_params(IN ICamCtrlHandle hCtrl, IN const CamOsdParams *params);

/* set rgb gains */
extern Int32 icam_set_rgb_gains(IN ICamCtrlHandle hCtrl, IN const CamRGBGains *params);

/* set road info */
extern Int32 icam_set_road_info(IN ICamCtrlHandle hCtrl, IN const CamRoadInfo *info);

/* set rtp params for h.264 transfer */
extern Int32 icam_set_rtp_params(IN ICamCtrlHandle hCtrl, IN const CamRtpParams *params);

/* set special capture params */
extern Int32 icam_set_spec_cap_params(IN ICamCtrlHandle hCtrl, IN const CamSpecCapParams *params);

/* set strobe control params */
extern Int32 icam_set_strobe_params(IN ICamCtrlHandle hCtrl, IN const CamStrobeCtrlParam *params);

/* set cam date time */
extern Int32 icam_set_time(IN ICamCtrlHandle hCtrl, IN const CamDateTime *dateTime);

/* set traffic light regions */
extern Int32 icam_set_traffic_light_regions(IN ICamCtrlHandle hCtrl, IN const CamTrafficLightRegions *params);

/* set upload protocol */
extern Int32 icam_set_upload_proto(IN ICamCtrlHandle hCtrl, IN CamImgUploadProto proto);

/* set work mode */
extern Int32 icam_set_work_mode(IN ICamCtrlHandle hCtrl, IN const CamWorkMode *params);

/* ask cam to send all files in a dir */
extern Int32 icam_snd_dir(IN ICamCtrlHandle hCtrl, IN const char *dirPath);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __ICAM_CTRL_H__ */

