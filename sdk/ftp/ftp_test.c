#include "common.h"
#include "ftp_client.h"
#include "log.h"

#define DEF_FTP_SRV		"192.168.0.11"
#define DEF_FTP_PORT	21
#define DEF_USER_NAME	"test"
#define DEF_PASSWD		"123456"
#define DEF_LOOP_CNT	5

#define BUF_LEN			(2*1024*1024)

static void ftp_generate_file_name(char *fileNameBuf, int len)
{
	struct tm tm;
	struct timeval tv;

    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm);

	snprintf(fileNameBuf, len, "%04d%02d%02d_%02d%02d%02d",
				tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, 
				tm.tm_min, tm.tm_sec);
}


static void main_loop(const char *srvIp, Uint16 port, const char *username, const char *passwd, int cnt)
{
	FtpHandle hFtp = ftp_create(NULL, NULL, NULL, NULL);
	
	assert(hFtp);

	int i, err;
	char *bufWr = malloc(BUF_LEN);
	char *bufRd = malloc(BUF_LEN);
	//static char bufWr[BUF_LEN];
	//static char bufRd[BUF_LEN];
	assert(bufWr && bufRd);
	for(i = 0; i < BUF_LEN; i++) {
		bufWr[i] = i%0xFF;
	}

	err = ftp_set_user_name(hFtp,username);
	err |= ftp_set_password(hFtp, passwd);
	err |= ftp_set_server_ip(hFtp, srvIp);
	err |= ftp_set_server_port(hFtp, port);
	assert(err == 0);

	err = ftp_connect_server(hFtp, 0);
	if(err) {
		INFO("conncet server failed...");
		goto exit;
	}

	/* File send test */
	char fileName[256], dirPath[256], homePath[256];
	memset(dirPath, 0, sizeof(dirPath));
	err = ftp_get_working_dir(hFtp, dirPath, sizeof(dirPath));
	assert(err == 0);
	DBG("Current dir: %s", dirPath);
	strcpy(homePath, dirPath);

	err = ftp_get_list(hFtp, bufRd, BUF_LEN, dirPath, &i);

#if 1		
	for(i = 0; i < cnt; i++) {
		/* Dir test */
		err = ftp_get_working_dir(hFtp, bufRd, BUF_LEN);
		snprintf(dirPath, sizeof(dirPath), "%s/%u", bufRd, i);
		err = ftp_make_dir(hFtp, dirPath);
		//err = ftp_delete_dir(hFtp, bufWr);	
		//err = ftp_change_working_dir(hFtp, bufWr);
		usleep(1000);
	}
#endif

	
	Uint32 fileSize = BUF_LEN, fileSizeRecv;
	char fileNewName[256];
	struct timeval tmStart,tmEnd; 
	float   timeUse;
	
	for(i = 0; i < cnt; i++) {
		
		//ftp_generate_file_name(fileName, sizeof(fileName));
		snprintf(fileName, sizeof(fileName), "%s/test%d.dat", dirPath, i);
		
	#if 1
		DBG("start upload file: %s", fileName);

		gettimeofday(&tmStart,NULL);
		err = ftp_upload_file(hFtp, bufWr, fileSize, fileName);
		gettimeofday(&tmEnd,NULL); 
		if(err)
			continue;
		
		timeUse = 1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec; 
		DBG("upload %s ok, %d bytes transfered, cost %.0f us.", fileName, fileSize, timeUse);
	#endif
	#if 1	
		memset(bufRd, 0, BUF_LEN);

		DBG("start download file: %s", fileName);

		gettimeofday(&tmStart,NULL);
		err = ftp_download_file(hFtp, bufRd, BUF_LEN, fileName, &fileSizeRecv);
		gettimeofday(&tmEnd,NULL); 
		if(err) {
			DBG("download file: %s failed.", fileName);
			continue;
		}

		timeUse = 1000000*(tmEnd.tv_sec-tmStart.tv_sec)+tmEnd.tv_usec-tmStart.tv_usec; 
		DBG("download file %s ok, size: %d, cost: %.0f us", fileName, fileSizeRecv, timeUse);
		err = memcmp(bufWr, bufRd, fileSizeRecv);
		//assert(err == 0);
		if(err)
			DBG("file data different.!");
	#endif
		DBG("rename file");

		snprintf(fileNewName, sizeof(fileNewName), "%s/rename%d.dat", dirPath, i);
		err = ftp_rename(hFtp, fileNewName, fileName);

		DBG("delete file");
		err = ftp_delete_file(hFtp, fileName);
		err = ftp_delete_file(hFtp, fileNewName);
		if(err)
			DBG("delete file %s err.\n", fileName);
		else
			DBG("delete file %s ok.\n", fileName);

		
		err = ftp_keep_alive(hFtp);
	}

	//rename dir
	err = ftp_change_working_dir(hFtp, homePath);
	sprintf(fileNewName, "%s/rename", homePath);
	sprintf(fileName, "%s/%u", homePath, i-1);
	
	err = ftp_rename(hFtp, fileNewName, fileName);
	if(err)
		DBG("rename %s to %s failed ", fileName, fileNewName);

	err = ftp_disconnect_server(hFtp);
	if(err)
		INFO("<%d> disconnect server failed...", i);

exit:
	err = ftp_delete(hFtp);
	assert(err == E_NO);
	DBG("ftp test complete.");
	
}

static void usage(void)
{
    printf("testFtp Compiled on %s %s with gcc %s\n", __DATE__, __TIME__, __VERSION__);
    printf("Usage:\n");
    printf("./testFtp [options]\n");
    printf("Options:\n");
    printf(" -h get help\n");
	printf(" -s server ip, default: %s\n", DEF_FTP_SRV);
	printf(" -p server port, default: %u\n", DEF_FTP_PORT);
	printf(" -u user name for login, default: %s\n", DEF_USER_NAME);
	printf(" -w password for this user, default: %s\n", DEF_PASSWD);
	printf(" -n loop count, default: %d\n", DEF_LOOP_CNT);
    printf("Example:\n");
    printf(" use default params: ./testFtp\n");
    printf(" use specified params: ./testFtp -s 192.168.0.15\n");
}

int main(int argc, char **argv)
{
	int c;
    char *options = "s:p:u:w:n:h";
	Uint16 port = DEF_FTP_PORT;
	char *ftpSrv = DEF_FTP_SRV;
	char *username = DEF_USER_NAME;
	char *passwd = DEF_PASSWD;
	int cnt = DEF_LOOP_CNT;

	while ((c=getopt(argc, argv, options)) != -1) {
		switch (c) {
		case 's':
			ftpSrv = optarg;
			break;
		case 'p':
			port = atoi(optarg);
			break;
		case 'u':
			username = optarg;
			break;
		case 'w':
			passwd = optarg;
			break;
		case 'n':
			cnt = atoi(optarg);
			break;
		case 'h':
		default:
			usage();
			return -1;
		}
	}

	main_loop(ftpSrv, port, username, passwd, cnt);

	exit(0);
}

