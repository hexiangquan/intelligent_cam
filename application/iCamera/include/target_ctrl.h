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

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/**
 * target_ctrl_init -- init target ctrl module 
 */
extern Int32 target_ctrl_init(Uint32 timeout);

/**
 * target_ctrl_exit -- exit target ctrl module 
 */
extern Int32 target_ctrl_exit();

/**
 * target_set_params -- cfg params to target
 */
extern Int32 target_set_params(const void *data, size_t dataLen);

/**
 * target_get_params -- get params from target
 */
extern Int32 target_get_params(const void *info, size_t infoLen, void *data, size_t dataLen);

/**
 * target_reset -- reset target
 */
extern Int32 target_reset();

/**
 * target_day_night_switch -- switch day night mode
 */
extern Int32 target_day_night_switch(Bool isNightMode);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __DSP_SRV_H__ */
