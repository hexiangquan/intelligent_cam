/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : data_capture.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/6
  Last Modified :
  Description   : data_capture.c header file
  Function List :
  History       :
  1.Date        : 2012/3/6
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __DATA_CAPTURE_H__
#define __DATA_CAPTURE_H__

#include "cam_status.h"
#include "cam_detector.h"
#include "converter.h"
#include "capture.h"
#include "msg.h"
#include "cam_info.h"

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

/* init argument for this module */
typedef struct {
	Uint16				maxOutWidth;
	Uint16				maxOutHeight;
	CamWorkMode			workMode;
	CamDetectorParam	detectorParams;
	ConverterParams		convParams;
	CapHandle			hCapture;
	CamRoadInfo			roadInfo;
}DataCapAttrs;

typedef struct DataCapObj *DataCapHandle;

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*****************************************************************************
 Prototype    : data_capture_create
 Description  : create this module
 Input        : DataCapAttrs *attrs  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/16
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern DataCapHandle data_capture_create(const DataCapAttrs *attrs);

/*****************************************************************************
 Prototype    : data_capture_delete
 Description  : delete collector handle
 Input        : DataCapHandle hDataCap
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/16
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 data_capture_delete(DataCapHandle hDataCap, MsgHandle hCurMsg);

/*****************************************************************************
 Prototype    : data_capture_run
 Description  : run data collect, capture frame, recv trigger, etc.
 Input        : DataCapHandle hDataCap
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/16
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 data_capture_run(DataCapHandle hDataCap);

/*****************************************************************************
 Prototype    : data_capture_set_work_mode
 Description  : set work mode
 Input        : DataCapHandle hDataCap  
                MsgHandle hCurMsg       
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/21
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 data_capture_set_work_mode(DataCapHandle hDataCap, MsgHandle hCurMsg, const CamWorkMode *workMode);

/*****************************************************************************
 Prototype    : data_capture_conv_params
 Description  : set convert params
 Input        : DataCapHandle hDataCap  
                MsgHandle hCurMsg       
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/21
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 data_capture_set_conv_params(DataCapHandle hDataCap, MsgHandle hCurMsg, const ConverterParams *params);

/*****************************************************************************
 Prototype    : data_capture_set_detector_params
 Description  : set detector params
 Input        : DataCapHandle hDataCap          
                MsgHandle hCurMsg               
                const CamDetectorParam *params  
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 data_capture_set_detector_params(DataCapHandle hDataCap, MsgHandle hCurMsg, const CamDetectorParam *params);

/*****************************************************************************
 Prototype    : data_capture_get_input_info
 Description  : get input info
 Input        : DataCapHandle hDataCap   
                CamInputInfo *inputInfo  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 data_capture_get_input_info(DataCapHandle hDataCap, ImgDimension *inputInfo);

/*****************************************************************************
 Prototype    : data_capture_ctrl
 Description  : change capture status
 Input        : DataCapHandle hDataCap  
                MsgHandle hCurMsg       
                CamCapCtrl ctrl         
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 data_capture_ctrl(DataCapHandle hDataCap, MsgHandle hCurMsg, Int32 ctrl);

/*****************************************************************************
 Prototype    : data_capture_set_road_info
 Description  : update road info
 Input        : DataCapHandle hDataCap  
                MsgHandle hCurMsg       
                CamRoadInfo *info       
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/1
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 data_capture_set_road_info(DataCapHandle hDataCap, MsgHandle hCurMsg, CamRoadInfo *info);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __CAPTURE_THR_H__ */
