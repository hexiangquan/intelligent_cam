#include "net_utils.h"

#define DEF_SERVER_IP	"192.168.0.11"
#define DEF_SERVER_PORT	21

static void main_loop(const char *serverIp, Uint16 port)
{
	int sockTcp = socket_tcp_server(56790, 5);
	assert(sockTcp >= 0);
	int sockUdp = socket_udp_server(56790);
	assert(sockUdp >= 0);

	int err = set_sock_send_timeout(sockTcp, 5);
	err |= set_sock_recv_timeout(sockTcp, 5);
	assert(err == 0);

	err = set_sock_buf_size(sockUdp, 512 * 1024, 512 * 1024);
	err |= set_sock_linger(sockUdp, TRUE, 2);
	err |= set_sock_block(sockUdp, FALSE);
	DBG("err = %d", err);
	assert(err == 0);

	close(sockTcp);
	close(sockUdp);

	sockTcp = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in addr;
	init_sock_addr(&addr, serverIp, port);

	err = connect_nonblock(sockTcp, (struct sockaddr *)&addr, sizeof(addr), 5000);
	if(err < 0) {
		ERR("Connect nonblocking server failed.");
		goto exit;
	} else {
		DBG("Connect to %s:%u success!", serverIp, port);
	}

	close(sockTcp);
	close(sockUdp);
	sockTcp = sockUdp = -1;
	
	sockTcp = socket(AF_INET, SOCK_STREAM, 0);
	err = connect(sockTcp, (struct sockaddr *)&addr, sizeof(addr));
	if(err < 0) {
		ERR("Connect server failed.");
		goto exit;
	} else {
		DBG("Connect to %s:%u success!", serverIp, port);
	}

	sockUdp = socket(AF_INET, SOCK_DGRAM, 0);
	char *sendLine = "Hello";
	err = sendto(sockUdp, sendLine, strlen(sendLine), 0, (struct sockaddr *)&addr, sizeof(addr));
	if(err < 0)
		ERR("Send to failed.");
	else
		DBG("Sendto ok...");

exit:

	if(sockTcp > 0)
		close(sockTcp);
	if(sockUdp > 0)
		close(sockUdp);
	
}

static void usage(void)
{
    printf("netUtilsTest Compiled on %s %s with gcc %s\n", __DATE__, __TIME__, __VERSION__);
    printf("Usage:\n");
    printf("./netUtilsTest [options]\n");
    printf("Options:\n");
	printf(" -s server ip, default: %s\n", DEF_SERVER_IP);
	printf(" -p server port, default: %u\n", DEF_SERVER_PORT);
    printf(" -h get help\n");
    printf("Example:\n");
    printf(" use default params: ./netUtilsTest\n");
}

int main(int argc, char **argv)
{
	int c;
    char *options = "s:p:h";
	char *serverIp = DEF_SERVER_IP;
	Uint16 port = DEF_SERVER_PORT;

	while ((c=getopt(argc, argv, options)) != -1)
	{
		switch (c)
		{
			case 's':
				serverIp = optarg;
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

	main_loop(serverIp, port);

	exit(0);
}

