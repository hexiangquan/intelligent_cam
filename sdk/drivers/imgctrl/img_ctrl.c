/*
 *  drivers/imgctrl/fpga.c
 *
 *  Copyright (C) 2012, 2013  S.K. Sun
 *
 *  APIs for fpga read & write at HDCAM 
 *   
 *  created, Apr. 2012, S.K. Sun <shengkai.sun@gmail.com>
 */
 
#include <linux/module.h>  
#include <linux/fs.h>  
#include <linux/init.h>  
#include <linux/kernel.h>  
#include <linux/types.h>  
#include <linux/errno.h>  
#include <linux/fcntl.h> 
#include <linux/delay.h> 
#include <linux/clk.h> 
#include <linux/wait.h> 
#include <linux/sched.h> 
#include <asm/io.h> 
#include <asm/irq.h> 
#include <asm/uaccess.h>
#include <linux/cdev.h> 
#include <linux/miscdevice.h> 
#include <linux/spinlock.h>
#include <linux/fpga.h>
#include "img_ctrl.h"

#define DEVICE_NAME 		"imgctrl" 

#ifdef _DEBUG 
#define _DBG(fmt, args...)	printk(KERN_INFO fmt"\n", ##args) 
#else 
#define _DBG(fmt, args...)
#endif

#define _ERR(fmt, args...)	printk(KERN_ALERT "func: %s, "fmt"\n", __func__, ##args)

/* Lock used for synchronization */
spinlock_t imgctrl_lock = SPIN_LOCK_UNLOCKED;	

struct img_ctrl_dev {
	void __iomem *fpga_base;	/* fpga base addr */
};


/**
 * img_ctrl_open - Open fpga based img ctrl device
 * @inode:  Pointer to inode structure.
 * @filp:     File pointer.
 *
 * This function initilize img ctrl device.
 * It returns 0 if alloc mem and get fpag base successed otherwise it
 * returns negative error value.
 */
static int img_ctrl_open(struct inode *inode, struct file *filp) 
{ 
	struct img_ctrl_dev *dev;
	int 				err;

	dev = kmalloc(sizeof(struct img_ctrl_dev), GFP_KERNEL);
	if(!dev) {
		_ERR("alloc mem failed");
		return -ENOMEM;
	}

	dev->fpga_base = fpga_get_base();
	if(!dev->fpga_base) {
		_ERR("invalid fpga base addr");
		err = -EINVAL;
		goto exit;
	}
	
 	filp->private_data = dev;
	_DBG("img ctrl opened, fpga base: 0x%04X", (u32)dev->fpga_base); 
	
	return 0; 

exit:
	kfree(dev);
	return err;
} 
 
/**
 * img_ctrl_release - Close fpga based img ctrl device
 * @inode:  Pointer to inode structure.
 * @filp:     File pointer.
 *
 * This function just free mem allocated.
 * It always returns 0.
 */
static int img_ctrl_release(struct inode *inode, struct file *filp) 
{ 
	struct img_ctrl_dev *dev = filp->private_data;

	if(dev)
		kfree(dev);

	_DBG("img ctrl closed");

	return 0;
}

/**
 * region_validate - Validate point position within the range of image
 * @dev:        Device pointer
 * @region:    Region pointer.
 *
 * This function checks the point position of region in the image, corrects if out range.
 * It returns 0 if params are valid, otherwise returns negative error value.
 */
static int region_validate(struct img_ctrl_dev *dev, struct hdcam_region *region)
{
	u16 width, height;

	/* check point positions */
	if( region->endX > region->startX || 
		region->endY > region->startY ) {
		_ERR("end position must bigger than start position.");
		return -EINVAL;
	}

	/* read current image width and height */
	width = fpga_read(dev->fpga_base, FPGA_REG_CAP_WIDTH);
	height = fpga_read(dev->fpga_base, FPGA_REG_CAP_HEIGHT);

	/* keep region in the image */
	if(region->endX > width - 1)
		region->endX = width - 1;

	if(region->endY > height - 1)
		region->endY = height - 1;

	return 0;
}

/**
 * ab_cfg_validate - Validate ab cfg params
 * @dev:       Device pointer
 * @cfg:        AB cfg pointer.
 *
 * This function checks the value in ab cfg, make sure the params are correct.
 * It returns 0 if params are valid, otherwise returns negative error value.
 */
static int ab_cfg_validate(struct img_ctrl_dev *dev, struct hdcam_ab_cfg *cfg)
{
	int i, result;
	
	/* validate target vaule */
	if(cfg->targetValue > HDCAM_AB_MAX_TARGET) {
		_ERR("target value: %d is bigger than max %d", cfg->targetValue, HDCAM_AB_MAX_TARGET);
		return -EINVAL;
	}

	/* validate max gain */
	if(cfg->maxGainValue > HDCAM_AB_MAX_GAIN) {
		_ERR("max gain: %d is bigger than max %d", cfg->maxGainValue, HDCAM_AB_MAX_GAIN);
		return -EINVAL;
	}

	/* validate roi */
	for(i = 0; i < HDCAM_AB_MAX_ROI; i++) {
		result = region_validate(dev, &cfg->roi[i]);
		if(result)
			break;
	}

	return result;
}

/**
 * ab_hw_setup - Setup auto brightness hardware
 * @dev:       Device pointer
 * @cfg:        AB cfg params pointer.
 *
 * This function set the value in ab cfg to hardware.
 * It returns 0 if cfg successful, otherwise returns negative error value.
 */
static int ab_hw_setup(struct img_ctrl_dev *dev, struct hdcam_ab_cfg *cfg)
{	
	u16 data;
	
	/* get lock first */
	spin_lock(&imgctrl_lock);

	/* stop AB first */
	data = fpga_read(dev->fpga_base, FPGA_REG_AUTO_ADJ_CTL);
	data &= ~(0x000B);
	fpga_write(dev->fpga_base, FPGA_REG_AUTO_ADJ_CTL, data);

	if(cfg->flags) {
		/* set target value */
		fpga_write(dev->fpga_base, FPGA_REG_AB_TARGET_VALUE, 
			cfg->targetValue);
		
		/* set min & max exposure time */
		fpga_write(dev->fpga_base, FPGA_REG_AE_MIN_EXPOSURE_TIME0, 
			cfg->minShutterTime & 0xFFFF);
		fpga_write(dev->fpga_base, FPGA_REG_AE_MIN_EXPOSURE_TIME1, 
			(cfg->minShutterTime >> 16) & 0xFFFF);
		fpga_write(dev->fpga_base, FPGA_REG_AE_MAX_EXPOSURE_TIME0, 
			cfg->maxShutterTime & 0xFFFF);
		fpga_write(dev->fpga_base, FPGA_REG_AE_MAX_EXPOSURE_TIME1, 
			(cfg->maxShutterTime >> 16) & 0xFFFF);

		/* set max global gain*/
		fpga_write(dev->fpga_base, FPGA_REG_AG_MAX_GAIN, cfg->maxGainValue);

		/* set min & max aperture */
		fpga_write(dev->fpga_base, FPGA_REG_AUTO_APERTURE_PARAM, 
			(cfg->minAperture & 0xFF) | ((cfg->maxAperture & 0xFF) << 8));
		
		/* set roi regions */
		fpga_write(dev->fpga_base, FPGA_REG_AB_ROI0_STARTX, 
			cfg->roi[0].startX);
		fpga_write(dev->fpga_base, FPGA_REG_AB_ROI0_STARTY, 
			cfg->roi[0].startY);
		fpga_write(dev->fpga_base, FPGA_REG_AB_ROI0_ENDX, 
			cfg->roi[0].endX);
		fpga_write(dev->fpga_base, FPGA_REG_AB_ROI0_ENDY, 
			cfg->roi[0].endY);

		fpga_write(dev->fpga_base, FPGA_REG_AB_ROI1_STARTX, 
			cfg->roi[1].startX);
		fpga_write(dev->fpga_base, FPGA_REG_AB_ROI1_STARTY, 
			cfg->roi[1].startY);
		fpga_write(dev->fpga_base, FPGA_REG_AB_ROI1_ENDX, 
			cfg->roi[1].endX);
		fpga_write(dev->fpga_base, FPGA_REG_AB_ROI1_ENDY, 
			cfg->roi[1].endY);

		fpga_write(dev->fpga_base, FPGA_REG_AB_ROI2_STARTX, 
			cfg->roi[2].startX);
		fpga_write(dev->fpga_base, FPGA_REG_AB_ROI2_STARTY, 
			cfg->roi[2].startY);
		fpga_write(dev->fpga_base, FPGA_REG_AB_ROI2_ENDX, 
			cfg->roi[2].endX);
		fpga_write(dev->fpga_base, FPGA_REG_AB_ROI2_ENDY, 
			cfg->roi[2].endY);

		fpga_write(dev->fpga_base, FPGA_REG_AB_ROI3_STARTX, 
			cfg->roi[3].startX);
		fpga_write(dev->fpga_base, FPGA_REG_AB_ROI3_STARTY, 
			cfg->roi[3].startY);
		fpga_write(dev->fpga_base, FPGA_REG_AB_ROI3_ENDX, 
			cfg->roi[3].endX);
		fpga_write(dev->fpga_base, FPGA_REG_AB_ROI3_ENDY, 
			cfg->roi[3].endY);

		fpga_write(dev->fpga_base, FPGA_REG_AB_ROI4_STARTX, 
			cfg->roi[4].startX);
		fpga_write(dev->fpga_base, FPGA_REG_AB_ROI4_STARTY, 
			cfg->roi[4].startY);
		fpga_write(dev->fpga_base, FPGA_REG_AB_ROI4_ENDX, 
			cfg->roi[4].endX);
		fpga_write(dev->fpga_base, FPGA_REG_AB_ROI4_ENDY, 
			cfg->roi[4].endY);

		/* set enable flags */
		if(cfg->flags & HDCAM_AB_FLAG_AE_EN)
			data |= BIT(0);
		if(cfg->flags & HDCAM_AB_FLAG_AG_EN)
			data |= BIT(1);
		if(cfg->flags & HDCAM_AB_FLAG_AA_EN)
			data |= BIT(3);
		fpga_write(dev->fpga_base, FPGA_REG_AUTO_ADJ_CTL, data);
	}

	/* release lock */
	spin_unlock(&imgctrl_lock);

	return 0;
}

/**
 * ab_set_cfg - Configure auto brightness module
 * @dev:       Device pointer
 * @cfg:       user space ptr for AB cfg params pointer.
 *
 * This function set the configure params for auto brightness.
 * It returns 0 if cfg successful, otherwise returns negative error value.
 */
static int ab_set_cfg(struct img_ctrl_dev *dev, unsigned long usrptr)
{
	struct hdcam_ab_cfg cfg;
	int result;
	
	/* Copy config structure passed by user */
	if (copy_from_user(&cfg, (struct hdcam_ab_cfg *)usrptr,
			   sizeof(struct hdcam_ab_cfg))) {
		_ERR("cpy from user failed.");
		return -EFAULT;
	}

	/* validate cfg params */
	result = ab_cfg_validate(dev, &cfg);
	if(result)
		return result;

	_DBG("ab flags: 0x%X", (u32)cfg.flags);
	_DBG("target value: %u", (u32)cfg.targetValue);

	/* Call aew_hardware_setup to perform register configuration */
	result = ab_hw_setup(dev, &cfg);
	if (result) {
		/* Change Configuration Structure to original */
		_ERR("set ab hardware failed");
	}

	return result;
}

/**
 * awb_cfg_validate - Validate awb cfg params
 * @dev:       Device pointer
 * @cfg:        AWB cfg pointer.
 *
 * This function checks the value in awb cfg, make sure the params are correct.
 * It returns 0 if params are valid, otherwise returns negative error value.
 */
static int awb_cfg_validate(struct img_ctrl_dev *dev, struct hdcam_awb_cfg *cfg)
{
	if( cfg->minRedGain > cfg->maxRedGain || 
		cfg->minGreenGain > cfg->maxGreenGain ||
		cfg->minBlueGain > cfg->maxBlueGain ) {
		_ERR("min gain should not bigger than max gain.");
		return -EINVAL;
	}

	if( cfg->maxRedGain > HDCAM_MAX_RED_GAIN || 
		cfg->maxGreenGain > HDCAM_MAX_GREEN_GAIN ||
		cfg->maxBlueGain > HDCAM_MAX_BLUE_GAIN ) {
		_ERR("max gain is bigger than limited value.");
		return -EINVAL;
	}

	if( cfg->initRedGain[0] > HDCAM_MAX_RED_GAIN || 
		cfg->initRedGain[1] > HDCAM_MAX_RED_GAIN ||
		cfg->initGreenGain[0] > HDCAM_MAX_GREEN_GAIN ||
		cfg->initGreenGain[1] > HDCAM_MAX_GREEN_GAIN ||
		cfg->initBlueGain[0] > HDCAM_MAX_BLUE_GAIN || 
		cfg->initBlueGain[1] > HDCAM_MAX_BLUE_GAIN ) {
		_ERR("init gain is bigger than limited value.");
		return -EINVAL;
	}

	return 0;
}

/**
 * awb_hw_setup - Setup auto white balance hardware
 * @dev:       Device pointer
 * @cfg:        AWB cfg params pointer.
 *
 * This function set the value in ab cfg to hardware.
 * It returns 0 if cfg successful, otherwise returns negative error value.
 */
static int awb_hw_setup(struct img_ctrl_dev *dev, struct hdcam_awb_cfg *cfg)
{	
	u16 data;
	
	/* get lock first */
	spin_lock(&imgctrl_lock);

	/* stop AWB first */
	data = fpga_read(dev->fpga_base, FPGA_REG_AUTO_ADJ_CTL);
	data &= ~BIT(2);
	fpga_write(dev->fpga_base, FPGA_REG_AUTO_ADJ_CTL, data);

	if(cfg->flags) {
		/* set init value */
		fpga_write(dev->fpga_base, FPGA_REG_AWB_RED_INIT_VAL, 
			(cfg->initRedGain[0] & 0xFF) | ((cfg->initRedGain[1] & 0xFF) << 8));
		fpga_write(dev->fpga_base, FPGA_REG_AWB_GREEN_INIT_VAL, 
			(cfg->initGreenGain[0] & 0xFF) | ((cfg->initGreenGain[1] & 0xFF) << 8));
		fpga_write(dev->fpga_base, FPGA_REG_AWB_BLUE_INIT_VAL, 
			(cfg->initBlueGain[0] & 0xFF) | ((cfg->initBlueGain[1] & 0xFF) << 8));
		
		/* set min & max gains */
		fpga_write(dev->fpga_base, FPGA_REG_AWB_MAX_RED_GAIN, 
			cfg->maxRedGain);
		fpga_write(dev->fpga_base, FPGA_REG_AWB_MAX_BLUE_GAIN, 
			cfg->maxBlueGain);

		/* set adjust rotio */
		fpga_write(dev->fpga_base, FPGA_REG_AWB_RED_MODIFY_RATIO, 
			cfg->redModifyRatio);
		fpga_write(dev->fpga_base, FPGA_REG_AWB_BLUE_MODIFY_RATIO, 
			cfg->blueModifyRatio);

		/* set enable flags */
		if(cfg->flags & HDCAM_AWB_FLAG_EN)
			data |= BIT(2);
		fpga_write(dev->fpga_base, FPGA_REG_AUTO_ADJ_CTL, data);
	}

	/* release lock */
	spin_unlock(&imgctrl_lock);

	return 0;
}

/**
 * awb_set_cfg - Configure auto white balance module
 * @dev:      Device pointer
 * @usrptr:   User space ptr for AWB cfg params pointer.
 *
 * This function set the configure params to awb engine.
 * It returns 0 if cfg successful, otherwise returns negative error value.
 */
static int awb_set_cfg(struct img_ctrl_dev *dev, unsigned long usrptr)
{
	struct hdcam_awb_cfg cfg;
	int result;
	
	/* Copy config structure passed by user */
	if (copy_from_user(&cfg, (struct hdcam_awb_cfg *)usrptr,
			   sizeof(cfg))) {
		_ERR("cpy from user failed.");
		return -EFAULT;
	}

	/* validate cfg params */
	result = awb_cfg_validate(dev, &cfg);
	if(result)
		return result;

	_DBG("awb flags: 0x%X", (u32)cfg.flags);
	_DBG("red gain: %u - %u", (u32)cfg.minRedGain, (u32)cfg.maxRedGain);
	_DBG("green gain: %u - %u", (u32)cfg.minGreenGain, (u32)cfg.maxGreenGain);
	_DBG("blue gain: %u - %u", (u32)cfg.minBlueGain, (u32)cfg.maxBlueGain);

	/* Call awb_hardware_setup to perform register configuration */
	result = awb_hw_setup(dev, &cfg);
	if (result) {
		/* Change Configuration Structure to original */
		_ERR("set awb hardware failed");
	}

	return result;
}

/**
 * lum_info_setup - Configure lumanice info
 * @dev:       Device pointer
 * @cfg:       user space ptr for lum info params.
 *
 * This function set the lum params for image sensor luminance adjust
 * It returns 0 if cfg successful, otherwise returns negative error value.
 */
static int lum_info_setup(struct img_ctrl_dev *dev, unsigned long usrptr)
{
	struct hdcam_lum_info info;
	
	/* Copy config structure passed by user */
	if (copy_from_user(&info, (struct hdcam_lum_info *)usrptr,
			   sizeof(struct hdcam_lum_info))) {
		_ERR("cpy from user failed.");
		return -EFAULT;
	}

	_DBG("exposure time: %u", info.exposureTime);
	_DBG("global gain: %u", (u32)info.globalGain);
	

	/* validate data */
	if(info.globalGain > HDCAM_AB_MAX_GAIN) {
		_ERR("gain(%d) is bigger than max %d", info.globalGain, HDCAM_AB_MAX_GAIN);
		return -EINVAL;
	}

	/* get lock first */
	spin_lock(&imgctrl_lock);
	
	/* cfg fpga register */
	fpga_write(dev->fpga_base, FPGA_REG_EXPOSURE_TIME0, 
		info.exposureTime & 0xFFFF);
	fpga_write(dev->fpga_base, FPGA_REG_EXPOSURE_TIME1, 
		(info.exposureTime >> 16) & 0xFFFF);
	fpga_write(dev->fpga_base, FPGA_REG_AFE_GAIN, 
		info.globalGain );

	/* release lock */
	spin_unlock(&imgctrl_lock);
	
	return 0;
}
 
 /**
  * lum_info_read - Read lumanice info
  * @dev:	Device pointer
  * @cfg:		user space ptr for lum info params.
  *
  * This function set the lum params for luminance adjust
  * It returns 0 if cfg successful, otherwise returns negative error value.
  */
static int lum_info_read(struct img_ctrl_dev *dev, unsigned long usrptr)
{
	struct hdcam_lum_info info;

	memset(&info, 0, sizeof(info));
	
	/* get lock first */
	spin_lock(&imgctrl_lock);

	/* read fpga register */
	info.exposureTime = fpga_read(dev->fpga_base, FPGA_REG_EXPOSURE_TIME0);
	info.exposureTime |= fpga_read(dev->fpga_base, FPGA_REG_EXPOSURE_TIME1) << 16;
	info.globalGain  = fpga_read(dev->fpga_base, FPGA_REG_AFE_GAIN);

	/* release lock */
	spin_unlock(&imgctrl_lock);

	/* Copy config structure passed by user */
	if (copy_to_user((struct hdcam_lum_info *)usrptr, &info, sizeof(info))) {
		_ERR("cpy to user failed.");
		return -EFAULT;
	}

	return 0;
 }
  
/**
 * chroma_info_setup - Configure chroma info
 * @dev:		 Device pointer
 * @cfg:		 user space ptr for lum info params.
 *
 * This function set the chroma engine for color adjust
 * It returns 0 if cfg successful, otherwise returns negative error value.
 */
static int chroma_info_setup(struct img_ctrl_dev *dev, unsigned long usrptr)
{
	struct hdcam_chroma_info info;

	/* Copy config structure passed by user */
	if (copy_from_user(&info, (struct hdcam_chroma_info *)usrptr,
			 sizeof(info))) {
		_ERR("cpy from user failed.");
		return -EFAULT;
	}

	_DBG("RGB: %u, %u, %u", info.redGain, info.greenGain, info.blueGain);	  

	/* validate data */
	if( info.redGain > HDCAM_MAX_RED_GAIN || 
		info.greenGain > HDCAM_MAX_GREEN_GAIN || 
		info.blueGain > HDCAM_MAX_BLUE_GAIN) {
	  _ERR("rgb gains are bigger than max value");
	  return -EINVAL;
	}


	/* get lock first */
	spin_lock(&imgctrl_lock);

	/* cfg fpga register */
	fpga_write(dev->fpga_base, FPGA_REG_RED_GAIN, 
	  info.redGain);
	fpga_write(dev->fpga_base, FPGA_REG_GREEN_GAIN, 
	  info.greenGain);
	fpga_write(dev->fpga_base, FPGA_REG_BLUE_GAIN, 
	  info.blueGain);

	/* release lock */
	spin_unlock(&imgctrl_lock);

	return 0;
}

/**
 * chroma_info_read - Read chroma info
 * @dev:       Device pointer
 * @usrptr:    User space ptr for lum info params.
 *
 * This function get the current chroma params
 * It returns 0 if cfg successful, otherwise returns negative error value.
 */
static int chroma_info_read(struct img_ctrl_dev *dev, unsigned long usrptr)
{
	struct hdcam_chroma_info info;

	memset(&info, 0, sizeof(info));

	/* get lock first */
	spin_lock(&imgctrl_lock);

	/* read fpga register */
	info.redGain = fpga_read(dev->fpga_base, FPGA_REG_RED_GAIN);
	info.greenGain = fpga_read(dev->fpga_base, FPGA_REG_GREEN_GAIN);
	info.blueGain = fpga_read(dev->fpga_base, FPGA_REG_BLUE_GAIN);

	/* release lock */
	spin_unlock(&imgctrl_lock);

	/* Copy config structure passed by user */
	if (copy_to_user((struct hdcam_chroma_info *)usrptr, &info, sizeof(info))) {
		_ERR("cpy to user failed.");
		return -EFAULT;
	}

	return 0;
}

/**
 * img_enhance_setup - Configure img enhance module
 * @dev:		 Device pointer
 * @cfg:		 user space ptr for img enhance cfg.
 *
 * This function set the img enhance engine for img quality adjust
 * It returns 0 if cfg successful, otherwise returns negative error value.
 */
static int img_enhance_setup(struct img_ctrl_dev *dev, unsigned long usrptr)
{
	u16 data = 0;
	struct hdcam_img_enhance_cfg cfg;

	/* Copy config structure passed by user */
	if (copy_from_user(&cfg, (struct hdcam_img_enhance_cfg *)usrptr,
			 sizeof(cfg))) {
		_ERR("cpy from user failed.");
		return -EFAULT;
	}

	_DBG("enhance flags: 0x%X", (u32)cfg.flags);
	_DBG("contrast: %u, sharpness: %u, drc: %u", 
		(u32)cfg.contrast, (u32)cfg.sharpness, (u32)cfg.drcStrength);

	/* validate data */
	if( cfg.contrast > HDCAM_MAX_CONTRAST || 
		cfg.sharpness > HDCAM_MAX_SHARPNESS || 
		cfg.drcStrength > HDCAM_MAX_DRC_STRENGTH ||
		cfg.brightness > HDCAM_MAX_BRIGHTNESS) {
	  _ERR("params are bigger than max value");
	  return -EINVAL;
	}

	/* get lock first */
	spin_lock(&imgctrl_lock);

	/* cfg fpga register */
	if(cfg.flags & HDCAM_ENH_FLAG_CONTRAST_EN) {
		/* enable contrast */
		data |= BIT(0);
		fpga_write(dev->fpga_base, FPGA_REG_CONTRAST, cfg.contrast);
	}

	if(cfg.flags & HDCAM_ENH_FLAG_SHARP_EN) {
		/* enable sharpness */
		data |= BIT(1);
		fpga_write(dev->fpga_base, FPGA_REG_SHARPNESS, cfg.sharpness);
	}

	if(cfg.flags & HDCAM_ENH_FLAG_NF_EN) {
		/* enable median filter */
		data |= BIT(2);
	}

	if(cfg.flags & HDCAM_ENH_FLAG_SAT_EN) {
		/* enable sturation */
		data |= BIT(3);
		fpga_write(dev->fpga_base, FPGA_REG_SATURATION, cfg.saturation);
	}

	if(cfg.flags & HDCAM_ENH_FLAG_DRC_EN) {
		/* enable dynamic range compression, reserved */
		data |= BIT(5);
		fpga_write(dev->fpga_base, FPGA_REG_DRC_STRENGTH, cfg.drcStrength);
	}

	if(cfg.flags & HDCAM_ENH_FLAG_BRIGHT_EN) {
		/* set brightness */
		data |= BIT(4);
		fpga_write(dev->fpga_base, FPGA_REG_BRIGHTNESS, cfg.brightness);
	}

	/* set enable ctrl reg */
	fpga_write(dev->fpga_base, FPGA_REG_IMG_ENHANCE_CTL, data);
	
	/* release lock */
	spin_unlock(&imgctrl_lock);

	return 0;
}

#ifdef _DEBUG
/* test fpga reg rw */
static void fpga_test(void)
{
	u16 data;
	void __iomem *fpga_base = fpga_get_base();
	
	_DBG("fpga version: 0x%04X", 
		(u32)fpga_read(fpga_base, FPGA_REG_VERSION));

	fpga_write(fpga_base, FPGA_REG_EXPOSURE_TIME0, 0x5A5A);
	data = fpga_read(fpga_base, FPGA_REG_EXPOSURE_TIME0);

	if(data != 0x5A5A) {
		_DBG("rw exposure reg diff.");
	} else {
		_DBG("rw exposure reg the same.");
	}
}
#endif


/**
 * img_ctrl_ioctl - Do ctrl of img ctrl device, such as auto brightness cfg.
 * @inode:  Pointer to inode structure.
 * @filp:     File pointer.
 * @cmd:    Cmd to execute
 * @arg:     Args for the cmd
 *
 * This function write or read fpga to ctrl img adjust functions.
 * It returns 0 if W/R success, otherwise it
 * returns negative error value.
 */
static int img_ctrl_ioctl(struct inode *inode, struct file *filep,
		     unsigned int cmd, unsigned long arg)
{
	int result = 0;
	struct img_ctrl_dev *dev = filep->private_data;

	if(!dev) {
		_ERR("invalid dev data for img ctrl");
		return -ENOTTY;
	}
	
	/*
	 * Extract the type and number bitfields, and don't decode wrong cmds:
	 * verify the magic number
	 */
	if (_IOC_TYPE(cmd) != IMGCTRL_MAGIC_NO) {
		_ERR("invalid magic for img ctrl");
		return -ENOTTY;
	}

	/* check for the permission of the operation */
	if (_IOC_DIR(cmd) & _IOC_READ)
		result = !access_ok(VERIFY_WRITE, (void __user *)arg,_IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		result = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));

	if (result) {
		_ERR("check rw access for img ctrl failed");
		return -EFAULT;
	}

	/* Switch according to IOCTL command */
	switch (cmd) {

	/* set luminance info */
	case IMGCTRL_S_LUM:
		result = lum_info_setup(dev, arg);
		break;

	/* get luminance info */
	case IMGCTRL_G_LUM:
		result = lum_info_read(dev, arg);
		break;

	/* set chroma info */
	case IMGCTRL_S_CHROMA:
		result = chroma_info_setup(dev, arg);
		break;

	/* get chroma info */
	case IMGCTRL_G_CHROMA:
		result = chroma_info_read(dev, arg);
		break;
	
	/* set auto brightness params */
	case IMGCTRL_S_ABCFG:
		result = ab_set_cfg(dev, arg);
		break;
		
	/* set auto white balance params */
	case IMGCTRL_S_AWBCFG:
		result = awb_set_cfg(dev, arg);
		break;

	/* set image enhance adjust params */
	case IMGCTRL_S_ENHCFG:
		result = img_enhance_setup(dev, arg);
		break;

#ifdef _DEBUG
	/* test fpag rw */
	case IMGCTRL_HW_TEST:
		fpga_test();
		result = 0;
		break;
#endif
	
	/* Invalid Command */
	default:
		_ERR("Invalid cmd");
		result = -ENOTTY;
		break;
	}

	return result;

}

/* file ops of this module */
static struct file_operations dev_fops = { 
	.owner = THIS_MODULE, 
	.open = img_ctrl_open, 
	.release = img_ctrl_release, 
	.ioctl = img_ctrl_ioctl,
}; 


/* misc device info */
static struct miscdevice misc_dev = { 
	.minor = MISC_DYNAMIC_MINOR, 
	.name = DEVICE_NAME, 
	.fops = &dev_fops, 
};

/* device init function, register dev */
static int __init dev_init(void) 
{ 
	int ret; 

	ret = misc_register(&misc_dev); 

	_DBG(DEVICE_NAME"\tinitialized\n");

#ifdef _DEBUG
	fpga_test();
#endif

	return ret;
}

/* device exit function, deregister dev */
static void __exit dev_exit(void) 
{ 
	misc_deregister(&misc_dev);
} 
 

module_init(dev_init); 
module_exit(dev_exit); 
MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("S.K. Sun"); 


