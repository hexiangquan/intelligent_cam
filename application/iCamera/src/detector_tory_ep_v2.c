#include "detector_uart_generic.h"
#include "log.h"

/* use same parse functions because protocol is similar */
extern void tory_retrograde_cap_pre_parse(Uint16 reCapFlags, Uint8 reCapTable[], Uint8 *lastFrameId);


//#define TORY_EP2_DEBUG

/*****************************************************************************
 Prototype    : ep_cap_pre_parse
 Description  : deceide whehter there is a need to capture a image in Electronic-Police mode 
			根据红灯标志位，选择闯红灯模式时需要拍摄的照片
			红灯标志位：DETECTOR_FLAG_LOOP2_NEG_CAP||DETECTOR_FLAG_LOOP2_POS_CAP|| DETECTOR_FLAG_LOOP1_NEG_CAP||DETECTOR_FLAG_LOOP1_POS_CAP
			标志位序号：				4							3							2								1
			图片序号	：				0x04							0x02							0x03								0x01
			红灯压线圈顺序：压上1线圈				压1线圈压上2线圈		压2线圈离开1线圈		离开2线圈
			线圈触发信号顺序：DETECTOR_FLAG_LOOP1_POS_CAP	DETECTOR_FLAG_LOOP2_POS_CAP	DETECTOR_FLAG_LOOP1_NEG_CAP	DETECTOR_FLAG_LOOP2_NEG_CAP
 Input        : Uint16 redLightCapFlags
 Output       : Uint8 *epCapTable,Uint8 lastFrameId
 Return Value : static
 Calls        : 
 Called By    : DetectorToryEp2_wait_trig
 
  History        :
  1.Date         : 2011/7/27
    Author       : Wu
    Modification : Created function

*****************************************************************************/
static void ep_cap_pre_parse(Uint16 redLightCapFlags, Uint8 *epCapTable, Uint8 *lastFrameId)
{
	/* Clear table */
	memset(epCapTable, 0xFF, CAP_TABLE_SIZE);
	
	switch(redLightCapFlags & 0x0F) {
	/* Only cap at loop1 pos edge */ 
	case DETECTOR_FLAG_LOOP1_POS_CAP:
		epCapTable[0] = FRAME_TRIG_BASE;	//First frame 
		*lastFrameId = 0x01;
		break;
	/* Only cap at loop2 pos edge */   
	case DETECTOR_FLAG_LOOP2_POS_CAP:
		epCapTable[1] = FRAME_TRIG_BASE;	//First frame 
		*lastFrameId = 0x02;
		break;
	/* Cap at loop1 pos edge and loop2 pos edge */
	case DETECTOR_FLAG_LOOP2_POS_CAP|DETECTOR_FLAG_LOOP1_POS_CAP:
		epCapTable[0] = FRAME_TRIG_BASE;
		epCapTable[1] = FRAME_TRIG_BASE + 1;
		*lastFrameId = 0x02;
		break;
	/* Only cap at loop1 negative edge */  
	case DETECTOR_FLAG_LOOP1_NEG_CAP:
		epCapTable[2] = FRAME_TRIG_BASE;
		*lastFrameId = 0x03;
		break;
	/* Cap at loop1 pos edge and loop1 negative edge */
	case DETECTOR_FLAG_LOOP1_NEG_CAP|DETECTOR_FLAG_LOOP1_POS_CAP:
		epCapTable[0] = FRAME_TRIG_BASE;
		epCapTable[2] = FRAME_TRIG_BASE + 1;
		*lastFrameId = 0x03;
		break;
	/* Cap at loop2 pos edge and loop1 negative edge */
	case DETECTOR_FLAG_LOOP1_NEG_CAP|DETECTOR_FLAG_LOOP2_POS_CAP:
		epCapTable[1] = FRAME_TRIG_BASE;
		epCapTable[2] = FRAME_TRIG_BASE + 1;
		*lastFrameId = 0x03;
		break;
	/* Cap at loop1 pos, loop2 pos and loop1 negative edge */
	case DETECTOR_FLAG_LOOP1_NEG_CAP|DETECTOR_FLAG_LOOP2_POS_CAP|DETECTOR_FLAG_LOOP1_POS_CAP:
		epCapTable[0] = FRAME_TRIG_BASE;
		epCapTable[1] = FRAME_TRIG_BASE + 1;
		epCapTable[2] = FRAME_TRIG_BASE + 2;
		*lastFrameId = 0x03;
		break;
	/* Only cap at loop2 negative edge */ 
	case DETECTOR_FLAG_LOOP2_NEG_CAP:
		epCapTable[3] = FRAME_TRIG_BASE;
		*lastFrameId = 0x04;
		break;
	/* Cap at loop1 pos edge and loop2 negative edge */
	case DETECTOR_FLAG_LOOP2_NEG_CAP|DETECTOR_FLAG_LOOP1_POS_CAP:
		epCapTable[0] = FRAME_TRIG_BASE;
		epCapTable[3] = FRAME_TRIG_BASE + 1;
		*lastFrameId = 0x04;
		break;
	/* Cap at loop2 pos edge and loop2 negative edge */
	case DETECTOR_FLAG_LOOP2_NEG_CAP|DETECTOR_FLAG_LOOP2_POS_CAP:
		epCapTable[1] = FRAME_TRIG_BASE;
		epCapTable[3] = FRAME_TRIG_BASE + 1;
		*lastFrameId = 0x04;
		break;
	/* Cap at loop1 pos, loop2 pos and loop2 negative edge */
	case DETECTOR_FLAG_LOOP2_NEG_CAP|DETECTOR_FLAG_LOOP2_POS_CAP|DETECTOR_FLAG_LOOP1_POS_CAP:
		epCapTable[0] = FRAME_TRIG_BASE;
		epCapTable[1] = FRAME_TRIG_BASE + 1;
		epCapTable[3] = FRAME_TRIG_BASE + 2;
		*lastFrameId = 0x04;
		break;
	/* Cap at loop1 negative and loop2 negative edge */
	case DETECTOR_FLAG_LOOP2_NEG_CAP|DETECTOR_FLAG_LOOP1_NEG_CAP:
		epCapTable[2] = FRAME_TRIG_BASE;
		epCapTable[3] = FRAME_TRIG_BASE + 1;
		*lastFrameId = 0x04;
		break;
	/* Cap at loop1 pos, loop1 negative andd loop2 negative edge */
	case DETECTOR_FLAG_LOOP2_NEG_CAP|DETECTOR_FLAG_LOOP1_NEG_CAP|DETECTOR_FLAG_LOOP1_POS_CAP:
		epCapTable[0] = FRAME_TRIG_BASE;
		epCapTable[2] = FRAME_TRIG_BASE + 1;
		epCapTable[3] = FRAME_TRIG_BASE + 2;
		*lastFrameId = 0x04;
		break;
	/* Cap at loop2 pos, loop1 negative and loop2 negative edge */
	case DETECTOR_FLAG_LOOP2_NEG_CAP|DETECTOR_FLAG_LOOP1_NEG_CAP|DETECTOR_FLAG_LOOP2_POS_CAP:
		epCapTable[1] = FRAME_TRIG_BASE;
		epCapTable[2] = FRAME_TRIG_BASE + 1;
		epCapTable[3] = FRAME_TRIG_BASE + 2;
		*lastFrameId = 0x04;
		break;
	/* Cap at all the edges */
	case DETECTOR_FLAG_LOOP2_NEG_CAP|DETECTOR_FLAG_LOOP1_NEG_CAP|DETECTOR_FLAG_LOOP2_POS_CAP|DETECTOR_FLAG_LOOP1_POS_CAP:
		epCapTable[0] = FRAME_TRIG_BASE;
		epCapTable[1] = FRAME_TRIG_BASE + 1;
		epCapTable[2] = FRAME_TRIG_BASE + 2;
		epCapTable[3] = FRAME_TRIG_BASE + 3;
		*lastFrameId = 0x04;
		break;
	default:
		break;
	}
}

/*****************************************************************************
 Prototype    : ep_cap_parse
 Description  : parse ep capture
 Input        : DetectorUart *dev               
                const CamDetectorParam *params  
                Uint8 *rxBuf                    
                TriggerInfo *info               
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/6
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 ep_cap_parse(DetectorUart *dev, const CamDetectorParam *params, Uint8 *rxBuf, TriggerInfo *info)
{	
	Int32 wayNum;

	if((rxBuf[3] & 0x0F) != 0x01 || rxBuf[4] > 0x04)	//Check way number
		return E_INVAL;

	wayNum = (rxBuf[3]>>4) & 0x0F;
	if(rxBuf[4] < 0x03)
		dev->redlightTime[wayNum - 1] = (rxBuf[5] << 8) + rxBuf[6];

	/* Check if capture is needed at this position */
	if(dev->epCapTable[rxBuf[4] - 1] == 0xFF)
		return E_NOTEXIST; //Should not capture at this position

	/* Set frame Id and flag */
	info->frameId = dev->epCapTable[rxBuf[4] - 1];
	info->flags = TRIG_INFO_RED_LIGHT;
	info->speed = 0;

	/* Way Number */
	info->wayNum = wayNum;

	/* Redlight time, unit: 10ms */
	info->redlightTime = dev->redlightTime[wayNum - 1]; 

	/* Group Num */
	if(info->frameId == FRAME_TRIG_BASE)
		dev->groupId[wayNum - 1]++;
	info->groupId = dev->groupId[wayNum - 1];

	/* Check if delay cap is set */
	if(rxBuf[4] == dev->lastFrameCode[TRIG_EP]) {
		info->flags |= TRIG_INFO_LAST_FRAME;
		if(params->redLightCapFlag & DETECTOR_FLAG_DELAY_CAP)
			info->flags |= TRIG_INFO_DELAY_CAP;
	}

	return E_NO;
}


/*****************************************************************************
 Prototype    : retrograde_cap_parse
 Description  : parse retrograde
 Input        : DetectorUart *dev           
                const CamDetectorParam *params  
                Uint8 *rxBuf                    
                CaptureInfo *capInfo            
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

	if((rxBuf[3] & 0x0F) != 0x01 || rxBuf[4] > 0x03)	//Check way number
		return E_INVAL;

	/* Check if capture is needed at this position */
	if(dev->reCapTable[rxBuf[4]] == 0xFF)
		return E_NOTEXIST; //Should not capture at this position

	/* Set frame Id and flag */
	info->frameId = dev->reCapTable[rxBuf[4]];
	info->flags = 0;
	info->redlightTime = 0;
	info->speed = 0;

	/* Way Number */
	wayNum = (rxBuf[3] >> 4) & 0x0F;
	info->wayNum = wayNum;

	/* Set retrograde flag */
	info->flags |= TRIG_INFO_RETROGRADE;
	
	/* Group Num */
	if(info->frameId == FRAME_TRIG_BASE)
		dev->groupId[wayNum - 1]++;
	info->groupId = dev->groupId[wayNum - 1];

	/* Check if it is last frame of the group */
	if(rxBuf[4] == dev->lastFrameCode[TRIG_RE])
		info->flags |= TRIG_INFO_LAST_FRAME;

	return E_NO;
}

/*****************************************************************************
 Prototype    : cp_cap_parse
 Description  : parse for checkpost data
 Input        : DetectorUart *dev           
                const CamDetectorParam *params  
                Uint8 *rxBuf                    
                CaptureInfo *capInfo            
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
	Int32 wayNum;

	/* Check if capture is needed at this position */
	if(!params->greenLightCapFlag)
		return E_NOTEXIST; //Should not capture at this position

	/* Validate data */
	if((rxBuf[3] & 0x0F) != 0x01 || rxBuf[4])	
		return E_INVAL;
	
	/* Set frame Id and flag */
	info->frameId = FRAME_TRIG_BASE;
	info->flags = TRIG_INFO_LAST_FRAME;
	info->speed = 0;
	info->redlightTime = 0;

	/* Way Number */
	wayNum = (rxBuf[3] >> 4) & 0x0F;
	info->wayNum = wayNum;

	/* Group Num */
	dev->groupId[wayNum - 1]++;
	info->groupId = dev->groupId[wayNum - 1];

	/* Calc speed */
	info->speed = detector_calc_speed(params, wayNum, (rxBuf[5]<<8)|(rxBuf[6])*10);
	if(info->speed > params->limitSpeed)
		info->flags |= TRIG_INFO_OVERSPEED;

	return E_NO;
}

/*****************************************************************************
 Prototype    : tory_ep2_pre_parse
 Description  : pre parse data
 Input        : DetectorUart *dev               
                const CamDetectorParam *params  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/6
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static void tory_ep2_pre_parse(DetectorUart *dev, const CamDetectorParam *params)
{
	if(!dev || !params)
		return;

	ep_cap_pre_parse(params->redLightCapFlag, dev->epCapTable, &dev->lastFrameCode[TRIG_EP]);
	tory_retrograde_cap_pre_parse(params->retrogradeCapFlag, dev->reCapTable, &dev->lastFrameCode[TRIG_RE]);
}


/*****************************************************************************
 Prototype    : tory_ep2_sync
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
static Bool tory_ep2_sync(DetectorUart *dev, Uint8 data)
{
	if(data == 0xFA || data == 0xFB || data == 0xFC || data == 0xFD) {
		return TRUE;
	}

	return FALSE;
}

/*****************************************************************************
 Prototype    : tory_ep2_parse
 Description  : parse trigger data
 Input        : DetectorUart *dev           
                const CamDetectorParam *params  
                Uint8 *buf                      
                CaptureInfo *capInfo            
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/5
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 tory_ep2_parse(DetectorUart *dev,
								const CamDetectorParam *params, 
								Uint8 *buf, 
								TriggerInfo *info )	
{
	Int32 status = E_NO;
	
	switch(buf[0]) {
	case 0xFA:
		/* Epolice mode */
		status = ep_cap_parse(dev, params, buf, info);
		break;
	case 0xFB:
		/* Check post */
		status = cp_cap_parse(dev, params, buf, info);
		break;
	case 0xFC:
		/* Retrograde */	
		status = retrograde_cap_parse(dev, params, buf, info);
		break;
	default:
		status = E_INVAL;
		break;
	}

	return status;
}


/*****************************************************************************
 Prototype    : tory_ep2_init
 Description  : probe & init
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
static Int32 tory_ep2_init(DetectorUart *dev, Uint16 detectorId)
{
	if(!dev || detectorId != DETECTOR_TORY_EP_V2) {
		return  E_INVAL;
	}

	/* init params */
	dev->devName = UART_RS485;
	dev->baudRate = UART_B9600;
	dev->dataBits = UART_D8;
	dev->parity = UART_POFF;
	dev->stopBits = UART_S1;

	dev->packetLen = 7;
	dev->private = NULL;

	return E_NO;
}


/* opt fxns for uart detector */
const DetectorUartFxns tory_ep2_opts = {
	.init = tory_ep2_init,
	.capPreParse = tory_ep2_pre_parse,
	.startCodeSync = tory_ep2_sync,
	.singleTrigParse = tory_ep2_parse,
	.exit = NULL,
};


/*****************************************************************************
 Prototype    : tory_ep2_test
 Description  : test tory ep new detector
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
Int32 tory_ep2_test()
{
	CamDetectorParam params;
	Int32 err;

	bzero(&params, sizeof(params));
	params.detecotorId = DETECTOR_TORY_EP_V2;
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
	const DetectorUartFxns *fxns = &tory_ep2_opts;
	DetectorUart uartDev;

	bzero(&uartDev, sizeof(uartDev));
	uartDev.fd = 0;
	uartDev.baudRate = UART_B9600;
	uartDev.opts = fxns;
	uartDev.timeout = 50;
	
	err = fxns->init(&uartDev, DETECTOR_TORY_EP_V2);
	assert(err == E_NO);

	if(fxns->capPreParse)
		fxns->capPreParse(&uartDev, &params);

	Uint8 data[8] = {0xFA, };
	err = fxns->startCodeSync(&uartDev, data[0]);
	assert(err == TRUE);
	data[0] = 0xFB;
	err = fxns->startCodeSync(&uartDev, data[0]);
	assert(err == TRUE);
	data[0] = 0xFC;
	err = fxns->startCodeSync(&uartDev, data[0]);
	assert(err == TRUE);
	data[0] = 0xFD;
	err = fxns->startCodeSync(&uartDev, data[0]);
	assert(err == TRUE);

	int i, j;
	TriggerInfo trigInfo;
	
	/* test red light capture */
	data[0] = 0xFA; // start code
	data[1] = 0x01; // ID, direction
	data[2] = 0;	// frame cnt
	data[3] = 0x01; // Way num
	for(j = 0; j < 3; j++) {
		data[3] += 0x10;
		for(i = 0; i < 4; i++) {
			/* mock detector trigger data */	
			data[2]++;
			data[4] = 0x01 + i;
			data[5] = 0x00;
			data[6] = 200 + i * 10;

			err = fxns->singleTrigParse(&uartDev, &params, data, &trigInfo);
			if(err)
				ERR("single trig parse ret: %s", str_err(err));
			assert(err == E_NO);

			assert(trigInfo.frameId == FRAME_TRIG_BASE + i);
			assert(trigInfo.flags & TRIG_INFO_RED_LIGHT);

			DBG("%s parse trig info:", __func__);
			DBG("  way num: %d, group id: %d, frame id: %d", 
				(int)trigInfo.wayNum, (int)trigInfo.groupId, (int)trigInfo.frameId);
			DBG("  flags: 0x%X, redlight time: %d, speed: %d Km/h", 
				(unsigned)trigInfo.flags, (int)trigInfo.redlightTime, (int)trigInfo.speed);
		}
	}

	/* test check post parse */
	data[0] = 0xFB;	//start code
	data[1] = 0x02;	// ID
	data[2]++; 		// frame cnt 
	data[3] = 0x21; // way num
	data[4] = 0;
	data[5] = 0;
	data[6] = 20;

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

	/* test retrograde */
	data[0] = 0xFC;
	data[1] = 0x03;

	data[3] = 0x31;
	data[4] = 0x01;
	data[5] = data[4] = 0;
	
	for(i = 0; i < 3; i++) {
		/* mock detector trigger data */		
		data[4] = 0x01 + i;

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
	}
	
	if(fxns->exit)
		fxns->exit(&uartDev);
	
	DBG("%s done", __func__);

	return err;
}


