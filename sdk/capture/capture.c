/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : capture.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/2/23
  Last Modified :
  Description   : Capture module
  Function List :
              capture_create
              capture_delete
  History       :
  1.Date        : 2012/2/23
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "capture.h"
#include "log.h"
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/videodev2.h>
#include <media/davinci/videohd.h>
#include "previewer.h"
#include "resize.h"
#include <media/davinci/dm365_ccdc.h>
#include <media/davinci/vpfe_capture.h>
#include <cmem.h>
#include <media/hdcam.h>
#include <pthread.h>


/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/
#define CAP_BUF_FLAG_USED		(1 << 0)

/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/

/*----------------------------------------------*
 * external routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/

/* Capture buffer allocated */
typedef struct _CapBuf {
	void			*userAddr;
	unsigned long 	phyAddr;
	Uint32			byetesUsed;
	Int16			flags;
	Int16			refCnt;				/* reference count */
}CapBuf;

/* Object for this module */
typedef struct CapObj {
	int					fdCap;			/* fd for capture */
	Bool				userAlloc;		/* buffer alloc method, user ptr or mmap */
	Bool				capStarted;		/* flag for capture started or not */
	int					bufNum;			/* num of buffers */
	Uint32				bufSize;		/* size of single buf */
	CMEM_AllocParams 	memAllocParams;	/* params for cmem alloc */
	CapBuf				*capBufs;		/* buf pool */
	Int32				capIndex;		/* cnt for capture */
	CapInputInfo		inputInfo;		/* current capture info */
	pthread_mutex_t		mutex;			/* mutex for thread safe */
	Int16				defRefCnt;		/* default reference count when buffer alloc */
}CapObj;

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
 * routines' implementations                    *
 *----------------------------------------------*/


/*****************************************************************************
 Prototype    : capture_get_input_name
 Description  : Get input name
 Input        : CaptureInput input  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/24
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline const char *capture_get_input_name(CaptureInput input)
{
	const char *name;
	
	switch(input) {
	case CAP_INPUT_CAMERA:
		name = "Camera";
		break;
	case CAP_INPUT_COMPONENT:
		name = "Component";
		break;
	case CAP_INPUT_COMPOSITE:
		name = "Composite";
		break;
	case CAP_INPUT_SVIDEO:
		name = "S-Video";
		break;
	default:
		name = NULL;
		break;
	}

	return name;
}

/*****************************************************************************
 Prototype    : capture_convert_fmt
 Description  : Convert V4L2 fmt to app fmt
 Input        : CapHandle hCap           
                struct v4l2_format *fmt  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 capture_convert_fmt(CapHandle hCap, struct v4l2_format *fmt)
{
	hCap->inputInfo.width = fmt->fmt.pix.width;
	hCap->inputInfo.height = fmt->fmt.pix.height;
	
	hCap->inputInfo.bytesPerLine = fmt->fmt.pix.bytesperline;
	hCap->inputInfo.size = fmt->fmt.pix.sizeimage;

	switch(fmt->fmt.pix.pixelformat) {
	case V4L2_PIX_FMT_SBGGR8:
		hCap->inputInfo.colorSpace = FMT_BAYER_RGBG;
		break;
	case V4L2_PIX_FMT_UYVY:
		hCap->inputInfo.colorSpace = FMT_YUV_422ILE;
		break;
	case V4L2_PIX_FMT_NV12:
		hCap->inputInfo.colorSpace = FMT_YUV_420SP;
		break;
	default:
		ERR("unsupported fmt.");
		return E_UNSUPT;
	}

	return E_NO;
	
}

/*****************************************************************************
 Prototype    : capture_get_input_res
 Description  : Get input resolution
 Input        : CapHandle hCap  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/24
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 capture_get_input_res(CapHandle hCap, CaptureStd std, struct v4l2_format *fmt)
{
	memset(fmt, 0, sizeof(struct v4l2_format));
	
	fmt->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt->fmt.pix.field = V4L2_FIELD_ANY;
	//fmt.fmt.pix.colorspace = V4L2_COLORSPACE_SRGB;

	if(std == CAP_STD_FULL_FRAME) {
		/* Set to big enough for driver to return a correct value */
		fmt->fmt.pix.width = 1 << 15;
		fmt->fmt.pix.height = 1 << 15;
	} else {
		/* Set to small enough for driver to return a correct value */
		fmt->fmt.pix.width = 1;
		fmt->fmt.pix.height = 1;
	}


	fmt->fmt.pix.pixelformat = V4L2_PIX_FMT_SBGGR8;

	/* Now get input resolution */
	if (ioctl(hCap->fdCap, VIDIOC_TRY_FMT, fmt) < 0) {
        ERRSTR("Failed VIDIOC_TRY_FMT");
        return E_IO;
    }

	
	return capture_convert_fmt(hCap, fmt);
}


#if 0
static int config_ccdc_raw(int captFd)
{
	struct ccdc_config_params_raw raw_params;
	/* Change these values for testing Gain - Offsets */
	struct ccdc_float_16 r = {0, 511};
	struct ccdc_float_16 gr = {0, 511};
	struct ccdc_float_16 gb = {0, 511};
	struct ccdc_float_16 b = {0, 511};
	struct ccdc_float_8 csc_coef_val = { 1, 0 };
	int i;

	bzero(&raw_params, sizeof(raw_params));

	/* First get the parameters */
	if (-1 == ioctl(captFd, VPFE_CMD_G_CCDC_RAW_PARAMS, &raw_params)) {
		ERR("InitDevice:ioctl:VPFE_CMD_G_CCDC_PARAMS, %p", &raw_params);
		return E_IO;
	}

	raw_params.compress.alg = CCDC_NO_COMPRESSION;
	raw_params.gain_offset.gain.r_ye = r;
	raw_params.gain_offset.gain.gr_cy = gr;
	raw_params.gain_offset.gain.gb_g = gb;
	raw_params.gain_offset.gain.b_mg = b;
	raw_params.gain_offset.gain_sdram_en = 1;
	raw_params.gain_offset.gain_ipipe_en = 1;
	raw_params.gain_offset.offset = 0;
	raw_params.gain_offset.offset_sdram_en = 1;

	/* To test linearization, set this to 1, and update the
	 * linearization table with correct data
	 */
	if (0) {
		raw_params.linearize.en = 1;
		raw_params.linearize.corr_shft = CCDC_1BIT_SHIFT;
		raw_params.linearize.scale_fact.integer = 0;
		raw_params.linearize.scale_fact.decimal = 10;

		for (i = 0; i < CCDC_LINEAR_TAB_SIZE; i++)
			raw_params.linearize.table[i] = i;
	} else {
		raw_params.linearize.en = 0;
	}

	/* CSC */
	if (0) {
		raw_params.df_csc.df_or_csc = 0;
		raw_params.df_csc.csc.en = 1;
		/* I am hardcoding this here. But this should
		 * really match with that of the capture standard
		 */
		raw_params.df_csc.start_pix = 1;
		raw_params.df_csc.num_pixels = 720;
		raw_params.df_csc.start_line = 1;
		raw_params.df_csc.num_lines = 480;
		/* These are unit test values. For real case, use
		 * correct values in this table
		 */
		raw_params.df_csc.csc.coeff[0] = csc_coef_val;
		raw_params.df_csc.csc.coeff[1].decimal = 1;
		raw_params.df_csc.csc.coeff[2].decimal = 2;
		raw_params.df_csc.csc.coeff[3].decimal = 3;
		raw_params.df_csc.csc.coeff[4].decimal = 4;
		raw_params.df_csc.csc.coeff[5].decimal = 5;
		raw_params.df_csc.csc.coeff[6].decimal = 6;
		raw_params.df_csc.csc.coeff[7].decimal = 7;
		raw_params.df_csc.csc.coeff[8].decimal = 8;
		raw_params.df_csc.csc.coeff[9].decimal = 9;
		raw_params.df_csc.csc.coeff[10].decimal = 10;
		raw_params.df_csc.csc.coeff[11].decimal = 11;
		raw_params.df_csc.csc.coeff[12].decimal = 12;
		raw_params.df_csc.csc.coeff[13].decimal = 13;
		raw_params.df_csc.csc.coeff[14].decimal = 14;
		raw_params.df_csc.csc.coeff[15].decimal = 15;

	} else {
		raw_params.df_csc.df_or_csc = 0;
		raw_params.df_csc.csc.en = 0;
	}

	/* vertical line defect correction */
	if (0) {
		raw_params.dfc.en = 1;
		// correction method
		raw_params.dfc.corr_mode = CCDC_VDFC_HORZ_INTERPOL_IF_SAT;
		// not pixels upper than the defect corrected
		raw_params.dfc.corr_whole_line = 1;
		raw_params.dfc.def_level_shift = CCDC_VDFC_SHIFT_2;
		raw_params.dfc.def_sat_level = 20;
		raw_params.dfc.num_vdefects = 7;
		for (i = 0; i < raw_params.dfc.num_vdefects; i++) {
			raw_params.dfc.table[i].pos_vert = i;
			raw_params.dfc.table[i].pos_horz = i + 1;
			raw_params.dfc.table[i].level_at_pos = i + 5;
			raw_params.dfc.table[i].level_up_pixels = i + 6;
			raw_params.dfc.table[i].level_low_pixels = i + 7;
		}
		printf("DFC enabled\n");
	} else {
		raw_params.dfc.en = 0;
	}

	if (en_culling) {

		printf("Culling enabled\n");
		raw_params.culling.hcpat_odd  = 0xaa;
		raw_params.culling.hcpat_even = 0xaa;
		raw_params.culling.vcpat = 0x55;
		raw_params.culling.en_lpf = 1;
	} else {
		raw_params.culling.hcpat_odd  = 0xFF;
		raw_params.culling.hcpat_even = 0xFF;
		raw_params.culling.vcpat = 0xFF;
	}
	raw_params.col_pat_field0.olop = CCDC_GREEN_BLUE;
	raw_params.col_pat_field0.olep = CCDC_BLUE;
	raw_params.col_pat_field0.elop = CCDC_RED;
	raw_params.col_pat_field0.elep = CCDC_GREEN_RED;
	raw_params.col_pat_field1.olop = CCDC_GREEN_BLUE;
	raw_params.col_pat_field1.olep = CCDC_BLUE;
	raw_params.col_pat_field1.elop = CCDC_RED;
	raw_params.col_pat_field1.elep = CCDC_GREEN_RED;
	raw_params.data_size = CCDC_12_BITS;
	raw_params.data_shift = CCDC_NO_SHIFT;

	printf("VPFE_CMD_S_CCDC_RAW_PARAMS, size = %d, address = %p\n", sizeof(raw_params),
				&raw_params);
	if (-1 == ioctl(captFd, VPFE_CMD_S_CCDC_RAW_PARAMS, &raw_params)) {
		printf("InitDevice:ioctl:VPFE_CMD_S_CCDC_PARAMS, %p", &raw_params);
		return -1;
	}

	return 0;
}
#endif


/*****************************************************************************
 Prototype    : capture_set_data_format
 Description  : Set capture data format
 Input        : CapHandle hCap   
                CapAttrs *attrs  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/24
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 capture_set_data_format(CapHandle hCap, CapAttrs *attrs)
{
	int ret;
	struct v4l2_input input;
	int fdCapture = hCap->fdCap;
	const char *inputName = capture_get_input_name(attrs->inputType);

	if(!inputName) {
		ERR("invalid input type");
		return E_INVAL;
	}

	/* first set the input */
	input.type = V4L2_INPUT_TYPE_CAMERA;
	input.index = 0;
	ret = E_UNSUPT;
	while (ioctl(fdCapture, VIDIOC_ENUMINPUT, &input) >= 0) {
		//DBG("input.name = %s", input.name);
		if (!strcmp((char*)input.name, inputName)) {
            DBG("Found the input %s", inputName);
			ret = E_NO;
			break;
        }
		input.index++;
	}

	if(ret) {
		ERRSTR("Can't find required input");
		return ret;
	}

	ret = input.index;
	if (ioctl (fdCapture, VIDIOC_S_INPUT, &ret) < 0) {
		ERRSTR("set input failed");
		return E_IO;
	}

	/* Get supported format */
    int i;
    struct v4l2_fmtdesc fmtdesc;

    for(i=0; ; i++) {
        memset(&fmtdesc, 0, sizeof(fmtdesc));
        fmtdesc.index = i;
	    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	    if(ioctl (fdCapture, VIDIOC_ENUM_FMT, &fmtdesc) < 0) {
            break;
	    }
        //DBG("pixelformat: %d, description: %s", fmtdesc.pixelformat, fmtdesc.description);
	}

	
	/* Get input format */
	struct v4l2_format fmt;
	
	ret = capture_get_input_res(hCap, attrs->std, &fmt);
	if(ret < 0) {
		return ret;
	}

	/* Get current format */
    memset(&fmt, 0, sizeof(fmt));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(ioctl (fdCapture, VIDIOC_G_FMT, &fmt) < 0) {
		ERRSTR("get fmt failed");
		return E_IO;
	}

	/* Set std for non-camera input */
	if(attrs->inputType != CAP_INPUT_CAMERA) {
		/* Get current std */
		Int32 failCount = 0;
		v4l2_std_id std;
		
		do {
		    DBG("Query the current standard");
		    ret = ioctl(fdCapture, VIDIOC_QUERYSTD, &std);

		    if (ret < 0 && errno == EAGAIN) {
		        usleep(1);
		        failCount++;
		        DBG("trying again ...");
		    }
		} while (ret == -1 && errno == EAGAIN && failCount < 5);


		/* Set std */
		if(ioctl(fdCapture, VIDIOC_S_STD, &std) < 0) {
			DBG("set_data_format:unable to set standard automatically");
			return E_IO;
		} else
			DBG("\nset std done");
	}

	/* Set capture out format */
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = hCap->inputInfo.width;
	fmt.fmt.pix.height = hCap->inputInfo.height;
	fmt.fmt.pix.bytesperline = hCap->inputInfo.bytesPerLine;
	fmt.fmt.pix.field = V4L2_FIELD_NONE;
	
    int pixelformat = fmt.fmt.pix.pixelformat;
	
	if (ioctl(fdCapture, VIDIOC_S_FMT, &fmt) < 0) {
		ERRSTR("set fmt failed.");
		return E_IO;
	}
	
    if (pixelformat != fmt.fmt.pix.pixelformat) {
		ERR("set fmt different");
		return E_IO;
    }

	/* Set crop */
	if (0) {
		struct v4l2_crop crop;
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (ioctl(fdCapture, VIDIOC_S_CROP, &crop) < 0) {
			ERRSTR("Set crop error");
			return E_IO;
		} else
			DBG("S_CROP Done\n");
	}

	/* Record pixel format */
	memset(&fmt, 0, sizeof(fmt));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(ioctl (fdCapture, VIDIOC_G_FMT, &fmt) < 0) {
		ERRSTR("get fmt failed");
		return E_IO;
	}

	/* Set capture mode */
	

	ret = capture_convert_fmt(hCap, &fmt);
	return ret;
}

/*****************************************************************************
 Prototype    : calc_image_size
 Description  : Calc raw image size
 Input        : CapOutAttrs *outAttrs  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/24
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline Uint32 calc_image_size(CapHandle hCap)
{
	Uint32 size = hCap->inputInfo.width * hCap->inputInfo.height;
	
	if(hCap->inputInfo.colorSpace == FMT_YUV_420SP) {
		size = size * 3 / 2;
	} else if(hCap->inputInfo.colorSpace == FMT_YUV_422ILE){
		size <<= 1;
	}
	size = ROUND_UP(size, 4096);
	return size;
}

/*****************************************************************************
 Prototype    : capture_alloc_user_buf
 Description  : Alloc user buffer
 Input        : CapHandle hCap   
                CapAttrs *attrs  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/24
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 capture_alloc_buf(CapHandle hCap, CapAttrs *attrs)
{
	Uint32 size;
	CMEM_AllocParams *allocAttrs = &hCap->memAllocParams;
	Int32 i;

	hCap->capBufs = calloc(attrs->bufNum, sizeof(CapBuf));
	if(!hCap->capBufs) {
		ERR("alloc cap bufs array failed...");
		return E_NOMEM;
	}

	allocAttrs->alignment = 256;
	allocAttrs->flags = CMEM_NONCACHED;
	allocAttrs->type = CMEM_POOL;

	size = calc_image_size(hCap);

	for(i = 0; i < attrs->bufNum; i++) {
		if(attrs->userAlloc) {
			hCap->capBufs[i].userAddr = CMEM_alloc(size, allocAttrs);
			if(!hCap->capBufs[i].userAddr) {
				ERR("Alloc buffer mem failed.");
				break;
			}
			hCap->capBufs[i].byetesUsed = size;
		} else {
			hCap->capBufs[i].userAddr = (void *)(-1);
		}
		hCap->capBufs[i].flags = 0;
	}
	
	if(i == 0)
		return E_NOMEM;

	hCap->bufNum = i;
	hCap->bufSize = size;
	
	return E_NO;
}

/*****************************************************************************
 Prototype    : capture_map_buf
 Description  : Map buffer to user space for driver alloced buf
 Input        : CapHandle hCap   
                CapAttrs *attrs  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/25
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 capture_map_buf(CapHandle hCap, CapAttrs *attrs)
{
	Int32 bufIdx;
	struct v4l2_buffer v4l2buf;
	void *virtPtr;
	
	for (bufIdx = 0; bufIdx < hCap->bufNum; bufIdx++) {
		/* Ask for information about the driver buffer */
		memset(&v4l2buf, 0, sizeof(v4l2buf));
		v4l2buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		v4l2buf.memory = V4L2_MEMORY_MMAP;
		v4l2buf.index  = bufIdx;

		if (ioctl(hCap->fdCap, VIDIOC_QUERYBUF, &v4l2buf) == -1) {
		    ERRSTR("Failed VIDIOC_QUERYBUF");
		    return E_IO;
		}


		/* Map the driver buffer to user space */
		virtPtr = mmap(NULL,
		               v4l2buf.length,
		               PROT_READ | PROT_WRITE,
		               MAP_SHARED,
		               hCap->fdCap,
		               v4l2buf.m.offset);

		if (virtPtr == MAP_FAILED) {
		    ERRSTR("Failed to mmap buffer");
		    return E_IO;
		}

		/* Initialize the Buffer with driver buffer information */
		hCap->capBufs[bufIdx].userAddr = virtPtr;
		hCap->capBufs[bufIdx].byetesUsed = v4l2buf.length;
	}

	return E_NO;
}

/*****************************************************************************
 Prototype    : capture_unmap_buf
 Description  : Unmap buffer from driver
 Input        : CapHandle hCap  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/27
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 capture_unmap_buf(CapHandle hCap)
{
	Int32 		bufIdx;
	void	 	*addr;
	Int32		ret = E_NO;

	if(!hCap->capBufs)
		return E_INVAL;
	
	for (bufIdx = 0; bufIdx < hCap->bufNum; bufIdx++) {

		addr = hCap->capBufs[bufIdx].userAddr;
		if(!addr) {
			ERR("get invalid buf handle");
			continue;
		}
		if(munmap(addr, hCap->capBufs[bufIdx].byetesUsed) < 0) {
		    ERRSTR("Failed to unmap capture buffer %d", (int)bufIdx);
		    ret = E_IO;
		}
		hCap->capBufs[bufIdx].userAddr = NULL;
	}

	return ret;
}

/*****************************************************************************
 Prototype    : capture_free_buf
 Description  : Free buf allocated
 Input        : CapHandle hCap  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/27
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 capture_free_buf(CapHandle hCap)
{
	Int32 i;

	if(!hCap->capBufs)
		return E_INVAL;

	if(!hCap->userAlloc) {
		capture_unmap_buf(hCap);
	}
	else {
		for(i = 0; i < hCap->bufNum; i++) {
			if(hCap->capBufs[i].userAddr) {
				CMEM_free(hCap->capBufs[i].userAddr, &hCap->memAllocParams);
			}
		}
	}

	free(hCap->capBufs);
	hCap->capBufs = NULL;

	return E_NO;
}


/*****************************************************************************
 Prototype    : capture_init_buf
 Description  : Init capture buffer
 Input        : CapHandle hCap   
                CapAttrs *attrs  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/24
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 capture_init_buf(CapHandle hCap, CapAttrs *attrs)
{
	struct v4l2_requestbuffers req;
	Int32 ret;

	/* Alloc buffer pool */
	ret = capture_alloc_buf(hCap, attrs);
	if(ret) {
		ERR("alloc usr buf failed");
		return ret;
	}

	memset(&req, 0, sizeof(req));
	
	req.count = hCap->bufNum;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = attrs->userAlloc ? V4L2_MEMORY_USERPTR: V4L2_MEMORY_MMAP;

	if (ioctl(hCap->fdCap, VIDIOC_REQBUFS, &req) < 0) {
		ERRSTR("request buf failed");
		return E_IO;
	} //else
		//DBG("REQBUF Done");

	if (req.count != hCap->bufNum) {
		ERR("VIDIOC_REQBUFS failed for capture");
		return E_NOMEM;
	}

	/* Map buffer to user space */
	if(!attrs->userAlloc) {
		ret = capture_map_buf(hCap, attrs);
	}

	hCap->userAlloc = attrs->userAlloc;
	hCap->defRefCnt = attrs->defRefCnt > 0 ? attrs->defRefCnt : 1;
	return ret;
}

/*****************************************************************************
 Prototype    : capture_ctrl
 Description  : Do control
 Input        : int fd        
                Uint32 ctrId  
                Int32 val     
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/2
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline Int32 capture_ctrl(int fd, Uint32 ctrlId, Int32 val)
{
	struct v4l2_control         ctrl;

	ctrl.id = ctrlId;
	ctrl.value = val;
	if (ioctl(fd, VIDIOC_S_CTRL, &ctrl) < 0) {
	    ERRSTR("set ctrl failed");
	    return E_IO;
	}

	return E_NO;
}

/*****************************************************************************
 Prototype    : capture_init
 Description  : Init capture device
 Input        : CapHandle hCap   
                CapAttrs *attrs  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/24
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 capture_init(CapHandle hCap, CapAttrs *attrs)
{
	int fdCap;
    struct v4l2_capability capability;

	hCap->fdCap = -1;

    if ((fdCap = open(attrs->devName, O_RDWR, 0)) < 0) {
        ERRSTR("open capture dev failed");
		return E_IO;
    }

    /* Is capture supported? */
    if (ioctl(fdCap, VIDIOC_QUERYCAP, &capability) < 0) {
        ERRSTR("ioctl:VIDIOC_QUERYCAP failed");
		close(fdCap);
		return E_IO;
    }

    if (!(capability.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        ERR("capture is not supported");
		close(fdCap);
		return E_IO;
    }

    /* Is MMAP-IO supported? */
    if (!(capability.capabilities & V4L2_CAP_STREAMING) && !attrs->userAlloc) {
        ERR("IO method MMAP is not supported");
		close(fdCap);
		return E_IO;
    }

	//DBG("setting data format");
	hCap->fdCap = fdCap;
	if (capture_set_data_format(hCap, attrs) < 0) {
		ERR("SetDataFormat failed");
		close(fdCap);
		return E_IO;
	}

	//DBG("initializing capture buffers");
	if (capture_init_buf(hCap, attrs) < 0) {
		DBG("InitCaptureBuffers failed");
		close(fdCap);
		return E_IO;
	}

	/* Set capture mode */
	Int32 val = (attrs->mode == CAP_MODE_TRIG) ? HDCAM_MODE_TRIG : HDCAM_MODE_CONT;
	if(capture_ctrl(hCap->fdCap, V4L2_CID_HUE, val)) {
		return E_IO;
	}

	DBG("Capture initialized");
	return E_NO;
}


/*****************************************************************************
 Prototype    : capture_create
 Description  : Create capture module and init hardware
 Input        : CapAttrs *attrs  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/23
    Author       : Sun
    Modification : Created function

*****************************************************************************/
CapHandle capture_create(CapAttrs *attrs)
{
	CapHandle hCap;

	if(!attrs)
		return NULL;

	hCap = calloc(1, sizeof(CapObj));
	if(!hCap) {
		ERR("alloc mem failed.");
		return NULL;
	}
	
	/* Open capture dev */
	Int32 ret = capture_init(hCap, attrs);
	if(ret < 0) {
		goto err_quit;
	}

	/* init lock */
	ret = pthread_mutex_init(&hCap->mutex, NULL);
	if(ret < 0) {
		ERRSTR("init mutex failed");
		goto err_quit;
	}
	
	return hCap;

err_quit:
	
	if(hCap->capBufs)
		capture_free_buf(hCap);
	
	if(hCap->fdCap > 0)
		close(hCap->fdCap);
	
	if(hCap)
		free(hCap);

	return NULL;
}

/*****************************************************************************
 Prototype    : capture_delete
 Description  : Delete capture module
 Input        : CapHandle hCapture  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/23
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 capture_delete(CapHandle hCapture)
{
	if(!hCapture)
		return E_INVAL;

	if(hCapture->capBufs)
		capture_free_buf(hCapture);

	if(hCapture->fdCap > 0)
		close(hCapture->fdCap);

	if(pthread_mutex_destroy(&hCapture->mutex) < 0) {
		/* try unlock and then destory */
		pthread_mutex_unlock(&hCapture->mutex);
		pthread_mutex_destroy(&hCapture->mutex);
	}
	
	free(hCapture);

	return E_NO;
}

/*****************************************************************************
 Prototype    : capture_start
 Description  : Start capture
 Input        : CapHandle hCap  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/27
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 capture_start(CapHandle hCap)
{
	if(!hCap || hCap->fdCap <= 0)
		return E_INVAL;

	if(hCap->capStarted) {
		WARN("capture already started...");
		return E_MODE;
	}
	
	int 				i = 0;
	int 				bufNum = hCap->bufNum;
	struct v4l2_buffer	v4l2buf;

	memset(&v4l2buf, 0, sizeof(v4l2buf));
	v4l2buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2buf.memory = hCap->userAlloc ? V4L2_MEMORY_USERPTR : V4L2_MEMORY_MMAP;
	v4l2buf.field = V4L2_FIELD_NONE;
	v4l2buf.length = hCap->bufSize;

	/* Que buffer to driver */
	for (i = 0; i < bufNum; i++) {
		v4l2buf.index = i;
		v4l2buf.m.userptr = (unsigned long)hCap->capBufs[i].userAddr;
		if(!v4l2buf.m.userptr) {
			ERR("get buf %d from pool failed", i);
			return E_NOMEM;
		}
		
		hCap->capBufs[i].flags = 0;
		if(ioctl(hCap->fdCap, VIDIOC_QBUF, &v4l2buf) < 0) {
			ERRSTR("Que buffer <%d> failed, size: %u", i, v4l2buf.length);
			return E_IO;
		}
	}
	
	/* all done, start stream */
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (ioctl(hCap->fdCap, VIDIOC_STREAMON, &type) < 0) {
		ERRSTR("Stream on failed");
		return E_IO;
	} 

	hCap->capStarted = TRUE;

	//DBG("Start stream ok~");

	return E_NO;
}


/*****************************************************************************
 Prototype    : capture_stop
 Description  : Stop capture
 Input        : CapHandle hCap  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/27
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 capture_stop(CapHandle hCap)
{
	if(!hCap || hCap->fdCap <= 0)
		return E_INVAL;

	if(!hCap->capStarted) {
		WARN("capture hasn't started yet");
		return E_MODE;
	}

	enum v4l2_buf_type 	type;

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if(ioctl(hCap->fdCap, VIDIOC_STREAMOFF, &type) < 0) {
		ERRSTR("Stream off failed");
		return E_IO;
	}

	/* Clear falgs and use count */
	Int32 i;

	for(i = 0; i < hCap->bufNum; i++) {
		hCap->capBufs[i].flags &= ~CAP_BUF_FLAG_USED;
	}

	hCap->capStarted = FALSE;

	//DBG("Capture stopped");
	
	return E_NO;
}

/*****************************************************************************
 Prototype    : capture_get_frame
 Description  : Get a new frame
 Input        : CapHandle hCap   
                Int32 flags      
                FrameInfo *info  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/27
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 capture_get_frame(CapHandle hCap, FrameBuf *frameBuf)
{
	if(!hCap || !frameBuf)
		return E_INVAL;

	if(!hCap->capStarted)
		return E_MODE;

	struct v4l2_buffer 	v4l2buf;

 	/* Set V4L2 buffer type */
    memset(&v4l2buf, 0, sizeof(v4l2buf));
    v4l2buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4l2buf.memory = hCap->userAlloc ? V4L2_MEMORY_USERPTR : V4L2_MEMORY_MMAP;

    /* Get a frame buffer with captured data */
    if(ioctl(hCap->fdCap, VIDIOC_DQBUF, &v4l2buf) < 0) {
        ERRSTR("deque buf failed");
        return E_IO;
    }

	/* Get corresponding buffer from pool */
	if(v4l2buf.index >= hCap->bufNum) {
		ERR("index out of range.");
		ioctl(hCap->fdCap, VIDIOC_QBUF, &v4l2buf);
		return E_IO;
	}

	if( hCap->userAlloc && 
		(unsigned long)hCap->capBufs[v4l2buf.index].userAddr != v4l2buf.m.userptr ) {
		ERR("ptr not the same");
		ioctl(hCap->fdCap, VIDIOC_QBUF, &v4l2buf);
		return E_IO;
	}
	 
	/* Fill frameBuf info */
	frameBuf->index = ++(hCap->capIndex);
	frameBuf->timeStamp = v4l2buf.timestamp;
	frameBuf->dataBuf = (void *)v4l2buf.m.userptr;
	frameBuf->bufSize = hCap->bufSize;
	frameBuf->bytesUsed = v4l2buf.bytesused;
	frameBuf->private = v4l2buf.index; //Record index 
	frameBuf->reserved = 0;

	/* Set reference cnt */
	hCap->capBufs[v4l2buf.index].refCnt = hCap->defRefCnt;
    
    return E_NO;
}

/*****************************************************************************
 Prototype    : capture_set_def_frame_ref_cnt
 Description  : set default reference count
 Input        : CapHandle hCap   
                Int32 defRefCnt  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/16
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 capture_set_def_frame_ref_cnt(CapHandle hCap, Int32 defRefCnt)
{
	if(!hCap)
		return E_INVAL;
	
	if(hCap->capStarted) {
		ERR("default ref cnt can only set when capture is stopped.");
		return E_MODE;
	}

	hCap->defRefCnt = defRefCnt;

	return E_NO;	
}

/*****************************************************************************
 Prototype    : capture_find_frame
 Description  : find frame in pool
 Input        : CapHandle hCap            
                const FrameBuf *frameBuf  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/16
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline Int32 capture_find_frame(CapHandle hCap, const FrameBuf *frameBuf)
{
	Uint32 index = frameBuf->private;

	/* Validate the buffer */
	if( hCap->capBufs[index].userAddr != frameBuf->dataBuf ||
		index >= hCap->bufNum ) {

		DBG("search cap buf in pool.");

		/* Look through the pool */
		for(index = 0; index < hCap->bufNum; index++){
			if(hCap->capBufs[index].userAddr == frameBuf->dataBuf)
				break;
		}
		
		if(index >= hCap->bufNum) {
			ERR("invalid buffer handle, not belong to capture module");
			return E_NOTEXIST;
		}
	}

	return index;
}

/*****************************************************************************
 Prototype    : capture_inc_frame_ref
 Description  : increase frame reference count
 Input        : CapHandle hCap            
                const FrameBuf *frameBuf  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/16
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 capture_inc_frame_ref(CapHandle hCap, const FrameBuf *frameBuf) 
{
	if(!hCap || !frameBuf)
		return E_INVAL;
	
	/* find frame in pool */
	Int32 index = capture_find_frame(hCap, frameBuf);
	if(index < 0) 
		return index;

	if(hCap->capBufs[index].refCnt <= 0) {
		WARN("this buffer has been released.");
		return E_INVAL;
	}

	pthread_mutex_lock(&hCap->mutex);
	hCap->capBufs[index].refCnt++;
	pthread_mutex_unlock(&hCap->mutex);

	return E_NO;
}

/*****************************************************************************
 Prototype    : capture_free_frame
 Description  : Free a frame to buffer
 Input        : CapHandle hCap  
                BufHandle hBuf  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/27
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 capture_free_frame(CapHandle hCap, FrameBuf *frameBuf)
{
	if(!hCap || !frameBuf)
		return E_INVAL;

	if(!hCap->capStarted)
		return E_MODE;

	/* find frame in pool */
	Int32 index = capture_find_frame(hCap, frameBuf);
	if(index < 0) 
		return index;

	/* this must be threads-safe */
	hCap->capBufs[index].refCnt--;
	

	/* if reference count > 0, we should not free */
	if(hCap->capBufs[index].refCnt > 0) {
		return E_NO;
	}

	//DBG("cap free frame: %d", index);
	/* Fill v4l2 buffer */
	struct v4l2_buffer 	v4l2buf;
	memset(&v4l2buf, 0, sizeof(v4l2buf));
	v4l2buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2buf.memory = hCap->userAlloc ? V4L2_MEMORY_USERPTR : V4L2_MEMORY_MMAP;
	v4l2buf.field = V4L2_FIELD_NONE;
	v4l2buf.length = hCap->capBufs[index].byetesUsed;
	v4l2buf.index = index;
	v4l2buf.m.userptr = (unsigned long)hCap->capBufs[index].userAddr;

    /* Issue captured frame buffer back to device driver */
	//DBG("que buf");
    if(ioctl(hCap->fdCap, VIDIOC_QBUF, &v4l2buf) < 0) {
        ERRSTR("que buf failed");
        return E_IO;
    }
	//DBG("que buf done");
	return E_NO;
}

/*****************************************************************************
 Prototype    : capture_get_fd
 Description  : Get capture fd for select, etc...
 Input        : CapHandle hCap  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 capture_get_fd(CapHandle hCap)
{	
	if(!hCap)
		return E_INVAL;

	return hCap->fdCap;
}

/*****************************************************************************
 Prototype    : capture_get_input_info
 Description  : Get input info
 Input        : CapHandle hCap      
                CapInputInfo *info  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 capture_get_input_info(CapHandle hCap, CapInputInfo *info)
{
	if(!hCap || !info) {
		return E_INVAL;
	}

	*info = hCap->inputInfo;
	return E_NO;
}

/*****************************************************************************
 Prototype    : capture_config
 Description  : Change capture attrs at run time
 Input        : CapHandle hCap    
                CaptureStd std    
                CaptureMode mode  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/2
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 capture_config(CapHandle hCap, CaptureStd std, CaptureMode mode)
{
	if(!hCap)
		return E_INVAL;

	if(hCap->capStarted) {
		ERR("Capture std can only be changed when capture is stopped.");
		return E_BUSY;
	}

	/* Set capture resolution */
	struct v4l2_format fmt;
	Int32 err = capture_get_input_res(hCap, std, &fmt);
	if(err)
		return err;

	Uint32 pixelformat = fmt.fmt.pix.pixelformat;
	
	if (ioctl(hCap->fdCap, VIDIOC_S_FMT, &fmt) < 0) {
		ERRSTR("set fmt failed.");
		return E_IO;
	}
	
    if (pixelformat != fmt.fmt.pix.pixelformat) {
		ERR("set fmt different");
		return E_IO;
    }
	

	/* Set capture mode */
	Int32 val = (mode == CAP_MODE_TRIG) ? HDCAM_MODE_TRIG : HDCAM_MODE_CONT;
	err = capture_ctrl(hCap->fdCap, V4L2_CID_HUE, val);

	return err;
}

