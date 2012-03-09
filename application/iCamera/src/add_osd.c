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
	Int32 		err, i = 0;
	Uint16 		yStep = 40;
	Int8   		cBuf[128];
	Int32 		offset;
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
	inArgs.strOsd = cBuf;
	
#ifdef DISP_CNT_NUM
	sprintf(cBuf, "The %d th picture ", imgMsg->index);
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
		inArgs.strOsd = cBuf;
	}

	/* Add date & time */
	if(osdInfo->flags & CAM_OSD_FLAG_TIME_EN) {
		CamDateTime *dateTime = &imgMsg->timeStamp;
		
		sprintf( cBuf, "%04d.%02d.%02d %02d:%02d:%02d", 
				 dateTime->year, dateTime->month, dateTime->day,
				 dateTime->hour, dateTime->minute, dateTime->second);
		err = osd_process(hOsd, &inBuf, &inArgs, NULL, NULL);
		if(err)
			return err;
		inArgs.startY += yStep;
	}

#if 0
	if(pstCapInf->unCapCnt > APP_MAX_CAP_CNT)
		return EINVPARAM;

	for(i = 0; i < pstCapInf->unCapCnt; i++)
	{
		nOffset = 0;
		cBuf[0] = 0x00;
		
		if(pOsdInfo->usFlag & OSDINFO_FLAG_PRINT_WAY_NUM)
			nOffset += sprintf( cBuf + nOffset, "Way Num: %u. ", pstCapInf->stTriggerInfo[i].ucWayNum);

		if(pOsdInfo->usFlag & OSDINFO_FLAG_PRINT_GROUP_ID)
			nOffset += sprintf( cBuf + nOffset, "Group Id: %u. ", pstCapInf->stTriggerInfo[i].usGroupNum);

		if(pOsdInfo->usFlag & OSDINFO_FLAG_PRINT_FRAME_NUM)
			nOffset += sprintf( cBuf + nOffset, "Frame Num: %u. ", pstCapInf->stTriggerInfo[i].ucFrameId);
					
		if(pOsdInfo->usFlag & OSDINFO_FLAG_PRINT_RETROGRADE &&
			pstCapInf->stTriggerInfo[i].ucFrameId == FRAME_RETROGRADE_1ST )
		{
			nOffset += sprintf( cBuf + nOffset, "Way %u Retrograde. ", pstCapInf->stTriggerInfo[i].ucWayNum ); 
		}
		else if( (pstCapInf->stTriggerInfo[i].usFlag & TRIG_INFO_RED_LIGHT) && 
				 (pOsdInfo->usFlag & OSDINFO_FLAG_PRINT_RED_LIGHT_TIME))
		{
			nOffset += sprintf( cBuf + nOffset, "Red light time: %u s. ", pstCapInf->stTriggerInfo[i].usRedlightTime);
		}
		else if( !(pstCapInf->stTriggerInfo[i].usFlag & TRIG_INFO_RED_LIGHT) && 
				 !(pOsdInfo->usFlag & OSDINFO_FLAG_PRINT_RETROGRADE))
		
		{
			if(pOsdInfo->usFlag & OSDINFO_FLAG_PRINT_SPEED)
			{
				nOffset += sprintf( cBuf + nOffset, "Speed: %uKM/H. ", pstCapInf->stTriggerInfo[i].ucSpeed);
			}
			if( (pstCapInf->stTriggerInfo[i].usFlag & TRIG_INFO_OVERSPEED) && 
				(pOsdInfo->usFlag & OSDINFO_FLAG_PRINT_OVERSPEED_INFO) && pstCapInf->ucLimitSpeed)
			{
				Uint32 unOverSpeedRatio = pstCapInf->stTriggerInfo[i].ucSpeed - pstCapInf->ucLimitSpeed;
				unOverSpeedRatio = unOverSpeedRatio * 100 / pstCapInf->ucLimitSpeed;
				nOffset += sprintf( cBuf + nOffset, "Limit speed: %uKM/H Overspeed ratio: %u ", pstCapInf->ucLimitSpeed, unOverSpeedRatio);
			}
		}

		/* Check if something has been printed in the buffer */
		if(cBuf[0])
		{
			osd_add(hOsd, pYuvBuf, cBuf, usXPos, usYPos);
			usYPos += usHStep;		
		}
	}			
#endif
		
	return E_NO;
}

