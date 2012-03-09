#ifndef _NET_PROTOCOL_H_
#define _NET_PROTOCOL_H_

typedef unsigned int	Uint32;
typedef unsigned char	Uint8;
typedef unsigned short	Uint16;

typedef	char			Int8;
typedef short			Int16;
typedef int				Int32;

typedef int				Bool;

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define CMD_FLAG_SAVE_AS_DEFAULT (1 << 0)
#define CMD_FLAG_MASK			 (1 << 0)

typedef struct {
	Uint32	serialNum;
	Uint32	command;
	Uint16	commandType;
	Uint16	commandErr;
	Uint32	flag;			// Flag for extra info
	Uint32	checkSum;
	Uint32	dataLen;
}TCPCommand;

enum CommandType
{
	CMDTYPE_UNDEFINED = 0,
	CMDTYPE_REQUEST,
	CMDTYPE_RESPONSE
};

enum NetCameraErrorCode
{
	NETERROR_NOERROR = 0x0000u,
	NETERROR_CONNECT_REFUSED = 0x0001u,

	NETERROR_INVALID_DATA_LENGTH = 0x2001u,
	NETERROR_INVALID_DATA_TYPE = 0x2002u,
	NETERROR_INVALID_COMMAND_TYPE = 0x2003u,
	NETERROR_INVALID_COMMAND = 0x2004u,
	NETERROR_INVALID_CHECKSUM = 0x2005u,
	NETERROR_MEMORY_ERROR = 0x2006u,
	NETERROR_NOSUPPORT_COMMAND = 0x2007u,
	NETERROR_INVALID_PARAMETER = 0x2008u,
	NETERROR_IO_OPERATION = 0x2009u,
	NETERROR_TIMEOUT = 0x200Au,
	NETERROR_BAD_MODE = 0x200Bu,
	NETERROR_NOT_READY = 0x200Cu,
	NETERROR_ABORT = 0x200Du,

	NETERROR_CAMERA_ALREADY_CAPTURING = 0x2101u,
	NETERROR_NETWORK = 0x2102u,
	NETERROR_SIZE_OVERFLOW = 0x2103u,
	NETERROR_CAMERA_NOT_CAPTURING = 0x2104u,
	NETERROR_CAMERA_NOT_SOFTTRIGGER = 0x2105u,
	NETERROR_CANT_CHANGE_PARAMETER = 0x2106u,
	NETERROR_MAX
};

#define NET_ENO 					0
#define NET_ETIMEOUT				(-1)
#define NET_ECONNECT				(-2)
#define NET_ENOMEM					(-3)
#define NET_EINVARG					(-4)
#define NET_EDENY					(-5)
#define NET_EBADMODE				(-6)
#define NET_ECHECKSUM				(-7)
#define NET_EIO						(-8)
#define NET_EOVERFLOW				(-9)
#define NET_EINVCMD					(-10)
#define NET_EINVDATA				(-11)

typedef enum {
	CAM_ENO = 0,
	CAM_ETIMEOUT = 1,
	CAM_ECONNECT,
	CAM_EMEM,
	CAM_EINVPARAM,
	CAM_EDENY,
	CAM_EBADMODE,
	CAM_ENOTRDY,
	CAM_ECHECKSUM,
	CAM_EIO,
	CAM_EOVERFLOW,
	CAM_EINVCMD,
	CAM_ESNDDATA,
	CAM_ERCVDATA,
}CamErr;


#define NET_SEND_TIMEOUT			5000 //ms
#define NET_RECV_TIMEOUT			5000 //ms

#define CAM_IMG_SYNC_START			{0x3C, 0x49, 0x3E, 0x20}

#ifdef __cpluscplus
extern "C" {
#endif

extern int ConnectServer(char * serverIp, Uint16 port);

extern CamErr DisConnectServer(int sock);

extern CamErr SetTimeout(int sock, Bool isSend,int timeoutMs);

extern CamErr SendCmd(int sock, Uint16 cmd, void *cmdData, Uint32 dataLen, Uint32 flag, void *replyBuf, Uint32 replyBufLen);


#ifdef __cpluscplus
}
#endif

#endif
