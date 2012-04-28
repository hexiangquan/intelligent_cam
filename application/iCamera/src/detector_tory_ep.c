/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : detector_tory_ep.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/4/5
  Last Modified :
  Description   : detecor parse for Tory EP
  Function List :
              cp_cap_parse
              ep_cap_parse
              parse_trigger_data
              recv_start_code
              retrograde_cap_parse
              tory_ep_cap_pre_parse
              tory_ep_close
              tory_ep_control
              tory_ep_detect
              tory_ep_open
              tory_ep_start
              tory_ep_stop
              tory_ep_update
              tory_retrograde_cap_pre_parse
  History       :
  1.Date        : 2012/4/5
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "detector_uart_generic.h"
#include "log.h"

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
#define DETECTOR_ID_TORY_EP		0x0F
#define TORY_EP_DEBUG

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/


/*****************************************************************************
 Prototype    : tory_ep_cap_pre_parse
 Description  : parase for epolice capture
 Input        : Uint16 redLightCapFlags  
                Uint8 epCapTable[]       
                Uint8 *lastFrameId       
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/30
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static void tory_ep_cap_pre_parse(Uint16 redLightCapFlags, Uint8 epCapTable[], Uint8 *lastFrameId)
{
	/* Clear table */
	memset(epCapTable, 0xFF, CAP_TABLE_SIZE);
	
	switch(redLightCapFlags & 0x0F) {
	/* Only cap at loop1 pos edge */ 
	case DETECTOR_FLAG_LOOP1_POS_CAP:
		epCapTable[1] = FRAME_TRIG_BASE;	//First frame 
		*lastFrameId = 0x01;
		break;
	/* Only cap at loop1 negative edge */  
	case DETECTOR_FLAG_LOOP1_NEG_CAP:
		epCapTable[2] = FRAME_TRIG_BASE;
		*lastFrameId = 0x02;
		break;
	/* Cap at loop1 pos edge and loop1 negative edge */
	case (DETECTOR_FLAG_LOOP1_NEG_CAP|DETECTOR_FLAG_LOOP1_POS_CAP):
		epCapTable[1] = FRAME_TRIG_BASE;
		epCapTable[2] = FRAME_TRIG_BASE + 1;
		*lastFrameId = 0x02;
		break;
	/* Only cap at loop2 negative edge */ 
	case DETECTOR_FLAG_LOOP2_NEG_CAP:
		epCapTable[3] = FRAME_TRIG_BASE;
		*lastFrameId = 0x03;
		break;
	/* Cap at loop1 pos edge and loop2 negative edge */
	case (DETECTOR_FLAG_LOOP2_NEG_CAP|DETECTOR_FLAG_LOOP1_POS_CAP):
		epCapTable[1] = FRAME_TRIG_BASE;
		epCapTable[3] = FRAME_TRIG_BASE + 1;
		*lastFrameId = 0x03;
		break;
	/* Cap at loop1 negative and loop2 negative edge */
	case (DETECTOR_FLAG_LOOP2_NEG_CAP|DETECTOR_FLAG_LOOP1_NEG_CAP):
		epCapTable[2] = FRAME_TRIG_BASE;
		epCapTable[3] = FRAME_TRIG_BASE + 1;
		*lastFrameId = 0x03;
		break;
	/* Cap at loop1 pos, loop1 negative and loop2 negative edge */
	case (DETECTOR_FLAG_LOOP2_NEG_CAP|DETECTOR_FLAG_LOOP1_NEG_CAP|DETECTOR_FLAG_LOOP1_POS_CAP):
		epCapTable[1] = FRAME_TRIG_BASE;
		epCapTable[2] = FRAME_TRIG_BASE + 1;
		epCapTable[3] = FRAME_TRIG_BASE + 2;
		*lastFrameId = 0x03;
		break;
	default:
		break;
	}
}


/*****************************************************************************
 Prototype    : tory_retrograde_cap_pre_parse
 Description  : deceide which pictures are  need to be captured in Retrograde mode 
 			根据逆行标志，选择逆行模式时需要拍摄的照片
			逆行标志位		：DETECTOR_FLAG_LOOP2_NEG_CAP||DETECTOR_FLAG_LOOP1_NEG_CAP|| DETECTOR_FLAG_LOOP2_POS_CAP||DETECTOR_FLAG_LOOP1_POS_CAP
			标志位位置		：				4							3							2								1
			触发信息			：				0x03						0x04						0x01							0x02
			逆行压线圈顺序	：			压上2线圈				压2线圈压上1线圈				压1上线圈离开2线圈					离开1线圈
			逆行线圈触发信号：DETECTOR_FLAG_LOOP2_POS_CAP	DETECTOR_FLAG_LOOP1_POS_CAP		DETECTOR_FLAG_LOOP2_NEG_CAP		DETECTOR_FLAG_LOOP1_NEG_CAP
 Input        : Uint16 reCapFlags
 Output       : ucJudgeRetrogradeCap
 Return Value : static
 Calls        : 
 Called By    : DetectorToryEp2_wait_trig
 
  History        :
  1.Date         : 2011/7/27
    Author       : Wu
    Modification : Created function

*****************************************************************************/
void tory_retrograde_cap_pre_parse(Uint16 reCapFlags, Uint8 reCapTable[])
{
	/* Clear table */
	memset(reCapTable, 0xFF, CAP_TABLE_SIZE);

	/* Clear this bit because detector will not send trigger data at loop1 negative edge */
	reCapFlags &= ~DETECTOR_FLAG_LOOP1_NEG_CAP;
	
	switch(reCapFlags & 0x0F) {
	/* Only cap at loop1 pos edge */
	case DETECTOR_FLAG_LOOP1_POS_CAP:
		reCapTable[2] = FRAME_TRIG_BASE;
		break;
	/* Only cap at loop2 pos edge */
	case DETECTOR_FLAG_LOOP2_POS_CAP:
		reCapTable[1] = FRAME_TRIG_BASE;
		break;
	/* Cap at loop2 pos edge and loop1 pos edge */
	case DETECTOR_FLAG_LOOP2_POS_CAP|DETECTOR_FLAG_LOOP1_POS_CAP:
		reCapTable[1] = FRAME_TRIG_BASE;
		reCapTable[2] = FRAME_TRIG_BASE + 1;
		break;
	/* Cap at loop2 neg edge */
	case DETECTOR_FLAG_LOOP2_NEG_CAP:
		reCapTable[3] = FRAME_TRIG_BASE;
		break;
	/* Cap at loop2 neg edge and loop1 pos edge */
	case DETECTOR_FLAG_LOOP2_NEG_CAP|DETECTOR_FLAG_LOOP1_POS_CAP:
		reCapTable[2] = FRAME_TRIG_BASE;
		reCapTable[3] = FRAME_TRIG_BASE + 1;
		break;
	/* Cap at loop2 neg edge and loop2 pos edge */
	case DETECTOR_FLAG_LOOP2_NEG_CAP|DETECTOR_FLAG_LOOP2_POS_CAP:
		reCapTable[1] = FRAME_TRIG_BASE;
		reCapTable[3] = FRAME_TRIG_BASE + 1;
		break;
	/* Cap at loop2 pos edge, loop1 pos edge and loop2 neg edge */
	case DETECTOR_FLAG_LOOP2_NEG_CAP|DETECTOR_FLAG_LOOP2_POS_CAP|DETECTOR_FLAG_LOOP1_POS_CAP:
		reCapTable[1] = FRAME_TRIG_BASE;
		reCapTable[2] = FRAME_TRIG_BASE + 1;
		reCapTable[3] = FRAME_TRIG_BASE + 2;
		break;
	default:
		break;
	}
}


/*****************************************************************************
 Prototype    : tory_ep_pre_parse
 Description  : pre parse trigger params
 Input        : DetectorUart *dev               
                const CamDetectorParam *params  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/9
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static void tory_ep_pre_parse(DetectorUart *dev, const CamDetectorParam *params)
{
	if(!dev || !params)
		return;
	
	tory_ep_cap_pre_parse(params->redLightCapFlag, dev->epCapTable, &dev->lastFrameCode[TRIG_EP]);
	tory_retrograde_cap_pre_parse(params->retrogradeCapFlag, dev->reCapTable);
}


/*****************************************************************************
 Prototype    : tory_ep_sync
 Description  : sync start code
 Input        : DetectorUart *dev  
                Uint8 data         
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/6
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Bool tory_ep_sync(DetectorUart *dev, Uint8 data)
{
	if(data == DETECTOR_ID_TORY_EP) {
		return TRUE;
	}

	return FALSE;
}


/*****************************************************************************
 Prototype    : ep_cap_parse
 Description  : parse ep data
 Input        : DetectorUart *dev       
                CamDetectorParam *params  
                Uint8 *rxBuf              
                CaptureInfo *capInfo      
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/30
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 ep_cap_parse(DetectorUart *dev, const CamDetectorParam *params, Uint8 *rxBuf, TriggerInfo *info)
{	
	Int32 wayNum;

	wayNum = (rxBuf[1]>>4) & 0x0F;

	if(wayNum > APP_MAX_CAP_CNT)
		return E_INVAL;

	/* Record redlight time, unit: 10ms */
	if(rxBuf[2] < 0x03)
		dev->redlightTime[wayNum - 1] = ((rxBuf[3] << 8) + rxBuf[4])/10;

	/* Check if capture is needed at this position */
	if(dev->epCapTable[rxBuf[2]] == 0xFF)
		return E_NOTEXIST; //Should not capture at this position

	/* Set frame Id and flag */
	info->frameId = dev->epCapTable[rxBuf[2]];
	info->flags = 0;

	/* Way Number */
	info->wayNum = wayNum;

	/* Redlight time, unit: 10ms */
	info->redlightTime = dev->redlightTime[wayNum - 1]; 
	
	/* Set redlight flag */
	info->flags |= TRIG_INFO_RED_LIGHT;

	/* Group Num */
	if(info->frameId == FRAME_TRIG_BASE)
		dev->groupId[wayNum - 1]++;
	info->groupId = dev->groupId[wayNum - 1];

	/* Check if delay cap is set */
	if((params->redLightCapFlag & DETECTOR_FLAG_DELAY_CAP) &&
		rxBuf[2] == dev->lastFrameCode[TRIG_EP]) {
		info->flags |= TRIG_INFO_DELAY_CAP;
	}
	
	return E_NO;
}


/*****************************************************************************
 Prototype    : retrograde_cap_parse
 Description  : parse retrograde data
 Input        : DetectorUart *dev       
                CamDetectorParam *params  
                Uint8 *rxBuf              
                CaptureInfo *capInfo      
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/30
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 retrograde_cap_parse(DetectorUart *dev, const CamDetectorParam *params, Uint8 *rxBuf, TriggerInfo *info)
{
	Int32 wayNum;

	/* Way Number */
	wayNum = (rxBuf[1] >> 4) & 0x0F;

	if(wayNum > APP_MAX_CAP_CNT)
		return E_INVAL;

	/* Check if capture is needed at this position */
	if(dev->reCapTable[rxBuf[2]] == 0xFF)
		return E_NOTEXIST; //Should not capture at this position

	/* Set frame Id and flag */
	info->frameId = dev->reCapTable[rxBuf[2]];
	info->flags = 0;

	info->wayNum = wayNum;

	/* Set retrograde flag */
	info->flags |= TRIG_INFO_RETROGRADE;
	
	/* Group Num */
	if(info->frameId == FRAME_TRIG_BASE)
		dev->groupId[wayNum - 1]++;
	info->groupId = dev->groupId[wayNum - 1];
	
	return E_NO;
}

/*****************************************************************************
 Prototype    : cp_cap_parse
 Description  : parse data for green light
 Input        : DetectorUart *dev       
                CamDetectorParam *params  
                Uint8 *rxBuf              
                CaptureInfo *capInfo      
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/30
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 cp_cap_parse(DetectorUart *dev, const CamDetectorParam *params, Uint8 *rxBuf, TriggerInfo *info)
{
	Int32 wayNum;

	/* Check if capture is needed at this position */
	if(!(params->greenLightCapFlag & DETECTOR_FLAG_LOOP1_NEG_CAP))
		return E_NOTEXIST; //Should not capture at this position

	/* Way Number */
	wayNum = (rxBuf[1] >> 4) & 0x0F;

	if(wayNum > APP_MAX_CAP_CNT)
		return E_INVAL;
	
	/* Set frame Id and flag */
	info->frameId = FRAME_TRIG_BASE;
	info->flags = 0;

	/* Way Number */
	info->wayNum = wayNum;

	/* Group Num */
	dev->groupId[wayNum - 1]++;
	info->groupId = dev->groupId[wayNum - 1];

	/* Calc speed */
	info->speed = detector_calc_speed(params, wayNum, (rxBuf[5]<<8)|rxBuf[6]);
	if(info->speed > params->limitSpeed)
		info->flags |= TRIG_INFO_OVERSPEED;

	return E_NO;
}

/*****************************************************************************
 Prototype    : tory_ep_parse
 Description  : parse trigger data
 Input        : DetectorUart *dev             
                const CamDetectorParam *params  
                Uint8 *rxBuf                    
                CaptureInfo *capInfo            
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/30
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 tory_ep_parse(DetectorUart *dev, const CamDetectorParam *params, Uint8 *rxBuf, TriggerInfo *info)
{
	/* Validate data */
	if((rxBuf[1] & 0x0F) != 0x01 || rxBuf[2] > 0x03)
		return E_INVAL;

	if(!rxBuf[2]) {
		/* Check post */
		return cp_cap_parse(dev, params, rxBuf, info);
	}

	if(rxBuf[3] || rxBuf[4]) {
		/* Epolice mode */
		return ep_cap_parse(dev, params, rxBuf, info);
	}

	/* Retrograde */
	return retrograde_cap_parse(dev, params, rxBuf, info);
	
}


/*****************************************************************************
 Prototype    : tory_ep_init
 Description  : init device
 Input        : DetectorUart *dev  
                Uint16 detectorId  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/9
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 tory_ep_init(DetectorUart *dev, Uint16 detectorId)
{
	if(!dev || detectorId != DETECTOR_TORY_EP) {
		return  E_INVAL;
	}

	/* init params */
	dev->devName = UART_RS485;
	dev->baudRate = UART_B9600;
	dev->dataBits = UART_D8;
	dev->parity = UART_POFF;
	dev->stopBits = UART_S1;

	dev->packetLen = 5;
	dev->private = NULL;

	return E_NO;
}


/* opt fxns for uart detector */
const DetectorUartFxns tory_ep_opts = {
	.init = tory_ep_init,
	.capPreParse = tory_ep_pre_parse,
	.startCodeSync = tory_ep_sync,
	.singleTrigParse = tory_ep_parse,
	.exit = NULL,
};


