/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : data_collector.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/6
  Last Modified :
  Description   : data_collector.c header file
  Function List :
  History       :
  1.Date        : 2012/3/6
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __DATA_COLLECTOR_H__
#define __DATA_COLLECTOR_H__

#include "params_mng.h"
#include "capture.h"
#include "msg.h"

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
	ParamsMngHandle hParamsMng;
	CapHandle		hCap;
}CollectorAttrs;

typedef struct CollectorObj *CollectorHandle;

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
extern CollectorHandle data_collector_create(CollectorAttrs *attrs);

/*****************************************************************************
 Prototype    : data_collector_delete
 Description  : delete collector handle
 Input        : CollectorHandle hCollector  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/16
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 data_collector_delete(CollectorHandle hCollector, MsgHandle hCurMsg);

/*****************************************************************************
 Prototype    : data_collector_run
 Description  : run data collect, capture frame, recv trigger, etc.
 Input        : CollectorHandle hCollector  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/16
    Author       : Sun
    Modification : Created function

*****************************************************************************/
extern Int32 data_collector_run(CollectorHandle hCollector);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __CAPTURE_THR_H__ */
