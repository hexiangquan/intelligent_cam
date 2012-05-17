/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : previewer.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/2/24
  Last Modified :
  Description   : previewer control module
  Function List :
              previewer_init
              previewer_set_gamma
              previewer_set_lum
              previewer_set_nf
              previewer_set_wb
              previewer_update
  History       :
  1.Date        : 2012/2/24
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "common.h"
#include "previewer.h"
#include <linux/videodev.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <media/davinci/videohd.h>
#include <media/davinci/dm365_ccdc.h>
#include <media/davinci/vpfe_capture.h>
#include <media/davinci/imp_previewer.h>
#include <media/davinci/imp_resizer.h>
#include <media/davinci/dm365_ipipe.h>
#include <media/davinci/imp_common.h>
#include <termios.h>
#include <unistd.h>
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
#define YEE_LUT_MAX_SIZE 1024

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/*****************************************************************************
 Prototype    : pix_format_convert
 Description  : convert from app color space to ipipe defined
 Input        : ChromaFormat cs  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/24
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline enum ipipe_pix_formats pix_format_convert(ChromaFormat cs)
{
	switch(cs) {
		case FMT_YUV_420SP:
			return IPIPE_YUV420SP;
		case FMT_YUV_422ILE:
			return IPIPE_UYVY;
		case FMT_BAYER_RGBG:
			return IPIPE_BAYER_8BIT_PACK;
		default:
			return E_INVAL;
	};
}

/*****************************************************************************
 Prototype    : previewer_ss_config
 Description  : Set previewer single shot params
 Input        : int fd               
                PreviewAttrs *attrs  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/1
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 previewer_ss_config(int fd, PreviewAttrs *attrs)
{
	struct prev_channel_config prev_chan_config;
	struct prev_single_shot_config prev_ss_config;
		
	prev_chan_config.oper_mode = IMP_MODE_SINGLE_SHOT;
	prev_chan_config.len = sizeof(struct prev_single_shot_config);
	prev_chan_config.config = &prev_ss_config;

	/* Get current config */
	if (ioctl(fd, PREV_G_CONFIG, &prev_chan_config) < 0) {
		ERRSTR("Error in getting configuration from driver");
		return E_IO;
	}
	
	prev_chan_config.oper_mode = IMP_MODE_SINGLE_SHOT;
	prev_chan_config.len = sizeof(struct prev_single_shot_config);
	prev_chan_config.config = &prev_ss_config;

	prev_ss_config.bypass = IPIPE_BYPASS_OFF;
	prev_ss_config.input.image_width = attrs->inputWidth;
	prev_ss_config.input.image_height = attrs->inputHeight;
	prev_ss_config.input.line_length = attrs->inputPitch;
    prev_ss_config.input.hst = attrs->inputHStart;
    prev_ss_config.input.vst = attrs->inputVStart;
	prev_ss_config.input.ppln = attrs->inputWidth + 8;
	prev_ss_config.input.lpfr = attrs->inputHeight + 10;
	prev_ss_config.input.pix_fmt = pix_format_convert(attrs->inputFmt);
	prev_ss_config.input.gain = attrs->digiGain;
	prev_ss_config.input.avg_filter_en = (attrs->ctrlFlags & CONV_FLAG_AVG_EN) ? 1 : 0;
	
        prev_ss_config.input.frame_div_mode_en = 0;

	if(prev_ss_config.input.pix_fmt == IPIPE_BAYER) {
		prev_ss_config.input.data_shift = IPIPEIF_5_1_BITS11_0;
	}
	
	if(ioctl(fd, PREV_S_CONFIG, &prev_chan_config) < 0) {
		ERRSTR("Error in setting default configuration");
		return E_IO;
	}

	return 0;
}

/*****************************************************************************
 Prototype    : previewer_init
 Description  : Init previewer
 Input        : Int32 user_mode         
                Bool setRgbColorPallet  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/24
    Author       : Sun
    Modification : Created function

*****************************************************************************/
int previewer_init(const char *name, Bool onTheFly, PreviewAttrs *attrs)
{
    int preview_fd, err = E_NO;
    unsigned int user_mode, oper_mode;
    struct prev_channel_config prev_chan_config;
   
    struct prev_cap cap;
    struct prev_module_param mod_param;
    
    preview_fd = open((const char *)PREVIEWER_DEVICE, O_RDWR);
    if(preview_fd <= 0) {
        ERRSTR("Cannot open previewer device");
        return E_IO;
    }

	user_mode = onTheFly ? IMP_MODE_CONTINUOUS : IMP_MODE_SINGLE_SHOT;
    if (ioctl(preview_fd,PREV_S_OPER_MODE, &user_mode) < 0) {
        ERRSTR("Can't set operation mode in previewer");
        err = E_IO;
		goto err_quit;
    }

    if (ioctl(preview_fd,PREV_G_OPER_MODE, &oper_mode) < 0) {
        ERRSTR("Can't get operation mode from previewer");
        err = E_IO;
		goto err_quit;
    }

    if (oper_mode == user_mode) {
        DBG("Operating mode changed successfully in previewer");
    } else {
        ERRSTR("failed to set mode to continuous in previewer");
       	err = E_IO;
		goto err_quit;
    }

    prev_chan_config.oper_mode = oper_mode;
    prev_chan_config.len = 0;
    prev_chan_config.config = NULL; /* to set defaults in driver */

    if (ioctl(preview_fd, PREV_S_CONFIG, &prev_chan_config) < 0) {
        ERRSTR("Error in setting default previewer configuration");
        err = E_IO;
		goto err_quit;
    }

	/* Set default params */
	cap.index=0;
    while (1) {
        if (ioctl(preview_fd , PREV_ENUM_CAP, &cap) < 0) {
            break;
        }

        strcpy(mod_param.version, cap.version);
        mod_param.module_id = cap.module_id;

       	/* using defaults */
        //DBG("Setting default for %s", cap.module_name);
        mod_param.param = NULL;
        if(ioctl(preview_fd, PREV_S_PARAM, &mod_param) < 0) {
            ERR("Error in Setting %s params from driver", cap.module_name);
            err = E_IO;
			goto err_quit;
        }
        cap.index++;
    }

    if (onTheFly && attrs->setRgbColorPallet) {

		struct prev_continuous_config prev_cont_config;
		 
        DBG("Setting RGB color pallet");
        prev_chan_config.oper_mode = oper_mode;
        prev_chan_config.len = sizeof(struct prev_continuous_config);
        prev_chan_config.config = &prev_cont_config; /* to set defaults in driver */

        if (ioctl(preview_fd, PREV_G_CONFIG, &prev_chan_config) < 0) {
            ERRSTR("Error in setting default previewer configuration ");
           	err = E_IO;
			goto err_quit;
        }

        prev_chan_config.oper_mode = oper_mode;
        prev_chan_config.len = sizeof(struct prev_continuous_config);
        prev_chan_config.config = &prev_cont_config; /* to set defaults in driver */

        prev_cont_config.input.colp_elep= IPIPE_BLUE;
        prev_cont_config.input.colp_elop= IPIPE_GREEN_BLUE;
        prev_cont_config.input.colp_olep= IPIPE_GREEN_RED;
        prev_cont_config.input.colp_olop= IPIPE_RED;

        if (ioctl(preview_fd, PREV_S_CONFIG, &prev_chan_config) < 0) {
            ERRSTR("Error in setting default previewer configuration");
           	err = E_IO;
			goto err_quit;
        }
    }

	if(!onTheFly) {
		err = previewer_ss_config(preview_fd, attrs);
		if(err) {
			goto err_quit;
		}
	}

    DBG("Previewer initialized");
    return preview_fd;
	
err_quit:
	if(preview_fd > 0)
		close(preview_fd);

	return err;

}

/*****************************************************************************
 Prototype    : previewer_set_wb
 Description  : Set white balance params
 Input        : struct prev_module_param *modParam  
                struct prev_wb *prevWb              
                const Int32 *wbTab                  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/24
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 previewer_set_wb(int fd, struct prev_module_param *modParam, struct prev_wb *prevWb, const Int32 *wbTab)
{
#if 0
	prevWb->gain_r.integer = U4_9_INT(wbTab[0]);
	prevWb->gain_r.decimal = U4_9_DEC(wbTab[0]);
	prevWb->gain_gr.integer = U4_9_INT(wbTab[1]);
	prevWb->gain_gr.decimal = U4_9_DEC(wbTab[1]);
	prevWb->gain_gb.integer = U4_9_INT(wbTab[2]);
	prevWb->gain_gb.decimal = U4_9_DEC(wbTab[2]);
	prevWb->gain_b.integer = U4_9_INT(wbTab[3]);
	prevWb->gain_b.decimal = U4_9_DEC(wbTab[3]);
	prevWb->ofst_r = wbTab[4];
	prevWb->ofst_gr = wbTab[5];
	prevWb->ofst_gb = wbTab[6];
	prevWb->ofst_b = wbTab[7];
	
	modParam->len = sizeof(struct prev_wb);
	modParam->param = &wb;


	/* Get current value */
	if (ioctl(fd, PREV_G_PARAM, modParam) < 0) {
		ERRSTR("Error in Setting %s params from driver", modParam->version);
		return E_IO;
	}
		
	prevWb->gain_r.integer = 2;
	prevWb->gain_r.decimal = 0;
	prevWb->gain_b.integer = 2;
	prevWb->gain_b.decimal = 0;
	prevWb->gain_gr.integer = 2;
	prevWb->gain_gr.decimal = 0;
	prevWb->gain_gb.integer = 2;
	prevWb->gain_gb.decimal = 0;
#else

	modParam->len = 0;
	modParam->param = NULL;
#endif

	return E_NO;
}

/*****************************************************************************
 Prototype    : previewer_set_gamma
 Description  : set gamma params
 Input        : struct prev_module_param *modParam  
                struct prev_gamma *gamma            
                Bool enable                         
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/24
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 previewer_set_gamma(struct prev_module_param *modParam, struct prev_gamma *gamma, Bool bypass)
{
	bzero(gamma, sizeof(struct prev_gamma));
	gamma->bypass_r = bypass;
	gamma->bypass_b = bypass;
	gamma->bypass_g = bypass;
	gamma->tbl_sel = IPIPE_GAMMA_TBL_ROM;
	modParam->len = sizeof (struct prev_gamma);
	modParam->param = gamma;
	return E_NO;
}

/*****************************************************************************
 Prototype    : previewer_set_nf
 Description  : Set noise filter params
 Input        : struct prev_module_param *modParam  
                struct prev_nf *nf                  
                Bool enable                         
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/24
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 previewer_set_nf(struct prev_module_param *modParam, struct prev_nf *nf, Bool enable)
{
	struct prev_nf def_nf_params = {
		.en = 1,
		.gr_sample_meth = IPIPE_NF_BOX,
		.shft_val = 0,
		.spread_val = 3,
		.apply_lsc_gain = 0,
		.thr = { 120, 130, 135, 140, 150, 160, 170, 200 },
		.str = { 16, 16, 15, 15, 15, 15, 15, 15 },
		.edge_det_min_thr = 0,
		.edge_det_max_thr = 2047
	};
	
	if(enable) {
		*nf = def_nf_params;
	} else {
		memset(nf, 0, sizeof(*nf));
	}

	modParam->param = nf;
	modParam->len = sizeof (struct prev_nf);
	return E_NO;
}

/*****************************************************************************
 Prototype    : previewer_set_lum
 Description  : Set luma adjust params
 Input        : struct prev_module_param *modParam  
                struct prev_lum_adj *lum            
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
static Int32 previewer_set_lum(struct prev_module_param *modParam, struct prev_lum_adj *lum, PreviewAttrs *attrs)
{
	if(attrs->ctrlFlags & CONV_FLAG_LUMA_EN) {
		lum->brightness = attrs->brightness;
		lum->contrast = attrs->contrast;
		modParam->param = lum;
		modParam->len = sizeof (struct prev_lum_adj);
	} else {
		modParam->param = NULL;
		modParam->len = 0;
	}

	return E_NO;
}

#define YEE_TAB_MAX		24
#define YEE_TAB_MIN		(-12)
/*****************************************************************************
 Prototype    : previewer_init_yee_table
 Description  : init default YEE table 
 Input        : None
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/1
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static short *previewer_init_yee_table()
{
	short *yeeTab = malloc(YEE_LUT_MAX_SIZE * sizeof(short));

	if(!yeeTab)
		return NULL;

	Int32 i, j = 0;
	Int32 offset[] = {
		21, 28, 39, 49, 60, 70, 81, 92, 102, 113, 124, 140, 384, 400,
		411, 422, 432, 443, 454, 464, 475, 486, 497, 512, 513, 525,
		532, 538, 544, 549, 555, 560, 565, 571, 576, 581, 587, 592,
		597, 603, 608, 613, 619, 624, 630, 636, 643, 655, 891, 910,
		916, 922, 927, 933, 938, 943, 949, 954, 959, 965, 970, 975, 
		981, 986, 992, 998, 1005, 1012, 1024};
	
	short value = 0, step = -1;
	
	/* init default value */
	for(i = 0; i < ARRAY_SIZE(offset); i++) {
		for(; j < offset[i]; j++)
			yeeTab[j] = value;

		if(value <= YEE_TAB_MIN)
			step = 1;
		else if(value >= YEE_TAB_MAX)
			step = -1;

		value += step;
	}

	return yeeTab;
	
}

/*****************************************************************************
 Prototype    : previewer_set_yee
 Description  : Set YEE params
 Input        : struct prev_module_param *modParam  
                struct prev_yee *yee                
                short *yeeTab                       
                PreviewAttrs *attrs                 
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/1
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 previewer_set_yee(struct prev_module_param *modParam, struct prev_yee *yee, short *yeeTab, PreviewAttrs *attrs)
{
	struct prev_yee yee_params_def = {
		.en = 1,
		.en_halo_red = 0,
		.merge_meth = IPIPE_YEE_EE_ES,
		.hpf_shft = 10,
		.hpf_coef_00 = 84,
		.hpf_coef_01 = (-8 & 0x3FF),
		.hpf_coef_02 = (-4 & 0x3FF),
		.hpf_coef_10 = (-8 & 0x3FF),
		.hpf_coef_11 = (-4 & 0x3FF),
		.hpf_coef_12 = (-2 & 0x3FF),
		.hpf_coef_20 = (-4 & 0x3FF),
		.hpf_coef_21 = (-2 & 0x3FF),
		.hpf_coef_22 = (-1 & 0x3FF),
		.yee_thr = 20,
		.es_gain = 128,
		.es_thr1 = 768,
		.es_thr2 = 32,
		.es_gain_grad = 32,
		.es_ofst_grad = 0, 
	};

	if((attrs->ctrlFlags & CONV_FLAG_EE_EN) && attrs->eeTable) {
		*yee = yee_params_def;
		yee->table = yeeTab ? yeeTab : attrs->eeTable;
		modParam->param = yee;
		modParam->len = sizeof (struct prev_yee);
	} else {
		modParam->param = NULL;
		modParam->len = 0;
	}

	return E_NO;
}

/*****************************************************************************
 Prototype    : ipipe_update
 Description  : Update ipipe params
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
Int32 previewer_cap_update(int fdPrev, PreviewAttrs *attrs)
{
	int  ret = E_NO;
	struct prev_cap cap;
	struct prev_module_param modParam;
	struct prev_gamma gamma;
	struct prev_lum_adj lum;
	struct prev_nf nf;
	struct prev_wb wb;
	struct prev_yee yee;
	short *yeeTab = NULL;
	Bool bypass;

	if(fdPrev <= 0) {
		ERR("previewer device has not opened");
		return E_MODE;
	}

	cap.index=0;
	while (1) {
		ret = ioctl(fdPrev , PREV_ENUM_CAP, &cap);
		if (ret < 0) {
			ret = E_NO;
			break;
		}
		
		/* find the defaults for this module */
		strcpy(modParam.version, cap.version);
		modParam.module_id = cap.module_id;

		/* try set parameter for this module */
		switch(cap.module_id) {
		case PREV_NF1:
		case PREV_NF2:
			previewer_set_nf(&modParam, &nf, attrs->ctrlFlags & CONV_FLAG_NF_EN);
			break;
		case PREV_WB:
			previewer_set_wb(fdPrev, &modParam, &wb, NULL);
			break;
		case PREV_GAMMA:
			bypass = (attrs->ctrlFlags & CONV_FLAG_GAMMA_EN) ? 0 : 1;
			previewer_set_gamma(&modParam, &gamma, bypass);
			break;
		case PREV_LUM_ADJ:
			previewer_set_lum(&modParam, &lum, attrs);
			break;
		case PREV_YEE:
			if(attrs->ctrlFlags & CONV_FLAG_EE_EN) {
				if(!attrs->eeTable || attrs->eeTabSize < YEE_LUT_MAX_SIZE) {
					/* Use default table */
					//yeeTab = previewer_init_yee_table();
					attrs->eeTable = yeeTab;
				}
				previewer_set_yee(&modParam, &yee, yeeTab, attrs);
			} else {
				modParam.param = NULL;
			}
			break;
		default:
			/* using defaults */
			modParam.param = NULL;
			modParam.len = 0;
			break;	
		}

		/* Set params to driver */
		if (ioctl(fdPrev, PREV_S_PARAM, &modParam) < 0) {
			ERRSTR("Error in Setting %s params from driver", cap.module_name);
			ret = E_IO;
			break;
		}
		
		cap.index++;
	}

	if(yeeTab)
		free(yeeTab);

	return ret;
}

/*****************************************************************************
 Prototype    : previewer_convert
 Description  : Run convert
 Input        : int fd            
                void *inBuf       
                Int32 inBufSize   
                void *outBuf      
                Int32 outBufSize  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 previewer_convert(int fd, AlgBuf *inBuf, AlgBuf *outBuf)
{
	struct imp_convert convert;

	memset(&convert, 0, sizeof(convert));

	convert.in_buff.buf_type = IMP_BUF_IN;
	convert.in_buff.index = -1; 	/* user ptr */
	convert.in_buff.offset = (unsigned long)inBuf->buf;
	convert.in_buff.size = inBuf->bufSize;
	convert.out_buff1.buf_type = IMP_BUF_OUT1;
	convert.out_buff1.index = -1;	/* user ptr */
	convert.out_buff1.offset = (unsigned long)outBuf->buf;
	convert.out_buff1.size = outBuf->bufSize;

	if(ioctl(fd, PREV_PREVIEW, &convert) < 0) {
	//if(ioctl(fd, RSZ_RESIZE, &convert) < 0) {
		ERRSTR("Error in doing preview");
		return E_IO;
	}

	return E_NO;
}

