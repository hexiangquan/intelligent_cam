/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : display.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/8/28
  Last Modified :
  Description   : display module using V4L2
  Function List :
  History       :
  1.Date        : 2012/8/28
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "display.h"
#include "cmem.h"
#include "log.h"
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/videodev2.h>
#include <media/davinci/videohd.h>

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
#define DISPLAY_BUF_NUM		3
#define DISPLAY_MAX_WIDTH	PAL_WIDTH
#define DISPLAY_MAX_HEIGHT	PAL_HEIGHT
#define DISPLAY_BUF_SIZE	\
			ROUND_UP(DISPLAY_MAX_WIDTH * DISPLAY_MAX_HEIGHT* 2, 4096)
	
#define DISPLAY_INTERFACE_COMPOSITE	"COMPOSITE"
#define DISPLAY_INTERFACE_COMPONENT	"COMPONENT"
#define DISPLAY_MODE_NAME_PAL		"PAL"
#define DISPLAY_MODE_NAME_NTSC		"NTSC"
#define DISPLAY_MODE_NAME_720P		"720P-60"		
#define DISPLAY_MODE_NAME_1080I		"1080I-30"		
/* Standards and output information */
#define ATTRIB_MODE					"mode"
#define ATTRIB_OUTPUT				"output"

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/* Object of this module */
struct DisplayObj {
	int 				fdDisplay;
	DisplayBuf			displayBuf[DISPLAY_BUF_NUM];
	CMEM_AllocParams	memAllocParams;
	Bool				isStreamOn;
	DisplayAttrs		attrs;
};

/*****************************************************************************
 Prototype    : display_buf_free
 Description  : free mem allocated
 Input        : DisplayHanlde hDisplay  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 display_buf_free(DisplayHanlde hDisplay)
{
	Int32 i;
	
	for(i = 0; i < DISPLAY_BUF_NUM; i++) {
		if(hDisplay->displayBuf[i].userAddr) {
			CMEM_free(hDisplay->displayBuf[i].userAddr, &hDisplay->memAllocParams);
			hDisplay->displayBuf[i].userAddr = NULL;
		}
	}

	return E_NO;
}

/*****************************************************************************
 Prototype    : display_buf_alloc
 Description  : alloc buffer for display
 Input        : DisplayHanlde hDisplay  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 display_buf_alloc(DisplayHanlde hDisplay)
{
	Uint32 size;
	CMEM_AllocParams *allocAttrs = &hDisplay->memAllocParams;
	Int32 i, err = E_NO;
	
	allocAttrs->alignment = 256;
	allocAttrs->flags = CMEM_NONCACHED;
	allocAttrs->type = CMEM_POOL;

	size = DISPLAY_BUF_SIZE;

	for(i = 0; i < DISPLAY_BUF_NUM; i++) {
		hDisplay->displayBuf[i].userAddr = CMEM_alloc(size, allocAttrs);
		if(!hDisplay->displayBuf[i].userAddr) {
			ERR("Alloc buffer mem failed.");
			err = E_NOMEM;
			break;
		}
		hDisplay->displayBuf[i].index = i;
		hDisplay->displayBuf[i].bufSize = size;
		hDisplay->displayBuf[i].phyAddr = 0;
	}

	/* request buffers from driver */
	struct v4l2_requestbuffers req;
	req.count = DISPLAY_BUF_NUM;
	req.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	req.memory = V4L2_MEMORY_USERPTR;
	err = ioctl(hDisplay->fdDisplay, VIDIOC_REQBUFS, &req);
	if(err < 0) {
		ERRSTR("cannot request buf");
		return E_IO;
	}

	return err;
}

/*****************************************************************************
 Prototype    : display_dev_open
 Description  : open device
 Input        : DisplayHanlde hDisplay  
                Uint32 chanId           
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 display_dev_open(DisplayHanlde hDisplay, Uint32 chanId)
{
	/* open device if necessary */
	if(hDisplay->fdDisplay <= 0) {
		char devName[32];
		snprintf(devName, sizeof(devName), "/dev/video%u", chanId + 2);
		hDisplay->fdDisplay = open(devName, O_RDWR);
		if(hDisplay->fdDisplay < 0) {
			ERRSTR("can't open device: %s", devName);
			return E_IO;
		}
		DBG("open %s ok.", devName);
	}

	return E_NO;
}

/*****************************************************************************
 Prototype    : display_create
 Description  : create display module
 Input        : const DisplayAttrs *attrs  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
DisplayHanlde display_create(const DisplayAttrs *attrs)
{
	if( !attrs || 
		attrs->chanId > DISPLAY_MAX_CHAN_ID || 
		attrs->mode >= DISPLAY_MODE_MAX)
		return NULL;

	DisplayHanlde hDisplay;

	hDisplay = calloc(1, sizeof(struct DisplayObj));
	if(!hDisplay) {
		ERR("alloc mem failed");
		return NULL;
	}

	Int32 err = display_dev_open(hDisplay, attrs->chanId);
	err |= display_config(hDisplay, attrs);
	if(err)
		goto exit;

	err = display_buf_alloc(hDisplay);
	if(err)
		goto exit;

	return hDisplay;

exit:
	display_delete(hDisplay);
	return NULL;
}

/*****************************************************************************
 Prototype    : display_delete
 Description  : delete display module
 Input        : DisplayHanlde hDisplay  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 display_delete(DisplayHanlde hDisplay)
{
	if(!hDisplay)
		return E_INVAL;

	if(hDisplay->isStreamOn)
		display_stop(hDisplay);
	
	display_buf_free(hDisplay);
	free(hDisplay);

	return E_NO;
}

/*****************************************************************************
 Prototype    : display_sysfs_cfg
 Description  : cfg driver by sysfs
 Input        : const char *attribute  
                const char *value      
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 display_sysfs_cfg(const char *attribute, const char *value)
{
	int sysfd = -1;
	char initVal[32];
	char attrTag[128];

	bzero(initVal, sizeof(initVal));
	strcpy(attrTag, "/sys/class/davinci_display/ch0/");
	strcat(attrTag, attribute);

	sysfd = open(attrTag, O_RDWR);
	if (!sysfd) {
		ERRSTR("cannot open %s", attrTag);
		return E_IO;
	}
	
	read(sysfd, initVal, sizeof(initVal));
	lseek(sysfd, 0, SEEK_SET);

	write(sysfd, value, 1 + strlen(value));
	lseek(sysfd, 0, SEEK_SET);

	bzero(initVal, sizeof(initVal));
	read(sysfd, initVal, sizeof(initVal));
	lseek(sysfd, 0, SEEK_SET);
	DBG("Display, changed %s to %s", attribute, initVal);

	close(sysfd);
	return E_NO;
}


/*****************************************************************************
 Prototype    : display_config
 Description  : config attrs for display module
 Input        : DisplayHanlde hDisplay     
                const DisplayAttrs *attrs  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 display_config(DisplayHanlde hDisplay, const DisplayAttrs *attrs)
{
	if(!hDisplay || !attrs)
		return E_INVAL;

	Int32 err;
	
	err = display_dev_open(hDisplay, attrs->chanId);
	if(err)
		return err;

	/* set attrs  */
	const char *output, *mode;
	struct v4l2_format fmt;
	Uint32 width, height;

	bzero(&fmt, sizeof(fmt));
	
	switch(attrs->mode) {
	case DISPLAY_MODE_PAL:
		output = DISPLAY_INTERFACE_COMPOSITE;
		mode = DISPLAY_MODE_NAME_PAL;
		fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
		width = PAL_WIDTH;
		height = PAL_HEIGHT;
		break;
	case DISPLAY_MODE_NTSC:
		output = DISPLAY_INTERFACE_COMPOSITE;
		mode = DISPLAY_MODE_NAME_NTSC;
		fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
		width = NTSC_WIDTH;
		height = NTSC_HEIGHT;
		break;
	default:
		ERR("unsupport display mode: %u", attrs->mode);
		return E_INVAL;
	}

	/* cfg driver by sysfs */
	err = display_sysfs_cfg(ATTRIB_OUTPUT, output);
	if(err)
		return err;
	err = display_sysfs_cfg(ATTRIB_MODE, mode);
	if(err)
		return err;

	/* set v4l2 format */
	if(attrs->outputFmt == FMT_YUV_422ILE) {
		fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_UYVY;
		fmt.fmt.pix.bytesperline = width * 2;
		fmt.fmt.pix.sizeimage = (fmt.fmt.pix.bytesperline * height);
	} else if(attrs->outputFmt == FMT_YUV_420SP) {
		fmt.fmt.pix.bytesperline = ROUND_UP(width, 32);
		fmt.fmt.pix.sizeimage = 
			fmt.fmt.pix.bytesperline * height + fmt.fmt.pix.bytesperline * (height >> 1);
		fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_NV12;
	} else {
		ERR("invalid output fmt: %d", attrs->outputFmt);
		return E_INVAL;
	}

	fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	fmt.fmt.pix.width = width;
	fmt.fmt.pix.height = height;

	err = ioctl(hDisplay->fdDisplay, VIDIOC_S_FMT, &fmt);
	if(err < 0) {
		ERRSTR("VIDIOC_S_FMT failed");
		return E_IO;
	}

	hDisplay->attrs = *attrs;
	return E_NO;
}

/*****************************************************************************
 Prototype    : display_start
 Description  : start display stream
 Input        : DisplayHanlde hDisplay  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 display_start(DisplayHanlde hDisplay)
{	
	if(!hDisplay)
		return E_INVAL;

	if(hDisplay->isStreamOn) {
		ERR("stream has already been started, stop first!");
		return E_BUSY;
	}
	
	int i = 0;
	assert(hDisplay->fdDisplay > 0);
	
	/* que buffer into driver */
	for (i = 0; i < DISPLAY_BUF_NUM; i++) {
		struct v4l2_buffer buf;
		
		bzero(&buf, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		buf.memory = V4L2_MEMORY_USERPTR;
		buf.index = i;
		buf.length = DISPLAY_BUF_SIZE;
		//buf.field = V4L2_FIELD_NONE;
		buf.m.userptr = (unsigned long)hDisplay->displayBuf[i].userAddr;

		if (-1 == ioctl(hDisplay->fdDisplay, VIDIOC_QBUF, &buf)) {
			ERRSTR("<%d> que buf failed", i);
			return E_IO;
		}
	}
	
	/* all done , get set go */
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	if (-1 == ioctl(hDisplay->fdDisplay, VIDIOC_STREAMON, &type)) {
		ERRSTR("start display streaming failed");
		return E_IO;
	}

	hDisplay->isStreamOn = TRUE;
	
	return E_NO;
}

/*****************************************************************************
 Prototype    : display_stop
 Description  : stop display stream
 Input        : DisplayHanlde hDisplay  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 display_stop(DisplayHanlde hDisplay)
{
	if(!hDisplay)
		return E_INVAL;

	if(!hDisplay->isStreamOn) {
		WARN("display hasn't started yet");
		return E_MODE;
	}

	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_OUTPUT;

	if(ioctl(hDisplay->fdDisplay, VIDIOC_STREAMOFF, &type) < 0) {
		ERRSTR("Stream off failed");
		return E_IO;
	}
	
	hDisplay->isStreamOn = FALSE;

	//DBG("Capture stopped");
	
	return E_NO;
}

/*****************************************************************************
 Prototype    : display_get_free_buf
 Description  : alloc a free buffer from driver for filling with new image 
 Input        : DisplayHanlde hDisplay  
                DisplayBuf *buf         
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 display_get_free_buf(DisplayHanlde hDisplay, DisplayBuf *buf)
{
	if(!hDisplay || !buf)
		return E_INVAL;

	if(!hDisplay->isStreamOn)
		return E_MODE;

	struct v4l2_buffer dispBuf;

	bzero(&dispBuf, sizeof(dispBuf));
	dispBuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	dispBuf.memory = V4L2_MEMORY_USERPTR;

	Int32 ret =  ioctl(hDisplay->fdDisplay, VIDIOC_DQBUF, &dispBuf);
	if (ret < 0) {
		ERRSTR("deque for display failed");
		return E_IO;
	}

	buf->userAddr = (void *)dispBuf.m.userptr;
	buf->phyAddr = 0;
	buf->index = dispBuf.index;
	buf->bufSize = DISPLAY_BUF_SIZE;

	return E_NO;
}

/*****************************************************************************
 Prototype    : display_find_buf_index
 Description  : find index of this buffer
 Input        : DisplayHanlde hDisplay  
                const DisplayBuf *buf   
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 display_find_buf_index(DisplayHanlde hDisplay, const DisplayBuf *buf)
{
	Int32 index = buf->index;

	if(hDisplay->displayBuf[index].userAddr == buf->userAddr)
		return index;

	for(index = 0; index != DISPLAY_BUF_NUM; ++index) {
		if(hDisplay->displayBuf[index].userAddr == buf->userAddr)
			break;
	}

	if(index == DISPLAY_BUF_NUM)
		return -1;

	return index;
}

/*****************************************************************************
 Prototype    : display_put_buf
 Description  : put buffer to display
 Input        : DisplayHanlde hDisplay  
                const DisplayBuf *buf   
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 display_put_buf(DisplayHanlde hDisplay, const DisplayBuf *buf)
{
	if(!hDisplay || !buf)
		return E_INVAL;

	if(!hDisplay->isStreamOn)
		return E_MODE;

	Int32 index = display_find_buf_index(hDisplay, buf);
	if(index < 0) {
		ERR("invalid buf, must get free first.");
		return E_INVAL;
	}

	struct v4l2_buffer dispBuf;

	bzero(&dispBuf, sizeof(dispBuf));
	dispBuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	dispBuf.memory = V4L2_MEMORY_USERPTR;
	dispBuf.m.userptr = (unsigned long)buf->userAddr;
	dispBuf.length = buf->bufSize;
	dispBuf.index = index;
	
	Int32 ret =  ioctl(hDisplay->fdDisplay, VIDIOC_QBUF, &dispBuf);
	if (ret < 0) {
		ERRSTR("que buf for display failed");
		return E_IO;
	}

	return E_NO;
}


