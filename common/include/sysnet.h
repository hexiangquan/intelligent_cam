#ifndef __SYS_NET_H__
#define __SYS_NET_H_

/*
*  This file contains .h files that needed by network programming
*/

#include <sys/types.h>		/* basic system data types */
#include <sys/socket.h>		/* basic socket definitions */
#include <sys/time.h>		/* timeval{} for select() */
#include <time.h>			/* timespec{} for pselect() */
#include <netinet/in.h>		/* sockaddr_in{} and other Internet definitions */
#include <arpa/inet.h> 
#include <errno.h>

#include <fcntl.h>			/* for nonblocking */
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include <sys/select.h>
#include <sys/sysctl.h>
#include <sys/ioctl.h>
#include <linux/un.h>

#include <pthread.h>


//typedef struct sockaddr  SA;
//typedef struct sockaddr	*PSA;
//typedef struct sockaddr_in SA_IN;
//typedef struct sockaddr_in *PSA_IN;


#endif
