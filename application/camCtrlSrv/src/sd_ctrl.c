#include "sd_ctrl.h"
#include "log.h"
#include "cam_file.h"
#include <dirent.h>
#include <sys/stat.h>

/*****************************************************************************
 Prototype    : sd_get_root_path
 Description  : get sd root path name
 Input        : Int32 id    
                void *buf   
                Int32 size  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/5/7
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 sd_get_root_path(Int32 id, void *buf, Int32 size)
{
	Int32 len;

	if(id > SD_MAX_ID || !buf || size <= 0)
		return E_INVAL;

	*(Int32 *)buf = id;

	len = snprintf((Int8 *)buf + sizeof(Int32), size, SD_MNT_FMT, id);

	return len + sizeof(Int32);
}

/*****************************************************************************
 Prototype    : sd_get_dir_info
 Description  : get sd dir info
 Input        : const char *pathName  
                Int8 *buf             
                Int32 size            
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/5/7
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 sd_get_dir_info(const char *pathName, Int8 *buf, Int32 size)
{
	CamFileInfoHdr 	*info = NULL;
    DIR 			*dir = NULL;
    struct dirent 	*dirent = NULL;
    Int32 			 count, offset;

	if(!pathName || !buf || size <= sizeof(Uint32) + sizeof(CamFileInfoHdr))
		return E_INVAL;

	DBG("Get dir info of %s", pathName);
	
    dir = opendir(pathName);
    if (!dir) {
        ERRSTR("opendir <%s> err", pathName);
       	return E_NOTEXIST;
    }
    
    count = 0;
    offset = sizeof(Uint32);

	/* read dir and record name */
    for(dirent = readdir(dir); dirent; ) {
		/* ignore "." and ".." */
        if ('.' != dirent->d_name[0]) {
            info = (CamFileInfoHdr *)(buf + offset);

			/* check file type */
            if (DT_DIR==dirent->d_type) {
                info->type = FILE_TYPE_DIR;
            } else if(DT_REG==dirent->d_type) {
                info->type = FILE_TYPE_NORMAL;
            } else {
                info->type = FILE_TYPE_OTHER;
            }

			/* calc size */
            info->size = strlen(dirent->d_name) + 1;
            info->size = ROUND_UP(info->size, 4);
			offset += sizeof(*info);
			
			if(offset + info->size > size) {
				ERR("buf not enough for dir info.");
				offset = E_NOMEM;
				break;
			}

			/* copy file name */
            strcpy(buf + offset, dirent->d_name);
            offset += info->size;
            count++;
        }
		
        dirent = readdir(dir);
    }

    *(Uint32 *)buf = count;

    if (dir)
        closedir(dir);

	/* len of dir info */
	return offset;
}

/*****************************************************************************
 Prototype    : sd_del_dir
 Description  : delete a dir and all the files in it
 Input        : const char *pathName  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/5/7
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 sd_del_dir(const char *pathName)
{
	int 	ret;
	int 	fd;
    char 	cmdLine[PATH_MAX] = "";

    if(strncmp(SD_MNT_FMT, pathName, strlen(SD_MNT_FMT) - 2)) {
        ERR("unexpect dir <%s>", pathName);
        return E_INVAL;
    }

	/* open to test this dir is exist */
	fd = open(pathName, O_RDONLY);
	if(fd < 0) {
		ERRSTR("Delete sd dir <%s> err", pathName);
		return E_NOTEXIST;
	}
	close(fd);

    snprintf(cmdLine, sizeof(cmdLine), "rm -rf %s \r\n", pathName);
    ret = system(cmdLine);
    if(0 == ret) {
        DBG("sd: <%s> deleted", pathName);
    } else {
        ERRSTR("del dir <%s> err", pathName);
		ret = E_IO;
    }

    return ret;
}

/*****************************************************************************
 Prototype    : sd_format
 Description  : format sd card
 Input        : Int32 id  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/5/7
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 sd_format(Int32 id)
{
	char 	cmdLine[PATH_MAX] = "";

	if(id > SD_MAX_ID)
		return E_INVAL;

	/* generate dir path */
	snprintf(cmdLine, sizeof(cmdLine), "rm -rf "SD_MNT_FMT"/* \r\n", id);

	Int32 ret = system(cmdLine);
    if(0 == ret) {
        DBG("sd %d format success", id);
    } else {
        ERRSTR("format sd %d err", id);
		ret = E_IO;
    }

	sync();

    return ret;
}


