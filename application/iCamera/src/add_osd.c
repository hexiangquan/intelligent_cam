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
	Uint16 		yStep = 40;
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
		
		sprintf( buf, "%04d.%02d.%02d %02d:%02d:%02d", 
				 dateTime->year, dateTime->month, dateTime->day,
				 dateTime->hour, dateTime->minute, dateTime->second);
		err = osd_process(hOsd, &inBuf, &inArgs, NULL, NULL);
		if(err)
			return err;
		inArgs.startY += yStep;
	}

#if 1
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
			offset += sprintf( buf + offset, "Way Num: %u. ", 
						   capInfo->triggerInfo[i].wayNum );
		/* print group id */
		if(osdInfo->flags & CAM_OSD_FLAG_GROUP_ID_EN)
			offset += sprintf( buf + offset, "Group Id: %u. ", 
							   capInfo->triggerInfo[i].groupId );
		/* print frame num  */
		if(osdInfo->flags & CAM_OSD_FLAG_FRAME_NUM_EN)
			offset += sprintf( buf + offset, "Frame Num: %u. ", 
							   capInfo->triggerInfo[i].frameId);
		/* print retrograde info  */			
		if( osdInfo->flags & CAM_OSD_FLAG_RETROGRADE_EN &&
			(capInfo->triggerInfo[i].flags & TRIG_INFO_RETROGRADE) ) {
			offset += sprintf( buf + offset, "Way %u Retrograde. ", 
							   capInfo->triggerInfo[i].wayNum ); 
		} else if((capInfo->triggerInfo[i].flags & TRIG_INFO_RED_LIGHT) && 
				  (osdInfo->flags & CAM_OSD_FLAG_RED_LIGHT_TIME_EN)) {
			/* print red light time */
			offset += sprintf( buf + offset, "Red light time: %u s. ", 
							   capInfo->triggerInfo[i].redlightTime);
		} else if(!(capInfo->triggerInfo[i].flags & TRIG_INFO_RED_LIGHT) && 
				  !(capInfo->triggerInfo[i].flags & CAM_OSD_FLAG_RETROGRADE_EN)) {
			/* print speed */
			if(osdInfo->flags & CAM_OSD_FLAG_SPEED_EN) {
				offset += sprintf(buf + offset, "Speed: %uKM/H. ", 
								  capInfo->triggerInfo[i].speed);
			}
			if( (capInfo->triggerInfo[i].flags & TRIG_INFO_OVERSPEED) && 
				(osdInfo->flags & CAM_OSD_FLAG_OVERSPEED_INFO_EN) && 
				capInfo->limitSpeed ) {
				/* print over speed info */
				Uint32 overSpeedRatio = capInfo->triggerInfo[i].speed - capInfo->limitSpeed;
				overSpeedRatio = overSpeedRatio * 100 / capInfo->limitSpeed;
				offset += sprintf(buf + offset, "Limit speed: %uKM/H Overspeed ratio: %u ", 
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
#endif
		
	return err;
}

