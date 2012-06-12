/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : fpga_load.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/6/12
  Last Modified :
  Description   : load fpga firmware
  Function List :
              fpga_firmware_load
  History       :
  1.Date        : 2012/6/12
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "common.h"
#include "log.h"
#include "ext_io.h"
#include <sys/stat.h>
	
#define EXT_IO_DEV	"/dev/extio"

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

/*****************************************************************************
 Prototype    : fpga_firmware_load
 Description  : load fpga rom from file
 Input        : const char *fileName  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/6/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 fpga_firmware_load(const char *fileName)
{
	struct hdcam_firmware firmware;
	Int32		ret;
	struct stat fstat;
	Int32		fdExtIo = -1;	
	__u8 		*buf = NULL;
	FILE 		*fp = NULL;

	ret = stat(fileName, &fstat);
	if(ret < 0) {
		ERRSTR("stat <%s> err", fileName);
		return E_NOTEXIST;
	}

	if(!S_ISREG(fstat.st_mode)) {
		/* ignore unregular file */
		ERR("not a regular file");
		return E_INVAL;
	}

	/* alloc buffer for read */
	buf = malloc(fstat.st_size);
	if(!buf) {
		ERR("alloc mem failed");
		goto exit;
	}

	/* open firmware file */
	fp = fopen(fileName, "rb");
	if(!fp) {
	    ERRSTR("open <%s> err", fileName);
	    return E_NOTEXIST;
	}

	/* set firmware struct */
	firmware.len = fstat.st_size;
	firmware.magic = 0;	
	firmware.check_sum = 0;
	firmware.data = buf;
		
	/* read data */
	fseek(fp, 0, SEEK_SET);
	ret = fread(buf, firmware.len, 1, fp);
	if(ret < 0) {
	    ERRSTR("read data from <%s> err", fileName);
		ret = E_IO;
		goto exit;
	}

	/* open driver for firmware load */
	fdExtIo = open(EXT_IO_DEV, O_RDWR);
	if(fdExtIo < 0) {
		ERRSTR("can't open dev %s", EXT_IO_DEV);
		ret = E_NOTEXIST;
		goto exit;
	}
	
	/* load firmware */
	ret = ioctl(fdExtIo, EXTIO_S_FIRMWARE, &firmware);
	if(ret < 0) {
		ERRSTR("firmware upload failed");
		ret = E_IO;
	} else {
		DBG("firmware upload success");
		ret = E_NO;
	}
	
exit:

	if(buf)
		free(buf);

	if(fp)
		fclose(fp);

	if(fdExtIo > 0)
		close(fdExtIo);

	return ret;
}

