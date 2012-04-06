#include "detector.h"
#include "uart.h"
#include "log.h"

/* use same parse functions because protocol is similar */
extern void tory_retrograde_cap_pre_parse(Uint16 reCapFlags, Uint8 reCapTable[]);

extern Int32 tory_ep_start(Int32 *fd, Uint8 timeout);

extern Int32 tory_ep_stop(Int32 fd);

//#define TORY_EP2_DEBUG
	
#define DETECTOR_ID				0x03
#define RX_BUF_SIZE				(8)
#define CAP_TABLE_SIZE			4

/* Private params used at runtime */
typedef struct
{
	int			fd;									//Rs485 fd
	Uint16		groupId[APP_MAX_CAP_CNT];			//GroupNum for each way
	Uint16		redlightTime[APP_MAX_CAP_CNT];		//Redlight time record for each way
	Uint8		epCapTable[CAP_TABLE_SIZE];			//Epolice trigger code to frame id table
	Uint8		reCapTable[CAP_TABLE_SIZE];			//Retrograde trigger code to frame id table
	Uint8		lastFrameId;						// Trigger code for the last frame 
	Uint8		timeout;							//recv timeout: 100ms
}DetectorToryEpV2;


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
	
	switch(redLightCapFlags & 0x0F)
	{
		/* Only cap at loop1 pos edge */ 
		case DETECTOR_FLAG_LOOP1_POS_CAP:
		{
			epCapTable[0] = FRAME_EPOLICE_1ST;	//First frame 
			*lastFrameId = 0x01;
			break;
		}
		/* Only cap at loop2 pos edge */   
		case DETECTOR_FLAG_LOOP2_POS_CAP:
		{
			epCapTable[1] = FRAME_EPOLICE_1ST;	//First frame 
			*lastFrameId = 0x02;
			break;
		}
		/* Cap at loop1 pos edge and loop2 pos edge */
		case DETECTOR_FLAG_LOOP2_POS_CAP|DETECTOR_FLAG_LOOP1_POS_CAP:
		{
			epCapTable[0] = FRAME_EPOLICE_1ST;
			epCapTable[1] = FRAME_EPOLICE_2ND;
			*lastFrameId = 0x02;
			break;
		}
		/* Only cap at loop1 negative edge */  
		case DETECTOR_FLAG_LOOP1_NEG_CAP:
		{
			epCapTable[2] = FRAME_EPOLICE_1ST;
			*lastFrameId = 0x03;
			break;
		}
		/* Cap at loop1 pos edge and loop1 negative edge */
		case DETECTOR_FLAG_LOOP1_NEG_CAP|DETECTOR_FLAG_LOOP1_POS_CAP:
		{
			epCapTable[0] = FRAME_EPOLICE_1ST;
			epCapTable[2] = FRAME_EPOLICE_2ND;
			*lastFrameId = 0x03;
			break;
		}
		/* Cap at loop2 pos edge and loop1 negative edge */
		case DETECTOR_FLAG_LOOP1_NEG_CAP|DETECTOR_FLAG_LOOP2_POS_CAP:
		{
			epCapTable[1] = FRAME_EPOLICE_1ST;
			epCapTable[2] = FRAME_EPOLICE_2ND;
			*lastFrameId = 0x03;
			break;
		}
		/* Cap at loop1 pos, loop2 pos and loop1 negative edge */
		case DETECTOR_FLAG_LOOP1_NEG_CAP|DETECTOR_FLAG_LOOP2_POS_CAP|DETECTOR_FLAG_LOOP1_POS_CAP:
		{
			epCapTable[0] = FRAME_EPOLICE_1ST;
			epCapTable[1] = FRAME_EPOLICE_2ND;
			epCapTable[2] = FRAME_EPOLICE_3RD;
			*lastFrameId = 0x03;
			break;
		}
		/* Only cap at loop2 negative edge */ 
		case DETECTOR_FLAG_LOOP2_NEG_CAP:
		{
			epCapTable[3] = FRAME_EPOLICE_1ST;
			*lastFrameId = 0x04;
			break;
		}
		/* Cap at loop1 pos edge and loop2 negative edge */
		case DETECTOR_FLAG_LOOP2_NEG_CAP|DETECTOR_FLAG_LOOP1_POS_CAP:
		{
			epCapTable[0] = FRAME_EPOLICE_1ST;
			epCapTable[3] = FRAME_EPOLICE_2ND;
			*lastFrameId = 0x04;
			break;
		}
		/* Cap at loop2 pos edge and loop2 negative edge */
		case DETECTOR_FLAG_LOOP2_NEG_CAP|DETECTOR_FLAG_LOOP2_POS_CAP:
		{
			epCapTable[1] = FRAME_EPOLICE_1ST;
			epCapTable[3] = FRAME_EPOLICE_2ND;
			*lastFrameId = 0x04;
			break;
		}
		/* Cap at loop1 pos, loop2 pos and loop2 negative edge */
		case DETECTOR_FLAG_LOOP2_NEG_CAP|DETECTOR_FLAG_LOOP2_POS_CAP|DETECTOR_FLAG_LOOP1_POS_CAP:
		{
			epCapTable[0] = FRAME_EPOLICE_1ST;
			epCapTable[1] = FRAME_EPOLICE_2ND;
			epCapTable[3] = FRAME_EPOLICE_3RD;
			*lastFrameId = 0x04;
			break;
		}
		/* Cap at loop1 negative and loop2 negative edge */
		case DETECTOR_FLAG_LOOP2_NEG_CAP|DETECTOR_FLAG_LOOP1_NEG_CAP:
		{
			epCapTable[2] = FRAME_EPOLICE_1ST;
			epCapTable[3] = FRAME_EPOLICE_2ND;
			*lastFrameId = 0x04;
			break;
		}
		/* Cap at loop1 pos, loop1 negative andd loop2 negative edge */
		case DETECTOR_FLAG_LOOP2_NEG_CAP|DETECTOR_FLAG_LOOP1_NEG_CAP|DETECTOR_FLAG_LOOP1_POS_CAP:
		{
			epCapTable[0] = FRAME_EPOLICE_1ST;
			epCapTable[2] = FRAME_EPOLICE_2ND;
			epCapTable[3] = FRAME_EPOLICE_3RD;
			*lastFrameId = 0x04;
			break;
		}
		/* Cap at loop2 pos, loop1 negative and loop2 negative edge */
		case DETECTOR_FLAG_LOOP2_NEG_CAP|DETECTOR_FLAG_LOOP1_NEG_CAP|DETECTOR_FLAG_LOOP2_POS_CAP:
		{
			epCapTable[1] = FRAME_EPOLICE_1ST;
			epCapTable[2] = FRAME_EPOLICE_2ND;
			epCapTable[3] = FRAME_EPOLICE_3RD;
			*lastFrameId = 0x04;
			break;
		}
		/* Cap at all the edges */
		case DETECTOR_FLAG_LOOP2_NEG_CAP|DETECTOR_FLAG_LOOP1_NEG_CAP|DETECTOR_FLAG_LOOP2_POS_CAP|DETECTOR_FLAG_LOOP1_POS_CAP:
		{
			epCapTable[0] = FRAME_EPOLICE_1ST;
			epCapTable[1] = FRAME_EPOLICE_2ND;
			epCapTable[2] = FRAME_EPOLICE_3RD;
			epCapTable[3] = FRAME_EPOLICE_4TH;
			*lastFrameId = 0x04;
			break;
		}
		default:
			break;
	}
}


static Int32 ep_cap_parse(DetectorToryEpV2 *dev, const CamDetectorParam *params, Uint8 *rxBuf, CaptureInfo *capInfo)
{	
	Int32 wayNum;
	Int32 index = capInfo->capCnt;

	if((rxBuf[3] & 0x0F) != 0x01 || rxBuf[4] > 0x04)	//Check way number
		return E_INVAL;

	wayNum = (rxBuf[3]>>4) & 0x0F;
	if(rxBuf[4] < 0x03)
		dev->redlightTime[wayNum - 1] = (rxBuf[5] << 8) + rxBuf[6];

	/* Check if capture is needed at this position */
	if(dev->epCapTable[rxBuf[4] - 1] == 0xFF)
		return E_NOTEXIST; //Should not capture at this position

	/* Set frame Id and flag */
	capInfo->triggerInfo[index].frameId = dev->epCapTable[rxBuf[4] - 1];
	capInfo->triggerInfo[index].flags = TRIG_INFO_RED_LIGHT;

	/* Way Number */
	capInfo->triggerInfo[index].wayNum = wayNum;

	/* Redlight time, unit: 10ms */
	capInfo->triggerInfo[index].redlightTime = dev->redlightTime[wayNum - 1]; 

	/* Group Num */
	if(capInfo->triggerInfo[index].frameId == FRAME_EPOLICE_1ST)
		dev->groupId[wayNum - 1]++;
	capInfo->triggerInfo[index].groupId = dev->groupId[wayNum - 1];

	/* Check if delay cap is set */
	if((params->redLightCapFlag & DETECTOR_FLAG_DELAY_CAP) &&
		rxBuf[4] == dev->lastFrameId) {
		capInfo->triggerInfo[index].flags |= TRIG_INFO_DELAY_CAP;
		capInfo->flags |= CAPINFO_FLAG_DELAY_CAP;
	}

	/* Capture count increase */
	capInfo->capCnt++;
	
	return E_NO;
}


/*****************************************************************************
 Prototype    : retrograde_cap_parse
 Description  : parse retrograde
 Input        : DetectorToryEpV2 *dev           
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
static Int32 retrograde_cap_parse(DetectorToryEpV2 *dev, const CamDetectorParam *params, Uint8 *rxBuf, CaptureInfo *capInfo)
{
	Int32 wayNum;
	Int32 index = capInfo->capCnt;

	if((rxBuf[3] & 0x0F) != 0x01 || rxBuf[4] > 0x03)	//Check way number
		return E_INVAL;

	/* Check if capture is needed at this position */
	if(dev->reCapTable[rxBuf[4]] == 0xFF)
		return E_NOTEXIST; //Should not capture at this position

	/* Set frame Id and flag */
	capInfo->triggerInfo[index].frameId = dev->epCapTable[rxBuf[4]];
	capInfo->triggerInfo[index].flags = 0;

	/* Way Number */
	wayNum = (rxBuf[3] >> 4) & 0x0F;
	capInfo->triggerInfo[index].wayNum = wayNum;

	/* Set retrograde flag */
	capInfo->triggerInfo[index].flags |= TRIG_INFO_RETROGRADE;
	
	/* Group Num */
	if(capInfo->triggerInfo[index].frameId == FRAME_RETROGRADE_1ST)
		dev->groupId[wayNum - 1]++;
	capInfo->triggerInfo[index].groupId = dev->groupId[wayNum - 1];

	/* Capture count increase */
	capInfo->capCnt++;
	
	return E_NO;
}

/*****************************************************************************
 Prototype    : cp_cap_parse
 Description  : parse for checkpost data
 Input        : DetectorToryEpV2 *dev           
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
static Int32 cp_cap_parse(DetectorToryEpV2 *dev, const CamDetectorParam *params, Uint8 *rxBuf, CaptureInfo *capInfo)
{
	Int32 wayNum;
	Int32 index = capInfo->capCnt;

	/* Check if capture is needed at this position */
	if(!params->greenLightCapFlag)
		return E_NOTEXIST; //Should not capture at this position

	/* Validate data */
	if((rxBuf[3] & 0x0F) != 0x01 || !rxBuf[4])	
		return E_INVAL;
	
	/* Set frame Id and flag */
	capInfo->triggerInfo[index].frameId = FRAME_EPOLICE_CP;
	capInfo->triggerInfo[index].flags = 0;
	capInfo->triggerInfo[index].speed = 0;

	/* Way Number */
	wayNum = (rxBuf[3] >> 4) & 0x0F;
	capInfo->triggerInfo[index].wayNum = wayNum;

	/* Group Num */
	dev->groupId[wayNum - 1]++;
	capInfo->triggerInfo[index].groupId = dev->groupId[wayNum - 1];

	/* Calc speed */
	capInfo->triggerInfo[index].speed = detector_calc_speed(params, wayNum, (rxBuf[5]<<8)|(rxBuf[6])*10);
	if(capInfo->triggerInfo[index].speed > params->limitSpeed)
		capInfo->triggerInfo[index].flags |= TRIG_INFO_OVERSPEED;

	/* Capture count increase */
	capInfo->capCnt++;
	
	return E_NO;
}

/*****************************************************************************
 Prototype    : tory_ep2_open
 Description  : open detector
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
static Int32 tory_ep2_open(DetectorHandle hDetector)
{

	Int32				status = 0;
	DetectorToryEpV2	*dev = DETECTOR_GET_PRIVATE(hDetector);
	const CamDetectorParam	*params = DETECTOR_GET_PARAMS(hDetector);

	dev =(DetectorToryEpV2 *)calloc(1, sizeof(DetectorToryEpV2));
	if(!dev) {
		ERR("memory allocation of dev failed.");
		return  E_NOMEM;
	}

	/* start detector */
	status = tory_ep_start(&dev->fd, dev->timeout);
	if(status) {
		ERR("start failed");
		goto free_buf;
	}

	/* parse data */
	ep_cap_pre_parse(params->redLightCapFlag, dev->epCapTable, &(dev->lastFrameId));
	tory_retrograde_cap_pre_parse(params->retrogradeCapFlag, dev->reCapTable);

	/* set private data as our dev info */
	DETECTOR_SET_PRIVATE(hDetector, dev);
	
	return E_NO;

free_buf:
	if(dev->fd > 0)
		close(dev->fd);
	
	if(dev)
		free(dev);
	return status;
}


/*****************************************************************************
 Prototype    : tory_ep2_close
 Description  : close detector
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
static Int32 tory_ep2_close(DetectorHandle hDetector)
{
	DetectorToryEpV2	*dev = DETECTOR_GET_PRIVATE(hDetector);
	//const CamDetectorParam	*params = DETECTOR_GET_PARAMS(hDetector);

	if(!dev || !hDetector)
		return E_INVAL;

	/* Close uart chan */
	close(dev->fd);
	
	free(dev);

	/* set private data as our dev info */
	DETECTOR_SET_PRIVATE(hDetector, NULL);
	
	return E_NO;
}

/*****************************************************************************
 Prototype    : recv_start_code
 Description  : recv start code for sync
 Input        : Int32 fd    
                Uint8 *buf  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/5
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline Int32 recv_start_code(Int32 fd, Uint8 *buf)
{
	Int32 status;

	while(1) {
		status = read(fd, buf, 1);
		if(status) {
			#ifdef TORY_EP2_DEBUG
			ERR("recv start code timeout.");		
			#endif
			status = E_TIMEOUT;
			break;
		}

		if(buf[0] == 0xFA || buf[0] == 0xFB || buf[0] == 0xFC || buf[0] == 0xFD) {
			status = E_NO;
			break;
		}
	}

	return status;
}

/*****************************************************************************
 Prototype    : parse_trigger_data
 Description  : parse trigger data
 Input        : DetectorToryEpV2 *dev           
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
static inline Int32 parse_trigger_data( DetectorToryEpV2 *dev,
										  	const CamDetectorParam *params, 
										 	Uint8 *buf, 
										 	CaptureInfo *capInfo)
{
	Int32 status = E_NO;
	
	switch(buf[0]) {
	case 0xFA:
		/* Epolice mode */
		status = ep_cap_parse(dev, params, buf, capInfo);
		break;
	case 0xFB:
		/* Check post */
		status = cp_cap_parse(dev, params, buf, capInfo);
		break;
	case 0xFC:
		/* Retrograde */	
		status = retrograde_cap_parse(dev, params, buf, capInfo);
		break;
	default:
		status = E_INVAL;
		break;
	}

	return status;
}


static Int32 tory_ep2_detect(DetectorHandle hDetector, CaptureInfo *capInfo)
{
	Uint8	rxBuf[RX_BUF_SIZE];
	Int32	status = E_NO;
	Int32 	i;
	
	DetectorToryEpV2	*dev = DETECTOR_GET_PRIVATE(hDetector);
	const CamDetectorParam	*params = DETECTOR_GET_PARAMS(hDetector);

	if(!params || !capInfo || !dev)
		return E_INVAL;

	/* Clear to 0 */
	capInfo->capCnt = 0;
	
	/* Recieve and parse trigger data  */
	for(i = 0; i < APP_MAX_CAP_CNT; i++)  {	
		/* Check if there is trigger data sent */
		status = recv_start_code(dev->fd, rxBuf);
		if(status) {
			if(capInfo->capCnt) //We have received some data
				status = E_NO;
			else
				status = E_NOTEXIST;
			break;
		}
		
		status = read(dev->fd, rxBuf + 1, 6);
		if(status) {	
			#ifdef TORY_EP2_DEBUG
			ERR("recv trigger code timeout.");	
			#endif
			status = E_TIMEOUT;
			break;
		}

		/* Parse trigger data */
		parse_trigger_data(dev, params, rxBuf, capInfo);
	}

	return status;
}

/*****************************************************************************
 Prototype    : tory_ep_update
 Description  : update params
 Input        : DetectorToryEp *dev             
                const CamDetectorParam *params  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/30
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline Int32 tory_ep2_update(DetectorToryEpV2 *dev, const CamDetectorParam *params)
{
	if(!params)
		return E_INVAL;

	/* Reparse Ep and retrograde table */
	ep_cap_pre_parse(params->redLightCapFlag, dev->epCapTable, &dev->lastFrameId);
	tory_retrograde_cap_pre_parse(params->retrogradeCapFlag, dev->reCapTable);

	return E_NO;
}

/*****************************************************************************
 Prototype    : tory_ep2_control
 Description  : control fxns
 Input        : DetectorHandle hDetector  
                DetectorCmd cmd           
                void *arg                 
                Int32 len                 
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/5
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 tory_ep2_control(DetectorHandle hDetector, DetectorCmd cmd, void *arg, Int32 len)
{
	Int32	 				status = E_INVAL;
	DetectorToryEpV2        *dev = DETECTOR_GET_PRIVATE(hDetector);
	const CamDetectorParam  *params = DETECTOR_GET_PARAMS(hDetector);
	
	if(!dev || !params)	
		return E_INVAL;

	switch(cmd) {
	case DETECTOR_CMD_SET_PARAMS:
		if(len == sizeof(CamDetectorParam))
			status = tory_ep2_update(dev, (CamDetectorParam *)arg);
		break;
	case DETECTOR_CMD_GET_FD:
		if(len >= sizeof(Int32)) {
			*(Int32 *)arg = dev->fd;
			status = E_NO;
		}
		break;
	case DETECTOR_CMD_SET_TIMEOUT:
		if(len == sizeof(Uint32))
			status = uart_set_timeout(dev->fd, 1, *(Uint32 *)arg/100);
		break;
	case DETECTOR_CMD_START:
		status = tory_ep_start(&dev->fd, dev->timeout);
		break;
	case DETECTOR_CMD_STOP:
		status = tory_ep_stop(dev->fd);
		break;
	default:
		status = E_UNSUPT;
		break;	
	}

	return status;
}

/* Global data for this detector functions */
const DetectorFxns TORY_EP2_FXNS = {
	.open = tory_ep2_open,
	.close = tory_ep2_close,
	.detect = tory_ep2_detect,
	.control = tory_ep2_control,
};


