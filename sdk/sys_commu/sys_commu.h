#ifndef __SYS_COMMU_H__
#define __SYS_COMMU_H__

#include <linux/types.h>
#include "syslink.h"

/* name of dev */
#define SYSLINK_DEV		"/dev/syslink"

typedef struct syslink_msg SysMsg;

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/**
 * Open a new chan for commu
 */
extern int sys_commu_open(struct syslink_attrs *attrs);

/**
 * Close a chan
 */
extern int sys_commu_close(int fd);


/* Read msg and following data, 
 * Note: buf should be no less than sizeof SysMsg and has the size of len,
 *          append data will be put continously after SysMsg header
 */
extern int sys_commu_read(int fd, SysMsg *buf, int len);

/* Write msg and following data, 
 * Note: buf should be no less than sizeof SysMsg and has the size of SysMsg.transLen + sizeof(SysMsg),
 *          append data shuold be put continously after SysMsg header
 */
extern int sys_commu_write(int fd, SysMsg *msg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif

