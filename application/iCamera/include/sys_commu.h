#ifndef __SYS_COMMU_H__
#define __SYS_COMMU_H__

#include <linux/types.h>

/* name of dev */
#define SYSLINK_DEV		"/dev/syslink"

#define SYS_MSG_MAGIC	0xC0DECAFEu		
#define SYS_MSG_ALIGN	(256)

/*
 * msg header for inter cpu commu
 * Note: addtional data should following this header in continous mem buf.
 */
typedef struct _SyslinkMsg {
	__u32 magic;		/* magic num for sync, filled by driver */
    __u32 cmd;			/* cmd id */
    __u32 dataLen;		/* len of addtion data */
    __u32 transLen;		/* len of transfer, calc by aligning data len, filled by driver */
    __u32 checksum;		/* checksum of addtion data, filled by driver */
    __u32 params[3];	/* general params, different meaning with diff cmd */
}SysMsg;

/**
 * cmd for inter processor communication
 */
typedef enum _SysLinkCmd {
	SYS_CMD_RESERVED = 0x0000,				//Reserved
	
	SYS_CMD_SYS_RESET = 0x0001, 			//Reset the system
	SYS_CMD_UPDATE = 0x0002,				//Update dsp firmware
	SYS_CMD_TRANS_TEST = 0x0003,			//Test data transfer
	SYS_CMD_DAY_NIGHT_SWITCH = 0x0004,		//Switch day night mode
	
	SYS_CMD_VLPR_CFG = 0x0010,				//Vehicle plate recog alg config		
	SYS_CMD_VLPR_RET = 0x0011,				//Vehicle plate result return
	
	SYS_CMD_VIDDETECT_CFG = 0x0020,			//Video detect config
	SYS_CMD_VIDDETECT_RET = 0x0021,			//Video detect result return
	
	SYS_CMD_MAX							
}SysCmd;

#endif

