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
	int SyslinkOpen();
	static void *ProcessThread(void *arg);
	virtual int ProcessLoop(int syslinkFd) = 0; 
		//{ std::cout << "Process loop" << std::endl;  return 0; }

protected:
	bool StopRunning() { return exit; }
};

#endif
