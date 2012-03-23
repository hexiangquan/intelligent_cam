/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : ctrl_thr.h
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/15
  Last Modified :
  Description   : ctrl_thr.c header file
  Function List :
  History       :
  1.Date        : 2012/3/15
    Author      : Sun
    Modification: Created file

******************************************************************************/
#ifndef __CTRL_SERVER_H__
#define __CTRL_SERVER_H__

#include "params_mng.h"
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

typedef struct CtrlSrvObj *CtrlSrvHandle;

/* init argument for this thread */
typedef struct {
	ParamsMngHandle hParamsMng;
	const char 		*msgName;
}CtrlSrvAttrs;

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*****************************************************************************
 Prototype    : ctrl_server_create
 Description  : create ctrl server
 Input        : CtrlSrvAttrs *attrs  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/22
    Author       : Sun
    Modification : Created function

*****************************************************************************/
CtrlSrvHandle ctrl_server_create(CtrlSrvAttrs *attrs);

/*****************************************************************************
 Prototype    : ctrl_server_run
 Description  : start ctrl server running
 Input        : CtrlSrvHandle hCtrlSrv  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/22
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 ctrl_server_run(CtrlSrvHandle hCtrlSrv);

/*****************************************************************************
 Prototype    : ctrl_server_delete
 Description  : delete this module
 Input        : CtrlSrvHandle hCtrlSrv  
                MsgHandle hCurMsg       
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/22
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 ctrl_server_delete(CtrlSrvHandle hCtrlSrv, MsgHandle hCurMsg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __CTRL_THR_H__ */
