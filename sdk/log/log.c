#include <assert.h>
#include "log.h"
#include "common.h"

static void log_generate_file_name(LogHandle hLog, char *fileNameBuf, int len)
{
	struct tm tm;
	struct timeval tv;

    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm);

	snprintf(fileNameBuf, len, "%s_%04d%02d%02d_%02d%02d%02d", hLog->filePathName,
				tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, 
				tm.tm_min, tm.tm_sec);
}

LogHandle log_create(LogLevel_t level, int flag, const char *pathName, int maxFileSize)
{
	LogHandle	hLog;
	FILE		*fp = NULL;
	char		*logFileName;
	char		fileNameBuf[LOG_MAX_PATH_NAME_LEN];

	hLog = calloc(1, sizeof(LogObj));
	if(!hLog) {
		ERR("Alloc mem failed.");
		return NULL;
	}

	if(flag & LOG_FLAG_SAVE_TO_FILE) {
		if(!pathName)
			logFileName = LOG_DEF_FILE;
		else
			logFileName = (char *)pathName;
		
		strncpy(hLog->filePathName, logFileName, sizeof(hLog->filePathName));
		log_generate_file_name(hLog, fileNameBuf, sizeof(fileNameBuf));
		fp = fopen(fileNameBuf, "ab+");
		if(!fp) {
			ERRSTR("open log file %s failed", fileNameBuf);
			goto err_quit;
		}
		
		hLog->fp = fp;
	}

	if(level > LOG_LEVEL_ERR)
		level = LOG_LEVEL_ERR;
	else if(level < LOG_LEVEL_DBG)
		level = LOG_LEVEL_DBG;

	hLog->level = level;
	hLog->fileCnt = 1;

	if(maxFileSize <= 0)
		hLog->maxFileSize = LOG_DEF_MAX_FILE_SIZE;
	else
		hLog->maxFileSize = maxFileSize;

	if(flag)
		hLog->flag = flag & LOG_FLAG_MASK;
	else {
		INFO("using default log flag.");
		hLog->flag = LOG_DEF_FLAG;
	}

	if(flag & LOG_FLAG_USE_MUTEX)
		pthread_mutex_init(&hLog->mutex, NULL);
	
	return hLog;
	
err_quit:	
	if(fp)
		fclose(fp);
	if(hLog)
		free(hLog);
	return NULL;
}

int log_delete(LogHandle hLog)
{
	assert(hLog);
	if(hLog->fp)
		fclose(hLog->fp);

	if(hLog->flag & LOG_FLAG_USE_MUTEX)
		pthread_mutex_destroy(&hLog->mutex);
	free(hLog);

	return E_NO;
}

static inline void log_switch_file(LogHandle hLog)
{
	FILE *fp;
	char fileName[LOG_MAX_PATH_NAME_LEN];
	
	fclose(hLog->fp);

	log_generate_file_name(hLog, fileName, sizeof(fileName));
	hLog->fileCnt++;
	
	fp = fopen(fileName, "ab+");
	if(!fp) {
		ERRSTR("open log file %s failed", fileName);
	}
		
	hLog->fp = fp;
}


void log_run(LogHandle hLog, int flag, const char *fmt, ...)
{
	int off = 0;
    char buf[1024] = "";
    int size = sizeof(buf);
    va_list va;
	int errno_save = errno;

	assert(hLog);

	if(hLog->flag & LOG_FLAG_TIME) {
		struct tm tm;
    	struct timeval tv;
	
	    gettimeofday(&tv, NULL);
	    localtime_r(&tv.tv_sec, &tm);

		off += snprintf(buf+off, size-off, "[%04d/%02d/%02d %02d:%02d:%02d] ", 
					tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	}
	
    va_start(va, fmt);
    off += vsnprintf(buf+off, size-off, fmt, va);
    va_end(va);

	if(hLog->flag & LOG_FLAG_ERR_STR || flag & LOG_FLAG_ERR_STR) {
		off += snprintf(buf + off, size - off, ": %s", strerror(errno_save));
	}

	strcat(buf, "\n");

    if(hLog->flag & LOG_FLAG_PRINT) 
		fprintf(stderr, "%s", buf);

	if(hLog->flag & LOG_FLAG_SAVE_TO_FILE && hLog->fp){
		if(hLog->flag & LOG_FLAG_USE_MUTEX)
			pthread_mutex_lock(&hLog->mutex);

		fprintf(hLog->fp, "%s", buf);
		fflush(hLog->fp);
		if(ftell(hLog->fp) >= hLog->maxFileSize)
			log_switch_file(hLog);
		
		if(hLog->flag & LOG_FLAG_USE_MUTEX)
			pthread_mutex_unlock(&hLog->mutex);
    }

	return;
}

int log_set_flag(LogHandle hLog, int flag)
{
	if(!hLog)
		return E_INVAL;

	hLog->flag = flag & LOG_FLAG_MASK;
	return E_NO;
}

int log_set_level(LogHandle hLog, LogLevel_t level)
{
	if(!hLog)
		return E_INVAL;

	if(level > LOG_LEVEL_ERR)
		level = LOG_LEVEL_ERR;
	else if(level < LOG_LEVEL_DBG)
		level = LOG_LEVEL_DBG;

	hLog->level = level;
	return E_NO;
}

int log_set_max_file_size(LogHandle hLog, int maxSize)
{
	if(!hLog || maxSize <= 0)
		return E_INVAL;

	hLog->maxFileSize = maxSize;
	return E_NO;
}

