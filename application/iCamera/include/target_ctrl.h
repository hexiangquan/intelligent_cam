/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : dsp_srv.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/11/9
  Last Modified :
  Description   : dsp_srv.c header file
  Function List :
  History       :
  1.Date        : 2012/11/9
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __TARGET_SRV_H__
#define __TARGET_SRV_H__

#include "common.h"
#include "cam_plate_recog.h"
#include "cam_detector.h"
#include "cam_status.h"

/* struct for keep data from target */
typedef struct {
	Bool				vidDetectFlag;
	Bool				plateInfoFlag;
	Uint32				trigFrameId;
	VideoDetectResult 	vidDetectInfo;
	Uint32				plateFrameId;		// frame id of this plate info
	LicensePlateInfo 	plateInfo;
}TargetInfo;


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/**
 * target_ctrl_process -- recv and process msg from target processor
 */
extern Int32 target_ctrl_process(Int32 fd, TargetInfo *info);

/**
 * target_plate_recog_cfg -- cfg plate recog alg
 */
extern Int32 target_plate_recog_cfg(Int32 fdSyslink, const CamPlateRecogCfg *cfg);

/**
 * target_vid_detect_cfg -- cfg video detect alg
 */
extern Int32 target_vid_detect_cfg(Int32 fdSyslink, const CamVidDetectCfg *cfg);

/**
 * target_day_night_cfg -- notify target to switch day night mode
 */
extern Int32 target_day_night_cfg(Int32 fdSyslink, CamDayNightMode mode);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __DSP_SRV_H__ */
