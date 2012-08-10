#include "detector_uart_generic.h"
#include "uart.h"
#include "log.h"


//#define TORY_CP_DEBUG
#define DETECTOR_ID_TORY_CP		0x0F
#define RX_BUF_SIZE				4


/*****************************************************************************
 Prototype    : tory_cp_init
 Description  : open detector device
 Input        : DetectorHandle hDetector  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/5
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 tory_cp_init(DetectorUart *dev, Uint16 detectorId)
{
	if(!dev || detectorId != DETECTOR_TORY_CP) {
		return  E_INVAL;
	}

	/* init params */
	dev->devName = UART_RS485;
	dev->baudRate = UART_B9600;
	dev->dataBits = UART_D8;
	dev->parity = UART_POFF;
	dev->stopBits = UART_S1;
	dev->chanId = UART_CHAN_RS485;

	dev->packetLen = 2;
	dev->private = NULL;

	return E_NO;
}

/*****************************************************************************
 Prototype    : tory_cp_sync
 Description  : compare start code
 Input        : Int32 fd    
                Uint8 buf  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/5
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Bool tory_cp_sync(DetectorUart *dev, Uint8 data)
{
	/* sync start code */
	if(data <= DETECTOR_ID_TORY_CP) {
		return TRUE;
	}
	
	return FALSE;
}

/*****************************************************************************
 Prototype    : retrograde_cap_parse
 Description  : retrograde capture parse
 Input        : DetectorUart *dev       
                Uint8 *rxBuf              
                CamDetectorParam *params  
                TriggerInfo *capInfo      
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/5
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 retrograde_cap_parse(DetectorUart *dev, const CamDetectorParam *params, Uint8 *rxBuf, TriggerInfo *info)
{
	Int32 wayNum;

	/* Check if capture is needed at this position */
	if(!params->retrogradeCapFlag)
		return E_NOTEXIST; //Should not capture at this position

	/* Set frame Id and flag */
	info->frameId = FRAME_TRIG_BASE;
	info->flags = 0;
	info->speed = 0;
	info->redlightTime = 0;

	/* Way Number */
	wayNum = (rxBuf[1] & 0x0F) + 1;
	info->wayNum = wayNum;

	/* Set retrograde flag */
	info->flags |= TRIG_INFO_RETROGRADE | TRIG_INFO_LAST_FRAME;
	
	/* Group Num */
	dev->groupId[wayNum - 1]++;
	info->groupId = dev->groupId[wayNum - 1];
	
	return E_NO;
}


/*****************************************************************************
 Prototype    : cp_cap_parse
 Description  : parse for checkpost
 Input        : DetectorUart *dev       
                Uint8 *rxBuf              
                CamDetectorParam *params  
                TriggerInfo *capInfo      
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/5
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 cp_cap_parse(DetectorUart *dev, const CamDetectorParam *params, Uint8 *rxBuf, TriggerInfo *info)
{
	Int32 	wayNum;
	Uint16	speed;

	/* Check if capture is needed at this position */
	if(!params->greenLightCapFlag)
		return E_NOTEXIST; //Should not capture at this position
	
	/* Set frame Id and flag */
	info->frameId = FRAME_TRIG_BASE;
	info->flags = TRIG_INFO_LAST_FRAME; // only one frame
	info->speed = 0;
	info->redlightTime = 0;

	/* Way Number */
	wayNum = (rxBuf[1] & 0x0F) + 1;
	info->wayNum = wayNum;

	/* Group Num */
	dev->groupId[wayNum - 1]++;
	info->groupId = dev->groupId[wayNum - 1];

	/* Calc speed */
	speed = rxBuf[2];
	if(params->speedModifyRatio[wayNum - 1]) //Modify speed
		speed = speed * (Uint16)(params->speedModifyRatio[wayNum-1]) / 100;
	info->speed = (Uint8)speed;
	
	if(info->speed > params->limitSpeed) {
		/* Set overspeed flag */
		info->flags |= TRIG_INFO_OVERSPEED;
	} else if(params->greenLightCapFlag & DETECTOR_FLAG_OVERSPEED_CAP)
		return E_NOTEXIST;	//Capture only when overspeed

	return E_NO;
}


/*****************************************************************************
 Prototype    : parse_trigger_data
 Description  : parse trigger data
 Input        : DetectorUart *dev       
                Uint8 *buf                
                CamDetectorParam *params  
                TriggerInfo *capInfo      
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/5
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 tory_cp_parse( DetectorUart *dev, 
							const CamDetectorParam *params,
							Uint8 *buf, 
							TriggerInfo *info )
{
	if((buf[1] & 0xF0) == 0x20) {
		/* Check post */
		return cp_cap_parse(dev, params, buf, info);
	}

	if((buf[1] & 0xF0) == 0x30 && buf[2] == 0x41) {
		/* Retrograde mode */
		return retrograde_cap_parse(dev, params, buf, info);
	}

	/* invalid data */
	return E_INVAL;
	
}

/* opt fxns for uart detector */
const DetectorUartFxns tory_cp_opts = {
	.init = tory_cp_init,
	.capPreParse = NULL,
	.startCodeSync = tory_cp_sync,
	.singleTrigParse = tory_cp_parse,
	.exit = NULL,
};


/*****************************************************************************
 Prototype    : tory_cp_test
 Description  : test tory cp detector
 Input        : None
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/5/22
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 tory_cp_test()
{
	CamDetectorParam params;
	Int32 err;

	bzero(&params, sizeof(params));
	params.detecotorId = DETECTOR_TORY_CP;
	params.capDelayTime = 0;
	params.redLightCapFlag = 
		DETECTOR_FLAG_LOOP1_POS_CAP | DETECTOR_FLAG_LOOP2_POS_CAP | DETECTOR_FLAG_LOOP1_NEG_CAP | DETECTOR_FLAG_LOOP2_NEG_CAP;
	params.greenLightCapFlag = DETECTOR_FLAG_LOOP1_NEG_CAP;
	params.retrogradeCapFlag = DETECTOR_FLAG_LOOP1_POS_CAP | DETECTOR_FLAG_LOOP2_POS_CAP | DETECTOR_FLAG_LOOP2_NEG_CAP;
	params.loopDist[0] = 500;
	params.loopDist[1] = 450;
	params.loopDist[2] = 300;
	params.loopDist[3] = 250;
	params.limitSpeed = 80;
	params.calcSpeed = 88;
	params.speedModifyRatio[0] = 100;
	params.speedModifyRatio[1] = 110;
	params.speedModifyRatio[2] = 100;
	params.speedModifyRatio[3] = 90;
	
	/* test parse fxns */
	const DetectorUartFxns *fxns = &tory_cp_opts;
	DetectorUart uartDev;

	bzero(&uartDev, sizeof(uartDev));
	uartDev.fd = 0;
	uartDev.baudRate = UART_B9600;
	uartDev.opts = fxns;
	uartDev.timeout = 50;
	
	err = fxns->init(&uartDev, DETECTOR_TORY_CP);
	assert(err == E_NO);

	if(fxns->capPreParse)
		fxns->capPreParse(&uartDev, &params);

	Uint8 data[8] = {0x0F, };
	err = fxns->startCodeSync(&uartDev, data[0]);
	assert(err == TRUE);

	TriggerInfo trigInfo;	
	
	/* test check post parse */
	data[0] = 0x0F;	//start code
	data[1] = 0x20;	// way num
	data[2] = 50;   //speed

	err = fxns->singleTrigParse(&uartDev, &params, data, &trigInfo);
	if(err)
		DBG("single trig parse ret: %s", str_err(err));
	assert(err == E_NO);
	assert(trigInfo.frameId == FRAME_TRIG_BASE);
	assert((trigInfo.flags & TRIG_INFO_RED_LIGHT) == 0);
	assert(trigInfo.redlightTime == 0);

	DBG("%s parse trig info:", __func__);
	DBG("  way num: %d, group id: %d, frame id: %d", 
		(int)trigInfo.wayNum, (int)trigInfo.groupId, (int)trigInfo.frameId);
	DBG("  flags: 0x%X, redlight time: %d, speed: %d Km/h", 
		(unsigned)trigInfo.flags, (int)trigInfo.redlightTime, (int)trigInfo.speed);

	/* test over speed */
	data[2] = 100;
	err = fxns->singleTrigParse(&uartDev, &params, data, &trigInfo);
	if(err)
		DBG("single trig parse ret: %s", str_err(err));
	assert(err == E_NO);
	assert(trigInfo.frameId == FRAME_TRIG_BASE);
	assert((trigInfo.flags & TRIG_INFO_RED_LIGHT) == 0);
	assert((trigInfo.flags & TRIG_INFO_OVERSPEED));
	assert(trigInfo.redlightTime == 0);

	DBG("%s parse trig info:", __func__);
	DBG("  way num: %d, group id: %d, frame id: %d", 
		(int)trigInfo.wayNum, (int)trigInfo.groupId, (int)trigInfo.frameId);
	DBG("  flags: 0x%X, redlight time: %d, speed: %d Km/h", 
		(unsigned)trigInfo.flags, (int)trigInfo.redlightTime, (int)trigInfo.speed);
	
	/* test retrograde */
	data[0] = 0x0F;
	data[1] = 0x31;
	data[2] = 0x41;	

	//bzero(&trigInfo, sizeof(trigInfo));
	err = fxns->singleTrigParse(&uartDev, &params, data, &trigInfo);
	if(err)
		DBG("single trig parse ret: %s", str_err(err));
	assert(err == E_NO);

	//assert(trigInfo.frameId == FRAME_TRIG_BASE + i);
	assert(trigInfo.flags & TRIG_INFO_RETROGRADE);

	DBG("%s parse trig info:", __func__);
	DBG("  way num: %d, group id: %d, frame id: %d", 
		(int)trigInfo.wayNum, (int)trigInfo.groupId, (int)trigInfo.frameId);
	DBG("  flags: 0x%X, redlight time: %d, speed: %d Km/h", 
		(unsigned)trigInfo.flags, (int)trigInfo.redlightTime, (int)trigInfo.speed);

	data[2] = 0x55;
	err = fxns->singleTrigParse(&uartDev, &params, data, &trigInfo);
	assert(err != E_NO);
	
	if(fxns->exit)
		fxns->exit(&uartDev);
	
	DBG("%s done", __func__);

	return E_NO;
}


