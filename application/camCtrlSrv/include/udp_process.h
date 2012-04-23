#ifndef __UDP_PROCESS_H__
#define __UDP_PROCESS_H__

#include "common.h"

typedef struct {
	Uint16 	cmdListenPort;
	Bool	needReboot;
}UdpProcEnv;

extern Int32 udp_process(Int32 sock, UdpProcEnv *env);

#endif

