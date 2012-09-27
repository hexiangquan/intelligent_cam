#ifndef _NTP_H_
#define _NTP_H_

#include "common.h"

#define NTP_DEFAULT_PORT	123
#define NTP_SEND_TIMEOUT	5	//s
#define NTP_RECV_TIMEOUT	5 	//s

#ifdef __cplusplus
extern "C" {
#endif


/* NTP Handle define */
typedef struct NtpObj *NtpHandle;


/* create a handle for NTP transfer, bind server IP and port
  * return NtpHandle when success, return NULL when fails
  */
NtpHandle ntp_create(const Int8 *serverIP, Uint16 port);

/* get current time via NTP
  * return error code
  */
Int32 ntp_get_time(NtpHandle hNtp, DateTime *pTime);

/* release the socket that have created
  * return error code
  */
Int32 ntp_delete(NtpHandle hNtp);

/*
 *	Set server IP
 */
Int32 ntp_set_server_info(NtpHandle hNtp, const Int8 *serverIP, Uint16 port);

#ifdef __cplusplus
}
#endif

#endif


