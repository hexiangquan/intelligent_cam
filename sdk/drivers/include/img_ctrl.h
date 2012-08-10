/* *
 * Copyright (C) 2012 S.K. Sun
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _IMG_CTRL_H_
#define _IMG_CTRL_H_

#include <linux/ioctl.h>
#include <linux/types.h>  

#ifdef __KERNEL__
#include <linux/kernel.h>  
#include <linux/wait.h>
#include <linux/mutex.h>
#include <asm/io.h>
#endif		/* end of #ifdef __KERNEL__ */

/* range of some cfg params */
#define HDCAM_AB_MAX_TARGET		255
#define HDCAM_AB_MAX_GAIN		1023

#define HDCAM_MAX_RED_GAIN		511
#define HDCAM_MAX_GREEN_GAIN	511
#define HDCAM_MAX_BLUE_GAIN		511

#define HDCAM_MAX_CONTRAST		255
#define HDCAM_MAX_SHARPNESS		255
#define HDCAM_MAX_BRIGHTNESS	255
#define HDCAM_MAX_DRC_STRENGTH	4095

/* List of ioctls */
#pragma pack(1)
#define IMGCTRL_MAGIC_NO	's'
#define IMGCTRL_S_LUM		_IOW(IMGCTRL_MAGIC_NO, 1, struct hdcam_lum_info *)
#define IMGCTRL_G_LUM		_IOR(IMGCTRL_MAGIC_NO, 2, struct hdcam_lum_info *)
#define IMGCTRL_S_CHROMA	_IOW(IMGCTRL_MAGIC_NO, 3, struct hdcam_chroma_info *)
#define IMGCTRL_G_CHROMA	_IOR(IMGCTRL_MAGIC_NO, 4, struct hdcam_chroma_info *)
#define IMGCTRL_S_ABCFG		_IOW(IMGCTRL_MAGIC_NO, 5, struct hdcam_ab_cfg *)
#define IMGCTRL_S_AWBCFG	_IOW(IMGCTRL_MAGIC_NO, 6, struct hdcam_awb_cfg *)
#define IMGCTRL_S_ENHCFG	_IOW(IMGCTRL_MAGIC_NO, 7, struct hdcam_img_enhance_cfg *)
#define IMGCTRL_G_VER		_IOR(IMGCTRL_MAGIC_NO, 8, __u32 *)
#define IMGCTRL_S_SPECCAP	_IOW(IMGCTRL_MAGIC_NO, 9, struct hdcam_img_enhance_cfg *)
#define IMGCTRL_SPECTRIG	_IOR(IMGCTRL_MAGIC_NO, 10, __u16 *)
#define IMGCTRL_TRIGCAP		_IO(IMGCTRL_MAGIC_NO, 11)

//#ifdef _DEBUG
#define IMGCTRL_HW_TEST		_IO(IMGCTRL_MAGIC_NO, 12)
//#endif
#pragma  pack()

/* 
 * Luminance info
 */
struct hdcam_lum_info {
	__u32	exposureTime;		//exposure time, unit: us
	__u16	globalGain;			//0~1023, equals 6db~42db
	__u16	apture;				//apture, reserved
	__u32	reserved;			//reserved, should be 0
};

/* 
 * Chroma info, RGB gains
 */
struct hdcam_chroma_info {
	__u16	redGain;			//Red gain, 0~511
	__u16	greenGain;			//Green gain,  0~511
	__u16	blueGain;			//Blue gain, 0~511
	__u16	reserved;			//reserved, should be 0
};

/* img enhance adjust ctrl, use image enhance struct */
#define HDCAM_CID_ADJ_CTRL 	(V4L2_CID_PRIVATE_BASE + 1)

/* alternating current sync ctrl */
#define HDCAM_CID_AC_SYNC 	(V4L2_CID_PRIVATE_BASE + 2)

/* structure for a region of an image */
struct hdcam_region {
	__u16	startX;		//X position of Start point
	__u16	startY;		//Y position of Start point
	__u16	endX;		//X position of End point
	__u16	endY;		//Y position of End point
};

/* 
 * auto brightness params 
 */

#define HDCAM_AB_MAX_ROI		5

/* Flags for auto brightness ctrl */
#define HDCAM_AB_FLAG_AE_EN		(1 << 0)	//enable auto exposure
#define HDCAM_AB_FLAG_AG_EN		(1 << 1)	//enable auto gain
#define HDCAM_AB_FLAG_AA_EN		(1 << 2)	//enable auto aperture, Not Supported

struct hdcam_ab_cfg{
	__u16		flags;					//Control flag
	__u16 		targetValue;	 		//Target brightness, 0~255
	__u32 		minShutterTime;			//Minimun shutter time, us
	__u32 		maxShutterTime;			//Maximum shutter time, us
	__u16 		minGainValue;			//Minimum global gain, 1~1023
	__u16 		maxGainValue;			//Maxmum global gain, usMinGainValue~1023
	__u16		minAperture;			//Min aperture value
	__u16		maxAperture;			//Max aperture 
	struct hdcam_region roi[HDCAM_AB_MAX_ROI];	//ROI region
};


/*
 * Auto white balance params
 */
#define HDCAM_AWB_FLAG_EN		(1 << 0)		//enable AWB

struct hdcam_awb_cfg{
	__u16		flags;					//control flag
	__u16		maxRedGain;				//max Red gain
    __u16		maxGreenGain;			//max green gain, unused in current version
    __u16		maxBlueGain;			//max blue gain
    __u16		minRedGain;				//min red gain
    __u16		minGreenGain;			//min green gain, unused in current version
	__u16		minBlueGain;			//min blue gain
	__u16		redModifyRatio;			//red gain modify ratio
	__u16		greenModifyRatio;		//green gain modify ratio, unused in current version
	__u16		blueModifyRatio;		//blue gain modify ratio
	__u16		initRedGain[2];			//Init Red value at day and night, 0-day, 1-night 
	__u16		initGreenGain[2];		//Init Green  value at day and night, 0-day, 1-night 
	__u16		initBlueGain[2];		//Init Blue value at day and night, 0-day, 1-night 
};

/* 
 * Image enhance params 
 */

/* Flags for image enhance */
#define HDCAM_ENH_FLAG_CONTRAST_EN		(1 << 0)	//enable contrast adjust
#define HDCAM_ENH_FLAG_SHARP_EN			(1 << 1)	//enable sharpness adjust
#define HDCAM_ENH_FLAG_GAMMA_EN			(1 << 2)	//enable brightness adjust
#define HDCAM_ENH_FLAG_SAT_EN			(1 << 3)	//enable saturation adjust
#define HDCAM_ENH_FLAG_NF_EN			(1 << 4)	//enable nosie filter
#define HDCAM_ENH_FLAG_DRC_EN			(1 << 5)	//enable dynamic range compression
#define HDCAM_ENH_FLAG_BRIGHT_EN		(1 << 6)	//enable brightness 

struct hdcam_img_enhance_cfg{
	__u16	flags;			//control flag
	__u16	contrast;		//value of contrast 
	__u16	sharpness;		//value of sharpness
	__u16	brightness;		//value of brightness, unused at current ver.
	__u16	saturation;		//value of saturation, unused at current ver.
	__u16	drcStrength;	//strength of DRC
	__u16	reserved[2];	//reserved
};

/*
  * Special capture cfg 
 */
struct hdcam_spec_cap_cfg{
	__u32	exposureTime;	// Unit: us
	__u16	globalGain;		// Range: 0~1023
	__u16	strobeCtrl;		// Bit[0:1]- strobe0, strobe1, 1-enable, 1-disable
	__u32	aeMinExpTime;	// AE for special capture, Min exposure time, us
	__u32	aeMaxExpTime;	// AE for special capture Max exposure time, us 
	__u16	aeMinGain;		// AE for special capture, Min global gain, 0~1023
	__u16	aeMaxGain;		// AE for special capture, Max global gain, 0~1023
	__u16	aeTargetVal;	// AE for special capture, target value, 0~255
	__u16	flags;			// flags for ctrl
	__u32	reserved[2];
};

/* bits define for flags of spec cap cfg */
#define HDCAM_SPEC_CAP_AE_EN	(1 << 0)

#endif /* end of #ifdef _IMG_CTRL_H_ */

