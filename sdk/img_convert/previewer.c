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
#define _ISOC99_SOURCE
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
#include <math.h>
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
#define YEE_LUT_MAX_SIZE 	1024
#define GAMMA_TBL_MAX_SIZE	512
#define RGB2YUV_Y_ADJ(x)	((x)-128)
#define CONTRAST_ADJ(x)		((x)>>3)
#define DIGITAL_GAIN_ADJ(x)	((x)+256)

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
	prev_ss_config.input.gain = DIGITAL_GAIN_ADJ(attrs->digiGain);
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

    if (ioctl(preview_fd, PREV_G_OPER_MODE, &oper_mode) < 0) {
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

        prev_cont_config.input.colp_elep = IPIPE_BLUE;
        prev_cont_config.input.colp_elop = IPIPE_GREEN_BLUE;
        prev_cont_config.input.colp_olep = IPIPE_GREEN_RED;
        prev_cont_config.input.colp_olop = IPIPE_RED;

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
 Prototype    : gamma_lut_calc
 Description  : calc LUT for single entry
 Input        : double index  
                double exp    
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/7/24
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static unsigned short gamma_lut_calc(double index, double exp)
{
	double lut;

	index = index / 512.0;

	if(index <= 0.018)
		lut = exp * 10.0 * index;
	else
		lut = 1.099 * pow(index, exp) - 0.099;

	lut *= 1024.0;

	return round(lut);
}


/*****************************************************************************
 Prototype    : gamma_table_generate
 Description  : generate the gamma table
 Input        : struct prev_gamma *params  
                Uint16 gamma               
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/7/24
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 gamma_table_generate(struct prev_gamma *params, Uint16 gamma)
{
	static struct ipipe_gamma_entry gamma_table[GAMMA_TBL_MAX_SIZE];

	if(gamma == 0) {
		params->tbl_sel = IPIPE_GAMMA_TBL_ROM;
		DBG("using gamma tabl in ROM.");
		return E_NO;
	}

	/* using table in ram */
	double exp = gamma;	
	Int32 i;
	Uint16 lut;

	/* gamma is 100 times of actual value, need divided by 1 for calc */
	exp = 1.0/(exp/100.0);
	gamma_table[0].offset = 0;
	
	for(i = 1; i < GAMMA_TBL_MAX_SIZE; ++i) {
		lut = gamma_lut_calc(i, exp);
		gamma_table[i].offset = lut;
		gamma_table[i - 1].slope = lut - gamma_table[i - 1].offset;
	}

	/* calc slope for last entry */
	i = GAMMA_TBL_MAX_SIZE - 1;
	gamma_table[i].slope = gamma_lut_calc(i + 1, exp) - gamma_table[i].offset;

	params->tbl_sel = IPIPE_GAMMA_TBL_RAM;
	params->tbl_size = IPIPE_GAMMA_TBL_SZ_512;
	params->table_r = gamma_table;
	params->table_g = gamma_table;
	params->table_b = gamma_table;

#ifdef CONV_DBG
		DBG("output gamma table to gamma.txt");
		FILE *fp = fopen("gamma.txt", "w");
		if(!fp) {
			ERRSTR("open gamma.txt failed.");
			return E_IO;
		}

		for(i = 0; i < GAMMA_TBL_MAX_SIZE; ++i) {
			fprintf(fp, "%d\t\t%d\n", gamma_table[i].offset, gamma_table[i].slope);
		}

		fclose(fp);
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
static Int32 previewer_set_gamma(struct prev_module_param *modParam, struct prev_gamma *params, Bool bypass, Uint16 gamma)
{
	memset(params, 0, sizeof(struct prev_gamma));
	params->bypass_r = bypass;
	params->bypass_b = bypass;
	params->bypass_g = bypass;
	params->tbl_sel = IPIPE_GAMMA_TBL_ROM;
	
	if(!bypass)
		gamma_table_generate(params, gamma);
	
	modParam->len = sizeof(struct prev_gamma);
	modParam->param = params;
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
		.thr = { 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023,},
		//120, 130, 135, 140, 150, 160, 170, 200 },
		.str = { 31, 31, 31, 31, 31, 31, 31, 31,},
		//16, 16, 15, 15, 15, 15, 15, 15 },
		.edge_det_min_thr = 0,
		.edge_det_max_thr = 2047
	};
	
	if(enable) {
		*nf = def_nf_params;
		modParam->param = nf;
		modParam->len = sizeof (struct prev_nf);
	} else {
		modParam->param = NULL;
		modParam->len = 0;
	}
	
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
	if(attrs->ctrlFlags & CONV_FLAG_CONTRAST_EN) {
		DBG("set contrast: %u, hw value: %u", attrs->contrast, CONTRAST_ADJ(attrs->contrast));
		lum->brightness = 0;
		lum->contrast = CONTRAST_ADJ(attrs->contrast);
		modParam->param = lum;
		modParam->len = sizeof (struct prev_lum_adj);
	} else {
		modParam->param = NULL;
		modParam->len = 0;
	}

	return E_NO;
}

/**
 * previewer_set_rgb2rgb -- adjust saturation by setting RGB2RGB coefficient
 */
static Int32 previewer_set_rgb2rgb(struct prev_module_param *modParam, struct prev_rgb2rgb *rgb2rgb, PreviewAttrs *attrs)
{
	if(attrs->saturation != 128) {
		DBG("set saturation: %u", attrs->saturation);

		/* calculate coefficient */
		int k = attrs->saturation * 1024 / 128;
		int coef;
		int mask = 0x0F;

		/* calculate RGB table */
		memset(rgb2rgb, 0, sizeof(*rgb2rgb));
		coef = (717 *(k - 1024)*256)/(1024*1024);
		rgb2rgb->coef_rr.integer = (coef >> 8) & mask;
		rgb2rgb->coef_rr.decimal = coef & 0xFF;

		coef = (601 *(1024 - k)*256)/(1024*1024);
		rgb2rgb->coef_gr.integer = (coef >> 8) & mask;
		rgb2rgb->coef_gr.decimal = coef & 0xFF;

		coef = (117 *(1024 - k)*256)/(1024*1024);
		rgb2rgb->coef_br.integer = (coef >> 8) & mask;
		rgb2rgb->coef_br.decimal = coef & 0xFF;

		coef = (601 *(1024 - k)*256)/(1024*1024);
		rgb2rgb->coef_rg.integer = (coef >> 8) & mask;
		rgb2rgb->coef_rg.decimal = coef & 0xFF;

		coef = (717 *(k - 1024)*256)/(1024*1024);
		rgb2rgb->coef_gg.integer = (coef >> 8) & mask;
		rgb2rgb->coef_gg.decimal = coef & 0xFF;

		coef = (117 *(1024 - k)*256)/(1024*1024);
		rgb2rgb->coef_bg.integer = (coef >> 8) & mask;
		rgb2rgb->coef_bg.decimal = coef & 0xFF;

		coef = (306 *(1024 - k)*256)/(1024*1024);
		rgb2rgb->coef_rb.integer = (coef >> 8) & mask;
		rgb2rgb->coef_rb.decimal = coef & 0xFF;

		coef = (311 *(1024 - k)*256)/(1024*1024);
		rgb2rgb->coef_gb.integer = (coef >> 8) & mask;
		rgb2rgb->coef_gb.decimal = coef & 0xFF;

		coef = (717 *(k - 1024)*256)/(1024*1024);
		rgb2rgb->coef_bb.integer = (coef >> 8) & mask;
		rgb2rgb->coef_bb.decimal = coef & 0xFF;
		
		modParam->param = rgb2rgb;
		modParam->len = sizeof (struct prev_rgb2rgb);
	} else {
		modParam->param = NULL;
		modParam->len = 0;
	}

	return E_NO;
}

/*****************************************************************************
 Prototype    : previewer_set_y_offset
 Description  : set y offset for brightness adjust
 Input        : int fd                              
                struct prev_module_param *modParam  
                struct prev_rgb2yuv *rgb2yuv        
                PreviewAttrs *attrs                 
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/17
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 previewer_set_y_offset(int fd, struct prev_module_param *modParam, struct prev_rgb2yuv *rgb2yuv, PreviewAttrs *attrs)
{
	Int32 err = 0;
	const struct prev_rgb2yuv rgb2yuvDefault = {
		.coef_ry = {0, 0x4D},
		.coef_gy = {0, 0x96},
		.coef_by = {0, 0x1D},
		.coef_rcb = {0xF, 0xD5},
		.coef_gcb = {0xF, 0xAB},
		.coef_bcb = {0, 0x80},
		.coef_rcr = {0, 0x80},
		.coef_gcr = {0xF, 0x95},
		.coef_bcr = {0xF, 0xEB},
		.out_ofst_y = 0,
		.out_ofst_cb = 0x80,
		.out_ofst_cr = 0x80
	};
	
	if(attrs->ctrlFlags & CONV_FLAG_LUMA_EN) {
		DBG("set brightness %u, hw value: %u", attrs->brightness, RGB2YUV_Y_ADJ(attrs->brightness));
		/* get current params */
		*rgb2yuv = rgb2yuvDefault;
		modParam->param = rgb2yuv;
		modParam->len = sizeof(*rgb2yuv);
		//err = ioctl(fd, PREV_G_PARAM, modParam);
		if(err == 0) {
			rgb2yuv->out_ofst_y = RGB2YUV_Y_ADJ(attrs->brightness);
		} else {
			ERRSTR("get rgb2yuv prams failed");
		}
	} 

	if(err < 0) {
		modParam->param = NULL;
		modParam->len = 0;
	}

	return E_NO;
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
static Int32 previewer_set_yee(struct prev_module_param *modParam, struct prev_yee *yee, PreviewAttrs *attrs)
{
	const short yee_table[] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1, -1, -1, -1, -1, 
		-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -4, -4, 
		-4, -4, -4, -4, -4, -4, -4, -4, -4, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -6, -6, -6, -6, 
		-6, -6, -6, -6, -6, -6, -6, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -8, -8, -8, -8, -8, 
		-8, -8, -8, -8, -8, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -10, -10, -10, -10, -10, -10,
		-10, -10, -10, -10, -10, -11, -11, -11, -11, -11, -11, -11, -11, -11, -11, -11, -11, -11, 
		-11, -11, -11, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, 
		-12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, 
		-12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, 
		-12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, 
		-12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, 
		-12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, 
		-12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, 
		-12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, 
		-12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, 
		-12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, 
		-12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, 
		-12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, 
		-12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, 
		-12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -11, -11, -11, -11, -11, 
		-11, -11, -11, -11, -11, -11, -11, -11, -11, -11, -11, -10, -10, -10, -10, -10, -10, -10, 
		-10, -10, -10, -10, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -8, -8, -8, -8, -8, -8, -8,
		-8, -8, -8, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -6, -6, -6, -6, -6, -6, -6, -6, -6,
		-6, -6, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4,
		-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -1, 
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
		1, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6,
		7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 12,
		12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 16, 16, 
		16, 16, 16, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 20, 20, 20, 20, 
		20, 20, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 23, 
		23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 
		24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 
		24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
		24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 
		24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 
		24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 
		24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 
		24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 
		24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 
		24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 
		24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 22, 
		22, 22, 22, 22, 22, 22, 21, 21, 21, 21, 21, 21, 20, 20, 20, 20, 20, 20, 19, 19, 19, 19, 19, 
		18, 18, 18, 18, 18, 18, 17, 17, 17, 17, 17, 16, 16, 16, 16, 16, 15, 15, 15, 15, 15, 15, 14, 
		14, 14, 14, 14, 13, 13, 13, 13, 13, 12, 12, 12, 12, 12, 12, 11, 11, 11, 11, 11, 10, 10, 10, 
		10, 10, 9, 9, 9, 9, 9, 9, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 
		5, 5, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	};

	const struct prev_yee yee_params_def = {
		.en = 1,
		.en_halo_red = 1,
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
		.yee_thr = 5,
		.es_gain = 128,
		.es_thr1 = 1000,
		.es_thr2 = 30,
		.es_gain_grad = 32,
		.es_ofst_grad = 24, 
	};

	//DBG("set yee params...");
	//assert(ARRAY_SIZE(yee_table) == 1024);

	if(attrs->ctrlFlags & CONV_FLAG_EE_EN) {
		*yee = yee_params_def;
		yee->table = (short *)yee_table;
		yee->es_gain = attrs->sharpness;
		yee->es_gain_grad = attrs->sharpness >> 2;
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
	struct prev_rgb2yuv rgb2yuv;
	struct prev_rgb2rgb rgb2rgb;
	
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
			/* sharpness and noise filter can't be set at same time */
			//if(!(attrs->ctrlFlags & CONV_FLAG_EE_EN))
			previewer_set_nf(&modParam, &nf, attrs->ctrlFlags & CONV_FLAG_NF_EN);
			break;
		case PREV_WB:
			previewer_set_wb(fdPrev, &modParam, &wb, NULL);
			break;
		case PREV_GAMMA:
			bypass = (attrs->ctrlFlags & CONV_FLAG_GAMMA_EN) ? 0 : 1;
			previewer_set_gamma(&modParam, &gamma, bypass, attrs->gamma);
			break;
		case PREV_LUM_ADJ:
			previewer_set_lum(&modParam, &lum, attrs);
			break;
		case PREV_YEE:
			previewer_set_yee(&modParam, &yee, attrs);
			break;
		case PREV_RGB2YUV:
			previewer_set_y_offset(fdPrev, &modParam, &rgb2yuv, attrs);
			break;
		case PREV_RGB2RGB_1:
			previewer_set_rgb2rgb(&modParam, &rgb2rgb, attrs);
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

	
	//DBG("set previwer params done...");
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
Int32 previewer_convert(int fd, AlgBuf *inBuf, AlgBuf *outBuf, Bool enChanB)
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

	if(enChanB && outBuf[1].buf) {
		convert.out_buff2.buf_type = IMP_BUF_OUT2;
		convert.out_buff2.index = -1;	/* user ptr */
		convert.out_buff2.offset = (unsigned long)outBuf[1].buf;
		convert.out_buff2.size = outBuf[1].bufSize;
	}

	if(ioctl(fd, PREV_PREVIEW, &convert) < 0) {
	//if(ioctl(fd, RSZ_RESIZE, &convert) < 0) {
		ERRSTR("Error in doing preview");
		return E_IO;
	}

	return E_NO;
}

