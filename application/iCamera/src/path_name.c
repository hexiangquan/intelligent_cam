/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : path_name.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/12
  Last Modified :
  Description   : generate file path name
  Function List :
              path_name_config
              path_name_create
              path_name_generate
              replace_on_index
              replace_string
              str_index
  History       :
  1.Date        : 2012/3/12
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "path_name.h"
#include "log.h"
#include "net_utils.h"
#include "cam_params.h"

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

/* object for this module */
struct PathNameObj{
	Int8 pathBuf[PATHNAME_MAX_CNT_NUM][PATHNAME_MAX_LINE_SIZE];
	Int8 fileNameBuf[PATHNAME_MAX_CNT_NUM][PATHNAME_MAX_LINE_SIZE];
	Int8 pathNamePattern[PATHNAME_MAX_LINE_SIZE];
};

/*****************************************************************************
 Prototype    : replace_on_index
 Description  : replace string by index
 Input        : char *originalStr    
                Uint32 maxStrLen  
                Uint32 startIndex    
                Uint32 replaceLen    
                char* replaceStr     
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline Int32 replace_on_index(char *originalStr, Uint32 maxStrLen, 
										Uint32 startIndex, Uint32 replaceLen, 
										char* replaceStr)
{
	Uint32 oriStrLen, tarStrLen, reserveOldStartIndex, reserveNewStartIndex, reserveLen;


	oriStrLen = strlen(originalStr);
	tarStrLen = strlen(replaceStr);
	memset(originalStr + oriStrLen, 0, (int)(maxStrLen - oriStrLen));
	if(oriStrLen > maxStrLen || startIndex >= oriStrLen)
		return -1;

	if(startIndex + replaceLen > oriStrLen)
		replaceLen = oriStrLen - startIndex;
	//if(startIndex + tarStrLen > oriStrLen)
	//	tarStrLen = oriStrLen - startIndex;

	reserveOldStartIndex = startIndex + replaceLen;
	reserveNewStartIndex = startIndex + tarStrLen;
	reserveLen = oriStrLen - (startIndex + replaceLen) + 1;

	if(reserveLen + reserveNewStartIndex > maxStrLen)
		reserveLen = maxStrLen - reserveNewStartIndex;
	if(reserveNewStartIndex < maxStrLen && reserveOldStartIndex != reserveNewStartIndex && reserveLen != 0)
		memmove(originalStr + reserveNewStartIndex, originalStr + reserveOldStartIndex, reserveLen);

	memcpy(originalStr + startIndex, replaceStr, tarStrLen);
	return 0;
}

/*****************************************************************************
 Prototype    : str_index
 Description  : get index of string
 Input        : char *originalStr  
                Uint32 maxStrLen   
                Uint32 startIndex  
                char* searchStr     
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 str_index(char *originalStr, Uint32 maxStrLen, Uint32 startIndex, char* searchStr)
{
	Uint32 oriStrLen, tarStrLen;
	Int32 posOri, posTar;
	oriStrLen = strlen(originalStr);
	tarStrLen = strlen(searchStr);

	if(oriStrLen > maxStrLen || startIndex >= oriStrLen)
		return -1;

	for(posOri = startIndex, posTar = 0; posOri < oriStrLen && posTar < tarStrLen; posOri++) {
		if(originalStr[posOri] == searchStr[posTar])
			posTar++;
		else {
			if(posTar != 0)
				posTar = 0;
		}
	}

	if(posTar == tarStrLen && posOri <= oriStrLen)
		return posOri - tarStrLen;

	return -1;
}

/*****************************************************************************
 Prototype    : replace_string
 Description  : replace string
 Input        : char *pStr         
                Uint32 nStrMaxLen  
                char* originalStr  
                char* pTargetStr   
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 replace_string(char *pStr, Uint32 nStrMaxLen, char* originalStr, char* pTargetStr)
{
	Uint32 oriStrLen, tarStrLen; //strLen
	Int32 index;

	//strLen = strlen(pStr);
	oriStrLen = strlen(originalStr);
	tarStrLen = strlen(pTargetStr);
	index = 0;

	while((index = str_index(pStr, nStrMaxLen, index, originalStr)) >= 0) {
		replace_on_index(pStr, nStrMaxLen, index, oriStrLen, pTargetStr);
		index += tarStrLen;
	}

	return E_NO;
}

/*****************************************************************************
 Prototype    : ftp_file_name_create
 Description  : create this module
 Input        : const Int8 *pattern  
                const Int8 *text     
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
PathNameHandle path_name_create(const Int8 *pattern, const Int8 *text)
{
	PathNameHandle	hPathName;

	if(!pattern || !text)
		return NULL;

	/* alloc memory */
	hPathName = calloc(1, sizeof(struct PathNameObj));
	if(!hPathName)
		return NULL;

	/* init params */
	path_name_config(hPathName, pattern, text);

	return hPathName;
}

/*****************************************************************************
 Prototype    : path_name_generate
 Description  : generate file path name
 Input        : PathNameHandle hPathName  
                const ImgMsg *imgBuf      
                PathNameInfo * info       
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 path_name_generate(PathNameHandle hPathName, const ImgMsg *imgBuf, PathNameInfo *info)
{
	Int8 			strBuf[128];
	Int8 			prePattern[PATHNAME_MAX_LINE_SIZE];
	Int8 			fullName[PATHNAME_MAX_LINE_SIZE];
	Uint32 			i, patternLen, index;
	Uint16 			capCnt = 1;
	//CaptureInfo     *capInfo = (CaptureInfo *)(imgBuf->capInfo);
	//Uint16 			redLightTime;
	const CamDateTime *capTime = &imgBuf->timeStamp;

	if(!hPathName || !imgBuf || !info)
		return E_INVAL;
		
	//capCnt = (Uint16)(capInfo->unCapCnt);
	patternLen = strlen(hPathName->pathNamePattern);
	memset(prePattern, 0, sizeof(prePattern));
	memset(fullName, 0, PATHNAME_MAX_LINE_SIZE);

	// replace datetime string
	memcpy(prePattern, hPathName->pathNamePattern, patternLen + 1);
	sprintf(strBuf, "%04d", capTime->year);
	replace_string(prePattern, PATHNAME_MAX_LINE_SIZE, "?YYY", strBuf);
	sprintf(strBuf, "%02d", capTime->month);
	replace_string(prePattern, PATHNAME_MAX_LINE_SIZE, "?M", strBuf);
	sprintf(strBuf, "%02d", capTime->day);
	replace_string(prePattern, PATHNAME_MAX_LINE_SIZE, "?D", strBuf);
	sprintf(strBuf, "%02d", capTime->hour);
	replace_string(prePattern, PATHNAME_MAX_LINE_SIZE, "?H", strBuf);
	sprintf(strBuf, "%02d", capTime->minute);
	replace_string(prePattern, PATHNAME_MAX_LINE_SIZE, "?m", strBuf);
	sprintf(strBuf, "%02d", capTime->second);
	replace_string(prePattern, PATHNAME_MAX_LINE_SIZE, "?S", strBuf);
	sprintf(strBuf, "%03d", capTime->ms);
	replace_string(prePattern, PATHNAME_MAX_LINE_SIZE, "?ss", strBuf);

	//printf("%s\n", prePattern);
	
	for(i = 0; i < capCnt; i++) {
		memcpy(fullName, prePattern, patternLen);

#if 0
		/* light, speed, disobey type */
		if(pCapInfo->stTriggerInfo[i].ucFrameId == FRAME_CHECKPOST_1ST || 
			pCapInfo->stTriggerInfo[i].ucFrameId == FRAME_CHECKPOST_2ND)
		{
			/* green light */
			sprintf(strBuf, "%s", "ÂÌ");
			replace_string(fullName, PATHNAME_MAX_LINE_SIZE, "?L", strBuf);
			sprintf(strBuf, "%03d", pCapInfo->stTriggerInfo[i].ucSpeed % 1000);
			replace_string(fullName, PATHNAME_MAX_LINE_SIZE, "?SN", strBuf);
			if(pCapInfo->stTriggerInfo[i].usFlag & TRIG_INFO_OVERSPEED)
				sprintf(strBuf, "%s", "¿¨¿Ú³¬ËÙ");
			else
				sprintf(strBuf, "%s", "¿¨¿ÚÎ´³¬ËÙ");
		}
		else
		{
			// red light
			sprintf(strBuf, "%s", "ºì");
			replace_string(fullName, PATHNAME_MAX_LINE_SIZE, "?L", strBuf);
			sprintf(strBuf, "%05d", pCapInfo->stTriggerInfo[i].ucSpeed);
			replace_string(fullName, PATHNAME_MAX_LINE_SIZE, "??N", strBuf);
			sprintf(strBuf, "%s", "µç¾¯´³ºìµÆ");
		}
		if(pCapInfo->stTriggerInfo[i].ucFrameId == FRAME_RETROGRADE_1ST || pCapInfo->stTriggerInfo[i].ucFrameId == FRAME_RETROGRADE_2ND ||
		pCapInfo->stTriggerInfo[i].ucFrameId == FRAME_RETROGRADE_3RD)
			sprintf(strBuf, "%s", "ÄæÐÐ");
		replace_string(fullName, PATHNAME_MAX_LINE_SIZE, "?DT4567890", strBuf);


		// redlight time
		usRedLightTime = pCapInfo->stTriggerInfo[i].usRedlightTime;
		//printf("%u\n", nRedLightTime);
		//nRedLightTime %= 1000;
		sprintf(strBuf, "%03d", (usRedLightTime /100) % 1000);
		replace_string(fullName, PATHNAME_MAX_LINE_SIZE, "?TT", strBuf);
		
		// way number
		sprintf(strBuf, "%d",  pCapInfo->stTriggerInfo[i].ucWayNum % 10);
		replace_string(fullName, PATHNAME_MAX_LINE_SIZE, "<", strBuf);

		// frame number
		// need change case number to enumeration string, to be modified!
		switch(pCapInfo->stTriggerInfo[i].ucFrameId)
		{
		case FRAME_EPOLICE_2ND:
		case FRAME_CHECKPOST_2ND:
			strBuf[0] = 'B';
			strBuf[1] = '\0';
			break;
		case FRAME_EPOLICE_3RD:
			strBuf[0] = 'C';
			strBuf[1] = '\0';
			break;
		case FRAME_RETROGRADE_1ST:
			strcpy(strBuf, "RA");
			break;
		case FRAME_RETROGRADE_2ND:
			strcpy(strBuf, "RB");
			break;
		case FRAME_RETROGRADE_3RD:
			strcpy(strBuf, "RC");
			break;
		default:
			strBuf[0] = 'A';
			strBuf[1] = '\0';
			break;
		}
		replace_string(fullName, PATHNAME_MAX_LINE_SIZE, ">", strBuf);
#endif
		
		// split the path and filename
		for(index = strlen(fullName) - 1; (int)index >= 0; index--) {
			if(fullName[index] == '/')
				break;
		}
		if((int)index > 0)
			memcpy(&hPathName->pathBuf[i][0], fullName, (int)index);
		else
			index = 0;
		
		hPathName->pathBuf[i][index] = '\0';

		info->path[i] = hPathName->pathBuf[i];

		sprintf(hPathName->fileNameBuf[i], "%s.jpg", &fullName[index+1]);	

		DBG("Generate file name: %s", fullName);
		info->fileName[i] = hPathName->fileNameBuf[i];
		
	}

	info->fileNum = capCnt;
	return E_NO;
}


/*****************************************************************************
 Prototype    : path_name_config
 Description  : config patter & text 
 Input        : PathNameHandle hPathName  
                const Int8 *pattern       
                const Int8 *text          
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 path_name_config(PathNameHandle hPathName, const Int8 *pattern, const Int8 *text)
{
	if(!hPathName)
		return E_INVAL;

	Int8	buf[128];
	Int32	len;

	/* Set to zero */
	memset(hPathName->pathNamePattern, 0, sizeof(hPathName->pathNamePattern));
	len = strlen(pattern);

	if(!len || len > PATHNAME_MAX_LINE_SIZE) {
		strcpy(hPathName->pathNamePattern, PATHNAME_PATTERN_DEFAULT);
		WARN("path_name_config, input string length is %d, using default pattern.", len);
	}
	
	// replace tag <IP> to IP string
	get_local_ip(buf, sizeof(buf));
	replace_string(hPathName->pathNamePattern, PATHNAME_MAX_LINE_SIZE, "<IP>", buf);
	DBG("local IP:%s", buf);

	// replace tag <OSD> to text string
	strcpy(buf, text);
	replace_string(hPathName->pathNamePattern, PATHNAME_MAX_LINE_SIZE, "<OSD>", buf);

	// replace tag <L> to "?LµÆ", which means redlight or greenlight 
	replace_string(hPathName->pathNamePattern, PATHNAME_MAX_LINE_SIZE, "<L>", "?LµÆ");

	// replace tag <T> to "?TT", which means redlight time
	replace_string(hPathName->pathNamePattern, PATHNAME_MAX_LINE_SIZE, "<T>", "?TT");

	// replace date and time tag <Y><M><D><H><m><S><s>
	replace_string(hPathName->pathNamePattern, PATHNAME_MAX_LINE_SIZE, "<Y>", "?YYY");
	replace_string(hPathName->pathNamePattern, PATHNAME_MAX_LINE_SIZE, "<M>", "?M");
	replace_string(hPathName->pathNamePattern, PATHNAME_MAX_LINE_SIZE, "<D>", "?D");
	replace_string(hPathName->pathNamePattern, PATHNAME_MAX_LINE_SIZE, "<H>", "?H");
	replace_string(hPathName->pathNamePattern, PATHNAME_MAX_LINE_SIZE, "<m>", "?m");
	replace_string(hPathName->pathNamePattern, PATHNAME_MAX_LINE_SIZE, "<S>", "?S");
	replace_string(hPathName->pathNamePattern, PATHNAME_MAX_LINE_SIZE, "<s>", "?ss");

	// replace tag <DT> to "?DT4567890", which means disobey type
	replace_string(hPathName->pathNamePattern, PATHNAME_MAX_LINE_SIZE, "<DT>", "?dt4567890");

	// replace tag <WN> to "?WN", which means way number
	replace_string(hPathName->pathNamePattern, PATHNAME_MAX_LINE_SIZE, "<WN>", "?WN");

	// replace tag <FN> to "?FN", which means frame number "A" or "B" or "C", etc
	replace_string(hPathName->pathNamePattern, PATHNAME_MAX_LINE_SIZE, "<FN>", "?FN");

	// replace tag <SN> to "?SN", which means speed(EP Mode) or frame group number(CP)
	replace_string(hPathName->pathNamePattern, PATHNAME_MAX_LINE_SIZE, "<SN>", "??N");

	// remove all "<" and ">"
	replace_string(hPathName->pathNamePattern, PATHNAME_MAX_LINE_SIZE, "<", "");
	replace_string(hPathName->pathNamePattern, PATHNAME_MAX_LINE_SIZE, ">", "");

	// replace "?WN" to "<"(WayNumber) and "?FN" to ">"(FrameNumber);
	replace_string(hPathName->pathNamePattern, PATHNAME_MAX_LINE_SIZE, "?WN", "<");
	replace_string(hPathName->pathNamePattern, PATHNAME_MAX_LINE_SIZE, "?FN", ">");

	// replace "\" to "/"
	replace_string(hPathName->pathNamePattern, PATHNAME_MAX_LINE_SIZE, "\\", "/");

	return E_NO;
}

/*****************************************************************************
 Prototype    : path_name_delete
 Description  : delete module
 Input        : PathNameHandle hPathName  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 path_name_delete(PathNameHandle hPathName)
{
	if(!hPathName)
		return E_INVAL;

	free(hPathName);

	return E_NO;
}

