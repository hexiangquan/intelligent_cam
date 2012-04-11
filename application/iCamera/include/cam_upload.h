#ifndef __CAM_UPLOAD_H__
#define __CAM_UPLOAD_H__

#include "common.h"

/* 
  * RTP params for H.264 transfer 
  */
typedef struct {
	Int8	dstIp[16];		//Destination IP address
	Uint16	dstPort;		//Destination port
	Uint16	localPort;		//Local port for RTP sending
	Uint16	flag;			//Control flag 
	Uint8	payloadType;	//Payload type, 0~200	
	Uint8	reserved;
} CamRtpParams;

#define CAM_RTP_FLAG_EN		(1 << 0) //Enable Rtp send

/* 
  * Image upload type 
  */
typedef enum {
	CAM_UPLOAD_PROTO_TCP = 0,
	CAM_UPLOAD_PROTO_FTP =1,	
	CAM_UPLOAD_PROTO_NONE,		//don't upload, save to local file system
	CAM_UPLOAD_PROTO_RTP,
	CAM_UPLOAD_PROTO_MAX,
} CamImgUploadProto;

/* 
  * TCP image recv server info 
  */
typedef struct {
	Int8	serverIP[16];	//Image server IP
	Uint16	serverPort;		//Image server listen port
	Uint16	flag;			//control flag, reserved
} CamTcpImageServerInfo;

/* 
  * FTP image server info 
  */
#define CAM_FTP_MAX_USERNAME_LEN		32
#define CAM_FTP_MAX_PASSWORD_LEN		32
#define CAM_FTP_MAX_PATHNAME_PATTERN	256
#define CAM_FTP_MAX_ROOT_DIR_LEN		64

typedef struct {
	Int8	serverIP[16];						//Ftp server Ip
	Uint16	serverPort;							//Listen port
	Uint16	flag;								//Flag for other function
	Int8	userName[CAM_FTP_MAX_USERNAME_LEN];	//User name
	Int8	password[CAM_FTP_MAX_PASSWORD_LEN];	//password
	Int8	rootDir[CAM_FTP_MAX_ROOT_DIR_LEN];	//root dir name
	Int8	pathNamePattern[CAM_FTP_MAX_PATHNAME_PATTERN];	//pathname generate pattern
} CamFtpImageServerInfo;

/*
  * NTP info for sync
  */
typedef struct {
	Uint8	serverIP[16];	//NTP server IP
	Uint16	serverPort;		//Listen port
	Uint16	syncPrd;		//sync interval period, unit: hour, set to 0 to disable sync
} CamNtpServerInfo;

#endif

