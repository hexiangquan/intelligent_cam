#include "common.h"
#include <string>
#include <iostream>
#include "dspNetSrv.h"
#include "dspFileSrv.h"
#include <signal.h>
#include "log.h"
#include "syslink_proto.h"

#define PROGRAM_NAME	"dspSrv"
#define DEF_SRV_IP		""				// get srv ip from cam server
#define DEF_SRV_PORT	0

using std::string;
using std::cin;
using std::cout;
using std::endl;

static pthread_cond_t g_cond;
static pthread_mutex_t g_mutex;

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

#if 1
/**
 * signal handler function
 */
static void sig_handler(int sig)
{
	if(sig == SIGINT || sig == SIGABRT) {
		DBG("%s, catch stop signal...", PROGRAM_NAME);
		pthread_cond_signal(&g_cond);
		return ;
	}
}
#endif

/**
 * main function
 */
int main(int argc, char **argv)
{
	const char *options = "s:p:h";
	int ret = 0;
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

	pthread_cond_init(&g_cond, NULL);
	pthread_mutex_init(&g_mutex, NULL);

	signal(SIGINT, sig_handler);
	signal(SIGPIPE, SIG_IGN);

	// create objects and start running
	DspNetSrv *netSrv[4];
	netSrv[0] = new DspNetSrv(LINK_NET0_BASE, NET0_CHAN_LEN, "net0");
	netSrv[1] = new DspNetSrv(LINK_NET1_BASE, NET1_CHAN_LEN, "net1");
	netSrv[2] = new DspNetSrv(LINK_NET2_BASE, NET2_CHAN_LEN, "net2");
	netSrv[3] = new DspNetSrv(LINK_NET3_BASE, NET3_CHAN_LEN, "net3");

	for(int i = 0; i != ARRAY_SIZE(netSrv); ++i) {
		netSrv[i]->Run();
	}

	// create channel for file server
	DspFileSrv fileSrv(LINK_FILE_BASE, FILE_CHAN_LEN, "file");

	// wait stop signal
	pthread_cond_wait(&g_cond, &g_mutex);

	// stop running and delete objects
	for(int i = 0; i != ARRAY_SIZE(netSrv); ++i) {
		netSrv[i]->Stop();
		delete netSrv[i];
	}

	pthread_cond_destroy(&g_cond);
	pthread_mutex_destroy(&g_mutex);

	return ret;
}


