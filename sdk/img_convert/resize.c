/* --COPYRIGHT--,BSD
 * Copyright (c) 2010, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <media/davinci/imp_previewer.h>
#include <media/davinci/imp_resizer.h>
#include <media/davinci/dm365_ipipe.h>
#include "common.h"
#include "log.h"
#include "resize.h"


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
			return IPIPE_BAYER_8BIT_PACK_ALAW;
		default:
			return E_INVAL;
	};
}


/*****************************************************************************
 Prototype    : resize_ss_config
 Description  : Single shot config for resize
 Input        : int fd           
                RszAttrs *attrs  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 resize_ss_config(int fd, RszAttrs *attrs)
{
    struct rsz_channel_config rsz_chan_config;
    struct rsz_single_shot_config rsz_ss_config;

    /* Make sure our input parameters are valid */
    if (fd <= 0 || !attrs) {
        ERR("invalid fd for resize");
        return E_INVAL;
    }  
   	
    if (!attrs->inWidth  || !attrs->inHeight || !attrs->outAttrs[0].enbale || 
		!attrs->outAttrs[0].width || !attrs->outAttrs[0].height) {
        ERR("Invalid resolution");
        return E_INVAL;
    }

	if(attrs->outAttrs[1].enbale &&
		(!attrs->outAttrs[1].width || !attrs->outAttrs[1].height))
		return E_INVAL;

    /* Check for valid buffer scaling */
    Uint32 hDif = attrs->inWidth  * 256 / attrs->outAttrs[0].width;
    Uint32 vDif = attrs->inHeight * 256 / attrs->outAttrs[0].height;

    if (hDif < 32 || hDif > 4096 || vDif < 32 || vDif > 4096) {
        ERR("Horizontal up-scaling must not exceed 8x or down-scaling must not less than 1/16x");
        return E_INVAL;
    }

#if 0
    /* Set the driver default parameters and retrieve what was set */
    memset(&rsz_chan_config, 0, sizeof(rsz_chan_config));
    rsz_chan_config.oper_mode = IMP_MODE_SINGLE_SHOT;
    rsz_chan_config.chain = attrs->isChained ? 1 : 0;
    rsz_chan_config.len = 0;
    rsz_chan_config.config = NULL; /* to set defaults in driver */
    if(ioctl(fd, RSZ_S_CONFIG, &rsz_chan_config) < 0) {
        ERRSTR("Error in setting default configuration for single shot mode");
        goto cleanup;
    }
#endif
    /* default configuration setting in Resizer successfull */
    memset(&rsz_ss_config, 0, sizeof(rsz_ss_config));
    rsz_chan_config.oper_mode = IMP_MODE_SINGLE_SHOT;
    rsz_chan_config.chain = attrs->isChained ? 1 : 0;
    rsz_chan_config.len = sizeof(struct rsz_single_shot_config);
    rsz_chan_config.config = &rsz_ss_config;

    if (ioctl(fd, RSZ_G_CONFIG, &rsz_chan_config) < 0) {
        ERRSTR("Error in getting resizer channel configuration from driver");
        goto cleanup;
    }

    /* input params are set at the resizer */
    rsz_ss_config.input.image_width  = attrs->inWidth;
    rsz_ss_config.input.image_height = attrs->inHeight;
    rsz_ss_config.input.line_length = attrs->inPitch;
    rsz_ss_config.input.hst = attrs->inHStart;
    rsz_ss_config.input.vst = attrs->inVStart;
    rsz_ss_config.input.ppln = rsz_ss_config.input.image_width + 8;
    rsz_ss_config.input.lpfr = rsz_ss_config.input.image_height + 10;
    //rsz_ss_config.input.pix_fmt = pix_format_convert(attrs->inFmt);
    rsz_ss_config.output1.pix_fmt = pix_format_convert(attrs->outAttrs[0].pixFmt);
    rsz_ss_config.output1.enable = 1;
    rsz_ss_config.output1.width = attrs->outAttrs[0].width;
    rsz_ss_config.output1.height = attrs->outAttrs[0].height;
	rsz_ss_config.output1.h_flip = attrs->outAttrs[0].hFlip;
	rsz_ss_config.output1.v_flip = attrs->outAttrs[0].vFlip;
	rsz_ss_config.output1.line_length = attrs->outAttrs[0].lineLength;
    rsz_ss_config.output2.enable = 0;
	if(attrs->outAttrs[1].enbale) {
		rsz_ss_config.output2.enable = 1;
		rsz_ss_config.output2.width = attrs->outAttrs[1].width;
		rsz_ss_config.output2.height = attrs->outAttrs[1].height;
		rsz_ss_config.output2.h_flip = attrs->outAttrs[1].hFlip;
		rsz_ss_config.output2.v_flip = attrs->outAttrs[1].vFlip;
		rsz_ss_config.output2.line_length= attrs->outAttrs[1].lineLength;
		rsz_ss_config.output2.pix_fmt = pix_format_convert(attrs->outAttrs[1].pixFmt);
	}

    rsz_chan_config.oper_mode = IMP_MODE_SINGLE_SHOT;
    rsz_chan_config.chain = attrs->isChained ? 1 : 0;
    rsz_chan_config.len = sizeof(struct rsz_single_shot_config);
    if (ioctl(fd, RSZ_S_CONFIG, &rsz_chan_config) < 0) {
        ERRSTR("Error in setting configuration for single shot mode");
        goto cleanup;
    }

#if 0
    rsz_chan_config.oper_mode = IMP_MODE_SINGLE_SHOT;
    rsz_chan_config.chain = attrs->isChained ? 1 : 0;
    rsz_chan_config.len = sizeof(struct rsz_single_shot_config);

    /* read again and verify */
    if (ioctl(fd, RSZ_G_CONFIG, &rsz_chan_config) < 0) {
        ERRSTR("Error in getting configuration from driver");
        goto cleanup;
    }
#endif

    return E_NO;
    
cleanup:
   
    return E_IO;

}


/*****************************************************************************
 Prototype    : resizer_init
 Description  : Init resizer
 Input        : Bool onTheFly            
                RszOutAttrs *outParams0  
                RszOutAttrs *outParams1  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/24
    Author       : Sun
    Modification : Created function

*****************************************************************************/
int resize_init(const char *name, Bool onTheFly, ConvOutAttrs outParams[])
{
	int rsz_fd;
	Uint32 user_mode, oper_mode;
	struct rsz_channel_config rsz_chan_config;

	if(!outParams || !outParams[0].enbale)
		return E_INVAL;
	
	DBG("opening resize device");
	rsz_fd = open(name, O_RDWR);
	if(rsz_fd <= 0) {
		ERRSTR("Cannot open resize device");
		return E_IO;
	}

	if(onTheFly)
		user_mode = IMP_MODE_CONTINUOUS;
	else
		user_mode = IMP_MODE_SINGLE_SHOT;

	if (ioctl(rsz_fd, RSZ_S_OPER_MODE, &user_mode) < 0) {
		ERRSTR("Can't get operation mode");
		close(rsz_fd);
		return E_IO;
	}

	if (ioctl(rsz_fd, RSZ_G_OPER_MODE, &oper_mode) < 0) {
		ERRSTR("Can't get operation mode");
		close(rsz_fd);
		return E_IO;
	}

	if (oper_mode == user_mode)
		DBG("Successfully set mode to %d in resizer", (int)user_mode);
	else {
		ERRSTR("failed to set mode to %d in resizer", (int)user_mode);
		close(rsz_fd);
		return E_IO;
	}

	/* set configuration to chain resizer with preview */
	rsz_chan_config.oper_mode = user_mode;
	rsz_chan_config.chain  = 1;
	rsz_chan_config.len = 0;
	rsz_chan_config.config = NULL; /* to set defaults in driver */
	if (ioctl(rsz_fd, RSZ_S_CONFIG, &rsz_chan_config) < 0) {
		ERRSTR("Error in setting default configuration in resizer");
		close(rsz_fd);
		return E_IO;
	}

	DBG("default configuration setting in Resizer successfull");
	if(onTheFly) {
		struct rsz_continuous_config rsz_cont_config; // continuous mode
		
		bzero(&rsz_cont_config, sizeof(struct rsz_continuous_config));
		rsz_chan_config.oper_mode = user_mode;
		rsz_chan_config.chain = 1;
		rsz_chan_config.len = sizeof(struct rsz_continuous_config);
		rsz_chan_config.config = &rsz_cont_config;

		if (ioctl(rsz_fd, RSZ_G_CONFIG, &rsz_chan_config) < 0) {
			ERRSTR("Error in getting resizer channel configuration from driver");
			close(rsz_fd);
			return E_IO;
		}

		/* we can ignore the input spec since we are chaining. So only
		  * set output specs
		  */
		rsz_cont_config.output1.enable = outParams[0].enbale;
		if (outParams[1].enbale) {
			if(outParams[1].width > CONV_CHANB_MAX_OUT_WIDTH) {
				ERR("out width for chan[B] too big");
				rsz_cont_config.output2.enable = 0;
			} else {
				rsz_cont_config.output2.enable = 1;
				rsz_cont_config.output2.width = outParams[1].width;
				rsz_cont_config.output2.height = outParams[1].height;
				rsz_cont_config.output2.pix_fmt = pix_format_convert(outParams[1].pixFmt);
				if(rsz_cont_config.output2.pix_fmt < 0) {
					ERR("unsupported out format");
					close(rsz_fd);
					return E_UNSUPT;
				}
			}
		} else
			rsz_cont_config.output2.enable = 0;

		rsz_chan_config.oper_mode = user_mode;
		rsz_chan_config.chain = 1;
		rsz_chan_config.len = sizeof(struct rsz_continuous_config);
		rsz_chan_config.config = &rsz_cont_config;
			
		if (ioctl(rsz_fd, RSZ_S_CONFIG, &rsz_chan_config) < 0) {
			ERRSTR("Error in setting configuration in resizer");
			close(rsz_fd);
			return E_IO;
		}
	}
	DBG("Resizer initialized");
	
	return rsz_fd;
}


