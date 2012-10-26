#include "common.h"
#include <string>
#include <iostream>
#include "dspSrv.h"
#include <signal.h>

#define PROGRAM_NAME	"dspSrv"
#define DEF_SRV_IP		""				// get srv ip from cam server
#define DEF_SRV_PORT	0
#define DEF_CAM_SRV_MSG	"/tmp/iCamCtrl"

using std::string;
using std::cin;
using std::cout;
using std::endl;

/**
 * Help function for this program
 */
static void usage(void)
{
	cout << PROGRAM_NAME << " Compiled on " << __DATE__ 
		<< " " << __TIME__ << " with gcc " << __VERSION__ << endl;
	cout << "Usage:" << endl;
	cout << "./" << PROGRAM_NAME << " [options]" << endl;
	cout << "  -h get help" << endl;
	cout << "  -s server ip, default: " << DEF_SRV_IP << endl;
	cout << "  -p server port, default: " << DEF_SRV_PORT << endl;
}

/**
 * signal handler function
 */
static void sig_handler(int sig)
{
	if(sig == SIGINT || sig == SIGABRT)
		DspSrv::Stop();
}

/**
 * main function
 */
int main(int argc, char **argv)
{
	const char *options = "s:p:h";
	int ret;
	string srvIp(DEF_SRV_IP);
	uint16_t port(DEF_SRV_PORT);

	int c;
	while((c = getopt(argc, argv, options)) != -1) {
		switch(c) {
			case 's':
				srvIp = optarg;
				break;
			case 'p':
				port = atoi(optarg);
				break;
			case 'h':
			default:
				usage();
				return -1;
		}
	}

	signal(SIGINT, sig_handler);
	signal(SIGPIPE, SIG_IGN);

	DspSrv *pDspSrv = new DspSrv(srvIp, port);
	ret = pDspSrv->Run();

	delete pDspSrv;

	return ret;
}
