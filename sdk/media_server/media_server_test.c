#include "media_server.h"
#include "log.h"
#include <signal.h>

#define PROG_NAME		"mediaSrvTest"

#define SRV_PORT_DEF	554
#define SESS_NAME		"h264"
#define SESS_DESP		"h.264 video from IPNC"
#define VID_BIT_RATE	4000000
#define DEF_TEST_FILE	"420.h264"

typedef struct _TestParams {
	Uint16	srvPort;
	const char *fileName;
}TestParams;

static Bool s_exit = FALSE;

static void sig_handler(int sig)
{
	if(sig == SIGINT || sig == SIGALRM) {
		DBG("got INT signal");
		s_exit = TRUE;
	}
	//alarm(1);
} 

void sigterm(int dummy)
{
	printf("caught SIGTERM: shutting down\n");
	
	//exit(1);
}

int nal_search(FILE *fp, int offset) {
	unsigned long testflg = 0;
	int result = 0;

	for(;;) {
		fseek(fp, offset, SEEK_SET);
		if( fread(&testflg, sizeof(testflg), 1, fp) <=  0 ) {
			result = -1;
			break;
		}

		//printf("testflg=0x%x \n",(int)testflg );
		
		if( testflg == 0x01000000 ) {
			break;
		}
		
		offset++;
	}

	if(result)
		return result;
	
	return offset;
	
}

int get_file_frame(FILE *fp, void *buf, int *size) {
	static int offset = 0;
	int offset1 = 0;
	int offset2 = 0;
	int framesize = 0;

	if( fp == NULL )
		return -1;
	
	fseek(fp, offset, SEEK_SET);
	
	offset1 = nal_search(fp, offset);
	if(offset1 < 0) {
		/* got the end of the file */
		fseek(fp, 0, SEEK_SET);
		offset = 0;
		return -1;
	}
	offset2 = nal_search(fp, offset1+4);
	if(offset2 < 0) {
		/* got the end of the file */
		fseek(fp, 0, SEEK_SET);
		offset = 0;
		return -1;
	}
	
	framesize = offset2 - offset1;

	/*reset position*/
	fseek(fp, offset1, SEEK_SET);
	int ret = fread(buf, framesize, 1, fp);
	if(ret < 0) {
		return -1;
	}

	offset = offset2;

	*size = framesize;
	
	return 0;
}

static Bool main_loop(TestParams *params)
{
	Bool ret = FALSE;
	MediaSrvHandle 		hSrv;
	MediaSessionHandle	hSession;
	MediaSubSessionHandle hSubSession;
	Int32 err;
	FILE *fp;
	
	/* catch signals */
	signal(SIGINT, sig_handler);

	
	hSrv = media_srv_create(params->srvPort);
	assert(hSrv != NULL);

	hSession = media_srv_create_session(hSrv, SESS_NAME, SESS_DESP, STREAMING_UNICAST);
	assert(hSession != NULL);

	hSubSession = media_srv_add_sub_session(hSession, MEDIA_TYPE_H264, 0, 0, VID_BIT_RATE);
	assert(hSubSession != NULL);

	fp = fopen(params->fileName, "rb");
	if(!fp) {
		ERR("can't open %s", params->fileName);
		goto exit;
	}

	MediaFrame frame;

	frame.data = malloc(64 * 1024);
	assert(frame.data);
	frame.frameType = FRAME_TYPE_I;
	frame.index = 0;

	err = media_srv_run(hSrv);
	if(err)
		goto exit;

	while(!s_exit) {
		// read a frame
		err = get_file_frame(fp, frame.data, &frame.dataLen);
		if(err < 0)
			continue;
		gettimeofday(&frame.timestamp, NULL);
		// write buffer 
		err = media_stream_in(hSubSession, &frame, TRUE);
		if(err && err != E_MODE) {
			ERR("write media err: %d", err);
		}
		frame.index++;
		usleep(30000);
	}

	ret = TRUE;
exit:

	DBG("deleting server...");
	media_srv_delete(hSrv);
	return ret;
}

static void usage(void)
{
    INFO("Compiled at %s %s with gcc %s", __DATE__, __TIME__, __VERSION__);
    INFO("Usage:");
    INFO("./%s [options]", PROG_NAME);
    INFO("Options:");
    INFO(" -h get help");
	INFO(" -p server port, default: %d", SRV_PORT_DEF);
	INFO(" -f test h.264 file name, default: %s", DEF_TEST_FILE);
    INFO("Example:");
    INFO(" use default params: ./%s", PROG_NAME);
    INFO(" use specific params: ./%s -p 8556", PROG_NAME);
}

int main(int argc, char **argv)
{
	int c;
    char *options = "p:f:h";
	TestParams params;
	
	params.srvPort = SRV_PORT_DEF;
	params.fileName = DEF_TEST_FILE;

	while ((c = getopt(argc, argv, options)) != -1) {
		switch (c) {
		case 'p':
			params.srvPort = atoi(optarg);
			break;
		case 'f':
			params.fileName = optarg;
			break;
		case 'h':
		default:
			usage();
			return -1;
		}
	}

	Bool ret = main_loop(&params);
	if(ret)
		INFO("test success!");
	else
		INFO("test failed!");

	exit(0);
}


