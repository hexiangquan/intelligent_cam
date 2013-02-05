#ifndef __DSP_SRV_H__
#define __DSP_SRV_H__

#include <iostream>
#include <string>
#include <unistd.h>
#include <pthread.h>

class DspSrv {
public:
	DspSrv(uint32_t addr): baseAddr(addr), pid(0), isRunning(0), exit(0) {}
	~DspSrv() { if(isRunning) Stop(); }
	int Run();
	int Stop();

private:
	uint32_t baseAddr;
	pthread_t pid;
	bool isRunning;
	bool exit;

private:
	int WaitOpen(int fd, uint32_t& type);
	int SyslinkOpen();
	static void *ProcessThread(void *arg);
};

#endif
