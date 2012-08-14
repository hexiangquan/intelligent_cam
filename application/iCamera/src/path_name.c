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
#include "cam_detector.h"
#include "cam_time.h"

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
#define PATH_NAME_EN_ONLY	//only use english name

#ifdef PATH_NAME_EN_ONLY
#define STR_RED_LIGHT		"Red"
#define STR_GREEN_LIGHT		"Green"
#define STR_RETROGRADE		"Retrograde"
#define STR_EPOLICE			"EPolice"
#define STR_CHECK_POST		"Check_Post"
#define STR_OVERSPEED		"Over_Speed"
#define STR_LIGHT			"light"
#else
#define STR_RED_LIGHT		"ºì"
#define STR_GREEN_LIGHT		"ÂÌ"
#define STR_RETROGRADE		"ÄæÐÐ"
#define STR_EPOLICE			"µç¾¯´³ºìµÆ"
#define STR_CHECK_POST		"¿¨¿ÚÎ´³¬ËÙ"
#define STR_OVERSPEED		"¿¨¿Ú³¬ËÙ"
#define STR_LIGHT			"µÆ"
#endif

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
	Int8 				strBuf[128];
	Int8 				prePattern[PATHNAME_MAX_LINE_SIZE];
	Int8 				fullName[PATHNAME_MAX_LINE_SIZE];
	Uint32 				i, patternLen, index;
	Int32 				capCnt = imgBuf->capInfo.capCnt;
	const CaptureInfo	*capInfo = &imgBuf->capInfo;
	Uint16 				redLightTime;
	const DateTime 		*capTime = &imgBuf->timeStamp;

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
		strcpy(fullName, prePattern);

#if 1
		/* group id */
		sprintf(strBuf, "%05d", capInfo->triggerInfo[i].groupId);
		replace_string(fullName, PATHNAME_MAX_LINE_SIZE, "??N", strBuf);
			
		/* light, speed, disobey type */
		if(capInfo->triggerInfo[i].flags & TRIG_INFO_RED_LIGHT) {
			/* red light */
			strcpy(strBuf, STR_RED_LIGHT);
			replace_string(fullName, PATHNAME_MAX_LINE_SIZE, "?L", strBuf);
		
			strcpy(strBuf, STR_EPOLICE);
		} else {
			/* green light */
			strcpy(strBuf, STR_GREEN_LIGHT);
			replace_string(fullName, PATHNAME_MAX_LINE_SIZE, "?L", strBuf);
			
			if(capInfo->triggerInfo[i].flags & TRIG_INFO_OVERSPEED)
				strcpy(strBuf, STR_OVERSPEED);
			else
				strcpy(strBuf, STR_CHECK_POST);
		}
		
		if(capInfo->triggerInfo[i].flags & TRIG_INFO_RETROGRADE)
			strcpy(strBuf, STR_RETROGRADE);

		/* type of capture */
		replace_string(fullName, PATHNAME_MAX_LINE_SIZE, "?dt4567890", strBuf);
		
		/* way number */
		sprintf(strBuf, "%d",  capInfo->triggerInfo[i].wayNum % 10);
		replace_string(fullName, PATHNAME_MAX_LINE_SIZE, "<", strBuf);

		/* speed */
		sprintf(strBuf, "%03d", capInfo->triggerInfo[i].speed);
		replace_string(fullName, PATHNAME_MAX_LINE_SIZE, "?P", strBuf);

		/* redlight time */
		redLightTime = capInfo->triggerInfo[i].redlightTime;			
		sprintf(strBuf, "%03d", (redLightTime /100) % 1000);
		replace_string(fullName, PATHNAME_MAX_LINE_SIZE, "?TT", strBuf);

		/* frame number
		  * need change case number to enumeration string, to be modified!
		  */
		if(capInfo->triggerInfo[i].flags & TRIG_INFO_RETROGRADE) {
			strBuf[0] = 'R';
			strBuf[1] = 'A' + capInfo->triggerInfo[i].frameId - FRAME_TRIG_BASE;
			strBuf[2] = '\0';
		} else {
			strBuf[0] = 'A' + capInfo->triggerInfo[i].frameId - FRAME_TRIG_BASE;
			strBuf[1] = '\0';
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

		//DBG("Generate file name: %s", fullName);
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
	} else {
		strcpy(hPathName->pathNamePattern, pattern);
	}
	
	// replace tag <IP> to IP string
	get_local_ip(buf, sizeof(buf));
	replace_string(hPathName->pathNamePattern, PATHNAME_MAX_LINE_SIZE, "<IP>", buf);

	// replace tag <OSD> to text string
	strcpy(buf, text);
	replace_string(hPathName->pathNamePattern, PATHNAME_MAX_LINE_SIZE, "<OSD>", buf);

	// replace tag <L> to "?LµÆ", which means redlight or greenlight 
	replace_string(hPathName->pathNamePattern, PATHNAME_MAX_LINE_SIZE, "<L>", "?L"STR_LIGHT);

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

	// replace tag <SN> to "?SN", which means frame group number
	replace_string(hPathName->pathNamePattern, PATHNAME_MAX_LINE_SIZE, "<SN>", "??N");
	
	// replace tag <P> to "?P", which means speed
	replace_string(hPathName->pathNamePattern, PATHNAME_MAX_LINE_SIZE, "<P>", "?P");

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

/*****************************************************************************
 Prototype    : path_name_test
 Description  : test path name generate
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
Int32 path_name_test()
{
	PathNameHandle hPathName;
	char pattern[128] = PATHNAME_PATTERN_DEFAULT;
	Int32 err;

	bzero(pattern, sizeof(pattern));
	strncpy(pattern, PATHNAME_PATTERN_DEFAULT, sizeof(pattern));
	DBG("pattern: %s", pattern);
	
	hPathName = path_name_create(pattern, "test");
	assert(hPathName);

	/* generate file name */
	ImgMsg img;
	CaptureInfo *capInfo = &img.capInfo;

	bzero(&img, sizeof(img));

	/* get time */
	gettimeofday(&img.timeCode, NULL);
	cam_get_time(&img.timeStamp);
	DBG("time: %d/%d/%d %d:%d:%d", img.timeStamp.year, img.timeStamp.month,
		img.timeStamp.day, img.timeStamp.hour, img.timeStamp.minute,
		img.timeStamp.second);

	/* set cap info */
	capInfo->capCnt = 1;
	capInfo->flags = 0;
	capInfo->limitSpeed = 80;

	/* set single way trig info */
	TriggerInfo *trigInfo = &capInfo->triggerInfo[0];
	trigInfo->frameId = FRAME_TRIG_BASE + 2;
	trigInfo->groupId = 1;
	trigInfo->redlightTime = 200;
	trigInfo->speed = 0;
	trigInfo->wayNum = 2;
	trigInfo->flags = TRIG_INFO_RED_LIGHT;

	capInfo->triggerInfo[1] = capInfo->triggerInfo[2] = *trigInfo;

	PathNameInfo nameInfo;

	/* calc time cost */
	struct timeval tmStart,tmEnd; 
	float   timeUse;

	gettimeofday(&tmStart,NULL);
	bzero(&nameInfo, sizeof(nameInfo));
	err = path_name_generate(hPathName, &img, &nameInfo);
	gettimeofday(&tmEnd,NULL);
	
	assert(err == E_NO);
	assert(nameInfo.fileNum == capInfo->capCnt);

	timeUse = 1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec;
	
	DBG("generated file path: %s, name: %s, cost: %.2fus", 
		nameInfo.path[0], nameInfo.fileName[0], timeUse);


	/* change pattern */
	strcpy(pattern, "<OSD>/<Y><M><D>/<DT>-<L>/<H><m><S><s>-<WN>-<SN>-<FN>-<T>");
	err = path_name_config(hPathName, pattern, "road");
	assert(err == E_NO);

	/* generate again */
	cam_get_time(&img.timeStamp);
	
	gettimeofday(&tmStart,NULL);
	bzero(&nameInfo, sizeof(nameInfo));
	err = path_name_generate(hPathName, &img, &nameInfo);
	gettimeofday(&tmEnd,NULL);
	
	assert(err == E_NO);
	assert(nameInfo.fileNum == capInfo->capCnt);

	timeUse = 1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec;
	
	DBG("generated file path: %s, name: %s, cost: %.2fus", 
		nameInfo.path[0], nameInfo.fileName[0], timeUse);

	/* check post file name */
	trigInfo = &capInfo->triggerInfo[1];
	capInfo->capCnt = 2;
	
	trigInfo->flags = 0;
	trigInfo->speed = 50;
	trigInfo->groupId = 2345;
	trigInfo->wayNum = 1;
	trigInfo->frameId = FRAME_TRIG_BASE;
	cam_get_time(&img.timeStamp);
	
	strcpy(pattern, "<IP>-<OSD>/<Y><M><D>/<DT>/<H><m><S><s>-<SN>-<FN>-<P>");
	err = path_name_config(hPathName, pattern, "road");
	assert(err == E_NO);

	gettimeofday(&tmStart,NULL);
	bzero(&nameInfo, sizeof(nameInfo));
	err = path_name_generate(hPathName, &img, &nameInfo);
	gettimeofday(&tmEnd,NULL);
	
	assert(err == E_NO);
	assert(nameInfo.fileNum == capInfo->capCnt);

	timeUse = 1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec;
	
	DBG("generated file path: %s, name: %s, cost: %.2fus", 
		nameInfo.path[1], nameInfo.fileName[1], timeUse);

	/* retrograde test */
	trigInfo = &capInfo->triggerInfo[2];
	capInfo->capCnt = 3;
	
	trigInfo->flags = TRIG_INFO_RETROGRADE;
	trigInfo->frameId = FRAME_TRIG_BASE + 1;
	trigInfo->groupId = 99;
	
	strcpy(pattern, "<OSD>/<Y><M><D>/<DT>/<H><m><S><s>-<WN>-<SN>-<FN>");
	err = path_name_config(hPathName, pattern, "road");
	assert(err == E_NO);

	gettimeofday(&tmStart,NULL);
	bzero(&nameInfo, sizeof(nameInfo));
	err = path_name_generate(hPathName, &img, &nameInfo);
	gettimeofday(&tmEnd,NULL);
	
	assert(err == E_NO);
	assert(nameInfo.fileNum == capInfo->capCnt);

	timeUse = 1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec;
	
	DBG("generated file path: %s, name: %s, cost: %.2fus", 
		nameInfo.path[2], nameInfo.fileName[2], timeUse);

	err = path_name_delete(hPathName);
	assert(err == E_NO);

	DBG("%s done", __func__);

	return E_NO;
}

