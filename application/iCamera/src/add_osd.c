/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : add_osd.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/8
  Last Modified :
  Description   : add osd for application
  Function List :
              add_osd
  History       :
  1.Date        : 2012/3/8
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "add_osd.h"
#include "cam_time.h"
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

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/


/*****************************************************************************
 Prototype    : add_osd
 Description  : Add osd for one frame
 Input        : OsdHandle hOsd       
                ImgMsg *imgMsg       
                CamOsdInfo *osdInfo  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/8
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 add_osd(OsdHandle hOsd, ImgMsg *imgMsg, CamOsdInfo *osdInfo)
{
	Int32 		err;
	Int8   		buf[128];
	AlgBuf		inBuf;
	OsdInArgs	inArgs;
	
	assert(hOsd && imgMsg && osdInfo && imgMsg->hBuf);

	if(!(osdInfo->flags & CAM_OSD_FLAG_EN))
		return E_NO;

	/* fill inbuf */
	inBuf.buf = buffer_get_user_addr(imgMsg->hBuf);
	inBuf.bufSize = buffer_get_size(imgMsg->hBuf);
	inArgs.size = sizeof(inArgs);
	inArgs.startX = 10;
	inArgs.startY = 10;
	inArgs.strOsd = buf;

	/* determine height of one line */	
	Uint16 	yStep = (osdInfo->size == CAM_OSD_SIZE_32x16) ? 20 : 36;
	
#ifdef DISP_CNT_NUM
	sprintf(buf, "The %d th picture ", imgMsg->index);
	err = osd_process(hOsd, &inBuf, &inArgs, NULL, NULL);
	if(err)
		return err;
	inArgs.startY += yStep;
#endif
	/* Add Text info */
	if(osdInfo->flags & CAM_OSD_FLAG_TEXT_EN) {
		inArgs.strOsd = osdInfo->osdString;
		err = osd_process(hOsd, &inBuf, &inArgs, NULL, NULL);
		if(err)
			return err;
		inArgs.startY += yStep;
		inArgs.strOsd = buf;
	}

	/* Add date & time */
	if(osdInfo->flags & CAM_OSD_FLAG_TIME_EN) {
		CamDateTime *dateTime = &imgMsg->timeStamp;
		if(osdInfo->flags & CAM_OSD_FLAG_MILISEC_EN) {
			sprintf( buf, "%04d.%02d.%02d %02d:%02d:%02d.%03d", 
				 dateTime->year, dateTime->month, dateTime->day, dateTime->hour, 
				 dateTime->minute, dateTime->second, dateTime->ms);
		} else {
			sprintf( buf, "%04d.%02d.%02d %02d:%02d:%02d", 
				 dateTime->year, dateTime->month, dateTime->day,
				 dateTime->hour, dateTime->minute, dateTime->second);
		}
		err = osd_process(hOsd, &inBuf, &inArgs, NULL, NULL);
		if(err)
			return err;
		inArgs.startY += yStep;
	}

	/* Add lum info */
	if(osdInfo->flags & CAM_OSD_FLAG_LUM_INFO_EN) {
		sprintf( buf, "曝光时间: %u us, 全局增益: %u, 平均亮度: %u", 
			 imgMsg->rawInfo.exposure, imgMsg->rawInfo.globalGain,
			 imgMsg->rawInfo.avgLum);
		err = osd_process(hOsd, &inBuf, &inArgs, NULL, NULL);
		if(err)
			return err;
		inArgs.startY += yStep;
	}

	/* Add RGB gains */
	if(osdInfo->flags & CAM_OSD_FLAG_RGB_GAIN_EN) {
		sprintf( buf, "RGB增益: %u, %u, %u", 
			 imgMsg->rawInfo.redGain, imgMsg->rawInfo.greenGain, 
			 imgMsg->rawInfo.blueGain);
		err = osd_process(hOsd, &inBuf, &inArgs, NULL, NULL);
		if(err)
			return err;
		inArgs.startY += yStep;
	}

	CaptureInfo	*capInfo = &imgMsg->capInfo;
	Int32 		offset, i;

	if(capInfo->capCnt > APP_MAX_CAP_CNT)
		return E_INVAL;

	/* print capture info */
	for(i = 0; i < capInfo->capCnt; i++) {
		offset = 0;
		buf[0] = 0x00;

		/* print way num */
		if(osdInfo->flags & CAM_OSD_FLAG_WAY_NUM_EN)
			offset += sprintf( buf + offset, "车道号: %u; ", 
						   capInfo->triggerInfo[i].wayNum );
		/* print group id */
		if(osdInfo->flags & CAM_OSD_FLAG_GROUP_ID_EN)
			offset += sprintf( buf + offset, "组号: %u; ", 
							   capInfo->triggerInfo[i].groupId );
		/* print frame num  */
		if(osdInfo->flags & CAM_OSD_FLAG_FRAME_NUM_EN)
			offset += sprintf( buf + offset, "帧号: %u; ", 
							   capInfo->triggerInfo[i].frameId);
		/* print retrograde info  */			
		if( osdInfo->flags & CAM_OSD_FLAG_RETROGRADE_EN &&
			(capInfo->triggerInfo[i].flags & TRIG_INFO_RETROGRADE) ) {
			offset += sprintf( buf + offset, "逆行; ", 
							   capInfo->triggerInfo[i].wayNum ); 
		} else if((capInfo->triggerInfo[i].flags & TRIG_INFO_RED_LIGHT) && 
				  (osdInfo->flags & CAM_OSD_FLAG_RED_LIGHT_TIME_EN)) {
			/* print red light time */
			offset += sprintf( buf + offset, "闯红灯, 红灯时间: %us; ", 
							   capInfo->triggerInfo[i].redlightTime/100);
		} else if(!(capInfo->triggerInfo[i].flags & TRIG_INFO_RED_LIGHT) && 
				  !(capInfo->triggerInfo[i].flags & CAM_OSD_FLAG_RETROGRADE_EN)) {
			/* print speed */
			if(osdInfo->flags & CAM_OSD_FLAG_SPEED_EN) {
				offset += sprintf(buf + offset, "卡口, 速度: %uKM/H; ", 
								  capInfo->triggerInfo[i].speed);
			}
			if( (capInfo->triggerInfo[i].flags & TRIG_INFO_OVERSPEED) && 
				(osdInfo->flags & CAM_OSD_FLAG_OVERSPEED_INFO_EN) && 
				capInfo->limitSpeed ) {
				/* print over speed info */
				Uint32 overSpeedRatio = capInfo->triggerInfo[i].speed - capInfo->limitSpeed;
				overSpeedRatio = overSpeedRatio * 100 / capInfo->limitSpeed;
				offset += sprintf(buf + offset, "限速: %uKM/H, 超速比率: %u%%", 
								  capInfo->limitSpeed, overSpeedRatio);
			}
		}

		/* Check if something has been printed in the buffer */
		if(buf[0]) {
			err = osd_process(hOsd, &inBuf, &inArgs, NULL, NULL);
			if(err)
				break;
			inArgs.startY += yStep;
		}
	}			
		
	return err;
}

