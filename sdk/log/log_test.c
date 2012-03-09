#include "log.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "common.h"

#define DEF_LOG_PATH_NAME	"test"
#define DEF_LOG_LEVL		LOG_LEVEL_DBG

static void main_loop(int flag, LogLevel_t level, const char *logName)
{
	int cnt = 0;
	DBG("Log Test Start: %d.", cnt++);
	INFO("Info: %d", cnt++);
	ERR("Err: %d", cnt++);
	FILE *fp = fopen("not_exit", "b");
	ERRSTR("Open none exist file");

	LogHandle hLog;

	hLog = log_create(level, flag, logName, 1024 * 1024);
	assert(hLog != NULL);
	
	for(level = LOG_LEVEL_DBG; level < LOG_LEVEL_MAX; level++)
	{
		log_print(hLog, "Log level: %d", level);
		log_set_level(hLog, level);
		
		log_debug(hLog, "Debug info, log name %s", logName);
		log_warning(hLog,"Warning, flag: %u, level: %d", flag, level);
		log_error(hLog,"Error, log handle addr: 0x%X", hLog);
	}

	const char *pErr = str_err((int)E_BUSY);
	printf("Err string: %s\n", pErr);

	log_print(hLog, "Log test success!");
}

static void usage(void)
{
    printf("LogTest Compiled on %s %s with gcc %s\n", __DATE__, __TIME__, __VERSION__);
    printf("Usage:\n");
    printf("./logTest [options]\n");
    printf("Options:\n");
    printf(" -h get help\n");
	printf(" -m use mutex for multi-thread\n");
	printf(" -l log level, 0-debug, 1-info, 2-warning, 3-error, default: %d\n", DEF_LOG_LEVL);
	printf(" -f log file path name, default: %s\n", DEF_LOG_PATH_NAME);
    printf("Example:\n");
    printf(" use default params: ./logTest\n");
    printf(" use specified log name and mutex: ./logTest -f testLog -m\n");
}

int main(int argc, char **argv)
{
	int c;
    char *options = "f:l:m";
	Int32 logFlag = LOG_FLAG_PRINT | LOG_FLAG_SAVE_TO_FILE | LOG_FLAG_TIME;
	char *logName = DEF_LOG_PATH_NAME;
	LogLevel_t level = DEF_LOG_LEVL;

	while ((c=getopt(argc, argv, options)) != -1)
	{
		switch (c)
		{
			case 'm':
				logFlag |= LOG_FLAG_USE_MUTEX;
				break;
			case 'f':
				logName = optarg;
				break;
			case 'l':
				level = atoi(optarg);
				break;
			case 'h':
			default:
				usage();
				return -1;
		}
	}

	main_loop(logFlag, level, logName);

	exit(0);
}


