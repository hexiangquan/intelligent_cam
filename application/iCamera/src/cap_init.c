#include "cap_init.h"
#include "log.h"

/*****************************************************************************
 Prototype    : cap_module_init
 Description  : create capture module
 Input        : ParamsMngHandle hParamsMng  
                CamWorkMode *workMode       
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/19
    Author       : Sun
    Modification : Created function

*****************************************************************************/
CapHandle cap_module_init(IN const CamWorkMode *workMode, OUT ImgDimension *inputInfo)
{
	/* get current work mode  & img enhance params */
	CapAttrs		capAttrs;
	CapHandle		hCap;

	if(!workMode)
		return NULL;

	/* fill attrs for capture create */
	capAttrs.devName = CAP_DEVICE;
	capAttrs.inputType = CAP_INPUT_CAMERA;
	capAttrs.userAlloc = TRUE;
	capAttrs.bufNum = CAP_BUF_NUM;
	capAttrs.defRefCnt = 1;

	capAttrs.std = 
		(workMode->resType == CAM_RES_FULL_FRAME) ? CAP_STD_FULL_FRAME : CAP_STD_HIGH_SPEED;
	capAttrs.mode = 
		(workMode->capMode == CAM_CAP_MODE_CONTINUE) ? CAP_MODE_CONT : CAP_MODE_TRIG;
	
	/* create capture object */
	hCap = capture_create(&capAttrs);
	if(!hCap) {
		ERR("create capture handle failed...");
		return NULL;
	}

	/* get input info */
	if(inputInfo)
		capture_get_input_info(hCap, inputInfo);

	return hCap;
}

