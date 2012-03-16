#include "frame_dispatch.h"
#include "log.h"
#include "cam_time.h"
#include <sys/stat.h>
#include "capture.h"

struct FrameDispObj {
	FrameTxStatus	txStat;
	FrameEncMode	encMode;
	FrameDispInfo	info;
	CapHandle		hCap;
};

/*****************************************************************************
 Prototype    : frame_disp_create
 Description  : create file dispatch handle
 Input        : FrameTxStatus curStat  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/8
    Author       : Sun
    Modification : Created function

*****************************************************************************/
FrameDispHandle frame_disp_create(FrameTxStatus curStat, FrameEncMode encMode, FrameDispInfo *info)
{
	FrameDispHandle hFrameDisp;

	hFrameDisp = malloc(sizeof(struct FrameDispObj));
	if(!hFrameDisp) {
		ERR("alloc mem for handle failed");
		return NULL;
	}

	hFrameDisp->txStat = curStat;
	hFrameDisp->encMode = encMode;
	hFrameDisp->info = *info;
	return hFrameDisp;
}

/*****************************************************************************
 Prototype    : frame_disp_set_capture_handle
 Description  : set capture handle
 Input        : FrameDispHandle hDispatch  
                CapHandle hCap             
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/13
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 frame_disp_register_capture(FrameDispHandle hDispatch, CapHandle hCap)
{
	if(!hDispatch || !hCap)
		return E_INVAL;

	hDispatch->hCap = hCap;
	return E_NO;
}

/*****************************************************************************
 Prototype    : frame_disp_delete
 Description  : free frame dispatch handle
 Input        : FrameDispHandle hFrameDisp  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/8
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 frame_disp_delete(FrameDispHandle hFrameDisp)
{
	if(!hFrameDisp)
		return E_INVAL;

	free(hFrameDisp);
	return E_NO;
}


/*****************************************************************************
 Prototype    : frame_disp_free_buf
 Description  : free buffer handle, to its pool or to the heap
 Input        : BufHandle hBuf  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/8
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 free_frame_buf(FrameDispHandle hDispatch, ImgMsg *img, Int32 flags)
{
	if(flags & FD_FLAG_NOT_FREE_BUF) {
		return E_NO;
	}

	if(flags & FD_FLAG_RAW_FRAME) {
		//return capture_free_frame(hDispatch->hCap, &img->rawFrame);
	}
	
	/* free buffer */
	BufPoolHandle hPool;

	/* try get pool of this buffer */
	hPool = buffer_get_pool(img->hBuf);

	if(hPool) {
		/* belong to a pool, put back */
		return buf_pool_free(img->hBuf);
	} else {
		/* free to heap */
		return buffer_free(img->hBuf);
	}
}

/*****************************************************************************
 Prototype    : mkdir_if_need
 Description  : check if dir is exist, if not, create one
 Input        : const char *dir  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/8
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 mkdir_if_need(const char *dir)
{
	char 		dirname[128] = "";
	struct stat fstat;
	Bool 		end = FALSE;
	Int32 		off = 0;
	Int32 		ret = 0;

	strncpy(dirname, dir, sizeof(dirname));
	memset(&fstat, 0, sizeof(&fstat));

	off = strspn(dirname, "/");
	for (;;) {
		off += strcspn(dirname+off, "/");
		if ('\0' == dirname[off]) {
			end = TRUE;
		} else {
			dirname[off] = '\0';
		}

		ret = stat(dirname, &fstat);
		if (ret < 0) {
			INFO("mkdir %s", dirname);
			ret = mkdir(dirname, S_IRWXU|S_IRWXG|S_IRWXO);
			if (ret < 0) {
				ERRSTR("mkdir <%s> err", dirname);
				ret = E_REFUSED;
				break;
			}
		}
		
		if(end)
			break;
		else 
			dirname[off] = '/';
		off += strspn(dirname+off, "/");
	}
	
	return ret;
}

/*****************************************************************************
 Prototype    : jpg_save
 Description  : save jpg file to local file system
 Input        : FrameDispHandle hFrameDisp  
                ImgMsg *frameBuf            
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/8
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 jpg_save(FrameDispHandle hFrameDisp, ImgMsg *frameBuf)
{
	char 		fileName[128];
	char		dirName[128];
	CamDateTime	*capTime = &frameBuf->timeStamp;
	
	/* generate dir file name */
	snprintf(dirName, sizeof(dirName), "%s/%04u_%02u_%02u_%02u", hFrameDisp->info.savePath,
             capTime->year, capTime->month, capTime->day, capTime->hour);
	snprintf(fileName, sizeof(fileName), "%s/%u_%u_%u.jpg", dirName, 
			capTime->minute, capTime->second, capTime->ms);

	mkdir_if_need(dirName);
	/* save file */
	int fd = open(fileName, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if(fd < 0) {
        ERRSTR("open <%s> err", fileName);
        return E_REFUSED;
    }

    Int32 ret = write(fd, buffer_get_user_addr(frameBuf->hBuf), frameBuf->dimension.size);
    if(ret != frameBuf->dimension.size) {
        ERRSTR("write <%s> err", fileName);
		ret = E_IO;
    } else 
    	ret = E_NO;
  
    DBG("<%s> saved, size: %d", fileName, frameBuf->dimension.size);
    close(fd);

	return ret;
}

/*****************************************************************************
 Prototype    : jpg_dispatch
 Description  : dispatch jpeg file
 Input        : FrameDispHandle hFrameDisp  
                ImgMsg *frameBuf            
                const char *dstMsgName      
                Int32 flags                 
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/8
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 jpg_dispatch(FrameDispHandle hFrameDisp, MsgHandle hMsg, ImgMsg *frameBuf, const char *dstMsgName, Int32 flags)
{
	Int32 err = E_NO;

	if(hMsg) {
		err = E_CONNECT;
		if(hFrameDisp->txStat == FT_SRV_CONNECTED && !(flags & FD_FLAG_SAVE_ONLY)) {
			/* send msg to next task */
			err = msg_send(hMsg, dstMsgName, frameBuf, sizeof(ImgMsg));
			if(!err)
				return E_NO;
		}
	}

	if(err || (flags & FD_FLAG_SAVE_ONLY))
		err = jpg_save(hFrameDisp, frameBuf);

	err = free_frame_buf(hFrameDisp, frameBuf, flags);

	return err;
}

/*****************************************************************************
 Prototype    : h264_dispatch
 Description  : dispatch h.264 frame
 Input        : FrameDispHandle hFrameDisp  
                ImgMsg *frameBuf            
                const char *dstMsgName      
                Int32 flags                 
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/9
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 h264_dispatch(FrameDispHandle hFrameDisp, MsgHandle hMsg, ImgMsg *frameBuf, const char *dstMsgName, Int32 flags)
{
	Int32 err = E_IO;

	/* using RTP to send */

	err = free_frame_buf(hFrameDisp, frameBuf, flags);

	return err;
}

/*****************************************************************************
 Prototype    : raw_dispatch
 Description  : dispatch raw image
 Input        : FrameDispHandle hFrameDisp  
                MsgHandle hMsg              
                ImgMsg *frameBuf            
                const char *dstMsgName      
                Int32 flags                 
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/9
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 raw_dispatch(FrameDispHandle hFrameDisp, MsgHandle hMsg, ImgMsg *frameBuf, const char *dstMsgName, Int32 flags)
{
	Int32 err = E_MODE;

	if(hMsg) {
		if(hFrameDisp->encMode == FRAME_ENC_ON) {
			/* send msg to encode */
			err = msg_send(hMsg, dstMsgName, frameBuf, sizeof(ImgMsg));
		} else if(hFrameDisp->txStat == FT_SRV_CONNECTED) {
			/* send msg for net tx */
			err = msg_send(hMsg, dstMsgName, frameBuf, sizeof(ImgMsg));
		}
	}

	if(err) {
		err = free_frame_buf(hFrameDisp, frameBuf, flags);
	}

	return err;
}

/*****************************************************************************
 Prototype    : frame_disp_run
 Description  : run frame dispatch
 Input        : FrameDispHandle hFrameDisp  
                ImgMsg *frameBuf            
                const char *dstMsgName      
                Int32 flags                 
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/8
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 frame_disp_run(FrameDispHandle hFrameDisp, MsgHandle hMsg, ImgMsg *frameBuf, const char *dstMsgName, Int32 flags)
{
	if(!hFrameDisp || !frameBuf)
		return E_INVAL;

	Int32 ret;

	switch(frameBuf->dimension.colorSpace) {
	case FMT_JPG:
		ret = jpg_dispatch(hFrameDisp, hMsg, frameBuf, dstMsgName, flags);
		break;
	case FMT_H264:
		ret = h264_dispatch(hFrameDisp, hMsg, frameBuf, dstMsgName, flags);
		break;
	case FMT_BAYER_RGBG:
	case FMT_YUV_420SP:
		ret = raw_dispatch(hFrameDisp, hMsg, frameBuf, dstMsgName, flags);
		break;
	default:
		WARN("unsupport fmt, just free buffer");
		free_frame_buf(hFrameDisp, frameBuf, flags);
		ret = E_UNSUPT;
		break;
	}

	return ret;
}

/*****************************************************************************
 Prototype    : frame_disp_set_tx_status
 Description  : set current send status
 Input        : FrameDispHandle hFrameDisp  
                FrameTxStatus status        
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/9
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 frame_disp_set_tx_status(FrameDispHandle hFrameDisp, FrameTxStatus status)
{
	if(!hFrameDisp)
		return E_INVAL;
	
	hFrameDisp->txStat = status;
	return E_NO;
}

/*****************************************************************************
 Prototype    : frame_disp_set_encode_mode
 Description  : set encode mode
 Input        : FrameDispHandle hFrameDisp  
                FrameEncMode mode           
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/9
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 frame_disp_set_encode_mode(FrameDispHandle hFrameDisp, FrameEncMode mode)
{
	if(!hFrameDisp)
		return E_INVAL;
	
	hFrameDisp->encMode = mode;
	return E_NO;
}

