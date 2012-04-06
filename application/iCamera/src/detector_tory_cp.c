#include "detector.h"
#include "uart.h"
#include "log.h"


//#define TORY_CP_DEBUG
#define DETECTOR_ID_TORY_CP		0x0F
#define RX_BUF_SIZE				4


/* Private info of this detector */
typedef struct
{
	int		fd;
	Uint16	groupId[APP_MAX_CAP_CNT];
	Uint8	timeout;
}DetectorToryCp;

/*****************************************************************************
 Prototype    : tory_cp_open
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
static Int32 tory_cp_open(DetectorHandle hDetector)
{
	Int32				status = 0;
	DetectorToryCp		*dev;

	if(!hDetector)
		return E_INVAL;

	dev = (DetectorToryCp *)calloc(1, sizeof(DetectorToryCp));
	if(!dev) {
		ERR("memory alloc for DetectorToryCp failed.");
		return  E_NOMEM;
	}

	//Initialize channel attributes
	dev->fd = uart_open(UART_RS485, UART_B9600, UART_D8, UART_S1, UART_POFF);
	if(dev->fd < 0) {
		ERR("open UART failed");
		status = E_IO;
		goto free_buf;
	}

	/* set timeout for 1 byte recv */
	dev->timeout = 1; //default timeout: 100ms
	uart_set_timeout(dev->fd, 1, dev->timeout);

	/* set private data as our dev info */
	DETECTOR_SET_PRIVATE(hDetector, dev);	

	return E_NO;

free_buf:
	if(dev)
		free(dev);
	return status;
}

/*****************************************************************************
 Prototype    : tory_cp_close
 Description  : close device
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
static Int32 tory_cp_close(DetectorHandle hDetector)
{
	DetectorToryCp	*dev = DETECTOR_GET_PRIVATE(hDetector);
	
	if(!hDetector || !dev)
		return E_INVAL;

	if(dev->fd > 0)
		close(dev->fd);
	
	free(dev);

	/* set private data as our dev info */
	DETECTOR_SET_PRIVATE(hDetector, NULL);
	
	return E_NO;
}


/*****************************************************************************
 Prototype    : recv_start_code
 Description  : recv start code
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
		/* recv timeout */
		if(status <= 0) {
			#ifdef TORY_CP_DEBUG
			ERR("RecieveStartCode, recv start code timeout.");		
			#endif
			status = E_TIMEOUT;
			break;
		}

		/* sync start code */
		if(buf[0] <= DETECTOR_ID_TORY_CP) {
			status = E_NO;
		}
	}

	return status;
}

/*****************************************************************************
 Prototype    : retrograde_cap_parse
 Description  : retrograde capture parse
 Input        : DetectorToryCp *dev       
                Uint8 *rxBuf              
                CamDetectorParam *params  
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
static Int32 retrograde_cap_parse(DetectorToryCp *dev, Uint8 *rxBuf, const CamDetectorParam *params, CaptureInfo *capInfo)
{
	Int32 wayNum;
	Int32 index = capInfo->capCnt;

	/* Check if capture is needed at this position */
	if(!params->retrogradeCapFlag)
		return E_NOTEXIST; //Should not capture at this position

	/* Set frame Id and flag */
	capInfo->triggerInfo[index].frameId = FRAME_RETROGRADE_1ST;
	capInfo->triggerInfo[index].flags = 0;

	/* Way Number */
	wayNum = (rxBuf[1] & 0x0F) + 1;
	capInfo->triggerInfo[index].wayNum = wayNum;

	/* Set retrograde flag */
	capInfo->triggerInfo[index].flags |= TRIG_INFO_RETROGRADE;
	
	/* Group Num */
	dev->groupId[wayNum - 1]++;
	capInfo->triggerInfo[index].groupId = dev->groupId[wayNum - 1];

	/* Capture count increase */
	capInfo->capCnt++;
	
	return E_NO;
}


/*****************************************************************************
 Prototype    : cp_cap_parse
 Description  : parse for checkpost
 Input        : DetectorToryCp *dev       
                Uint8 *rxBuf              
                CamDetectorParam *params  
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
static Int32 cp_cap_parse(DetectorToryCp *dev, Uint8 *rxBuf, const CamDetectorParam *params, CaptureInfo *capInfo)
{
	Int32 	wayNum;
	Int32 	index = capInfo->capCnt;
	Uint16	speed;

	/* Check if capture is needed at this position */
	if(!params->greenLightCapFlag)
		return E_NOTEXIST; //Should not capture at this position
	
	/* Set frame Id and flag */
	capInfo->triggerInfo[index].frameId = FRAME_CHECKPOST_1ST;
	capInfo->triggerInfo[index].flags = 0;
	capInfo->triggerInfo[index].speed = 0;

	/* Way Number */
	wayNum = (rxBuf[1] & 0x0F) + 1;
	capInfo->triggerInfo[index].wayNum = wayNum;

	/* Group Num */
	dev->groupId[wayNum - 1]++;
	capInfo->triggerInfo[index].groupId = dev->groupId[wayNum - 1];

	/* Calc speed */
	speed = rxBuf[2];
	if(params->speedModifyRatio[wayNum - 1]) //Modify speed
		speed = speed * (Uint16)(params->speedModifyRatio[wayNum-1]) / 100;
	capInfo->triggerInfo[index].speed = (Uint8)speed;
	
	if(capInfo->triggerInfo[index].speed > params->limitSpeed) {
		/* Set overspeed flag */
		capInfo->triggerInfo[index].flags |= TRIG_INFO_OVERSPEED;
		capInfo->flags |= CAPINFO_FLAG_OVERSPEED; 
	}
	else if(!(params->greenLightCapFlag & DETECTOR_FLAG_OVERSPEED_CAP))
		return E_NOTEXIST;	//Capture only when overspeed

	/* Capture count increase */
	capInfo->capCnt++;
	
	return E_NO;
}


/*****************************************************************************
 Prototype    : parse_trigger_data
 Description  : parse trigger data
 Input        : DetectorToryCp *dev       
                Uint8 *buf                
                CamDetectorParam *params  
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
static inline Int32 parse_trigger_data( DetectorToryCp *dev, 
										   Uint8 *buf, 
										   const CamDetectorParam *params, 
										   CaptureInfo *capInfo )
{
	if((buf[1] & 0xF0) == 0x20) {
		/* Check post */
		return cp_cap_parse(dev, buf, params, capInfo);
	}

	if((buf[1] & 0xF0) == 0x30 && buf[2] == 0x41) {
		/* Retrograde mode */
		return retrograde_cap_parse(dev, buf, params, capInfo);
	}

	/* invalid data */
	return E_INVAL;
	
}


/*****************************************************************************
 Prototype    : tory_cp_detect
 Description  : detect trigger data
 Input        : DetectorHandle hDetector  
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
static Int32 tory_cp_detect(DetectorHandle hDetector, CaptureInfo *capInfo)
{
	Uint8			rxBuf[RX_BUF_SIZE];
	Int32			status;
	Int32 			i;
	DetectorToryCp  *dev = DETECTOR_GET_PRIVATE(hDetector);
	const CamDetectorParam *params = DETECTOR_GET_PARAMS(hDetector);

	if(!hDetector || !capInfo || !params)
		return E_INVAL;

	/* Clear to 0 */
	capInfo->capCnt = 0;
	
	/* Recieve and parse trigger data */
	for(i = 0; i < APP_MAX_CAP_CNT; i++)  {	
		/* Check if there is trigger data sent */
		status = recv_start_code(dev->fd, rxBuf);
		if(status) {
			if(capInfo->capCnt) //We have received some data
				status = E_NO;
			break;
		}
		
		status = read(dev->fd, rxBuf + 1, 2);
		if(status != 2) {	
			#ifdef TORY_CP_DEBUG
			DBG("tory cp, recv trigger code timeout.");	
			#endif
			status = E_TIMEOUT;
			break;
		}

		/* Parse trigger data */
		parse_trigger_data(dev, rxBuf, params, capInfo);		
	}

	return status;
}

/*****************************************************************************
 Prototype    : tory_cp_control
 Description  : control detector
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
static Int32 tory_cp_control(DetectorHandle hDetector, DetectorCmd cmd, void *arg, Int32 len)
{
	Int32	 				status = E_NO;
	DetectorToryCp          *dev = DETECTOR_GET_PRIVATE(hDetector);
	const CamDetectorParam  *params = DETECTOR_GET_PARAMS(hDetector);
	
	if(!dev || !params)	
		return E_INVAL;

	switch(cmd) {
	case DETECTOR_CMD_SET_PARAMS:
		/* nothing to do */
		break;
	case DETECTOR_CMD_GET_FD:
		if(len >= sizeof(Int32)) {
			*(Int32 *)arg = dev->fd;
		} else 
			status = E_INVAL;
		break;
	case DETECTOR_CMD_SET_TIMEOUT:
		if(len == sizeof(Uint32)) {
			dev->timeout = *(Uint32 *)arg/100;
			status = uart_set_timeout(dev->fd, 1, dev->timeout);
		}
		break;
	case DETECTOR_CMD_START:
	case DETECTOR_CMD_STOP:
		/* just clear buffer */
		UART_FLUSH(dev->fd);
		break;
	default:
		status = E_UNSUPT;
		break;	
	}

	return status;
}


/* Global data for functions of this detector */
DetectorFxns TORY_CP_FXNS = {
	.open = tory_cp_open,
	.close = tory_cp_close,
	.detect = tory_cp_detect,
	.control = tory_cp_control,
};


