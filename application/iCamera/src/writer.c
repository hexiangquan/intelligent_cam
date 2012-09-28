/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : writer.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/20
  Last Modified :
  Description   : write file common code
  Function List :
              mkdir_if_need
              write_file
  History       :
  1.Date        : 2012/3/20
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "writer.h"
#include <sys/stat.h>
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

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/



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
Int32 mkdir_if_need(const char *dir)
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
Int32 write_file(const char *dirName, const char *fileName, const Int8 *header, Int32 hdrLen, const Int8 *data, Int32 len)
{
	if(!dirName || !fileName || !data)
		return E_INVAL;

	/* make dir */
	mkdir_if_need(dirName);
	
	/* save file */
	int fd = open(fileName, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if(fd < 0) {
        ERRSTR("open <%s> err", fileName);
        return E_REFUSED;
    }

	Int32 wrLen = 0;
	Int32 ret = E_NO;

	if(header && hdrLen > 0) {
		wrLen = write(fd, header, hdrLen);
		if(wrLen != hdrLen) {
			ERRSTR("write header failed.");
			ret = (errno == ENOSPC) ? E_NOSPC : E_IO;
			goto exit;
		}
	}
	
	while(len > 0) {
	    wrLen = write(fd, data, len);
	    if(wrLen < 0) {
	        ERRSTR("write <%s> err", fileName);
			ret = (errno == ENOSPC) ? E_NOSPC : E_IO;
			break;
	    } else {
			data += wrLen;
			len -= wrLen;
		}
	}

exit:
    close(fd);

	return ret;
}

