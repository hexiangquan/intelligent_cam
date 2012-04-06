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
#include "detector.h"
#include "uart.h"
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
#define CAP_TABLE_SIZE 			4
#define RX_BUF_SIZE				5
#define DETECTOR_ID_TORY_EP		0x0F
#define RECV_TRIG_TIMEOUT		20 //ms
#define TORY_EP_DEBUG

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/* Run time params */
typedef struct {
	Int32			fd;									//Rs485 fd
	Uint16			groupId[APP_MAX_CAP_CNT];			//GroupNum for each way
	Uint16			redlightTime[APP_MAX_CAP_CNT];		//Redlight time record for each way
	Uint8			epCapTable[CAP_TABLE_SIZE];			//Epolice trigger code to frame id table
	Uint8			retrogradeCapTable[CAP_TABLE_SIZE];	//Retrograde trigger code to frame id table
	Uint8			lastFrameId;						// Trigger code for the last frame 
	Uint8			timeout;							//recv timeout
}DetectorToryEp;


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
		epCapTable[1] = FRAME_EPOLICE_1ST;	//First frame 
		*lastFrameId = 0x01;
		break;
	/* Only cap at loop1 negative edge */  
	case DETECTOR_FLAG_LOOP1_NEG_CAP:
		epCapTable[2] = FRAME_EPOLICE_1ST;
		*lastFrameId = 0x02;
		break;
	/* Cap at loop1 pos edge and loop1 negative edge */
	case (DETECTOR_FLAG_LOOP1_NEG_CAP|DETECTOR_FLAG_LOOP1_POS_CAP):
		epCapTable[1] = FRAME_EPOLICE_1ST;
		epCapTable[2] = FRAME_EPOLICE_2ND;
		*lastFrameId = 0x02;
		break;
	/* Only cap at loop2 negative edge */ 
	case DETECTOR_FLAG_LOOP2_NEG_CAP:
		epCapTable[3] = FRAME_EPOLICE_1ST;
		*lastFrameId = 0x03;
		break;
	/* Cap at loop1 pos edge and loop2 negative edge */
	case (DETECTOR_FLAG_LOOP2_NEG_CAP|DETECTOR_FLAG_LOOP1_POS_CAP):
		epCapTable[1] = FRAME_EPOLICE_1ST;
		epCapTable[3] = FRAME_EPOLICE_2ND;
		*lastFrameId = 0x03;
		break;
	/* Cap at loop1 negative and loop2 negative edge */
	case (DETECTOR_FLAG_LOOP2_NEG_CAP|DETECTOR_FLAG_LOOP1_NEG_CAP):
		epCapTable[2] = FRAME_EPOLICE_1ST;
		epCapTable[3] = FRAME_EPOLICE_2ND;
		*lastFrameId = 0x03;
		break;
	/* Cap at loop1 pos, loop1 negative and loop2 negative edge */
	case (DETECTOR_FLAG_LOOP2_NEG_CAP|DETECTOR_FLAG_LOOP1_NEG_CAP|DETECTOR_FLAG_LOOP1_POS_CAP):
		epCapTable[1] = FRAME_EPOLICE_1ST;
		epCapTable[2] = FRAME_EPOLICE_2ND;
		epCapTable[3] = FRAME_EPOLICE_3RD;
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
		reCapTable[2] = FRAME_RETROGRADE_1ST;
		break;
	/* Only cap at loop2 pos edge */
	case DETECTOR_FLAG_LOOP2_POS_CAP:
		reCapTable[1] = FRAME_RETROGRADE_1ST;
		break;
	/* Cap at loop2 pos edge and loop1 pos edge */
	case DETECTOR_FLAG_LOOP2_POS_CAP|DETECTOR_FLAG_LOOP1_POS_CAP:
		reCapTable[1] = FRAME_RETROGRADE_1ST;
		reCapTable[2] = FRAME_RETROGRADE_2ND;
		break;
	/* Cap at loop2 neg edge */
	case DETECTOR_FLAG_LOOP2_NEG_CAP:
		reCapTable[3] = FRAME_RETROGRADE_1ST;
		break;
	/* Cap at loop2 neg edge and loop1 pos edge */
	case DETECTOR_FLAG_LOOP2_NEG_CAP|DETECTOR_FLAG_LOOP1_POS_CAP:
		reCapTable[2] = FRAME_RETROGRADE_1ST;
		reCapTable[3] = FRAME_RETROGRADE_2ND;
		break;
	/* Cap at loop2 neg edge and loop2 pos edge */
	case DETECTOR_FLAG_LOOP2_NEG_CAP|DETECTOR_FLAG_LOOP2_POS_CAP:
		reCapTable[1] = FRAME_RETROGRADE_1ST;
		reCapTable[3] = FRAME_RETROGRADE_2ND;
		break;
	/* Cap at loop2 pos edge, loop1 pos edge and loop2 neg edge */
	case DETECTOR_FLAG_LOOP2_NEG_CAP|DETECTOR_FLAG_LOOP2_POS_CAP|DETECTOR_FLAG_LOOP1_POS_CAP:
		reCapTable[1] = FRAME_RETROGRADE_1ST;
		reCapTable[2] = FRAME_RETROGRADE_2ND;
		reCapTable[3] = FRAME_RETROGRADE_3RD;
		break;
	default:
		break;
	}
}

/*****************************************************************************
 Prototype    : tory_ep_start
 Description  : start recv data
 Input        : DetectorToryEp *dev  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/30
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 tory_ep_start(Int32 *fd, Uint8 timeout) 
{
	Int32 ret = E_NO;

	if(*fd <= 0) {
		/* open uart */
		ret = uart_open(UART_RS485, UART_B9600, UART_D8, UART_S1, UART_POFF);
		if(ret < 0) {
			ERR("open UART failed");
			return ret;
		}

		/* record fd */
		*fd = ret;

		/* set timeout for 1 byte recv */
		ret = uart_set_timeout(*fd, 1, timeout);
	} else 
		UART_FLUSH(*fd);

	return ret;
}

/*****************************************************************************
 Prototype    : tory_ep_stop
 Description  : stop recv data
 Input        : DetectorToryEp *dev  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/30
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 tory_ep_stop(Int32 fd) 
{
	if(fd > 0)
		UART_FLUSH(fd);

	return E_NO;
}

/*****************************************************************************
 Prototype    : tory_ep_open
 Description  : open device
 Input        : DetectorHandle hDetector  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/30
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 tory_ep_open(DetectorHandle hDetector)
{
	Int32				ret = 0;
	DetectorToryEp 		*dev;

	assert(hDetector);

	dev = calloc(1, sizeof(DetectorToryEp));
	if(!dev) {
		ERR("memory allocation failed.");
		return  E_NOMEM;
	}

	dev->timeout = 1;

	/* open uart */
	ret = tory_ep_start(&dev->fd, dev->timeout);
	if(ret) {
		ERR("start failed");
		goto free_buf;
	}
	
	/* Pre parse capture flags */
	const CamDetectorParam *params = DETECTOR_GET_PARAMS(hDetector);
	
	tory_ep_cap_pre_parse(params->redLightCapFlag, dev->epCapTable, &dev->lastFrameId);
	tory_retrograde_cap_pre_parse(params->retrogradeCapFlag, dev->retrogradeCapTable);

	/* set private data as our dev info */
	DETECTOR_SET_PRIVATE(hDetector, dev);

	return E_NO;

free_buf:

	if(dev->fd > 0)
		close(dev->fd);
	
	if(dev)
		free(dev);
	return ret;
}

/*****************************************************************************
 Prototype    : tory_ep_close
 Description  : close device
 Input        : DetectorHandle hDetector  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/30
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 tory_ep_close(DetectorHandle hDetector)
{
	DetectorToryEp 	*dev = DETECTOR_GET_PRIVATE(hDetector);
	
	if(!hDetector || !dev)
		return E_INVAL;

	if(dev->fd > 0)
		close(dev->fd);

	free(dev);

	/* set private data as NULL */
	DETECTOR_SET_PRIVATE(hDetector, NULL);
	
	return E_NO;
}

/*****************************************************************************
 Prototype    : recv_start_code
 Description  : Receive start code accodring to protocol
 Input        : GIO_Handle hUart    
                Uint8 *pBuf         
                Uint32 unTimeoutMs  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2011/8/9
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline Int32 recv_start_code(Int32 fd, Uint8 *buf)
{
	Int32 status;

	while(1) {
		status = read(fd, buf, 1);
		if(status != 1) {
			#ifdef TORY_EP2_DEBUG
			ERR("recv start code timeout.");		
			#endif
			return E_TIMEOUT;
		}

		if(buf[0] == DETECTOR_ID_TORY_EP)
			return E_NO;
	}
}

/*****************************************************************************
 Prototype    : ep_cap_parse
 Description  : parse ep data
 Input        : DetectorToryEp *dev       
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
static Int32 ep_cap_parse(DetectorToryEp *dev, const CamDetectorParam *params, Uint8 *rxBuf, CaptureInfo *capInfo)
{	
	Int32 wayNum;
	Int32 index = capInfo->capCnt;

	wayNum = (rxBuf[1]>>4) & 0x0F;

	if(wayNum > APP_MAX_CAP_CNT || rxBuf[2] >= CAP_TABLE_SIZE)
		return E_INVAL;

	/* Record redlight time, unit: 10ms */
	if(rxBuf[2] < 0x03)
		dev->redlightTime[wayNum - 1] = ((rxBuf[3] << 8) + rxBuf[4])/10;

	/* Check if capture is needed at this position */
	if(dev->epCapTable[rxBuf[2]] == 0xFF)
		return E_NOTEXIST; //Should not capture at this position

	/* Set frame Id and flag */
	capInfo->triggerInfo[index].frameId = dev->epCapTable[rxBuf[2]];
	capInfo->triggerInfo[index].flags = 0;

	/* Way Number */
	capInfo->triggerInfo[index].wayNum = wayNum;

	/* Redlight time, unit: 10ms */
	capInfo->triggerInfo[index].redlightTime =
		dev->redlightTime[wayNum - 1]; 
	
	/* Set redlight flag */
	capInfo->triggerInfo[index].flags |= TRIG_INFO_RED_LIGHT;

	/* Group Num */
	if(capInfo->triggerInfo[index].frameId == FRAME_EPOLICE_1ST)
		dev->groupId[wayNum - 1]++;
	capInfo->triggerInfo[index].groupId= dev->groupId[wayNum - 1];

	/* Check if delay cap is set */
	if((params->redLightCapFlag & DETECTOR_FLAG_DELAY_CAP) &&
		rxBuf[2] == dev->lastFrameId) {
		capInfo->triggerInfo[index].flags |= TRIG_INFO_DELAY_CAP;
		capInfo->flags |= CAPINFO_FLAG_DELAY_CAP;
	}

	/* Capture count increase */
	capInfo->capCnt++;
	
	return E_NO;
}


/*****************************************************************************
 Prototype    : retrograde_cap_parse
 Description  : parse retrograde data
 Input        : DetectorToryEp *dev       
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
static Int32 retrograde_cap_parse(DetectorToryEp *dev, const CamDetectorParam *params, Uint8 *rxBuf, CaptureInfo *capInfo)
{
	Int32 wayNum;
	Int32 index = capInfo->capCnt;

	/* Way Number */
	wayNum = (rxBuf[1] >> 4) & 0x0F;

	if(wayNum > APP_MAX_CAP_CNT || rxBuf[2] >= CAP_TABLE_SIZE)
		return E_INVAL;

	/* Check if capture is needed at this position */
	if(dev->retrogradeCapTable[rxBuf[2]] == 0xFF)
		return E_NOTEXIST; //Should not capture at this position

	/* Set frame Id and flag */
	capInfo->triggerInfo[index].frameId = dev->retrogradeCapTable[rxBuf[2]];
	capInfo->triggerInfo[index].flags = 0;

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
 Description  : parse data for green light
 Input        : DetectorToryEp *dev       
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
static Int32 cp_cap_parse(DetectorToryEp *dev, const CamDetectorParam *params, Uint8 *rxBuf, CaptureInfo *capInfo)
{
	Int32 wayNum;
	Int32 index = capInfo->capCnt;

	/* Check if capture is needed at this position */
	if(!(params->greenLightCapFlag & DETECTOR_FLAG_LOOP1_NEG_CAP))
		return E_NOTEXIST; //Should not capture at this position

	/* Way Number */
	wayNum = (rxBuf[1] >> 4) & 0x0F;

	if(wayNum > APP_MAX_CAP_CNT)
		return E_INVAL;
	
	/* Set frame Id and flag */
	capInfo->triggerInfo[index].frameId = FRAME_EPOLICE_CP;
	capInfo->triggerInfo[index].flags = 0;

	/* Way Number */
	capInfo->triggerInfo[index].wayNum = wayNum;

	/* Group Num */
	dev->groupId[wayNum - 1]++;
	capInfo->triggerInfo[index].groupId = dev->groupId[wayNum - 1];

	/* Calc speed */
	capInfo->triggerInfo[index].speed = detector_calc_speed(params, wayNum, (rxBuf[5]<<8)|rxBuf[6]);
	if(capInfo->triggerInfo[index].speed > params->limitSpeed)
		capInfo->triggerInfo[index].flags |= TRIG_INFO_OVERSPEED;

	/* Capture count increase */
	capInfo->capCnt++;

	return E_NO;
}

/*****************************************************************************
 Prototype    : parse_trigger_data
 Description  : parse trigger data
 Input        : DetectorToryEp *dev             
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
static inline Int32 parse_trigger_data(DetectorToryEp *dev, const CamDetectorParam *params, Uint8 *rxBuf, CaptureInfo *capInfo)
{
	/* Validate data */
	if((rxBuf[1] & 0x0F) != 0x01 || rxBuf[2] > 0x03)
		return E_INVAL;

	if(!rxBuf[2]) {
		/* Check post */
		return cp_cap_parse(dev, params, rxBuf, capInfo);
	}

	if(rxBuf[3] || rxBuf[4]) {
		/* Epolice mode */
		return ep_cap_parse(dev, params, rxBuf, capInfo);
	}

	/* Retrograde */
	return retrograde_cap_parse(dev, params, rxBuf, capInfo);
	
}

/*****************************************************************************
 Prototype    : DetectorToryEp_wait_trig
 Description  : Wait trigger signal
 Input        : VehicleDetectorParams *pDetectorParams  
                CaptureInfo *capInfo                   
                Uint32 unTimeout                        
                void *pRunTimeParams                    
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2011/8/15
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 tory_ep_detect(DetectorHandle hDetector, CaptureInfo *capInfo)
{
	Uint8	rxBuf[RX_BUF_SIZE];
	Int32	status = 0;
	Int32 	i;
	DetectorToryEp   *dev = DETECTOR_GET_PRIVATE(hDetector);
	const CamDetectorParam *params = DETECTOR_GET_PARAMS(hDetector);

	if(!params || !capInfo || !dev)
		return E_INVAL;

	/* check if we have already start */
	if(dev->fd <= 0)
		tory_ep_start(&dev->fd, dev->timeout);

	/* Clear to 0 */
	capInfo->capCnt = 0;
	
	/* Recieve and parse trigger data */
	for(i = 0; i < APP_MAX_CAP_CNT; i++) {	
		/* Check if there is trigger data sent */
		status = recv_start_code(dev->fd, rxBuf);
		if(status) {
			if(capInfo->capCnt) //We have received some data
				status = E_NO;
			else
				status = E_NOTEXIST;
			break;
		}
		
		status = read(dev->fd, rxBuf + 1, 4);
		if(status != 4) {	
			#ifdef TORY_EP_DEBUG
			ERR("recv trigger code timeout.");	
			#endif
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
static inline Int32 tory_ep_update(DetectorToryEp *dev, const CamDetectorParam *params)
{
	if(!params)
		return E_INVAL;

	/* Reparse Ep and retrograde table */
	tory_ep_cap_pre_parse(params->redLightCapFlag, dev->epCapTable, &dev->lastFrameId);
	tory_retrograde_cap_pre_parse(params->retrogradeCapFlag, dev->retrogradeCapTable);

	return E_NO;
}

/*****************************************************************************
 Prototype    : tory_ep_control
 Description  : process cmd 
 Input        : DetectorHandle hDetector  
                DetectorCmd cmd           
                void *arg                 
                Int32 len                 
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/30
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 tory_ep_control(DetectorHandle hDetector, DetectorCmd cmd, void *arg, Int32 len)
{
	Int32	 				status = E_INVAL;
	DetectorToryEp          *dev = DETECTOR_GET_PRIVATE(hDetector);
	const CamDetectorParam  *params = DETECTOR_GET_PARAMS(hDetector);
	
	if(!dev || !params)	
		return E_INVAL;

	switch(cmd) {
	case DETECTOR_CMD_SET_PARAMS:
		if(len == sizeof(CamDetectorParam))
			status = tory_ep_update(dev, (CamDetectorParam *)arg);
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
const DetectorFxns TORY_EP_FXNS = {
	.open = tory_ep_open,
	.close = tory_ep_close,
	.detect = tory_ep_detect,
	.control = tory_ep_control,
};


