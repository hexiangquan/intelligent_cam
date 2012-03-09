#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <assert.h>
#include <pthread.h>
#include <sys/types.h>
#include "common.h"

#ifndef __LOG_H__
#define __LOG_H__

typedef enum {
	LOG_LEVEL_DBG	= 0,
	LOG_LEVEL_INFO,
	LOG_LEVEL_WARN,
	LOG_LEVEL_ERR,
	LOG_LEVEL_MAX
}LogLevel_t;

#ifndef LOG_LEVEL
#define LOG_LEVEL 	LOG_LEVEL_DBG
#endif

#if LOG_LEVEL > LOG_LEVEL_DBG
#define DBG(fmt, args...)
#else
#define DBG(fmt, args...)	fprintf(stdout, fmt "\n", ##args)
#endif

#if LOG_LEVEL > LOG_LEVEL_INFO
#define INFO(fmt, args...)
#else
#define INFO(fmt, args...)	fprintf(stdout, fmt "\n", ##args)
#endif

#if LOG_LEVEL > LOG_LEVEL_WARN
#define WARN(fmt, args...)
#else
#define WARN(fmt, args...)	fprintf(stdout, fmt "\n", ##args)
#endif


#define ERR(fmt, args...)		fprintf(stderr, "Err @ func: %s, " fmt "\n", __func__, ##args)

#define ERRSTR(fmt, args...)	fprintf(stderr, "Err @ func: %s, " fmt ": %s\n", __func__, ##args, strerror(errno))

#define LOG_FLAG_SAVE_TO_FILE	(1 << 0)	//save to log file
#define LOG_FLAG_PRINT			(1 << 1)	//print to screen
#define LOG_FLAG_ERR_STR		(1 << 2)	//print errno string
#define LOG_FLAG_TIME			(1 << 3)	//print time
#define LOG_FLAG_USE_MUTEX		(1 << 4)	//use mutex lock for multi threads

#define LOG_FLAG_MASK \
	(LOG_FLAG_SAVE_TO_FILE | LOG_FLAG_PRINT | LOG_FLAG_ERR_STR | LOG_FLAG_TIME | LOG_FLAG_USE_MUTEX)

#define LOG_DEF_FILE			"/tmp/tmpLog"
#define LOG_DEF_MAX_FILE_SIZE	(1024 * 1024)
#define LOG_DEF_FLAG			(LOG_FLAG_SAVE_TO_FILE | LOG_FLAG_PRINT | LOG_FLAG_TIME)

#define LOG_MAX_PATH_NAME_LEN	1024

typedef struct {

	FILE 		*fp; 			//fd for save log file
	LogLevel_t	level;			//log level
	Int32		flag;			//flags
	Int32		maxFileSize;
	Int8		filePathName[LOG_MAX_PATH_NAME_LEN];
	Int32		fileCnt;
	pthread_mutex_t mutex;
}LogObj, *LogHandle;

#ifdef __cplusplus
extern "C" {
#endif

LogHandle log_create(LogLevel_t level, int flag, const char * pathName, int maxFileSize);

int log_delete(LogHandle hLog);

int log_set_flag(LogHandle hLog,int flag);

int log_set_level(LogHandle hLog, LogLevel_t level);

void log_run(LogHandle hLog, int flag, const char * fmt,...);

#define log_print(hLog, fmt, args...) \
	log_run(hLog, 0, fmt, ##args)

#define log_error(hLog, fmt, args...) \
	do { \
		if(hLog->level <= LOG_LEVEL_ERR) \
			log_run(hLog, 0, "Err @ func %s, "fmt, __func__, ##args); \
	}while(0)

#define log_warning(hLog, fmt, args...) \
	do { \
		if(hLog->level <= LOG_LEVEL_WARN) \
			log_run(hLog, 0, "Warning @ func %s, "fmt, __func__, ##args); \
	}while(0)

#define log_debug(hLog, fmt, args...) \
	do { \
		if(hLog->level <= LOG_LEVEL_DBG) \
			log_run(hLog, 0, "Func %s, "fmt, __func__, ##args); \
	}while(0)
	
#define log_errorno(hLog, fmt, args...) \
	do { \
		if(hLog->level <= LOG_LEVEL_ERR) \
			log_run(hLog, LOG_FLAG_ERR_STR, "Func %s, "fmt, __func__, ##args); \
	}while(0)

#ifdef __cpluscplus
}
#endif

#endif

