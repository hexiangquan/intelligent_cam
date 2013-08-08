#ifndef __DSP_SRV_H__
#define __DSP_SRV_H__

#include <iostream>
#include <string>
#include <unistd.h>
#include <pthread.h>

// default channel buffer size 
#define CHAN_BUF_LEN	(1*1024*1024)

class DspSrv {
public:
	DspSrv(uint32_t addr): baseAddr(addr), pid(0), isRunning(0), exit(0), bufSize(CHAN_BUF_LEN) {}
	DspSrv(uint32_t addr, size_t bufLen): baseAddr(addr), pid(0), isRunning(0), exit(0), bufSize(bufLen) {}
	DspSrv(uint32_t addr, size_t bufLen, std::string name): baseAddr(addr), pid(0), isRunning(0), exit(0), bufSize(bufLen), chanName(name) {}
	virtual ~DspSrv() { if(isRunning) Stop(); }
	int Run();
	virtual int Stop();

private:
	uint32_t baseAddr;
	pthread_t pid;
	bool isRunning;
	bool exit;
	size_t bufSize;

protected:
	std::string chanName;

private:
	int SyslinkOpen();
	static void *ProcessThread(void *arg);
	virtual int ProcessLoop(int syslinkFd, size_t bufLen) = 0; 
		//{ std::cout << "Process loop" << std::endl;  return 0; }

protected:
	bool StopRunning() { return exit; }
};

#endif
