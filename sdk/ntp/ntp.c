#include "common.h"
#include "ntp.h"
#include "log.h"
#include "net_utils.h"

#define NTP_MEM_ALIGN			8

#define SECONDS_PER_DAY			86400
#define UTC_OFFSET				28800
#define DAYS_FROM1900TO2001		36890
#define DAYS_PER_YEAR			365
#define DAYS_PER_4YEARS			1461
#define NTP_PACKET_SIZE			48

struct NtpObj {
	int					sock;		//socket for data transfer
	struct sockaddr_in	serverAddr;	//server sock address
};

static inline void ntp_construct_packet(Int8 *buf)
{
	bzero(buf, NTP_PACKET_SIZE);
	*((Uint32*)buf) = 0xFA06041B;
}

static void ntp_parse_packet(const Int8 *buf, DateTime *pTime)
{
	unsigned long timestamp = 0;
	char* p = (char*)&timestamp	;
	Uint32 i, d, s, m, y1, y4;
	const Uint32 *days;
	const Uint32 daysToMonth365[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365}; 
	const Uint32 daysToMonth366[] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}; 
	
	// get NTP time and change it to little-Endian
	for(i = 0; i < 4; i++)
		*(p + i) = *(buf + 43 - i);
	timestamp += UTC_OFFSET;			// change the time zone from UTC to GMT+8
	
	// calculate time part
	s = (Uint32)(timestamp % SECONDS_PER_DAY);
	pTime->second = s % 60;
	s /= 60;
	pTime->minute = s % 60;
	pTime->hour = (s / 60) % 24;
	
	// calculate date part
	// d = number of days since 1/1/2001 
	d = (int)(timestamp / SECONDS_PER_DAY) - DAYS_FROM1900TO2001;
	// get 1-based day in week									
	pTime->weekDay = (d % 7) + 1;
	// y4 = number of whole 4-year periods within 100-year period 
	y4 = d / DAYS_PER_4YEARS;				
	// d = day number within 4-year period
	d -= y4 * DAYS_PER_4YEARS;
	// y1 = number of whole years within 4-year period 
	y1 = d / DAYS_PER_YEAR;	
	// Last year has an extra day, so decrement result if 4 				
	if(y1 == 4) 
		y1 = 3;								
	// d = day number within year				
	d -= y1 * DAYS_PER_YEAR;
	// get year
	pTime->year = 2001 + y4 * 4 + y1;
	
	days = (y1 == 3) ? &daysToMonth366[0] : &daysToMonth365[0];
	// All months have less than 32 days, so n >> 5 is a good conservative
	// estimate for the month
	m = (d >> 5) + 1;
	// get 1-based month number
	while(d > days[m])
		m++;
	pTime->month = m;
	
	// get 1-based day-of-month 
	pTime->day = d - days[m - 1] + 1;
	pTime->us = pTime->ms = 0;

#ifdef _DEBUG
	DBG("ntp get time: %04u.%02u.%02u %02u:%02u:%02u",
		pTime->year, pTime->month, pTime->day, pTime->hour, 
		pTime->minute, pTime->second);
#endif
}


/* create a socket for NTP transfer, bind server IP and port
 * return socket fd on success, INVALID_SOCKET on failure
 */
NtpHandle ntp_create(const Int8 *serverIP, Uint16 port)
{
	NtpHandle	hNtp;

	// Alloc memory
	hNtp = calloc(1, sizeof(struct NtpObj));
	if(!hNtp) {
		ERR("alloc memory failed...");
		return NULL;
	}
	
    // Create our UDP socket
    hNtp->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(hNtp->sock < 0) {
    	ERR("can't alloct socket.");
        goto err_quit;
    }

	// Bind the same as TCP ... Port = Any, IPAddr = Any
	bzero(&hNtp->serverAddr, sizeof(struct sockaddr_in));
    hNtp->serverAddr.sin_family = AF_INET;
	hNtp->serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
    if(bind(hNtp->sock, (struct sockaddr *)&hNtp->serverAddr, sizeof(hNtp->serverAddr)) < 0) {
    	ERR("can't bind socket.");
        goto err_quit;
    }
	
	// Set send and recv timeout
	Int32 err = set_sock_recv_timeout(hNtp->sock, NTP_RECV_TIMEOUT);
	err |= set_sock_send_timeout(hNtp->sock, NTP_SEND_TIMEOUT);
	if(err) {
		ERR("set timeout failed.");
		goto err_quit;
	}

	// Save ip and port
	err = ntp_set_server_info(hNtp, serverIP, port);
	if(err) {
		ERR("set server ip and port failed.");
		goto err_quit;
	}
	
	return hNtp;
	
err_quit:
	
	if(hNtp->sock >= 0)
		close(hNtp->sock);
	free(hNtp);
	return NULL;
}

/* release the socket that have created
 * return 0 on success, -1 when an error occurs
 */
Int32 ntp_delete(NtpHandle hNtp)
{
	if(hNtp == NULL)
		return E_INVAL;

	if(hNtp->sock >= 0)
		close(hNtp->sock);

	free(hNtp);

	return E_NO;
}

/* get current time via NTP
 * return 0 on success, -1 when an error occurs
 */
Int32 ntp_get_time(NtpHandle hNtp, DateTime *pTime)
{
	if(!hNtp || !pTime) {
		return E_INVAL;
	}

	/* Init ntp packet */
	Int8	ntpPacket[NTP_PACKET_SIZE];
	ntp_construct_packet(ntpPacket);

	struct sockaddr_in *addr = &hNtp->serverAddr;	
	socklen_t len = sizeof(*addr);

	//send NTP packet;
	if(sendto(hNtp->sock, ntpPacket, sizeof(ntpPacket), 0, 
		(struct sockaddr *)addr, len) != sizeof(ntpPacket)) {
		ERRSTR("send request to server failed");
		return E_TIMEOUT;
	}
	
	//recv NTP packet
	Int32 rcv = recvfrom(hNtp->sock, ntpPacket, NTP_PACKET_SIZE, 0, (struct sockaddr *)addr, &len);

	//validate data recieved
	if(rcv != NTP_PACKET_SIZE || (Uint8)ntpPacket[0] != 0x1C || *(long long *)(ntpPacket + 24)) {
		ERR("revcfrom server failed...");
		return E_TRANS;
	}

	ntp_parse_packet(ntpPacket, pTime);
	
	return E_NO;
}

/*
 *	Set server IP
 */
Int32 ntp_set_server_info(NtpHandle hNtp, const Int8 * serverIP, Uint16 port)
{
	if(!hNtp)
		return E_INVAL;

	bzero(&hNtp->serverAddr, sizeof(struct sockaddr_in));
    hNtp->serverAddr.sin_family = AF_INET;
	
	if(port)
		hNtp->serverAddr.sin_port = htons(port);
	else
		hNtp->serverAddr.sin_port = htons(NTP_DEFAULT_PORT);

	if(serverIP)
		inet_pton(AF_INET, serverIP, &hNtp->serverAddr.sin_addr);
	
	return E_NO;
}


