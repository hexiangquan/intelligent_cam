#ifndef __SYS_COMMU_H__
#define __SYS_COMMU_H__

#include <linux/types.h>

/* name of dev */
#define SYSLINK_DEV		"/dev/syslink"

/*
 * msg header for inter cpu commu
 * Note: addtional data should following this header in continous mem buf.
 */
typedef struct syslink_msg {
	__u32 magic;		/* magic num for sync, filled by driver */
    __u32 cmd;			/* cmd id */
    __u32 data_len;		/* len of addtion data */
    __u32 trans_len;	/* len of transfer, calc by aligning data len, filled by driver */
    __u32 checksum;		/* checksum of addtion data, filled by driver */
    __u32 params[3];	/* general params, different meaning with diff cmd */
}SysMsg;


#endif

