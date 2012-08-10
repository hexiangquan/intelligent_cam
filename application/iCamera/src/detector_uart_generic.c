/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : detector_uart_generic.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/4/6
  Last Modified :
  Description   : generic uart detector class
  Function List :
              detector_uart_close
              detector_uart_control
              detector_uart_open
              detector_uart_run
              detector_uart_start
              recv_start_code
  History       :
  1.Date        : 2012/4/6
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

/* probe fxns for each uart detector, must be implemented */
extern const DetectorUartFxns tory_ep2_opts;
extern const DetectorUartFxns tory_ep_opts;
extern const DetectorUartFxns tory_cp_opts;

/* probe fxns list */
static const DetectorUartFxns *uartDetectorFxns[] = {
	&tory_ep_opts,
	&tory_cp_opts,
	&tory_ep2_opts,
	NULL,
};

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

#define RX_BUF_SIZE				(32)
	
#define detector_uart_stop(dev)	\
		do { \
			if(dev->fd > 0) UART_FLUSH(dev->fd); \
		} while(0)

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/


/*****************************************************************************
 Prototype    : detector_uart_start
 Description  : start uart 
 Input        : DetectorUart *dev  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/6
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 detector_uart_start(DetectorUart *dev) 
{
	Int32 ret = E_NO;

	if(dev->fd <= 0) {
		/* open uart */
		ret = uart_open(dev->devName, dev->baudRate, dev->dataBits, dev->stopBits, dev->parity);
		if(ret < 0) {
			ERR("open UART failed");
			return ret;
		}

		/* record fd */
		dev->fd = ret;

		/* set timeout for 1 byte recv */
		ret = uart_set_timeout(dev->fd, dev->packetLen, dev->timeout);

		/* set transition delay time */
		if(!ret)
			ret = uart_set_trans_delay(dev->chanId, dev->baudRate);
	} else 
		UART_FLUSH(dev->fd);

	return ret;
}


/*****************************************************************************
 Prototype    : detector_uart_open
 Description  : open uart device 
 Input        : DetectorHandle hDetector  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/6
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 detector_uart_open(DetectorHandle hDetector)
{

	Int32						i, status = E_UNSUPT;
	DetectorUart				*dev;
	const CamDetectorParam	    *params = DETECTOR_GET_PARAMS(hDetector);

	dev = calloc(1, sizeof(DetectorUart));
	if(!dev) {
		ERR("memory allocation of dev failed.");
		return  E_NOMEM;
	}

	/* set private data as our dev info */
	DETECTOR_SET_PRIVATE(hDetector, dev);
	memset(dev->lastFrameCode, FRAME_TRIG_BASE, sizeof(dev->lastFrameCode));

	/* probe and init detector */
	i = 0;
	while(uartDetectorFxns[i] != NULL) {
		if(uartDetectorFxns[i]->init) {
			status = uartDetectorFxns[i]->init(dev, params->detecotorId);
			if(!status) {
				dev->opts = uartDetectorFxns[i];
				break;
			}
		}
		i++;
	}
	
	if(status) {
		ERR("init detector <%d> failed, %s", params->detecotorId, str_err(status));
		goto free_buf;
	}

	if(!dev->opts || !dev->opts->singleTrigParse) {
		ERR("opts should not be NULL");
		status = E_IO;
		goto free_buf;
	}

	/* start detector */
	dev->timeout = 1;	//set to default value: 100ms
	status = detector_uart_start(dev);
	if(status) {
		ERR("start failed");
		goto free_buf;
	}

	/* check packet len */
	if(dev->packetLen < 1 || dev->packetLen > RX_BUF_SIZE) {
		ERR("packet len invalid");
		status = E_NOSPC;
		goto free_buf;
	}

	/* pre parse params */
	if(dev->opts->capPreParse)
		dev->opts->capPreParse(dev, params);

	/* set dev fd */
	hDetector->fd = dev->fd;
	
	return E_NO;

free_buf:
	if(dev->fd > 0)
		close(dev->fd);
	
	if(dev)
		free(dev);

	DETECTOR_SET_PRIVATE(hDetector, NULL);
	return status;
}

/*****************************************************************************
 Prototype    : detector_uart_close
 Description  : close uart device
 Input        : DetectorHandle hDetector  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/6
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 detector_uart_close(DetectorHandle hDetector)
{
	DetectorUart	*dev = DETECTOR_GET_PRIVATE(hDetector);

	if(!dev || !hDetector)
		return E_INVAL;

	/* Close uart chan */
	close(dev->fd);
	
	free(dev);

	/* set private data as our dev info */
	DETECTOR_SET_PRIVATE(hDetector, NULL);

	/* set to invalid fd */
	hDetector->fd = -1;
	
	return E_NO;
}

/*****************************************************************************
 Prototype    : recv_start_code
 Description  : recv start code for sync
 Input        : DetectorUart *dev  
                Uint8 *buf         
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/6
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 recv_packet(DetectorUart *dev, Uint8 *buf, Int32 *offset, size_t bufSize)
{
	Int32	i;
	Int32 	len = *offset;

	if(len < dev->packetLen) {
		/* calc len recv this time add last time left */
		len += read(dev->fd, buf + *offset, bufSize);
		//DBG("len: %d, offset: %d, buflen: %u", len, *offset, bufSize);
		if(len < dev->packetLen) {
			return E_TIMEOUT;
		}
	}

	/* if no sync fxns, just return ok */
	if(!dev->opts->startCodeSync) {
		*offset = 0;
		return len;
	}

	for(i = 0; i != len; ++i) {
		/* call sync fxn for cmp */
		if(dev->opts->startCodeSync(dev, buf[i])) {
			*offset = i;
			return len;
		}
	}

	return E_CHECKSUM;
}


/*****************************************************************************
 Prototype    : detector_uart_run
 Description  : run detect
 Input        : DetectorHandle hDetector  
                CaptureInfo *capInfo      
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/6
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 detector_uart_run(DetectorHandle hDetector, CaptureInfo *capInfo)
{
	Uint8					rxBuf[RX_BUF_SIZE];
	Int32					status = E_NO;
	Int32 					i;
	DetectorUart			*dev = DETECTOR_GET_PRIVATE(hDetector);
	const CamDetectorParam	*params = DETECTOR_GET_PARAMS(hDetector);
	TriggerInfo				*trigInfo = &capInfo->triggerInfo[0];
	Int32					start = 0, offset = 0, recvLen;
	
	if(!capInfo || !dev || !params)
		return E_INVAL;

	/* Clear cap count */
	capInfo->capCnt = 0;
	
	/* Recieve and parse trigger data  */
	for(i = 0; i < APP_MAX_CAP_CNT; i++) {	
		/* Check if there is trigger data recved */
		status = recv_packet(dev, rxBuf + start, &offset, sizeof(rxBuf) - offset - start);
		if(status < 0) {
			break;
		}

		/* Parse trigger data */
		recvLen = status;
		start += offset;
		status = dev->opts->singleTrigParse(dev, params, rxBuf + start, trigInfo);
		if(!status) {
			capInfo->capCnt++;
			/* set flags for top level flags */
			if(trigInfo->flags & TRIG_INFO_DELAY_CAP)
				capInfo->flags |= CAPINFO_FLAG_DELAY_CAP;
			if(trigInfo->flags & TRIG_INFO_OVERSPEED)
				capInfo->flags |= CAPINFO_FLAG_OVERSPEED;

		#ifdef UART_DEBUG
			DBG("got trig data, way: %d, frame num: %d", trigInfo->wayNum, trigInfo->frameId);
		#endif

			trigInfo++;
		}

		/* Move to next packet */
		start += dev->packetLen;
		offset = recvLen - dev->packetLen;

		if(offset)
			usleep(5000);
	}

	/* We have received some data */
	if(capInfo->capCnt) {
		return E_NO;
	}
		
	return status;
}

/*****************************************************************************
 Prototype    : detector_uart_control
 Description  : config params
 Input        : DetectorHandle hDetector  
                DetectorCmd cmd           
                void *arg                 
                Int32 len                 
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/6
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 detector_uart_control(DetectorHandle hDetector, DetectorCmd cmd, void *arg, Int32 len)
{
	Int32	 				status = E_INVAL;
	DetectorUart            *dev = DETECTOR_GET_PRIVATE(hDetector);
	const CamDetectorParam  *params = DETECTOR_GET_PARAMS(hDetector);
	
	if(!dev || !params)	
		return E_INVAL;

	switch(cmd) {
	case DETECTOR_CMD_SET_PARAMS:
		if(len == sizeof(CamDetectorParam)) {
			if(dev->opts->capPreParse)
				dev->opts->capPreParse(dev, (CamDetectorParam *)arg);
			status = E_NO;
		}
		break;
	case DETECTOR_CMD_SET_TIMEOUT:
		if(len == sizeof(Uint32)) {
			dev->timeout = *(Uint32 *)arg/100;
			status = uart_set_timeout(dev->fd, 1, dev->timeout);	
		}
		break;
	case DETECTOR_CMD_START:
		status = detector_uart_start(dev);
		break;
	case DETECTOR_CMD_STOP:
		detector_uart_stop(dev);
		status = E_NO;
		break;
	default:
		status = E_UNSUPT;
		break;	
	}

	return status;
}

/* Global data for this detector functions */
const DetectorFxns DETECTOR_UART_FXNS = {
	.open = detector_uart_open,
	.close = detector_uart_close,
	.detect = detector_uart_run,
	.control = detector_uart_control,
};


