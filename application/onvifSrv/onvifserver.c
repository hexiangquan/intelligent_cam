
#include "soapStub.h"
#include "soapH.h"
#include "stdsoap2.h"
#include "wsddapi.h"

#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include<time.h>
#include <sys/stat.h>

#include "icam_ctrl.h"
#define ICAME_CTRL_FILR "/tmp/onvifctrl"
#define IPCPROFILE_1 "mainProfile"
#define VIDEOSOURCETOLEN "IPC_VIDEO_SOURCE"  
#define VIDEOSOURCECONFIG "IPCVideoSource"  
#define VIDEOENCODER1 "VideoEncoder1"  

#define ONVIF_IMAGE_WIDTH 2448
#define ONVIF_IMAGE_HEIGHT 2048

static int  INPUT_CONNECTORS_NUM =0;
static int  RELAY_OUTPUTS_NUM =0;

#define EVENTSETFILE "./event.set"
#define SYSFILE "./sys.set"



static int  ONVIF_SERVER_RORT =8088;
static int  ONVIF_EVENT_RORT=9099;

#define ONVIF_HTTP_SNAP_RORT 9098

#define MAX_USER_NUM 20

#define ONVIF__timg__
#define ONVIF__tptz__

//#define ONVIF__TEST__


ICamCtrlHandle g_icam_ctrl_handle;

#ifdef ONVIF__TEST__

ICamCtrlHandle icam_ctrl_create(IN const char *pathName, IN Int32 flags, IN Int32 transTimeout)
{
	return NULL;
}

/* delete icam control handle */
Int32 icam_ctrl_delete(IN ICamCtrlHandle hCtrl){
	return E_NO;

}

/* control capture, i.e. start, stop, restart */
 Int32 icam_capture_ctrl(IN ICamCtrlHandle hCtrl, IN CamCapCtrl ctrl){
 return E_NO;
}

/* get auto exposure params */
 Int32 icam_get_ae_params(IN ICamCtrlHandle hCtrl, OUT CamAEParam *params){
 return E_NO;
}

/* get analog video params */
 Int32 icam_get_av_params(IN ICamCtrlHandle hCtrl, OUT CamAVParam *params){
 return E_NO;
}

/* get auto white balance params */
 Int32 icam_get_awb_params(IN ICamCtrlHandle hCtrl, OUT CamAWBParam *params){
 return E_NO;
}

/* get day night switch mode params */
 Int32 icam_get_day_night_mode_params(ICamCtrlHandle hCtrl, CamDayNightModeCfg *params){
	return E_NO;

}

/* get detector params  */
 Int32 icam_get_detector_params(IN ICamCtrlHandle hCtrl, OUT CamDetectorParam *params){
	return E_NO;

}

/* get device info */
 Int32 icam_get_dev_info(IN ICamCtrlHandle hCtrl, OUT CamDeviceInfo *buf)
{
    buf->deviceSN=1243;
	sprintf(buf->manufacture,"js");
	sprintf(buf->model,"js");
	sprintf(buf->firmware,"js");
	sprintf(buf->hardware,"js");
	sprintf(buf->location,"js");
	sprintf(buf->name,"js");
	return E_NO;

}

/* get exposure params */
 Int32 icam_get_exposure_params(IN ICamCtrlHandle hCtrl, OUT CamExprosureParam *params){
	return E_NO;

}

/* get ftp server info */
 Int32 icam_get_ftp_srv_info(IN ICamCtrlHandle hCtrl, OUT CamFtpImageServerInfo *srvInfo){
	return E_NO;

}


/* get h.264 params */
 Int32 icam_get_h264_params(IN ICamCtrlHandle hCtrl, OUT CamH264Params *params)
{
	params->bitRate=800;
	params->frameRate=25;
	params->IPRatio=25;

 return E_NO;
}

/* get image encode params */
 Int32 icam_get_img_enc_params(IN ICamCtrlHandle hCtrl, OUT CamImgEncParams *params)
{
	return E_NO;

}

/* get image enhance params */
 Int32 icam_get_img_enhance_params(IN ICamCtrlHandle hCtrl, OUT CamImgEnhanceParams *params)
{
return E_NO;
}

/* get image server info */
 Int32 icam_get_img_srv_info(IN ICamCtrlHandle hCtrl, OUT CamTcpImageServerInfo *srvInfo)
{
return E_NO;
}

/* get io config params */
 Int32 icam_get_io_config(IN ICamCtrlHandle hCtrl, OUT CamIoCfg *params)
{
	params->direction=1;
	params->status=0;
   return E_NO;
}

/* get network info */
 Int32 icam_get_network_info(IN ICamCtrlHandle hCtrl, OUT CamNetworkInfo *info)
{
	snprintf(info->hostName,sizeof(info->hostName),"loaclhost");
	snprintf(info->ipAddr,sizeof(info->ipAddr),"192.178.117.4");
	snprintf(info->ipMask,sizeof(info->ipMask),"255.255.255.0");
	snprintf(info->gatewayIP,sizeof(info->gatewayIP),"192.178.117.1");
	snprintf(info->dnsServer,sizeof(info->dnsServer),"192.178.117.1");
	snprintf(info->domainName,sizeof(info->domainName),"loacdomainName");

return E_NO;
}

/* get ntp server info */
 Int32 icam_get_ntp_srv_info(IN ICamCtrlHandle hCtrl, OUT CamNtpServerInfo *srvInfo)
{
 sprintf(srvInfo->serverIP,"192.168.12.25");
 srvInfo->syncPrd=72;
return E_NO;
}

/* get osd params */
 Int32 icam_get_osd_params(IN ICamCtrlHandle hCtrl, OUT CamOsdParams *params)
{
return E_NO;
}

/* get rgb gains */
 Int32 icam_get_rgb_gains(IN ICamCtrlHandle hCtrl, OUT CamRGBGains *params)
{
return E_NO;
}

/* get road info */
 Int32 icam_get_road_info(IN ICamCtrlHandle hCtrl, OUT CamRoadInfo *buf)
{
return E_NO;
}

/* get rtp params */
 Int32 icam_get_rtp_params(IN ICamCtrlHandle hCtrl, OUT CamRtpParams *buf)
{
	buf->rtspSrvPort=554;
	sprintf(buf->streamName,"h264");
return E_NO;
}

/* get sd directory info */
 Int32 icam_get_sd_dir_info(IN ICamCtrlHandle hCtrl, IN const char *dirPath, OUT void *buf, IN Int32 *bufLen)
{
return E_NO;
}

/* get sd root path */
 Int32 icam_get_sd_root_path(IN ICamCtrlHandle hCtrl, OUT void *buf, INOUT Int32 *bufLen)
{
return E_NO;
}

/* get special capture params */
 Int32 icam_get_spec_cap_params(IN ICamCtrlHandle hCtrl, OUT CamSpecCapParams *params)
{
return E_NO;
}

/* get strobe control params */
 Int32 icam_get_strobe_params(IN ICamCtrlHandle hCtrl, OUT CamStrobeCtrlParam *params){
return E_NO;}

/* get date time of cam */
Int32 icam_get_time(IN ICamCtrlHandle hCtrl, OUT CamDateTime *buf)
{
     return E_NO;
}

/* get traffic light region params */
 Int32 icam_get_traffic_light_regions(IN ICamCtrlHandle hCtrl, OUT CamTrafficLightRegions *params){
return E_NO;}

/* get image upload cfg */
 Int32 icam_get_upload_cfg(IN ICamCtrlHandle hCtrl, OUT CamImgUploadCfg *buf){
return E_NO;}

/* get firmware version */
 Int32 icam_get_version(IN ICamCtrlHandle hCtrl, OUT CamVersionInfo *buf)
 {
return E_NO;
}

/* get work mode */
 Int32 icam_get_work_mode(IN ICamCtrlHandle hCtrl, OUT CamWorkMode *params)
 {
return E_NO;
}

/* get work status */
 Int32 icam_get_work_status(IN ICamCtrlHandle hCtrl, OUT CamWorkStatus *buf)
 {
return E_NO;
}

/* get capture input info */
 Int32 icam_get_input_info(IN ICamCtrlHandle hCtrl, OUT CamInputInfo * buf){
return E_NO;}

/* set auto exposure params */
 Int32 icam_set_ae_params(IN ICamCtrlHandle hCtrl, IN const CamAEParam *params){
return E_NO;}

/* set auto analog video params */
 Int32 icam_set_av_params(IN ICamCtrlHandle hCtrl, IN const CamAVParam *params){
return E_NO;}

/* set auto auto white balance params */
 Int32 icam_set_awb_params(IN ICamCtrlHandle hCtrl, IN const CamAWBParam *params){
return E_NO;}

/* set day night switch mode */
 Int32 icam_set_day_night_mode_params(IN ICamCtrlHandle hCtrl, IN const CamDayNightModeCfg *params){
return E_NO;}

/* set vehicle detector params */
 Int32 icam_set_detector_params(IN ICamCtrlHandle hCtrl, IN const CamDetectorParam *params)
 {
return E_NO;
}

/* set device info */
 Int32 icam_set_dev_info(IN ICamCtrlHandle hCtrl, IN const CamDeviceInfo *info){
return E_NO;}

/* set exposure params, only useful when auto exposure is off */
 Int32 icam_set_exposure_params(IN ICamCtrlHandle hCtrl, IN const CamExprosureParam *params){
return E_NO;}

/* set ftp server info */
 Int32 icam_set_ftp_srv_info(IN ICamCtrlHandle hCtrl, IN const CamFtpImageServerInfo *srvInfo){
return E_NO;}

/* set h.264 params */
 Int32 icam_set_h264_params(IN ICamCtrlHandle hCtrl, IN const CamH264Params *params)
{
if(params->IPRatio>200)
	return -1;
return E_NO;
}

/* set img encode params */
 Int32 icam_set_img_enc_params(IN ICamCtrlHandle hCtrl, IN const CamImgEncParams *params){
return E_NO;}

/* set img ehance params */
 Int32 icam_set_img_enhance_params(IN ICamCtrlHandle hCtrl, IN const CamImgEnhanceParams *params){
return E_NO;}

/* set img tcp server info */
 Int32 icam_set_img_srv_info(IN ICamCtrlHandle hCtrl, IN const CamTcpImageServerInfo *srvInfo){
return E_NO;}

/* set io config params */
 Int32 icam_set_io_config(IN ICamCtrlHandle hCtrl, IN const CamIoCfg *params)
 {
 
 printf("params->direction=%x , params->status=%x\n",params->direction,params->status);
return E_NO;
}

/* set network params */
 Int32 icam_set_network_info(IN ICamCtrlHandle hCtrl, IN const CamNetworkInfo *info)
{
return E_NO;
}

/* set ntp server info */
 Int32 icam_set_ntp_srv_info(IN ICamCtrlHandle hCtrl, IN const CamNtpServerInfo *srvInfo){
return E_NO;}

/* set osd params */
 Int32 icam_set_osd_params(IN ICamCtrlHandle hCtrl, IN const CamOsdParams *params){
return E_NO;}

/* set rgb gains */
 Int32 icam_set_rgb_gains(IN ICamCtrlHandle hCtrl, IN const CamRGBGains *params){
return E_NO;}

/* set road info */
Int32 icam_set_road_info(IN ICamCtrlHandle hCtrl, IN const CamRoadInfo *info){
return E_NO;}

/* set rtp params for h.264 transfer */
 Int32 icam_set_rtp_params(IN ICamCtrlHandle hCtrl, IN const CamRtpParams *params){
return E_NO;}

/* set special capture params */
Int32 icam_set_spec_cap_params(IN ICamCtrlHandle hCtrl, IN const CamSpecCapParams *params){
return E_NO;}

/* set strobe control params */
Int32 icam_set_strobe_params(IN ICamCtrlHandle hCtrl, IN const CamStrobeCtrlParam *params){
return E_NO;}

/* set cam date time */
Int32 icam_set_time(IN ICamCtrlHandle hCtrl, IN const CamDateTime *dateTime)
{
return E_NO;
}

/* set traffic light regions */
extern Int32 icam_set_traffic_light_regions(IN ICamCtrlHandle hCtrl, IN const CamTrafficLightRegions *params){
return E_NO;}

/* set upload protocol */
Int32 icam_set_upload_cfg(IN ICamCtrlHandle hCtrl, IN CamImgUploadCfg *cfg){
return E_NO;}

/* set work mode */
Int32 icam_set_work_mode(IN ICamCtrlHandle hCtrl, IN const CamWorkMode *params){
return E_NO;}

/* ask cam to send all files in a dir */
Int32 icam_snd_dir(IN ICamCtrlHandle hCtrl, IN const char *dirPath){
return E_NO;}

/* restore to default config */
Int32 icam_restore_cfg(IN ICamCtrlHandle hCtrl){
return E_NO;}

/* reset process and system */
Int32 icam_sys_reset(IN ICamCtrlHandle hCtrl){
return E_NO;}

/* get cfg file name */
Int32 icam_get_cfg_name(IN ICamCtrlHandle hCtrl, OUT char *fname, IN size_t bufLen){
return -1;}

/* general API for cam ctrl */
Int32 icam_ctrl_run(IN ICamCtrlHandle hCtrl, INOUT MsgHeader *data, IN Uint32 bufLen){
return E_NO;}

#endif



typedef struct NETWORK_INFO
{
char ethname[16];
int IPFromDHCP;
char ipaddr[16];
char netmask[16];
char gateway[16];
char mac[18];
char hostname[32];
int DNSFromDHCP;
char DNS1[32];
char DNS2[32];
char domainName[32];
}NETWORK_INFO;



typedef struct TAG_USER
{
	char Username[32];
	char Password[32];
	enum tt__UserLevel UserLevel;
	int enable;
}TAG_USER;

typedef struct SYS_PARAM_INFO
{
int usernum;
TAG_USER user[MAX_USER_NUM];
NETWORK_INFO network;
CamDeviceInfo DeviceInfo;
int WS_DiscoveryMode;
CamIoCfg Ioparams;
CamRtpParams rtpParamsbuf;
CamRtpParams httpParamsbuf;
struct tt__RelayOutputSettings alarmout[8];

}SYS_PARAM_INFO;

SYS_PARAM_INFO g_sys_info;
static char g_ipc_scopes[1024]=
{"onvif://www.onvif.org/Profile/Streaming \
onvif://www.onvif.org/type/NetworkVideoTransmitter \
onvif://www.onvif.org/location/js \
onvif://www.onvif.org/hardware/js \
onvif://www.onvif.org/name/js"};

typedef struct ONVIF_STATUS
{
    int running;
	 int sendbyeflag;
	int relayreset;
	struct tt__RelayOutputSettings alarmout[8];
}ONVIF_STATUS;
static ONVIF_STATUS g_onvif_status;



int check_ipaddr(char *ip)
{
unsigned int a,b,c,d;
if(!ip)
	return -1;
if (sscanf(ip, "%d.%d.%d.%d",&a, &b,&c,&d) != 4)
		return -1;
if(a>255||b>255||c>255||d>255)
	return -1;
return 0;

}

int get_dns(char *dns1, char *dns2)
{
    int fd = -1;
    int size = 0;
    char strBuf[100];
    int buf_size = sizeof(strBuf);
    char *p = NULL;
    char *q = NULL;
    int i = 0;
    int j = 0;
    int count = 0;
    *dns1='\0';
	*dns2='\0';
    fd = open("/etc/resolv.conf", O_RDONLY);
    if(-1 == fd)
    {
        printf("%s open error \n", __func__);
        return -1;
    }
    size = read(fd, strBuf, buf_size);
    if(size < 0)
    {
        printf("%s read file len error \n", __func__);
        close(fd);
        return -1;
    }
    strBuf[buf_size] = '\0';
    close(fd);

    while(i < buf_size)
    {
        if((p = strstr(&strBuf[i], "nameserver")) != NULL)
        {
            q=strpbrk(p,"0123456789");
			if(q!=NULL)
			{
				count = 0;
			    while(*(q+count)!= '\n')
			    {
			        count++;
					if(count>15)
						break;
			    }
				j++;
				//printf("count=%d p=%s \n",count,p);
				if(1 == j)
				{
					memcpy(dns1, q, count);
					dns1[count]='\0';
				}
				else if(2 == j)
				{
					memcpy(dns2, q, count);
					dns2[count]='\0';
				}
			}
            i = p-strBuf+10;
        } 
        else 
        {
           break;
        }
    }

    return 0;
}


int get_gateway(unsigned long  *p)     
{     
  FILE *fp;     
  char buf[256]; // 128 is enough for linux     
  char iface[16];     
  unsigned long dest_addr, gate_addr;     
  *p = 0;     
  fp = fopen("/proc/net/route", "r");     
  if (fp == NULL)     
    return -1;     
  /* Skip title line */     
  fgets(buf, sizeof(buf), fp);     
  while (fgets(buf, sizeof(buf), fp)) {     
    if (sscanf(buf, "%s\t%lX\t%lX", iface,&dest_addr, &gate_addr) != 3 ||     
  dest_addr != 0)     
  continue;     
  *p = gate_addr;     
  break;     
  }     
  fclose(fp);     
  return 0;     
}   


//#define ONVIF__tad__
//#define ONVIF__tan__
//#define ONVIF__trp__
//#define ONVIF__tse__
//#define ONVIF__timg__
//#define ONVIF__tptz__
//#define ONVIF__trc__
//#define ONVIF__tse__
//#define WSSE_SESSIOM_TIME_VERIFY /* authorize if HA1 and HA2 identical and not replay attack */
int get_ip(const char *ethname,NETWORK_INFO *network_info)
{
	char* ipaddr=network_info->ipaddr;
	char *netmask=network_info->netmask;
	char *mac=network_info->mac;
	struct in_addr gateway;
	get_gateway((unsigned long  *)&gateway.s_addr);
	sprintf(network_info->gateway, "%s", inet_ntoa(gateway));
	get_dns(network_info->DNS1,network_info->DNS2);
	sprintf(network_info->hostname,"localhost");

	int ret=-1;
	int fd, intrface;
	struct ifreq buf[16];
	struct ifconf ifc;
	struct ifreq ifr; 

	if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
	return -1;
	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = (caddr_t)buf;

	if (ioctl (fd, SIOCGIFCONF, (char *) &ifc) < 0)
	goto _error_;

	intrface = ifc.ifc_len/sizeof(struct ifreq);

		while(intrface-->0)
		{
			if (strstr(buf[intrface].ifr_name, ethname))
			{
			if ((ioctl (fd, SIOCGIFADDR, (char*)&buf[intrface])) < 0)
			goto _error_;
			sprintf(ipaddr, "%s", 
			inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr));

			if ((ioctl (fd, SIOCGIFNETMASK , (char*)&buf[intrface])) < 0)
			goto _error_;

			sprintf(netmask, "%s", 
			inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_netmask))->sin_addr));
			ret = 0;
			}
		}

		ifr.ifr_addr.sa_family = AF_INET; 
		strncpy(ifr.ifr_name, ethname, IFNAMSIZ-1); 

		if(ioctl(fd, SIOCGIFHWADDR, &ifr) == 0)
		{ 
			sprintf(mac, "%02x%02x%02x%02x%02x%02x", 
			(unsigned char)ifr.ifr_hwaddr.sa_data[0],
			(unsigned char)ifr.ifr_hwaddr.sa_data[1],
			(unsigned char)ifr.ifr_hwaddr.sa_data[2],
			(unsigned char)ifr.ifr_hwaddr.sa_data[3],
			(unsigned char)ifr.ifr_hwaddr.sa_data[4],
			(unsigned char)ifr.ifr_hwaddr.sa_data[5]);
			ret = 0;
		}
		_error_:
		close(fd);
return 0;
}

int get_passwd_of_usr(char *usr,char *passwd)
{
    int j;
	for(j=0;j<MAX_USER_NUM;j++)
	{
		if(g_sys_info.user[j].enable&&(strcmp(usr,g_sys_info.user[j].Username)==0))
		{
			snprintf(passwd,128,g_sys_info.user[j].Password);
			return 0;
		}
	}
    return 0;
}
static int g_alarmout[8];
static int g_alarmin[8];

int write_sys_file()
{
	FILE *fp=NULL;
	fp = fopen(SYSFILE, "wb");
	fwrite((char*)&g_sys_info, 1, sizeof(g_sys_info), fp);
	fclose(fp);
}
int init_sys_config()
{
	struct stat st;
	int i=0;
	FILE *fp=NULL;
	fp = fopen(SYSFILE, "rb");
	if(fp)
	{
		stat(SYSFILE, &st);
		if(st.st_size!=sizeof(g_sys_info))
		{
		fclose(fp);	
		fp=NULL;
		remove(SYSFILE);
		}
	}
	if(!fp)
	{
       memset((char *)&g_sys_info,0,sizeof(g_sys_info));	   
	   g_sys_info.usernum=1;
	   g_sys_info.user[0].enable=1;
	   sprintf(g_sys_info.user[0].Username,"admin");
	   sprintf(g_sys_info.user[0].Password,"admin");
	   g_sys_info.user[0].UserLevel=tt__UserLevel__Administrator;
	   g_sys_info.WS_DiscoveryMode=tt__DiscoveryMode__Discoverable;

	    fp = fopen(SYSFILE, "wb");
	    fwrite((char*)&g_sys_info, 1, sizeof(g_sys_info), fp);
	    fclose(fp);
		fp=NULL;
	}else
	{
	fread((char*)&g_sys_info, 1, sizeof(g_sys_info), fp);
	fclose(fp);
	fp=NULL;
	}
	
    get_ip("eth0",&g_sys_info.network);
	CamDeviceInfo DeviceInfo;
	CamNetworkInfo netinfo;
	if(icam_get_dev_info(g_icam_ctrl_handle,&DeviceInfo)==E_NO)
	 {
	 snprintf(g_ipc_scopes,1024,"onvif://www.onvif.org/Profile/Streaming onvif://www.onvif.org/type/NetworkVideoTransmitter onvif://www.onvif.org/location/%s onvif://www.onvif.org/hardware/%s onvif://www.onvif.org/name/%s",DeviceInfo.location,DeviceInfo.hardware,DeviceInfo.name);
	 g_sys_info.DeviceInfo=DeviceInfo;
	}
	if(icam_get_network_info(g_icam_ctrl_handle,&netinfo)==E_NO)
	{
		snprintf(g_sys_info.network.hostname,sizeof(netinfo.hostName),netinfo.hostName);
		//snprintf(g_sys_info.network.ipaddr,sizeof(netinfo.ipAddr),netinfo.ipAddr);
		//snprintf(g_sys_info.network.netmask,sizeof(netinfo.ipMask),netinfo.ipMask);
		snprintf(g_sys_info.network.gateway,sizeof(netinfo.gatewayIP),netinfo.gatewayIP);
		snprintf(g_sys_info.network.DNS1,sizeof(netinfo.dnsServer),netinfo.dnsServer);
		snprintf(g_sys_info.network.domainName,sizeof(netinfo.domainName),netinfo.domainName);
	}
	CamIoCfg Ioparams;
	if(icam_get_io_config(g_icam_ctrl_handle,&Ioparams)==E_NO)
	{
		g_sys_info.Ioparams=Ioparams;
		for(i=0;i<8;i++)
		{
			if(Ioparams.direction&(1<<i))
			{
			    g_alarmout[RELAY_OUTPUTS_NUM]=i;
				RELAY_OUTPUTS_NUM++;
			}
			else
			{	g_alarmin[RELAY_OUTPUTS_NUM]=i;
				INPUT_CONNECTORS_NUM++;
			}
		}
	}
	CamRtpParams rtpParamsbuf;
   if(icam_get_rtp_params(g_icam_ctrl_handle,&rtpParamsbuf)==E_NO)
	{
	g_sys_info.rtpParamsbuf=rtpParamsbuf;
   	}else
   	{
   	g_sys_info.rtpParamsbuf.rtspSrvPort=554;
	sprintf(g_sys_info.rtpParamsbuf.streamName,"h264");
   	}
   	g_sys_info.httpParamsbuf.rtspSrvPort=ONVIF_HTTP_SNAP_RORT;
	g_sys_info.httpParamsbuf.flags=0;
	sprintf(g_sys_info.httpParamsbuf.streamName,"snapshot");
	return 0;
}

Int32 utc_to_local(CamDateTime *dateTime)
{
		time_t atime ;
		struct tm timeptr;
		struct tm p;
		if(dateTime->year<2011||dateTime->year>2100
			||dateTime->month<1||dateTime->month>12
			||dateTime->day<1||dateTime->day>31
			||dateTime->hour>23
			||dateTime->minute>59
			||dateTime->second>59)
			return -1;
		timeptr.tm_year=dateTime->year-1900;
		timeptr.tm_mon=dateTime->month-1;
		timeptr.tm_mday=dateTime->day;
		timeptr.tm_hour=dateTime->hour;
		timeptr.tm_min=dateTime->minute;
		timeptr.tm_sec=dateTime->second;
		atime=mktime(&timeptr);
		atime+=8*60*60;
		gmtime(&atime);
		//localtime(&atime);
		dateTime->year=timeptr.tm_year+1900;
		dateTime->month=timeptr.tm_mon+1;
		dateTime->day =timeptr.tm_mday;
		dateTime->hour=timeptr.tm_hour;
		dateTime->minute=timeptr.tm_min;
		dateTime->second=timeptr.tm_sec;
		dateTime->weekDay=timeptr.tm_wday;
		dateTime->ms=0;
		dateTime->us=0;
       return E_NO;
}



#ifndef WITH_OPENSSL
const char *wsse_PasswordTextURI = "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordText";
const char *wsse_PasswordDigestURI = "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest";
/** SHA1 digest size in octets */
#define SOAP_SMD_SHA1_SIZE	(20)
#define SOAP_WSSE_CLKSKEW	(300)
#define SOAP_WSSE_NONCETIME	(SOAP_WSSE_CLKSKEW + 240)

struct soap_wsse_session
{ struct soap_wsse_session *next;	/**< Next session in list */
  time_t expired;			/**< Session expiration */
  char hash[SOAP_SMD_SHA1_SIZE];	/**< SHA1 digest */
  char nonce[1]; /**< Nonce string flows into region below this struct */
};

static MUTEX_TYPE soap_wsse_session_lock = MUTEX_INITIALIZER;
static struct soap_wsse_session *soap_wsse_session = NULL;
void
soap_wsse_delete_Security(struct soap *soap)
{ DBGFUN("soap_wsse_delete_Security");
  if (soap->header)
    soap->header->wsse__Security = NULL;
}

static void
soap_wsse_session_cleanup(struct soap *soap)
{ struct soap_wsse_session **session;
  time_t now = time(NULL);
  DBGFUN("soap_wsse_session_cleanup");
  /* enter mutex to purge expired session data */
  MUTEX_LOCK(soap_wsse_session_lock);
  session = &soap_wsse_session;
  while (*session)
  { if ((*session)->expired < now)
    { struct soap_wsse_session *p = *session;
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Deleting session nonce=%s\n", p->nonce));
      *session = p->next;
      free(p);
    }
    else
      session = &(*session)->next;
  }
  /* exit mutex */
  MUTEX_UNLOCK(soap_wsse_session_lock);
}

int
soap_wsse_sender_fault_subcode(struct soap *soap, const char *faultsubcode, const char *faultstring, const char *faultdetail)
{
#if defined(SOAP_WSA_2003) || defined(SOAP_WSA_2004) || defined(SOAP_WSA_200408) || defined(SOAP_WSA_2005)
  return soap_wsa_sender_fault_subcode(soap, faultsubcode, faultstring, faultdetail);
#else
  return soap_sender_fault_subcode(soap, faultsubcode, faultstring, faultdetail);
#endif
}

int
soap_wsse_fault(struct soap *soap, wsse__FaultcodeEnum fault, const char *detail)
{ const char *code = soap_wsse__FaultcodeEnum2s(soap, fault);
  DBGFUN2("soap_wsse_fault", "fault=%s", code?code:"", "detail=%s", detail?detail:"");
  /* remove incorrect or incomplete Security header */
  soap_wsse_delete_Security(soap);
  /* populate the SOAP Fault as per WS-Security spec */
  /* detail = NULL; */ /* uncomment when detail text not recommended */
  /* use WSA to populate the SOAP Header when WSA is used */
  switch (fault)
  { case wsse__UnsupportedSecurityToken:
      return soap_wsse_sender_fault_subcode(soap, code, "An unsupported token was provided", detail);
    case wsse__UnsupportedAlgorithm:
      return soap_wsse_sender_fault_subcode(soap, code, "An unsupported signature or encryption algorithm was used", detail);
    case wsse__InvalidSecurity:
      return soap_wsse_sender_fault_subcode(soap, code, "An error was discovered processing the <wsse:Security> header", detail);
    case wsse__InvalidSecurityToken:
      return soap_wsse_sender_fault_subcode(soap, code, "An invalid security token was provided", detail);
    case wsse__FailedAuthentication:
      return soap_wsse_sender_fault_subcode(soap, code, "The security token could not be authenticated or authorized", detail);
    case wsse__FailedCheck:
      return soap_wsse_sender_fault_subcode(soap, code, "The signature or decryption was invalid", detail);
    case wsse__SecurityTokenUnavailable:
      return soap_wsse_sender_fault_subcode(soap, code, "Referenced security token could not be retrieved", detail);
  }
  return SOAP_FAULT;
}


static int
soap_wsse_session_verify(struct soap *soap, const char hash[SOAP_SMD_SHA1_SIZE], const char *created, const char *nonce)
{ struct soap_wsse_session *session;
  time_t expired, now = time(NULL);
  DBGFUN("soap_wsse_session_verify");
  soap_s2dateTime(soap, created, &expired);
 // printf("now=%u expired=%u\n",now,expired);
  /* creation time in the future? */
  if (expired > now + SOAP_WSSE_CLKSKEW)
    return soap_wsse_fault(soap, wsse__FailedAuthentication, "Authorization request in future");
  expired += SOAP_WSSE_NONCETIME;
  /* expired? */
  if (expired <= now)
    return soap_wsse_fault(soap, wsse__FailedAuthentication, "Authentication expired");
  /* purge expired messages, but don't do this all the time to improve efficiency */
  if (now % 10 == 0)
    soap_wsse_session_cleanup(soap);
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Verifying session nonce=%s\n", nonce));
  /* enter mutex to check and update session */
  MUTEX_LOCK(soap_wsse_session_lock);
  for (session = soap_wsse_session; session; session = session->next)
  { if (!memcmp(session->hash, hash, SOAP_SMD_SHA1_SIZE) && !strcmp(session->nonce, nonce))
      break;
  }
  /* if not found, allocate new session data */
  if (!session)
  { session = (struct soap_wsse_session*)malloc(sizeof(struct soap_wsse_session) + strlen(nonce));
    if (session)
    { session->next = soap_wsse_session;
      session->expired = expired;
      memcpy(session->hash, hash, SOAP_SMD_SHA1_SIZE);
      strcpy(session->nonce, nonce);
      soap_wsse_session = session;
    }
    session = NULL;
  }
  /* exit mutex */
  MUTEX_UNLOCK(soap_wsse_session_lock);
  /* if replay attack, return non-descript failure */
  if (session)
    return soap_wsse_fault(soap, wsse__FailedAuthentication, NULL);
  return SOAP_OK;
}

struct _wsse__Security*
soap_wsse_Security(struct soap *soap)
{ if (soap->header)
    return soap->header->wsse__Security;
  return NULL;
}

struct _wsse__UsernameToken*
soap_wsse_UsernameToken(struct soap *soap, const char *id)
{ _wsse__Security *security = soap_wsse_Security(soap);
  if (security
   && security->UsernameToken
   && (!id || (security->UsernameToken->wsu__Id
            && !strcmp(security->UsernameToken->wsu__Id, id))))
    return security->UsernameToken;
  return NULL;
}
int
soap_wsse_verify_Password(struct soap *soap, const char *password)
{ _wsse__UsernameToken *token = soap_wsse_UsernameToken(soap, NULL);
  DBGFUN("soap_wsse_verify_Password");
 // printf("usr=%s Passwordtype=%s passwordtypeitem=%s \n Nonce=%s wsu__Created=%s wsu__Id=%s\n",
	//  token->Username,
	//  token->Password->Type,
	//  token->Password->__item,
	//  token->Nonce,
	//  token->wsu__Created,
	 // token->wsu__Id);
  
  /* if we have a UsernameToken with a Password, check it */
  if (token && token->Password)
  { /* password digest or text? */
    if (token->Password->Type
     && !strcmp(token->Password->Type, wsse_PasswordDigestURI))
    { /* check password digest: compute SHA1(created, nonce, password) */
      if (token->Nonce
       && token->wsu__Created
       && strlen(token->Password->__item) == 28)	/* digest pw len = 28 */
      { char HA1[SOAP_SMD_SHA1_SIZE], HA2[SOAP_SMD_SHA1_SIZE];
        /* The specs are not clear: compute digest over binary nonce or base64 nonce? The formet appears to be the case: */
        int noncelen;
        const char *nonce = soap_base642s(soap, token->Nonce, NULL, 0, &noncelen);
        /* compute HA1 = SHA1(created, nonce, password) */
		calc_SHA1(token->wsu__Created, nonce, noncelen, password, HA1);
        //calc_SHA1(token->wsu__Created, token->Nonce, strlen(token->Nonce), password, HA1);

        /* get HA2 = supplied digest from base64 Password */
        soap_base642s(soap, token->Password->__item, HA2, SOAP_SMD_SHA1_SIZE, NULL);
        /* compare HA1 to HA2 */
        if (!memcmp(HA1, HA2, SOAP_SMD_SHA1_SIZE))
        {
#ifdef WSSE_SESSIOM_TIME_VERIFY
        /* authorize if HA1 and HA2 identical and not replay attack */
          if (!soap_wsse_session_verify(soap, HA1, token->wsu__Created, token->Nonce))
            return SOAP_OK;
#else
          return SOAP_OK;
#endif
          return soap->error; 
        }
      }
    }
    else
    { /* check password text */
      if (!strcmp(token->Password->__item, password))
        return SOAP_OK;
    }
  }
  return soap_wsse_fault(soap, wsse__FailedAuthentication, NULL);
}
#endif

SOAP_FMAC5 int SOAP_FMAC6 soap_wsse_confirm(struct soap* soap)
{
    char passwd[128]={0};
	if(soap->header&&soap->header->wsse__Security&&soap->header->wsse__Security->UsernameToken)
	{
	get_passwd_of_usr(soap->header->wsse__Security->UsernameToken->Username,passwd);
	soap->error=soap_wsse_verify_Password(soap,passwd );
	}
	else
		soap->error=SOAP_USER_ERROR;
	//printf("soap->error=%d\n",soap->error);
	return soap->error;
}



SOAP_FMAC5 int SOAP_FMAC6 SOAP_ENV__Fault(struct soap* soap, char *faultcode, char *faultstring, char *faultactor, struct SOAP_ENV__Detail *detail, struct SOAP_ENV__Code *SOAP_ENV__Code, struct SOAP_ENV__Reason *SOAP_ENV__Reason, char *SOAP_ENV__Node, char *SOAP_ENV__Role, struct SOAP_ENV__Detail *SOAP_ENV__Detail)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
return 0;
}

//SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Hello(struct soap* soap, struct wsdd__HelloType *wsdd__Hello){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

//SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Bye(struct soap* soap, struct wsdd__ByeType *wsdd__Bye){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

//SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Probe(struct soap* soap, struct wsdd__ProbeType *wsdd__Probe){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

//SOAP_FMAC5 int SOAP_FMAC6 __wsdd__ProbeMatches(struct soap* soap, struct wsdd__ProbeMatchesType *wsdd__ProbeMatches){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

//SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Resolve(struct soap* soap, struct wsdd__ResolveType *wsdd__Resolve){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

//SOAP_FMAC5 int SOAP_FMAC6 __wsdd__ResolveMatches(struct soap* soap, struct wsdd__ResolveMatchesType *wsdd__ResolveMatches){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}
#ifdef ONVIF__tad__
SOAP_FMAC5 int SOAP_FMAC6 __tad__GetServiceCapabilities(struct soap* soap, struct _tad__GetServiceCapabilities *tad__GetServiceCapabilities, struct _tad__GetServiceCapabilitiesResponse *tad__GetServiceCapabilitiesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tad__DeleteAnalyticsEngineControl(struct soap* soap, struct _tad__DeleteAnalyticsEngineControl *tad__DeleteAnalyticsEngineControl, struct _tad__DeleteAnalyticsEngineControlResponse *tad__DeleteAnalyticsEngineControlResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tad__CreateAnalyticsEngineControl(struct soap* soap, struct _tad__CreateAnalyticsEngineControl *tad__CreateAnalyticsEngineControl, struct _tad__CreateAnalyticsEngineControlResponse *tad__CreateAnalyticsEngineControlResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tad__SetAnalyticsEngineControl(struct soap* soap, struct _tad__SetAnalyticsEngineControl *tad__SetAnalyticsEngineControl, struct _tad__SetAnalyticsEngineControlResponse *tad__SetAnalyticsEngineControlResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tad__GetAnalyticsEngineControl(struct soap* soap, struct _tad__GetAnalyticsEngineControl *tad__GetAnalyticsEngineControl, struct _tad__GetAnalyticsEngineControlResponse *tad__GetAnalyticsEngineControlResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tad__GetAnalyticsEngineControls(struct soap* soap, struct _tad__GetAnalyticsEngineControls *tad__GetAnalyticsEngineControls, struct _tad__GetAnalyticsEngineControlsResponse *tad__GetAnalyticsEngineControlsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tad__GetAnalyticsEngine(struct soap* soap, struct _tad__GetAnalyticsEngine *tad__GetAnalyticsEngine, struct _tad__GetAnalyticsEngineResponse *tad__GetAnalyticsEngineResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tad__GetAnalyticsEngines(struct soap* soap, struct _tad__GetAnalyticsEngines *tad__GetAnalyticsEngines, struct _tad__GetAnalyticsEnginesResponse *tad__GetAnalyticsEnginesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tad__SetVideoAnalyticsConfiguration(struct soap* soap, struct _tad__SetVideoAnalyticsConfiguration *tad__SetVideoAnalyticsConfiguration, struct _tad__SetVideoAnalyticsConfigurationResponse *tad__SetVideoAnalyticsConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tad__SetAnalyticsEngineInput(struct soap* soap, struct _tad__SetAnalyticsEngineInput *tad__SetAnalyticsEngineInput, struct _tad__SetAnalyticsEngineInputResponse *tad__SetAnalyticsEngineInputResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tad__GetAnalyticsEngineInput(struct soap* soap, struct _tad__GetAnalyticsEngineInput *tad__GetAnalyticsEngineInput, struct _tad__GetAnalyticsEngineInputResponse *tad__GetAnalyticsEngineInputResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tad__GetAnalyticsEngineInputs(struct soap* soap, struct _tad__GetAnalyticsEngineInputs *tad__GetAnalyticsEngineInputs, struct _tad__GetAnalyticsEngineInputsResponse *tad__GetAnalyticsEngineInputsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tad__GetAnalyticsDeviceStreamUri(struct soap* soap, struct _tad__GetAnalyticsDeviceStreamUri *tad__GetAnalyticsDeviceStreamUri, struct _tad__GetAnalyticsDeviceStreamUriResponse *tad__GetAnalyticsDeviceStreamUriResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tad__GetVideoAnalyticsConfiguration(struct soap* soap, struct _tad__GetVideoAnalyticsConfiguration *tad__GetVideoAnalyticsConfiguration, struct _tad__GetVideoAnalyticsConfigurationResponse *tad__GetVideoAnalyticsConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tad__CreateAnalyticsEngineInputs(struct soap* soap, struct _tad__CreateAnalyticsEngineInputs *tad__CreateAnalyticsEngineInputs, struct _tad__CreateAnalyticsEngineInputsResponse *tad__CreateAnalyticsEngineInputsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tad__DeleteAnalyticsEngineInputs(struct soap* soap, struct _tad__DeleteAnalyticsEngineInputs *tad__DeleteAnalyticsEngineInputs, struct _tad__DeleteAnalyticsEngineInputsResponse *tad__DeleteAnalyticsEngineInputsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tad__GetAnalyticsState(struct soap* soap, struct _tad__GetAnalyticsState *tad__GetAnalyticsState, struct _tad__GetAnalyticsStateResponse *tad__GetAnalyticsStateResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}
#endif
#ifdef ONVIF__tan__
SOAP_FMAC5 int SOAP_FMAC6 __tan__GetSupportedRules(struct soap* soap, struct _tan__GetSupportedRules *tan__GetSupportedRules, struct _tan__GetSupportedRulesResponse *tan__GetSupportedRulesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tan__CreateRules(struct soap* soap, struct _tan__CreateRules *tan__CreateRules, struct _tan__CreateRulesResponse *tan__CreateRulesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tan__DeleteRules(struct soap* soap, struct _tan__DeleteRules *tan__DeleteRules, struct _tan__DeleteRulesResponse *tan__DeleteRulesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tan__GetRules(struct soap* soap, struct _tan__GetRules *tan__GetRules, struct _tan__GetRulesResponse *tan__GetRulesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tan__ModifyRules(struct soap* soap, struct _tan__ModifyRules *tan__ModifyRules, struct _tan__ModifyRulesResponse *tan__ModifyRulesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tan__GetServiceCapabilities(struct soap* soap, struct _tan__GetServiceCapabilities *tan__GetServiceCapabilities, struct _tan__GetServiceCapabilitiesResponse *tan__GetServiceCapabilitiesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tan__GetSupportedAnalyticsModules(struct soap* soap, struct _tan__GetSupportedAnalyticsModules *tan__GetSupportedAnalyticsModules, struct _tan__GetSupportedAnalyticsModulesResponse *tan__GetSupportedAnalyticsModulesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tan__CreateAnalyticsModules(struct soap* soap, struct _tan__CreateAnalyticsModules *tan__CreateAnalyticsModules, struct _tan__CreateAnalyticsModulesResponse *tan__CreateAnalyticsModulesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tan__DeleteAnalyticsModules(struct soap* soap, struct _tan__DeleteAnalyticsModules *tan__DeleteAnalyticsModules, struct _tan__DeleteAnalyticsModulesResponse *tan__DeleteAnalyticsModulesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tan__GetAnalyticsModules(struct soap* soap, struct _tan__GetAnalyticsModules *tan__GetAnalyticsModules, struct _tan__GetAnalyticsModulesResponse *tan__GetAnalyticsModulesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tan__ModifyAnalyticsModules(struct soap* soap, struct _tan__ModifyAnalyticsModules *tan__ModifyAnalyticsModules, struct _tan__ModifyAnalyticsModulesResponse *tan__ModifyAnalyticsModulesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}
#endif

SOAP_FMAC5 int SOAP_FMAC6 __tdn__Hello(struct soap* soap, struct wsdd__HelloType tdn__Hello, struct wsdd__ResolveType *tdn__HelloResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tdn__Bye(struct soap* soap, struct wsdd__ByeType tdn__Bye, struct wsdd__ResolveType *tdn__ByeResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tdn__Probe(struct soap* soap, struct wsdd__ProbeType tdn__Probe, struct wsdd__ProbeMatchesType *tdn__ProbeResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetServices(struct soap* soap, struct _tds__GetServices *tds__GetServices, struct _tds__GetServicesResponse *tds__GetServicesResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
char *XAddr=NULL;
tds__GetServicesResponse->__sizeService=3;
tds__GetServicesResponse->Service=(struct tds__Service*)soap_malloc(soap,tds__GetServicesResponse->__sizeService*sizeof(struct tds__Service));
struct tds__Service *Service=tds__GetServicesResponse->Service;

//{"tdn", "http://www.onvif.org/ver10/network/wsdl", NULL, NULL},
//	{"trt", "http://www.onvif.org/ver10/media/wsdl", NULL, NULL},
//{"timg", "http://www.onvif.org/ver20/imaging/wsdl", NULL, NULL},

//{"tds", "http://www.onvif.org/ver10/device/wsdl", NULL, NULL},
XAddr=(char *)soap_malloc(soap,128*sizeof(char));
snprintf(XAddr,128,"http://%s:%d/onvif/device_service",g_sys_info.network.ipaddr,ONVIF_SERVER_RORT);
Service->Namespace="http://www.onvif.org/ver10/device/wsdl";
Service->XAddr=XAddr;
Service->Version=(struct tt__OnvifVersion*)soap_malloc(soap,sizeof(struct tt__OnvifVersion));
Service->Version->Major=1;
Service->Version->Minor=4;
if(tds__GetServices->IncludeCapability==xsd__boolean__true_)
	Service->Capabilities=(struct _tds__Service_Capabilities*)soap_malloc(soap,sizeof(struct _tds__Service_Capabilities));

	Service++;
	Service->Namespace="http://www.onvif.org/ver10/media/wsdl";
	Service->XAddr=XAddr;
	Service->Version=(struct tt__OnvifVersion*)soap_malloc(soap,sizeof(struct tt__OnvifVersion));
	Service->Version->Major=1;
	Service->Version->Minor=5;
	if(tds__GetServices->IncludeCapability==xsd__boolean__true_)
	Service->Capabilities=(struct _tds__Service_Capabilities*)soap_malloc(soap,sizeof(struct _tds__Service_Capabilities));

Service++;
Service->Namespace="tdn";
Service->XAddr=XAddr;
Service->Version=(struct tt__OnvifVersion*)soap_malloc(soap,sizeof(struct tt__OnvifVersion));
Service->Version->Major=1;
Service->Version->Minor=1;
if(tds__GetServices->IncludeCapability==xsd__boolean__true_)
	Service->Capabilities=(struct _tds__Service_Capabilities*)soap_malloc(soap,sizeof(struct _tds__Service_Capabilities));


return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetServiceCapabilities(struct soap* soap, struct _tds__GetServiceCapabilities *tds__GetServiceCapabilities, struct _tds__GetServiceCapabilitiesResponse *tds__GetServiceCapabilitiesResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
tds__GetServiceCapabilitiesResponse->Capabilities=
	(struct tds__DeviceServiceCapabilities*)soap_malloc(soap,sizeof(struct tds__DeviceServiceCapabilities));


tds__GetServiceCapabilitiesResponse->Capabilities->Network=	
	(struct tds__NetworkCapabilities*)soap_malloc(soap,sizeof(struct tds__NetworkCapabilities));
tds__GetServiceCapabilitiesResponse->Capabilities->Network->IPFilter=
	(enum xsd__boolean*)soap_malloc(soap,sizeof(enum xsd__boolean));
tds__GetServiceCapabilitiesResponse->Capabilities->Network->ZeroConfiguration=
	(enum xsd__boolean*)soap_malloc(soap,sizeof(enum xsd__boolean));
tds__GetServiceCapabilitiesResponse->Capabilities->Network->IPVersion6=
	(enum xsd__boolean*)soap_malloc(soap,sizeof(enum xsd__boolean));
tds__GetServiceCapabilitiesResponse->Capabilities->Network->DynDNS=
	(enum xsd__boolean*)soap_malloc(soap,sizeof(enum xsd__boolean));
tds__GetServiceCapabilitiesResponse->Capabilities->Network->Dot11Configuration=
	(enum xsd__boolean*)soap_malloc(soap,sizeof(enum xsd__boolean));
tds__GetServiceCapabilitiesResponse->Capabilities->Network->HostnameFromDHCP=
	(enum xsd__boolean*)soap_malloc(soap,sizeof(enum xsd__boolean));
*tds__GetServiceCapabilitiesResponse->Capabilities->Network->IPFilter=xsd__boolean__false_;
*tds__GetServiceCapabilitiesResponse->Capabilities->Network->ZeroConfiguration=xsd__boolean__false_;
*tds__GetServiceCapabilitiesResponse->Capabilities->Network->IPVersion6=xsd__boolean__false_;
*tds__GetServiceCapabilitiesResponse->Capabilities->Network->DynDNS=xsd__boolean__false_;
*tds__GetServiceCapabilitiesResponse->Capabilities->Network->Dot11Configuration=xsd__boolean__false_;
*tds__GetServiceCapabilitiesResponse->Capabilities->Network->HostnameFromDHCP=xsd__boolean__false_;

tds__GetServiceCapabilitiesResponse->Capabilities->Network->NTP=
	(int*)soap_malloc(soap,sizeof(int));
*tds__GetServiceCapabilitiesResponse->Capabilities->Network->NTP=1;



tds__GetServiceCapabilitiesResponse->Capabilities->Security=	
	(struct tds__SecurityCapabilities*)soap_malloc(soap,sizeof(struct tds__SecurityCapabilities));
tds__GetServiceCapabilitiesResponse->Capabilities->Security->UsernameToken=
	(enum xsd__boolean*)soap_malloc(soap,sizeof(enum xsd__boolean));
*tds__GetServiceCapabilitiesResponse->Capabilities->Security->UsernameToken=xsd__boolean__true_;


tds__GetServiceCapabilitiesResponse->Capabilities->System=	
	(struct tds__SystemCapabilities*)soap_malloc(soap,sizeof(struct tds__SystemCapabilities));

tds__GetServiceCapabilitiesResponse->Capabilities->System->DiscoveryBye=
		(enum xsd__boolean*)soap_malloc(soap,sizeof(enum xsd__boolean));
tds__GetServiceCapabilitiesResponse->Capabilities->System->SystemBackup=
		(enum xsd__boolean*)soap_malloc(soap,sizeof(enum xsd__boolean));
tds__GetServiceCapabilitiesResponse->Capabilities->System->FirmwareUpgrade=
		(enum xsd__boolean*)soap_malloc(soap,sizeof(enum xsd__boolean));

*tds__GetServiceCapabilitiesResponse->Capabilities->System->DiscoveryBye=xsd__boolean__true_;
*tds__GetServiceCapabilitiesResponse->Capabilities->System->SystemBackup=xsd__boolean__true_;
*tds__GetServiceCapabilitiesResponse->Capabilities->System->FirmwareUpgrade=xsd__boolean__true_;

return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDeviceInformation(struct soap* soap, struct _tds__GetDeviceInformation *tds__GetDeviceInformation, struct _tds__GetDeviceInformationResponse *tds__GetDeviceInformationResponse)
{
CamDeviceInfo *DeviceInfobuf=(CamDeviceInfo*)soap_malloc(soap,sizeof(CamDeviceInfo));
if(icam_get_dev_info(g_icam_ctrl_handle,DeviceInfobuf)!=E_NO)
	return SOAP_ERR;
tds__GetDeviceInformationResponse->SerialNumber=(char*)soap_malloc(soap,128);
sprintf(tds__GetDeviceInformationResponse->SerialNumber,"%u",DeviceInfobuf->deviceSN);
tds__GetDeviceInformationResponse->Manufacturer=DeviceInfobuf->manufacture;;
tds__GetDeviceInformationResponse->Model=DeviceInfobuf->model;
tds__GetDeviceInformationResponse->FirmwareVersion=DeviceInfobuf->firmware;;
tds__GetDeviceInformationResponse->HardwareId=DeviceInfobuf->hardware;
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetSystemDateAndTime(struct soap* soap, struct _tds__SetSystemDateAndTime *tds__SetSystemDateAndTime, struct _tds__SetSystemDateAndTimeResponse *tds__SetSystemDateAndTimeResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
	CamNtpServerInfo srvInfo;
	if(icam_get_ntp_srv_info(g_icam_ctrl_handle,&srvInfo))
		return SOAP_ERR;

	if(tds__SetSystemDateAndTime->DateTimeType==tt__SetDateTimeType__NTP)
	{
	if(srvInfo.syncPrd==0)
		srvInfo.syncPrd=72;
	if(icam_set_ntp_srv_info(g_icam_ctrl_handle,&srvInfo))
		return SOAP_ERR;
	}else
	{
		CamDateTime dateTime;
		if(tds__SetSystemDateAndTime->UTCDateTime
			&&tds__SetSystemDateAndTime->UTCDateTime->Date
			&&tds__SetSystemDateAndTime->UTCDateTime->Time)
		{
			dateTime.year=tds__SetSystemDateAndTime->UTCDateTime->Date->Year;
			dateTime.month=tds__SetSystemDateAndTime->UTCDateTime->Date->Month;
			dateTime.day=tds__SetSystemDateAndTime->UTCDateTime->Date->Day;
			dateTime.hour=tds__SetSystemDateAndTime->UTCDateTime->Time->Hour;
			dateTime.minute=tds__SetSystemDateAndTime->UTCDateTime->Time->Minute;
			dateTime.second=tds__SetSystemDateAndTime->UTCDateTime->Time->Second;
			if(utc_to_local(&dateTime)<0)
			{
			  soap_sender_fault_subcode(soap, "ter:InvalidArgVal/ter:InvalidDateTime", NULL, NULL);
			  return SOAP_ERR;
			}
			if(srvInfo.syncPrd!=0)
			{
			srvInfo.syncPrd=0;
			if(icam_get_ntp_srv_info(g_icam_ctrl_handle,&srvInfo))
				return SOAP_ERR;
			}
			icam_set_time(g_icam_ctrl_handle, &dateTime);
		}
	}

return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemDateAndTime(struct soap* soap, struct _tds__GetSystemDateAndTime *tds__GetSystemDateAndTime, struct _tds__GetSystemDateAndTimeResponse *tds__GetSystemDateAndTimeResponse)
{
	fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
	time_t timep;
	struct tm *p;
	time(&timep);

	tds__GetSystemDateAndTimeResponse->SystemDateAndTime=
		(struct tt__SystemDateTime*)soap_malloc(soap,sizeof(struct tt__SystemDateTime));
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->DateTimeType=tt__SetDateTimeType__Manual;//手动或NTP
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->DaylightSavings=xsd__boolean__false_;//夏令时标志
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->TimeZone=
		(struct tt__TimeZone*)soap_malloc(soap,sizeof(struct tt__TimeZone));
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->TimeZone->TZ="CST-8";//"GTM -8";

CamNtpServerInfo srvInfo;
if(icam_get_ntp_srv_info(g_icam_ctrl_handle,&srvInfo))
	return SOAP_ERR;
if(srvInfo.syncPrd==0)
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->DateTimeType=tt__SetDateTimeType__Manual;
else
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->DateTimeType=tt__SetDateTimeType__NTP;
tds__GetSystemDateAndTimeResponse->SystemDateAndTime->DaylightSavings=xsd__boolean__false_;

#if 1
	p=gmtime(&timep);

	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime=
	(struct tt__DateTime*)soap_malloc(soap,sizeof(struct tt__DateTime));
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date=
		(struct tt__Date*)soap_malloc(soap,sizeof(struct tt__Date));
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time=
		(struct tt__Time*)soap_malloc(soap,sizeof(struct tt__Time));

	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date->Year=1900+p->tm_year;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date->Month=1+p->tm_mon;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date->Day= p->tm_mday;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time->Hour=p->tm_hour;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time->Minute= p->tm_min;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time->Second= p->tm_sec;
	//fprintf(stderr,"UTCDateTime %d,%d,%d %d,%d,%d\n",(1900+p->tm_year),( 1+p->tm_mon),p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
#endif
	p=localtime(&timep); /*取得当地时间*/

	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime=
		(struct tt__DateTime*)soap_malloc(soap,sizeof(struct tt__DateTime));
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Date=
		(struct tt__Date*)soap_malloc(soap,sizeof(struct tt__Date));
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Time=
		(struct tt__Time*)soap_malloc(soap,sizeof(struct tt__Time));
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Date->Year=1900+p->tm_year;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Date->Month=1+p->tm_mon;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Date->Day= p->tm_mday;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Time->Hour=p->tm_hour;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Time->Minute= p->tm_min;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Time->Second= p->tm_sec;
	//fprintf(stderr,"LocalDateTime %d,%d,%d %d,%d,%d\n",(1900+p->tm_year),( 1+p->tm_mon),p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);

return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetSystemFactoryDefault(struct soap* soap, struct _tds__SetSystemFactoryDefault *tds__SetSystemFactoryDefault, struct _tds__SetSystemFactoryDefaultResponse *tds__SetSystemFactoryDefaultResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
icam_restore_cfg(g_icam_ctrl_handle);

return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__UpgradeSystemFirmware(struct soap* soap, struct _tds__UpgradeSystemFirmware *tds__UpgradeSystemFirmware, struct _tds__UpgradeSystemFirmwareResponse *tds__UpgradeSystemFirmwareResponse)
{
	fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
	char path[128];
	int ret=SOAP_ERR;
	FILE *pfile;
	int len;
	if(tds__UpgradeSystemFirmware->Firmware)
	{
		//printf("%x\n",tds__UpgradeSystemFirmware->Firmware->xmime__contentType);	
		printf("%s\n",tds__UpgradeSystemFirmware->Firmware->xop__Include.id);	
		printf("%s\n",tds__UpgradeSystemFirmware->Firmware->xop__Include.type);	
		//printf("%x\n",tds__UpgradeSystemFirmware->Firmware->xop__Include.options);	

		printf("tds__UpgradeSystemFirmware->Firmware->xop__Include.__ptr %x  size %d\n",tds__UpgradeSystemFirmware->Firmware->xop__Include.__ptr,
		tds__UpgradeSystemFirmware->Firmware->xop__Include.__size);
		if(tds__UpgradeSystemFirmware->Firmware->xop__Include.__ptr&&tds__UpgradeSystemFirmware->Firmware->xop__Include.__size>128)
		{
			snprintf(path,128,tds__UpgradeSystemFirmware->Firmware->xop__Include.__ptr);
			pfile=fopen(path,"wb");
		    if(pfile)
			{
				len=fwrite(tds__UpgradeSystemFirmware->Firmware->xop__Include.__ptr+128,1,tds__UpgradeSystemFirmware->Firmware->xop__Include.__size-128,pfile);
				fclose(pfile);
			   if((len+128)==tds__UpgradeSystemFirmware->Firmware->xop__Include.__size)
			   	tds__UpgradeSystemFirmwareResponse->Message="Upgrade successful, rebooting in x seconds";
			   ret=SOAP_OK;
			}
		}
		else
			tds__UpgradeSystemFirmwareResponse->Message="Upgrade error";
	}
		
return ret;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SystemReboot(struct soap* soap, struct _tds__SystemReboot *tds__SystemReboot, struct _tds__SystemRebootResponse *tds__SystemRebootResponse)
{
	fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
	//g_onvif_status.running=0;//IPC负责重启
	g_onvif_status.sendbyeflag=1;
	sleep(5);
	icam_sys_reset(g_icam_ctrl_handle);
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__RestoreSystem(struct soap* soap, struct _tds__RestoreSystem *tds__RestoreSystem, struct _tds__RestoreSystemResponse *tds__RestoreSystemResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
int i=0,ret;
struct tt__BackupFile *BackupFile;
char fname[128]={0};
char fname1[132]={0};
int len=0;
struct stat st;
FILE *pfile;

for(i=0;i<tds__RestoreSystem->__sizeBackupFiles;i++)
{
	BackupFile=tds__RestoreSystem->BackupFiles+i;
	if(BackupFile)
	{
		if(strcmp(BackupFile->Name,"onvif_param")==0)
		{
			if(BackupFile->Data&&BackupFile->Data->xop__Include.__ptr
				&&BackupFile->Data->xop__Include.__size==sizeof(SYS_PARAM_INFO))
			{
				g_sys_info=*(SYS_PARAM_INFO*)BackupFile->Data->xop__Include.__ptr;
				write_sys_file();
			}
		}else if(strcmp(BackupFile->Name,"ipc_system_param")==0)
		{
			ret=icam_get_cfg_name(g_icam_ctrl_handle,fname,128);
			if(ret==E_NO&&stat(fname, &st)==0)
			{
			
			   if(BackupFile->Data->xop__Include.__size==st.st_size)
				{
				 sprintf(fname1,"%stmp",fname);
				    pfile=fopen(fname1,"wb");
				   if(pfile)
				 	{
				      len=fwrite(BackupFile->Data->xop__Include.__ptr,1,BackupFile->Data->xop__Include.__size,pfile);
				      fclose(pfile);
				 	}
				   if(len==st.st_size)
				   	{
				   	rename(fname1,fname);
				   	}
				}
			}
		}
	}
}
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemBackup(struct soap* soap, struct _tds__GetSystemBackup *tds__GetSystemBackup, struct _tds__GetSystemBackupResponse *tds__GetSystemBackupResponse)
{
    fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
	char fname[128]={0};
	int ret=0;
	tds__GetSystemBackupResponse->__sizeBackupFiles=1;
	FILE *pfile=NULL;
	char *sysbuf=NULL;
	int syslen=0;
	struct tt__BackupFile *BackupFile;

	ret=icam_get_cfg_name(g_icam_ctrl_handle,fname,128);
	if(ret==E_NO)
	{	
	    struct stat st;
		if(stat(fname, &st)==0)
		{
		    syslen=st.st_size;
			sysbuf=(char*)soap_malloc(soap,st.st_size);
			pfile=fopen(fname,"rb");
			if(pfile)
			{
			tds__GetSystemBackupResponse->__sizeBackupFiles++;
			fread(sysbuf,1,st.st_size,pfile);
			fclose(pfile);
			}
		}
	}

	tds__GetSystemBackupResponse->BackupFiles=
		(struct tt__BackupFile*)soap_malloc(soap,tds__GetSystemBackupResponse->__sizeBackupFiles*sizeof(struct tt__BackupFile));
	tds__GetSystemBackupResponse->BackupFiles->Name="onvif_param";
	tds__GetSystemBackupResponse->BackupFiles->Data=
		(struct tt__AttachmentData*)soap_malloc(soap,sizeof(struct tt__AttachmentData));

	tds__GetSystemBackupResponse->BackupFiles->Data->xop__Include.__ptr=(char *)&g_sys_info;
	tds__GetSystemBackupResponse->BackupFiles->Data->xop__Include.__size=sizeof(g_sys_info);
	tds__GetSystemBackupResponse->BackupFiles->Data->xop__Include.type="application/octet-stream";

	if(sysbuf)
	{
		BackupFile=tds__GetSystemBackupResponse->BackupFiles+1;
		tds__GetSystemBackupResponse->BackupFiles->Name="ipc_system_param";
		tds__GetSystemBackupResponse->BackupFiles->Data=
			(struct tt__AttachmentData*)soap_malloc(soap,sizeof(struct tt__AttachmentData));
		tds__GetSystemBackupResponse->BackupFiles->Data->xop__Include.__ptr=sysbuf;
		tds__GetSystemBackupResponse->BackupFiles->Data->xop__Include.__size=sizeof(g_sys_info);
		tds__GetSystemBackupResponse->BackupFiles->Data->xop__Include.type="application/octet-stream";

	}

return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemLog(struct soap* soap, struct _tds__GetSystemLog *tds__GetSystemLog, struct _tds__GetSystemLogResponse *tds__GetSystemLogResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
//tds__GetSystemLog.LogType
//{tt__SystemLogType__System = 0, tt__SystemLogType__Access = 1};
tds__GetSystemLogResponse->SystemLog=
	(struct tt__SystemLog*)soap_malloc(soap,sizeof(struct tt__SystemLog));
tds__GetSystemLogResponse->SystemLog->String="no log file";


return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemSupportInformation(struct soap* soap, struct _tds__GetSystemSupportInformation *tds__GetSystemSupportInformation, struct _tds__GetSystemSupportInformationResponse *tds__GetSystemSupportInformationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetScopes(struct soap* soap, struct _tds__GetScopes *tds__GetScopes, struct _tds__GetScopesResponse *tds__GetScopesResponse)
{
	CamDeviceInfo DeviceInfo;
	if(icam_get_dev_info(g_icam_ctrl_handle,&DeviceInfo)==E_NO)
	{
	 snprintf(g_ipc_scopes,1024,"onvif://www.onvif.org/Profile/Streaming onvif://www.onvif.org/type/NetworkVideoTransmitter onvif://www.onvif.org/location/%s onvif://www.onvif.org/hardware/%s onvif://www.onvif.org/name/%s",DeviceInfo.location,DeviceInfo.hardware,DeviceInfo.name);
	 g_sys_info.DeviceInfo=DeviceInfo;
	}
	    tds__GetScopesResponse->__sizeScopes=5;
    	tds__GetScopesResponse->Scopes=
		(struct tt__Scope*)soap_malloc(soap,tds__GetScopesResponse->__sizeScopes*sizeof(struct tt__Scope));
		tds__GetScopesResponse->Scopes->ScopeItem="onvif://www.onvif.org/Profile/Streaming";
		tds__GetScopesResponse->Scopes->ScopeDef=tt__ScopeDefinition__Fixed;

		(tds__GetScopesResponse->Scopes+1)->ScopeItem=(char*)soap_malloc(soap,128);
		snprintf((tds__GetScopesResponse->Scopes+1)->ScopeItem,128,"onvif://www.onvif.org/location/%s",g_sys_info.DeviceInfo.location);
		(tds__GetScopesResponse->Scopes+1)->ScopeDef=tt__ScopeDefinition__Configurable;

		(tds__GetScopesResponse->Scopes+2)->ScopeItem=(char*)soap_malloc(soap,128);
		snprintf((tds__GetScopesResponse->Scopes+2)->ScopeItem,128,"onvif://www.onvif.org/type/NetworkVideoTransmitter");
		(tds__GetScopesResponse->Scopes+2)->ScopeDef=tt__ScopeDefinition__Fixed;

		(tds__GetScopesResponse->Scopes+3)->ScopeItem=(char*)soap_malloc(soap,128);
		snprintf((tds__GetScopesResponse->Scopes+3)->ScopeItem,128,"onvif://www.onvif.org/hardware/%s",g_sys_info.DeviceInfo.hardware);
		(tds__GetScopesResponse->Scopes+3)->ScopeDef=tt__ScopeDefinition__Fixed;


		(tds__GetScopesResponse->Scopes+4)->ScopeItem=(char*)soap_malloc(soap,128);
		snprintf((tds__GetScopesResponse->Scopes+4)->ScopeItem,128,"onvif://www.onvif.org/name/%s",g_sys_info.DeviceInfo.name);
		(tds__GetScopesResponse->Scopes+4)->ScopeDef=tt__ScopeDefinition__Configurable;
		fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetScopes(struct soap* soap, struct _tds__SetScopes *tds__SetScopes, struct _tds__SetScopesResponse *tds__SetScopesResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
int i;
char *p;
char *q;
CamDeviceInfo DeviceInfo;
if(icam_get_dev_info(g_icam_ctrl_handle,&DeviceInfo))
	return SOAP_ERR;
for(i=0;i<tds__SetScopes->__sizeScopes;i++)
{
    p=*(tds__SetScopes->Scopes+i);
	//printf("Scopes=%s\n",p);
	if(q=strstr(p,"name:"))
	{
		printf("Scopes=%s\n",q+strlen("name:"));
		snprintf(DeviceInfo.name,sizeof(DeviceInfo.name),q+strlen("name:"));
	}else if(q=strstr(p,"location:"))
	{
		printf("Scopes=%s\n",q+strlen("location:"));
		snprintf(DeviceInfo.location,sizeof(DeviceInfo.location),q+strlen("location:"));
	}
}
if(icam_set_dev_info(g_icam_ctrl_handle,&DeviceInfo))
	return SOAP_ERR;

snprintf(g_ipc_scopes,1024,"onvif://www.onvif.org/Profile/Streaming onvif://www.onvif.org/type/NetworkVideoTransmitter onvif://www.onvif.org/location/%s onvif://www.onvif.org/hardware/%s onvif://www.onvif.org/name/%s",DeviceInfo.location,DeviceInfo.hardware,DeviceInfo.name);
g_sys_info.DeviceInfo=DeviceInfo;

return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__AddScopes(struct soap* soap, struct _tds__AddScopes *tds__AddScopes, struct _tds__AddScopesResponse *tds__AddScopesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__RemoveScopes(struct soap* soap, struct _tds__RemoveScopes *tds__RemoveScopes, struct _tds__RemoveScopesResponse *tds__RemoveScopesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDiscoveryMode(struct soap* soap, struct _tds__GetDiscoveryMode *tds__GetDiscoveryMode, struct _tds__GetDiscoveryModeResponse *tds__GetDiscoveryModeResponse)
{
tds__GetDiscoveryModeResponse->DiscoveryMode=g_sys_info.WS_DiscoveryMode;
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
return 0;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDiscoveryMode(struct soap* soap, struct _tds__SetDiscoveryMode *tds__SetDiscoveryMode, struct _tds__SetDiscoveryModeResponse *tds__SetDiscoveryModeResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
g_sys_info.WS_DiscoveryMode=tds__SetDiscoveryMode->DiscoveryMode;
write_sys_file();
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetRemoteDiscoveryMode(struct soap* soap, struct _tds__GetRemoteDiscoveryMode *tds__GetRemoteDiscoveryMode, struct _tds__GetRemoteDiscoveryModeResponse *tds__GetRemoteDiscoveryModeResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetRemoteDiscoveryMode(struct soap* soap, struct _tds__SetRemoteDiscoveryMode *tds__SetRemoteDiscoveryMode, struct _tds__SetRemoteDiscoveryModeResponse *tds__SetRemoteDiscoveryModeResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDPAddresses(struct soap* soap, struct _tds__GetDPAddresses *tds__GetDPAddresses, struct _tds__GetDPAddressesResponse *tds__GetDPAddressesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetEndpointReference(struct soap* soap, struct _tds__GetEndpointReference *tds__GetEndpointReference, struct _tds__GetEndpointReferenceResponse *tds__GetEndpointReferenceResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetRemoteUser(struct soap* soap, struct _tds__GetRemoteUser *tds__GetRemoteUser, struct _tds__GetRemoteUserResponse *tds__GetRemoteUserResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetRemoteUser(struct soap* soap, struct _tds__SetRemoteUser *tds__SetRemoteUser, struct _tds__SetRemoteUserResponse *tds__SetRemoteUserResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetUsers(struct soap* soap, struct _tds__GetUsers *tds__GetUsers, struct _tds__GetUsersResponse *tds__GetUsersResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
int j=0;
int i=0;
int num=0;
for(j=0;j<MAX_USER_NUM;j++)
{
	if(g_sys_info.user[j].enable)
	{
	num++;
	}
}
tds__GetUsersResponse->__sizeUser=num;
g_sys_info.usernum=num;
tds__GetUsersResponse->User=(struct tt__User*)soap_malloc(soap,tds__GetUsersResponse->__sizeUser*sizeof(struct tt__User));
for(j=0;j<MAX_USER_NUM;j++)
{
	if(g_sys_info.user[j].enable)
	{
	(tds__GetUsersResponse->User+i)->UserLevel=g_sys_info.user[j].UserLevel;
	(tds__GetUsersResponse->User+i)->Username=(char*)soap_malloc(soap,sizeof(g_sys_info.user[j].Username));
	snprintf((tds__GetUsersResponse->User+i)->Username,sizeof(g_sys_info.user[j].Username),g_sys_info.user[j].Username);
	i++;
	}
}
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__CreateUsers(struct soap* soap, struct _tds__CreateUsers *tds__CreateUsers, struct _tds__CreateUsersResponse *tds__CreateUsersResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
	if((tds__CreateUsers->__sizeUser+g_sys_info.usernum)> MAX_USER_NUM)
	    return SOAP_ERR;
	int i=0;
	int j=0;
	for(i=0;i<tds__CreateUsers->__sizeUser;i++)
	{
		for(j=0;j<MAX_USER_NUM;j++)
		{
			if(g_sys_info.user[j].enable)
			{
				if(strcmp((tds__CreateUsers->User+i)->Username,g_sys_info.user[j].Username)==0)
				{
				 return soap_sender_fault_subcode(soap, "ter:OperationProhibited/ter:UsernameClash" , NULL, NULL);
				}
			}
		}
	}


	for(i=0;i<tds__CreateUsers->__sizeUser;i++)
	{
		for(j=0;j<MAX_USER_NUM;j++)
		{
			if(!g_sys_info.user[j].enable)
			{
			g_sys_info.user[j].enable=1;
			g_sys_info.usernum++;
			g_sys_info.user[j].UserLevel=(tds__CreateUsers->User+i)->UserLevel;
			snprintf(g_sys_info.user[j].Username,sizeof(g_sys_info.user[j].Username),(tds__CreateUsers->User+i)->Username);
			snprintf(g_sys_info.user[j].Password,sizeof(g_sys_info.user[j].Password),(tds__CreateUsers->User+i)->Password);
			break;
			}
		}
	}
	write_sys_file();
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__DeleteUsers(struct soap* soap, struct _tds__DeleteUsers *tds__DeleteUsers, struct _tds__DeleteUsersResponse *tds__DeleteUsersResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
int i=0;
int j=0;
int find=0;
int deletemap=0;
int deleteNum=0;

for(i=0;i<tds__DeleteUsers->__sizeUsername;i++)
{
	if(strcmp("admin",*(tds__DeleteUsers->Username+i))==0)
	{
	 return soap_sender_fault_subcode(soap, "ter:InvalidArgVal/ter:FixedUser" , NULL, NULL);
	}
	find=0;
	for(j=0;j<MAX_USER_NUM;j++)
	{
		if(g_sys_info.user[j].enable)
		{
			if(strcmp(g_sys_info.user[j].Username,*(tds__DeleteUsers->Username+i))==0)
			{
			find=1;
			deletemap|=(1<<j);
			break;
			}
		}
	}
	if(find==0)
	{
	return soap_sender_fault_subcode(soap, "ter:InvalidArgVal/ter:UsernameMissing" , NULL, NULL);
	}	
}

	for(j=0;j<MAX_USER_NUM;j++)
	{
		if((1<<j)&deletemap)
		{
		g_sys_info.user[j].enable=0;
		deleteNum++;
		}
	}
	g_sys_info.usernum-=deleteNum;
   write_sys_file();
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetUser(struct soap* soap, struct _tds__SetUser *tds__SetUser, struct _tds__SetUserResponse *tds__SetUserResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
int i=0;
int j=0;
int find=0;
for(i=0;i<tds__SetUser->__sizeUser;i++)
{
	find=0;
	for(j=0;j<MAX_USER_NUM;j++)
	{
		if(g_sys_info.user[j].enable)
		{
			if(strcmp(g_sys_info.user[j].Username,(tds__SetUser->User+i)->Username)==0)
			{
			find=1;
			break;
			}
		}
	}
	if(find==0)
	{
	return soap_sender_fault_subcode(soap, "ter:InvalidArgVal/ter:UsernameMissing" , NULL, NULL);
	}	
}


for(i=0;i<tds__SetUser->__sizeUser;i++)
{
	for(j=0;j<MAX_USER_NUM;j++)
	{
		if(g_sys_info.user[j].enable)
		{
			if(strcmp(g_sys_info.user[j].Username,(tds__SetUser->User+i)->Username)==0)
			{
			snprintf(g_sys_info.user[j].Password,sizeof(g_sys_info.user[j].Password),(tds__SetUser->User+i)->Password);
			g_sys_info.user[j].UserLevel=(tds__SetUser->User+i)->UserLevel;
			break;
			}
		}
	}
}
   write_sys_file();
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetWsdlUrl(struct soap* soap, struct _tds__GetWsdlUrl *tds__GetWsdlUrl, struct _tds__GetWsdlUrlResponse *tds__GetWsdlUrlResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
tds__GetWsdlUrlResponse->WsdlUrl=(char *)soap_malloc(soap,128*sizeof(char));
snprintf(tds__GetWsdlUrlResponse->WsdlUrl,128,"onvif://www.onvif.org/");

return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCapabilities(struct soap* soap, struct _tds__GetCapabilities *tds__GetCapabilities, struct _tds__GetCapabilitiesResponse *tds__GetCapabilitiesResponse)
{
	fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
	char *XAddr=(char *)soap_malloc(soap,128*sizeof(char));
	snprintf(XAddr,128,"http://%s:%d/onvif/device_service",g_sys_info.network.ipaddr,ONVIF_SERVER_RORT);

	tds__GetCapabilitiesResponse->Capabilities=(struct tt__Capabilities*)soap_malloc(soap,sizeof(struct tt__Capabilities));
int i=0;
int Category=tt__CapabilityCategory__All;
for(i=0;i<tds__GetCapabilities->__sizeCategory;i++)
{
	if((*(tds__GetCapabilities->Category+i))==tt__CapabilityCategory__All)
	{
	 Category=tt__CapabilityCategory__All;
	 break;
	}
	Category|=1<<(*(tds__GetCapabilities->Category+i));
// = 0,  = 1,  = 2, tt__CapabilityCategory__Events = 3, tt__CapabilityCategory__Imaging = 4, tt__CapabilityCategory__Media = 5, tt__CapabilityCategory__PTZ = 6
}
	
#ifdef ONVIF__tad__
if((Category==tt__CapabilityCategory__All)||(Category&(1<<tt__CapabilityCategory__Analytics)))
{
  tds__GetCapabilitiesResponse->Capabilities->Analytics=
    (struct tt__AnalyticsCapabilities*)soap_malloc(soap,sizeof(struct tt__AnalyticsCapabilities));;
	tds__GetCapabilitiesResponse->Capabilities->Analytics->XAddr=XAddr;
	tds__GetCapabilitiesResponse->Capabilities->Analytics->RuleSupport=xsd__boolean__false_;
	tds__GetCapabilitiesResponse->Capabilities->Analytics->AnalyticsModuleSupport=xsd__boolean__false_;
}
#endif
	if((Category==tt__CapabilityCategory__All)||(Category&(1<<tt__CapabilityCategory__Device)))
	{
		tds__GetCapabilitiesResponse->Capabilities->Device=(struct tt__DeviceCapabilities*)soap_malloc(soap,sizeof(struct tt__DeviceCapabilities));;
		tds__GetCapabilitiesResponse->Capabilities->Device->XAddr=XAddr;
		tds__GetCapabilitiesResponse->Capabilities->Device->Network=
			(struct tt__NetworkCapabilities*)soap_malloc(soap,sizeof(struct tt__NetworkCapabilities));
		tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPFilter=
			(enum xsd__boolean*)soap_malloc(soap,sizeof(enum xsd__boolean));
		tds__GetCapabilitiesResponse->Capabilities->Device->Network->ZeroConfiguration=
			(enum xsd__boolean*)soap_malloc(soap,sizeof(enum xsd__boolean));;
		tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPVersion6=
			(enum xsd__boolean*)soap_malloc(soap,sizeof(enum xsd__boolean));;
		tds__GetCapabilitiesResponse->Capabilities->Device->Network->DynDNS=
			(enum xsd__boolean*)soap_malloc(soap,sizeof(enum xsd__boolean));;

		*tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPFilter=
			xsd__boolean__false_;//IP地址过滤
		*tds__GetCapabilitiesResponse->Capabilities->Device->Network->ZeroConfiguration=
			xsd__boolean__false_;//零配置
		*tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPVersion6=
			xsd__boolean__false_;//IP6
		*tds__GetCapabilitiesResponse->Capabilities->Device->Network->DynDNS=
			xsd__boolean__false_;//ddns


		tds__GetCapabilitiesResponse->Capabilities->Device->System=
			(struct tt__SystemCapabilities*)soap_malloc(soap,sizeof(struct tt__SystemCapabilities));
		tds__GetCapabilitiesResponse->Capabilities->Device->System->DiscoveryBye
			=xsd__boolean__true_;//下线通知
		tds__GetCapabilitiesResponse->Capabilities->Device->System->DiscoveryResolve
			=xsd__boolean__false_;//Onvif大多数情况下，resolve和resolve match不是必须的，如为了与ws-Discovery协议兼容，可实现之
		tds__GetCapabilitiesResponse->Capabilities->Device->System->RemoteDiscovery
			=xsd__boolean__false_;//远程设备发现，需DP支持
		tds__GetCapabilitiesResponse->Capabilities->Device->System->SystemBackup
			=xsd__boolean__true_;//系统参数备份
		tds__GetCapabilitiesResponse->Capabilities->Device->System->SystemLogging
			=xsd__boolean__false_;//系统日志
		tds__GetCapabilitiesResponse->Capabilities->Device->System->FirmwareUpgrade
			=xsd__boolean__true_;//固件升级
		tds__GetCapabilitiesResponse->Capabilities->Device->System->__sizeSupportedVersions=1;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions=
			(struct tt__OnvifVersion*)soap_malloc(soap,tds__GetCapabilitiesResponse->Capabilities->Device->System->__sizeSupportedVersions*sizeof(struct tt__OnvifVersion));
		tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions->Major=2;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions->Minor=2;

		tds__GetCapabilitiesResponse->Capabilities->Device->IO=
			(struct tt__IOCapabilities*)soap_malloc(soap,sizeof(struct tt__IOCapabilities));
		tds__GetCapabilitiesResponse->Capabilities->Device->IO->InputConnectors=
			(int*)soap_malloc(soap,sizeof(int));

		*tds__GetCapabilitiesResponse->Capabilities->Device->IO->InputConnectors=INPUT_CONNECTORS_NUM;//输入连接器数
		tds__GetCapabilitiesResponse->Capabilities->Device->IO->RelayOutputs=
			(int*)soap_malloc(soap,sizeof(int));
		*tds__GetCapabilitiesResponse->Capabilities->Device->IO->RelayOutputs=RELAY_OUTPUTS_NUM;//输出继电器设备数

		tds__GetCapabilitiesResponse->Capabilities->Device->Security=
			(struct tt__SecurityCapabilities*)soap_malloc(soap,sizeof(struct tt__SecurityCapabilities));
		tds__GetCapabilitiesResponse->Capabilities->Device->Security->AccessPolicyConfig=xsd__boolean__true_;//用户权限管理
	}
	
#ifdef ONVIF__TEST__
	if((Category==tt__CapabilityCategory__All)||(Category&(1<<tt__CapabilityCategory__Events)))
	{
		tds__GetCapabilitiesResponse->Capabilities->Events=
			(struct tt__EventCapabilities*)soap_malloc(soap,sizeof(struct tt__EventCapabilities));
		tds__GetCapabilitiesResponse->Capabilities->Events->XAddr=XAddr;
		tds__GetCapabilitiesResponse->Capabilities->Events->WSPausableSubscriptionManagerInterfaceSupport
			=xsd__boolean__true_;
		tds__GetCapabilitiesResponse->Capabilities->Events->WSPullPointSupport
			=xsd__boolean__true_;
		tds__GetCapabilitiesResponse->Capabilities->Events->WSSubscriptionPolicySupport
			=xsd__boolean__false_;
	}
#endif

#ifdef ONVIF__timg__
	if((Category==tt__CapabilityCategory__All)||(Category&(1<<tt__CapabilityCategory__Imaging)))
	{
	tds__GetCapabilitiesResponse->Capabilities->Imaging=
		(struct tt__ImagingCapabilities*)soap_malloc(soap,sizeof(struct tt__ImagingCapabilities));;
	tds__GetCapabilitiesResponse->Capabilities->Imaging->XAddr=XAddr;
	}
#endif

	if((Category==tt__CapabilityCategory__All)||(Category&(1<<tt__CapabilityCategory__Media)))
	{
		tds__GetCapabilitiesResponse->Capabilities->Media=(struct tt__MediaCapabilities*)soap_malloc(soap,sizeof(struct tt__MediaCapabilities));
		tds__GetCapabilitiesResponse->Capabilities->Media->XAddr=XAddr;
		tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities=
			(struct tt__RealTimeStreamingCapabilities*)soap_malloc(soap,sizeof(struct tt__RealTimeStreamingCapabilities));
		tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTPMulticast=
			(enum xsd__boolean*)soap_malloc(soap,sizeof(enum xsd__boolean));
		tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORETCP=
			(enum xsd__boolean*)soap_malloc(soap,sizeof(enum xsd__boolean));
		tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORERTSP_USCORETCP=
			(enum xsd__boolean*)soap_malloc(soap,sizeof(enum xsd__boolean));
		*tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTPMulticast=xsd__boolean__false_;
		*tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORETCP=xsd__boolean__true_;
		*tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORERTSP_USCORETCP=xsd__boolean__true_;
		tds__GetCapabilitiesResponse->Capabilities->Media->Extension=
			(struct tt__MediaCapabilitiesExtension*)soap_malloc(soap,sizeof(struct tt__MediaCapabilitiesExtension));
		tds__GetCapabilitiesResponse->Capabilities->Media->Extension->ProfileCapabilities=	
		(struct tt__ProfileCapabilities*)soap_malloc(soap,sizeof(struct tt__ProfileCapabilities));
		tds__GetCapabilitiesResponse->Capabilities->Media->Extension->ProfileCapabilities->MaximumNumberOfProfiles=3;
	}
	
#ifdef ONVIF__tptz__
	if((Category==tt__CapabilityCategory__All)||(Category&(1<<tt__CapabilityCategory__PTZ)))
	{
		//tds__GetCapabilitiesResponse->Capabilities->PTZ=
		//(struct tt__PTZCapabilities*)soap_malloc(soap,sizeof(struct tt__PTZCapabilities));;
		//tds__GetCapabilitiesResponse->Capabilities->PTZ->XAddr=XAddr;
	}
#endif
	if(Category==tt__CapabilityCategory__All)
	{
		tds__GetCapabilitiesResponse->Capabilities->Extension=
		(struct tt__CapabilitiesExtension*)soap_malloc(soap,sizeof(struct tt__CapabilitiesExtension));
		tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO=
			(struct tt__DeviceIOCapabilities*)soap_malloc(soap,sizeof(struct tt__DeviceIOCapabilities));
		tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->VideoSources=1;
		tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->XAddr=XAddr;
	}

return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDPAddresses(struct soap* soap, struct _tds__SetDPAddresses *tds__SetDPAddresses, struct _tds__SetDPAddressesResponse *tds__SetDPAddressesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetHostname(struct soap* soap, struct _tds__GetHostname *tds__GetHostname, struct _tds__GetHostnameResponse *tds__GetHostnameResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
tds__GetHostnameResponse->HostnameInformation=
	(struct tt__HostnameInformation*)soap_malloc(soap,sizeof(struct tt__HostnameInformation));
tds__GetHostnameResponse->HostnameInformation->FromDHCP=xsd__boolean__false_;
tds__GetHostnameResponse->HostnameInformation->Name=g_sys_info.network.hostname;

return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetHostname(struct soap* soap, struct _tds__SetHostname *tds__SetHostname, struct _tds__SetHostnameResponse *tds__SetHostnameResponse)
{
	CamNetworkInfo info;
	if(icam_get_network_info(g_icam_ctrl_handle,&info))
	   return SOAP_ERR;
	snprintf(info.hostName,sizeof(info.hostName),tds__SetHostname->Name);
	if(icam_set_network_info(g_icam_ctrl_handle,&info))
	   return SOAP_ERR;
	snprintf(g_sys_info.network.hostname,sizeof(info.hostName),tds__SetHostname->Name);
	fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetHostnameFromDHCP(struct soap* soap, struct _tds__SetHostnameFromDHCP *tds__SetHostnameFromDHCP, struct _tds__SetHostnameFromDHCPResponse *tds__SetHostnameFromDHCPResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
if(tds__SetHostnameFromDHCP->FromDHCP==xsd__boolean__true_)
	return SOAP_ERR;

return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDNS(struct soap* soap, struct _tds__GetDNS *tds__GetDNS, struct _tds__GetDNSResponse *tds__GetDNSResponse)
{

fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
tds__GetDNSResponse->DNSInformation=
	(struct tt__DNSInformation*)soap_malloc(soap,sizeof(struct tt__DNSInformation));
tds__GetDNSResponse->DNSInformation->DNSFromDHCP=xsd__boolean__false_;

tds__GetDNSResponse->DNSInformation->__sizeDNSManual=1;
tds__GetDNSResponse->DNSInformation->DNSManual=
	(struct tt__IPAddress*)soap_malloc(soap,tds__GetDNSResponse->DNSInformation->__sizeDNSManual*sizeof(struct tt__IPAddress));
tds__GetDNSResponse->DNSInformation->DNSManual->Type=tt__IPType__IPv4;
tds__GetDNSResponse->DNSInformation->DNSManual->IPv4Address=g_sys_info.network.DNS1;

tds__GetDNSResponse->DNSInformation->__sizeSearchDomain=1;
tds__GetDNSResponse->DNSInformation->SearchDomain=
	(char**)soap_malloc(soap,tds__GetDNSResponse->DNSInformation->__sizeSearchDomain*sizeof(char*));
*tds__GetDNSResponse->DNSInformation->SearchDomain=g_sys_info.network.domainName;



return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDNS(struct soap* soap, struct _tds__SetDNS *tds__SetDNS, struct _tds__SetDNSResponse *tds__SetDNSResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
CamNetworkInfo info;
int setret=0;
if(icam_get_network_info(g_icam_ctrl_handle,&info))
   return SOAP_ERR;

if(tds__SetDNS->FromDHCP==xsd__boolean__true_)
{	// sprintf(info.ipAddr,"0.0.0.0");
	// if(icam_set_network_info(g_icam_ctrl_handle,&info))
	//	return SOAP_ERR;
	return SOAP_ERR;
}
else if(tds__SetDNS->__sizeDNSManual&&tds__SetDNS->DNSManual)
{
	if(!tds__SetDNS->DNSManual->IPv4Address)
		return SOAP_ERR;
	
	if(check_ipaddr(tds__SetDNS->DNSManual->IPv4Address)<0)
	{
		soap_sender_fault_subcode(soap, "ter:InvalidArgVal/ter:InvalidIPv4Address", NULL, NULL);
		return SOAP_ERR;
	}
	sprintf(info.dnsServer,tds__SetDNS->DNSManual->IPv4Address);
	setret=1;

}
if(tds__SetDNS->__sizeSearchDomain&&tds__SetDNS->SearchDomain)
{
	if(*tds__SetDNS->SearchDomain)
		snprintf(info.domainName,sizeof(info.domainName),*tds__SetDNS->SearchDomain);
	setret=1;
}
if(setret)
{
   if(icam_set_network_info(g_icam_ctrl_handle,&info))
      return SOAP_ERR;
}
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetNTP(struct soap* soap, struct _tds__GetNTP *tds__GetNTP, struct _tds__GetNTPResponse *tds__GetNTPResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
CamNtpServerInfo *srvInfo=
	(CamNtpServerInfo*)soap_malloc(soap,sizeof(CamNtpServerInfo));
if(icam_get_ntp_srv_info(g_icam_ctrl_handle,srvInfo)!=E_NO)
	return SOAP_ERR;
tds__GetNTPResponse->NTPInformation=
	(struct tt__NTPInformation*)soap_malloc(soap,sizeof(struct tt__NTPInformation));
tds__GetNTPResponse->NTPInformation->FromDHCP=xsd__boolean__false_;
tds__GetNTPResponse->NTPInformation->__sizeNTPManual=1;
tds__GetNTPResponse->NTPInformation->NTPManual=
	(struct tt__NetworkHost*)soap_malloc(soap,sizeof(struct tt__NetworkHost));
tds__GetNTPResponse->NTPInformation->NTPManual->Type=tt__NetworkHostType__IPv4;
tds__GetNTPResponse->NTPInformation->NTPManual->IPv4Address=srvInfo->serverIP;
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetNTP(struct soap* soap, struct _tds__SetNTP *tds__SetNTP, struct _tds__SetNTPResponse *tds__SetNTPResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
if(tds__SetNTP->FromDHCP)
	return SOAP_ERR;
if(!tds__SetNTP->__sizeNTPManual)
	return SOAP_ERR;
if(tds__SetNTP->NTPManual->Type!=tt__NetworkHostType__IPv4)
	return SOAP_ERR;

CamNtpServerInfo *srvInfo=
	(CamNtpServerInfo*)soap_malloc(soap,sizeof(CamNtpServerInfo));
if(icam_get_ntp_srv_info(g_icam_ctrl_handle,srvInfo)!=E_NO)
		return SOAP_ERR;
if(tds__SetNTP->NTPManual->IPv4Address)
{
	if(check_ipaddr(tds__SetNTP->NTPManual->IPv4Address)<0)
	{
		soap_sender_fault_subcode(soap, "ter:InvalidArgVal/ter:InvalidIPv4Address", NULL, NULL);
		return SOAP_ERR;
	}
	snprintf(srvInfo->serverIP,16,tds__SetNTP->NTPManual->IPv4Address);
}
if(icam_set_ntp_srv_info(g_icam_ctrl_handle,srvInfo)!=E_NO)
	return SOAP_ERR;


return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDynamicDNS(struct soap* soap, struct _tds__GetDynamicDNS *tds__GetDynamicDNS, struct _tds__GetDynamicDNSResponse *tds__GetDynamicDNSResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDynamicDNS(struct soap* soap, struct _tds__SetDynamicDNS *tds__SetDynamicDNS, struct _tds__SetDynamicDNSResponse *tds__SetDynamicDNSResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

static int PrefixLength_to_mask(int PrefixLength,char *mask)
{
	struct in_addr ip;
	int i;
	unsigned int u32ipmask=0;

	for(i=0;i<PrefixLength;i++)
	{
	u32ipmask|=1<<(31-i);
	}
	ip.s_addr=htonl(u32ipmask);

	sprintf(mask, "%s", inet_ntoa(ip));
	//printf("mask=%s\n",mask);
	return 0;
}

static int mask_to_PrefixLength(char *mask)
{
	unsigned long int ip= inet_addr(mask);
	int PrefixLength=0;
	int i;
	for(i=0;i<32;i++)
	{
	if(ip&(1<<i))
		PrefixLength++;
	}
	return PrefixLength;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetNetworkInterfaces(struct soap* soap, struct _tds__GetNetworkInterfaces *tds__GetNetworkInterfaces, struct _tds__GetNetworkInterfacesResponse *tds__GetNetworkInterfacesResponse)
{
	fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
	tds__GetNetworkInterfacesResponse->__sizeNetworkInterfaces=1;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces=
		(struct tt__NetworkInterface*)soap_malloc(soap,tds__GetNetworkInterfacesResponse->__sizeNetworkInterfaces*sizeof(struct tt__NetworkInterface));
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->token="eth0";
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->Enabled=xsd__boolean__true_;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info=
	(struct tt__NetworkInterfaceInfo*)soap_malloc(soap,sizeof(struct tt__NetworkInterfaceInfo));
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info->HwAddress=
		(char*)soap_malloc(soap,18);
	snprintf(tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info->HwAddress,18,	g_sys_info.network.mac);

	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4=
		(struct tt__IPv4NetworkInterface*)soap_malloc(soap,sizeof(struct tt__IPv4NetworkInterface));
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Enabled=xsd__boolean__true_;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config=
		(struct tt__IPv4Configuration*)soap_malloc(soap,sizeof(struct tt__IPv4Configuration));
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->__sizeManual=1;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->Manual=
		(struct tt__PrefixedIPv4Address*)soap_malloc(soap,tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->__sizeManual*sizeof(struct tt__PrefixedIPv4Address));
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->Manual->Address=
		(char*)soap_malloc(soap,16*sizeof(char));
	snprintf(tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->Manual->Address,16,g_sys_info.network.ipaddr);

//	g_sys_info.network.netmask
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->Manual->PrefixLength=mask_to_PrefixLength(g_sys_info.network.netmask);

	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->DHCP=xsd__boolean__false_;

	snprintf(tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info->HwAddress,18,	g_sys_info.network.mac);
//tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info=
//	(struct tt__NetworkInterfaceInfo*)soap_malloc(soap,sizeof(struct tt__NetworkInterfaceInfo));
//tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info->

return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetNetworkInterfaces(struct soap* soap, struct _tds__SetNetworkInterfaces *tds__SetNetworkInterfaces, struct _tds__SetNetworkInterfacesResponse *tds__SetNetworkInterfacesResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
if(tds__SetNetworkInterfaces->InterfaceToken)
{
     if(strcmp(tds__SetNetworkInterfaces->InterfaceToken,"eth0")!=0)
	     return SOAP_ERR;
}
CamNetworkInfo info;

if(tds__SetNetworkInterfaces->NetworkInterface)
{
	if(tds__SetNetworkInterfaces->NetworkInterface->IPv4)
	{
	  if(icam_get_network_info(g_icam_ctrl_handle,&info))
        return SOAP_ERR;
		if(tds__SetNetworkInterfaces->NetworkInterface->IPv4->DHCP&&
			*tds__SetNetworkInterfaces->NetworkInterface->IPv4->DHCP==xsd__boolean__true_)
		{
		       sprintf(info.ipAddr,"0.0.0.0");
				if(icam_set_network_info(g_icam_ctrl_handle,&info))
			       return SOAP_ERR;
		}
	     else if(tds__SetNetworkInterfaces->NetworkInterface->IPv4->__sizeManual)
		{
			if(check_ipaddr(tds__SetNetworkInterfaces->NetworkInterface->IPv4->Manual->Address)<0)
			{
			soap_sender_fault_subcode(soap, "ter:InvalidArgVal/ter:InvalidIPv4Address", NULL, NULL);
			return SOAP_ERR;
			}

		
			snprintf(info.ipAddr,sizeof(info.ipAddr),tds__SetNetworkInterfaces->NetworkInterface->IPv4->Manual->Address);
	    	PrefixLength_to_mask(tds__SetNetworkInterfaces->NetworkInterface->IPv4->Manual->PrefixLength,info.ipMask);	
			if(icam_set_network_info(g_icam_ctrl_handle,&info))
			   return SOAP_ERR;
			//sprintf(g_sys_info.network.ipaddr,info.ipAddr);
			//sprintf(g_sys_info.network.netmask,info.ipMask);
		}
		 tds__SetNetworkInterfacesResponse->RebootNeeded=xsd__boolean__true_;
	}
	else if(tds__SetNetworkInterfaces->NetworkInterface->IPv6)
		 return SOAP_ERR;

}

return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetNetworkProtocols(struct soap* soap, struct _tds__GetNetworkProtocols *tds__GetNetworkProtocols, struct _tds__GetNetworkProtocolsResponse *tds__GetNetworkProtocolsResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
struct tt__NetworkProtocol *NetworkProtocols;
tds__GetNetworkProtocolsResponse->__sizeNetworkProtocols=2;
tds__GetNetworkProtocolsResponse->NetworkProtocols=
	(struct tt__NetworkProtocol*)soap_malloc(soap,tds__GetNetworkProtocolsResponse->__sizeNetworkProtocols*sizeof(struct tt__NetworkProtocol));

NetworkProtocols=tds__GetNetworkProtocolsResponse->NetworkProtocols;

NetworkProtocols->Name=tt__NetworkProtocolType__RTSP;
if(g_sys_info.rtpParamsbuf.flags&CAM_RTP_FLAG_EN)
	NetworkProtocols->Enabled=xsd__boolean__true_;
else
	NetworkProtocols->Enabled=xsd__boolean__false_;
NetworkProtocols->__sizePort=1;
NetworkProtocols->Port=
(int*)soap_malloc(soap,tds__GetNetworkProtocolsResponse->NetworkProtocols->__sizePort*sizeof(int));
*NetworkProtocols->Port=g_sys_info.rtpParamsbuf.rtspSrvPort;

NetworkProtocols++;
	NetworkProtocols->Name=tt__NetworkProtocolType__HTTP;
if(g_sys_info.httpParamsbuf.flags)
	NetworkProtocols->Enabled=xsd__boolean__true_;
else
	NetworkProtocols->Enabled=xsd__boolean__false_;
NetworkProtocols->__sizePort=1;
NetworkProtocols->Port=
(int*)soap_malloc(soap,tds__GetNetworkProtocolsResponse->NetworkProtocols->__sizePort*sizeof(int));
*NetworkProtocols->Port=g_sys_info.httpParamsbuf.rtspSrvPort;

return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetNetworkProtocols(struct soap* soap, struct _tds__SetNetworkProtocols *tds__SetNetworkProtocols, struct _tds__SetNetworkProtocolsResponse *tds__SetNetworkProtocolsResponse)
{

fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
int i=0;
struct tt__NetworkProtocol *NetworkProtocols;
CamRtpParams rtpParamsbuf;

for(i=0;i<tds__SetNetworkProtocols->__sizeNetworkProtocols;i++)
{
	NetworkProtocols=tds__SetNetworkProtocols->NetworkProtocols+i;
	if(NetworkProtocols->Name==tt__NetworkProtocolType__RTSP)
	 {
		 if(icam_get_rtp_params(g_icam_ctrl_handle,&rtpParamsbuf))
		 	return SOAP_ERR;
		 if(NetworkProtocols->__sizePort)
		 	rtpParamsbuf.rtspSrvPort=*NetworkProtocols->Port;
		 if(NetworkProtocols->Enabled)
		 	rtpParamsbuf.flags|=~CAM_RTP_FLAG_EN;
		 else
			rtpParamsbuf.flags&=CAM_RTP_FLAG_EN;
		 if(icam_set_rtp_params(g_icam_ctrl_handle,&rtpParamsbuf))
		 	return SOAP_ERR;
		 g_sys_info.rtpParamsbuf=rtpParamsbuf;
	 } 
	else if(NetworkProtocols->Name==tt__NetworkProtocolType__HTTP)
	{
		if(NetworkProtocols->__sizePort)
		   g_sys_info.httpParamsbuf.rtspSrvPort=*NetworkProtocols->Port;
		if(NetworkProtocols->Enabled)
		   g_sys_info.httpParamsbuf.flags=1;
		else
		  g_sys_info.httpParamsbuf.flags=0;
	}
	else
	{
	    soap_sender_fault_subcode(soap, "ter:InvalidArgVal/ter:ServiceNotSupported", NULL, NULL);
	    return SOAP_ERR;
	}
} 
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetNetworkDefaultGateway(struct soap* soap, struct _tds__GetNetworkDefaultGateway *tds__GetNetworkDefaultGateway, struct _tds__GetNetworkDefaultGatewayResponse *tds__GetNetworkDefaultGatewayResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
tds__GetNetworkDefaultGatewayResponse->NetworkGateway=
	(struct tt__NetworkGateway*)soap_malloc(soap,sizeof(struct tt__NetworkGateway));
tds__GetNetworkDefaultGatewayResponse->NetworkGateway->__sizeIPv4Address=1;
tds__GetNetworkDefaultGatewayResponse->NetworkGateway->IPv4Address=
	(char**)soap_malloc(soap,tds__GetNetworkDefaultGatewayResponse->NetworkGateway->__sizeIPv4Address*sizeof(char *));
*tds__GetNetworkDefaultGatewayResponse->NetworkGateway->IPv4Address=
	(char*)soap_malloc(soap,18);
snprintf(*tds__GetNetworkDefaultGatewayResponse->NetworkGateway->IPv4Address,18,g_sys_info.network.gateway);

return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetNetworkDefaultGateway(struct soap* soap, struct _tds__SetNetworkDefaultGateway *tds__SetNetworkDefaultGateway, struct _tds__SetNetworkDefaultGatewayResponse *tds__SetNetworkDefaultGatewayResponse)
{

fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
CamNetworkInfo info;

if(0==tds__SetNetworkDefaultGateway->__sizeIPv4Address)
	 return SOAP_ERR;
if(tds__SetNetworkDefaultGateway->IPv4Address)
{
	if(check_ipaddr(*tds__SetNetworkDefaultGateway->IPv4Address)<0)
	{
	   soap_sender_fault_subcode(soap, "ter:InvalidArgVal/ter:InvalidGatewayAddress", NULL, NULL);
		return SOAP_ERR;
	}
}
if(icam_get_network_info(g_icam_ctrl_handle,&info))
   return SOAP_ERR;

snprintf(info.gatewayIP,sizeof(info.gatewayIP),*tds__SetNetworkDefaultGateway->IPv4Address);
if(icam_set_network_info(g_icam_ctrl_handle,&info))
   return SOAP_ERR;

sprintf(g_sys_info.network.gateway,info.gatewayIP);

return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetZeroConfiguration(struct soap* soap, struct _tds__GetZeroConfiguration *tds__GetZeroConfiguration, struct _tds__GetZeroConfigurationResponse *tds__GetZeroConfigurationResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetZeroConfiguration(struct soap* soap, struct _tds__SetZeroConfiguration *tds__SetZeroConfiguration, struct _tds__SetZeroConfigurationResponse *tds__SetZeroConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetIPAddressFilter(struct soap* soap, struct _tds__GetIPAddressFilter *tds__GetIPAddressFilter, struct _tds__GetIPAddressFilterResponse *tds__GetIPAddressFilterResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetIPAddressFilter(struct soap* soap, struct _tds__SetIPAddressFilter *tds__SetIPAddressFilter, struct _tds__SetIPAddressFilterResponse *tds__SetIPAddressFilterResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__AddIPAddressFilter(struct soap* soap, struct _tds__AddIPAddressFilter *tds__AddIPAddressFilter, struct _tds__AddIPAddressFilterResponse *tds__AddIPAddressFilterResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__RemoveIPAddressFilter(struct soap* soap, struct _tds__RemoveIPAddressFilter *tds__RemoveIPAddressFilter, struct _tds__RemoveIPAddressFilterResponse *tds__RemoveIPAddressFilterResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetAccessPolicy(struct soap* soap, struct _tds__GetAccessPolicy *tds__GetAccessPolicy, struct _tds__GetAccessPolicyResponse *tds__GetAccessPolicyResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetAccessPolicy(struct soap* soap, struct _tds__SetAccessPolicy *tds__SetAccessPolicy, struct _tds__SetAccessPolicyResponse *tds__SetAccessPolicyResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__CreateCertificate(struct soap* soap, struct _tds__CreateCertificate *tds__CreateCertificate, struct _tds__CreateCertificateResponse *tds__CreateCertificateResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCertificates(struct soap* soap, struct _tds__GetCertificates *tds__GetCertificates, struct _tds__GetCertificatesResponse *tds__GetCertificatesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCertificatesStatus(struct soap* soap, struct _tds__GetCertificatesStatus *tds__GetCertificatesStatus, struct _tds__GetCertificatesStatusResponse *tds__GetCertificatesStatusResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetCertificatesStatus(struct soap* soap, struct _tds__SetCertificatesStatus *tds__SetCertificatesStatus, struct _tds__SetCertificatesStatusResponse *tds__SetCertificatesStatusResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__DeleteCertificates(struct soap* soap, struct _tds__DeleteCertificates *tds__DeleteCertificates, struct _tds__DeleteCertificatesResponse *tds__DeleteCertificatesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetPkcs10Request(struct soap* soap, struct _tds__GetPkcs10Request *tds__GetPkcs10Request, struct _tds__GetPkcs10RequestResponse *tds__GetPkcs10RequestResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__LoadCertificates(struct soap* soap, struct _tds__LoadCertificates *tds__LoadCertificates, struct _tds__LoadCertificatesResponse *tds__LoadCertificatesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetClientCertificateMode(struct soap* soap, struct _tds__GetClientCertificateMode *tds__GetClientCertificateMode, struct _tds__GetClientCertificateModeResponse *tds__GetClientCertificateModeResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetClientCertificateMode(struct soap* soap, struct _tds__SetClientCertificateMode *tds__SetClientCertificateMode, struct _tds__SetClientCertificateModeResponse *tds__SetClientCertificateModeResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetRelayOutputs(struct soap* soap, struct _tds__GetRelayOutputs *tds__GetRelayOutputs, struct _tds__GetRelayOutputsResponse *tds__GetRelayOutputsResponse)
{
    fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
	tds__GetRelayOutputsResponse->__sizeRelayOutputs=RELAY_OUTPUTS_NUM;
	struct tt__RelayOutput *RelayOutput;
	int i;
	if(tds__GetRelayOutputsResponse->__sizeRelayOutputs>0)
    {
	
		tds__GetRelayOutputsResponse->RelayOutputs=
			(struct tt__RelayOutput *)soap_malloc(soap,tds__GetRelayOutputsResponse->__sizeRelayOutputs*sizeof(struct tt__RelayOutput ));
		RelayOutput=tds__GetRelayOutputsResponse->RelayOutputs;
		for(i=0;i<tds__GetRelayOutputsResponse->__sizeRelayOutputs;i++)
		{
			RelayOutput->token=
				(char *)soap_malloc(soap,128*sizeof(char));
			sprintf(RelayOutput->token,"IO%d",g_alarmout[i]+1);
			RelayOutput->Properties=
				(struct tt__RelayOutputSettings *)soap_malloc(soap,sizeof(struct tt__RelayOutputSettings ));
			RelayOutput->Properties->Mode=g_sys_info.alarmout[g_alarmout[i]].Mode;
			RelayOutput->Properties->IdleState=g_sys_info.alarmout[g_alarmout[i]].IdleState;
			RelayOutput->Properties->DelayTime=g_sys_info.alarmout[g_alarmout[i]].DelayTime;//单位ms
			RelayOutput++;
	   }
    }  
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetRelayOutputSettings(struct soap* soap, struct _tds__SetRelayOutputSettings *tds__SetRelayOutputSettings, struct _tds__SetRelayOutputSettingsResponse *tds__SetRelayOutputSettingsResponse)
{
    fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
    int relayidx=-1;
	if(!tds__SetRelayOutputSettings->RelayOutputToken)
		return SOAP_ERR;
	if (sscanf(tds__SetRelayOutputSettings->RelayOutputToken, "IO%d",&relayidx) != 1 )
		return SOAP_ERR;

	if(relayidx<1||relayidx>8)
		return SOAP_ERR;
	relayidx-=1;
	
	g_sys_info.alarmout[relayidx].Mode=tds__SetRelayOutputSettings->Properties->Mode;
	g_sys_info.alarmout[relayidx].DelayTime=tds__SetRelayOutputSettings->Properties->DelayTime;
	//printf("tds__SetRelayOutputSettings->Properties->DelayTime=%d\n",tds__SetRelayOutputSettings->Properties->DelayTime);
	g_sys_info.alarmout[relayidx].IdleState=tds__SetRelayOutputSettings->Properties->IdleState;
	write_sys_file();
return 0;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__SetRelayOutputState(struct soap* soap, struct _tds__SetRelayOutputState *tds__SetRelayOutputState, struct _tds__SetRelayOutputStateResponse *tds__SetRelayOutputStateResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);

int relayidx=-1;
int status;
if(!tds__SetRelayOutputState->RelayOutputToken)
	return SOAP_ERR;
if (sscanf(tds__SetRelayOutputState->RelayOutputToken, "IO%d",&relayidx) != 1 )
	return SOAP_ERR;
if(relayidx<1||relayidx>8)
	return SOAP_ERR;
relayidx-=1;
	if(g_sys_info.alarmout[relayidx].IdleState==tt__RelayIdleState__open)
	{
		if(tds__SetRelayOutputState->LogicalState==tt__RelayLogicalState__inactive)
			status=0;
		else
			status=1;
	}else
	{
		if(tds__SetRelayOutputState->LogicalState==tt__RelayLogicalState__inactive)
			status=1;
		else
			status=0;
	}
	CamIoCfg params;
	if(icam_get_io_config(g_icam_ctrl_handle, &params))
		return SOAP_ERR;
	if(!(params.direction&(1<<relayidx)))
		return SOAP_ERR;
	if(status)
		params.status|=1<<relayidx;
	else
		params.status&=~(1<<relayidx);
	if(icam_set_io_config(g_icam_ctrl_handle,&params))
		return SOAP_ERR;
	g_sys_info.Ioparams.status=params.status;
	if(g_sys_info.alarmout[relayidx].Mode==tt__RelayMode__Monostable
		&&tds__SetRelayOutputState->LogicalState==tt__RelayLogicalState__active)
	{
	 g_onvif_status.alarmout[relayidx].DelayTime=g_sys_info.alarmout[relayidx].DelayTime/1000;
	 g_onvif_status.alarmout[relayidx].IdleState=g_sys_info.alarmout[relayidx].IdleState;
	 g_onvif_status.alarmout[relayidx].Mode=g_sys_info.alarmout[relayidx].Mode;
	 g_onvif_status.relayreset=1;
	}
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SendAuxiliaryCommand(struct soap* soap, struct _tds__SendAuxiliaryCommand *tds__SendAuxiliaryCommand, struct _tds__SendAuxiliaryCommandResponse *tds__SendAuxiliaryCommandResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCACertificates(struct soap* soap, struct _tds__GetCACertificates *tds__GetCACertificates, struct _tds__GetCACertificatesResponse *tds__GetCACertificatesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__LoadCertificateWithPrivateKey(struct soap* soap, struct _tds__LoadCertificateWithPrivateKey *tds__LoadCertificateWithPrivateKey, struct _tds__LoadCertificateWithPrivateKeyResponse *tds__LoadCertificateWithPrivateKeyResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCertificateInformation(struct soap* soap, struct _tds__GetCertificateInformation *tds__GetCertificateInformation, struct _tds__GetCertificateInformationResponse *tds__GetCertificateInformationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__LoadCACertificates(struct soap* soap, struct _tds__LoadCACertificates *tds__LoadCACertificates, struct _tds__LoadCACertificatesResponse *tds__LoadCACertificatesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__CreateDot1XConfiguration(struct soap* soap, struct _tds__CreateDot1XConfiguration *tds__CreateDot1XConfiguration, struct _tds__CreateDot1XConfigurationResponse *tds__CreateDot1XConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDot1XConfiguration(struct soap* soap, struct _tds__SetDot1XConfiguration *tds__SetDot1XConfiguration, struct _tds__SetDot1XConfigurationResponse *tds__SetDot1XConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDot1XConfiguration(struct soap* soap, struct _tds__GetDot1XConfiguration *tds__GetDot1XConfiguration, struct _tds__GetDot1XConfigurationResponse *tds__GetDot1XConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDot1XConfigurations(struct soap* soap, struct _tds__GetDot1XConfigurations *tds__GetDot1XConfigurations, struct _tds__GetDot1XConfigurationsResponse *tds__GetDot1XConfigurationsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__DeleteDot1XConfiguration(struct soap* soap, struct _tds__DeleteDot1XConfiguration *tds__DeleteDot1XConfiguration, struct _tds__DeleteDot1XConfigurationResponse *tds__DeleteDot1XConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDot11Capabilities(struct soap* soap, struct _tds__GetDot11Capabilities *tds__GetDot11Capabilities, struct _tds__GetDot11CapabilitiesResponse *tds__GetDot11CapabilitiesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDot11Status(struct soap* soap, struct _tds__GetDot11Status *tds__GetDot11Status, struct _tds__GetDot11StatusResponse *tds__GetDot11StatusResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__ScanAvailableDot11Networks(struct soap* soap, struct _tds__ScanAvailableDot11Networks *tds__ScanAvailableDot11Networks, struct _tds__ScanAvailableDot11NetworksResponse *tds__ScanAvailableDot11NetworksResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemUris(struct soap* soap, struct _tds__GetSystemUris *tds__GetSystemUris, struct _tds__GetSystemUrisResponse *tds__GetSystemUrisResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__StartFirmwareUpgrade(struct soap* soap, struct _tds__StartFirmwareUpgrade *tds__StartFirmwareUpgrade, struct _tds__StartFirmwareUpgradeResponse *tds__StartFirmwareUpgradeResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tds__StartSystemRestore(struct soap* soap, struct _tds__StartSystemRestore *tds__StartSystemRestore, struct _tds__StartSystemRestoreResponse *tds__StartSystemRestoreResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}


SOAP_FMAC5 int SOAP_FMAC6 __tev__PullMessages(struct soap* soap, struct _tev__PullMessages *tev__PullMessages, struct _tev__PullMessagesResponse *tev__PullMessagesResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
unsigned int ntime= time(NULL);
int i=0;
struct wsnt__NotificationMessageHolderType *wsnt__NotificationMessage;

if(soap->header)
	soap->header->wsa5__Action=	"http://www.onvif.org/ver10/events/wsdl/PullPointSubscription/PullMessagesResponse";

sleep(2);
tev__PullMessagesResponse->CurrentTime=ntime;
tev__PullMessagesResponse->TerminationTime=ntime+600;
tev__PullMessagesResponse->__sizeNotificationMessage=1;
tev__PullMessagesResponse->wsnt__NotificationMessage=
	(struct wsnt__NotificationMessageHolderType *)soap_malloc(soap,tev__PullMessagesResponse->__sizeNotificationMessage*sizeof(struct wsnt__NotificationMessageHolderType ));
wsnt__NotificationMessage=tev__PullMessagesResponse->wsnt__NotificationMessage;


for(i=0;i<tev__PullMessagesResponse->__sizeNotificationMessage;i++)
{
	wsnt__NotificationMessage =tev__PullMessagesResponse->wsnt__NotificationMessage+i;

	wsnt__NotificationMessage->SubscriptionReference=	
		(struct wsa5__EndpointReferenceType *)soap_malloc(soap,sizeof(struct wsa5__EndpointReferenceType ));
	wsnt__NotificationMessage->SubscriptionReference->Address=g_sys_info.network.ipaddr;
	

	wsnt__NotificationMessage->Topic=	
		(struct wsnt__TopicExpressionType *)soap_malloc(soap,sizeof(struct wsnt__TopicExpressionType ));
	wsnt__NotificationMessage->Topic->Dialect="http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet";
	wsnt__NotificationMessage->Topic->__mixed="tns1:Device/HardwareFailure/StorageFailure";


	wsnt__NotificationMessage->ProducerReference=	
		(struct wsa5__EndpointReferenceType *)soap_malloc(soap,sizeof(struct wsa5__EndpointReferenceType ));
	wsnt__NotificationMessage->ProducerReference->Address=g_sys_info.network.ipaddr;

   wsnt__NotificationMessage->Message.message=
   			(struct _tt__Message *)soap_malloc(soap,sizeof(struct _tt__Message));
   wsnt__NotificationMessage->Message.message->UtcTime=ntime;
   
   wsnt__NotificationMessage->Message.message->Source=
	   (struct tt__ItemList *)soap_malloc(soap,sizeof(struct tt__ItemList));
   wsnt__NotificationMessage->Message.message->Source->__sizeSimpleItem=1;
   wsnt__NotificationMessage->Message.message->Source->SimpleItem=
	   (struct _tt__ItemList_SimpleItem *)soap_malloc(soap,wsnt__NotificationMessage->Message.message->Source->__sizeSimpleItem*sizeof(struct _tt__ItemList_SimpleItem));
   wsnt__NotificationMessage->Message.message->Source->SimpleItem->Name="HardDiskNo";
   wsnt__NotificationMessage->Message.message->Source->SimpleItem->Value="1";

#if 0
   wsnt__NotificationMessage->Message.message->Data=
	   (struct tt__ItemList *)soap_malloc(soap,sizeof(struct tt__ItemList));
   wsnt__NotificationMessage->Message.message->Data->__sizeSimpleItem=1;
   wsnt__NotificationMessage->Message.message->Data->SimpleItem=
	   (struct _tt__ItemList_SimpleItem *)soap_malloc(soap,wsnt__NotificationMessage->Message.message->Data->__sizeSimpleItem*sizeof(struct _tt__ItemList_SimpleItem));
   wsnt__NotificationMessage->Message.message->Data->SimpleItem->Name="IsInside";
   wsnt__NotificationMessage->Message.message->Data->SimpleItem->Value="false";


   wsnt__NotificationMessage->Message.message->Key=
	   (struct tt__ItemList *)soap_malloc(soap,sizeof(struct tt__ItemList));
   wsnt__NotificationMessage->Message.message->Key->__sizeSimpleItem=1;
   wsnt__NotificationMessage->Message.message->Key->SimpleItem=
	   (struct _tt__ItemList_SimpleItem *)soap_malloc(soap,wsnt__NotificationMessage->Message.message->Key->__sizeSimpleItem*sizeof(struct _tt__ItemList_SimpleItem));
   wsnt__NotificationMessage->Message.message->Key->SimpleItem->Name="ObjectId";
   wsnt__NotificationMessage->Message.message->Key->SimpleItem->Value="5";


   
	wsnt__NotificationMessage->Message.message->PropertyOperation=
		(enum tt__PropertyOperation*)soap_malloc(soap,sizeof(enum tt__PropertyOperation));
	*wsnt__NotificationMessage->Message.message->PropertyOperation=tt__PropertyOperation__Initialized;
#endif

}
//wsnt__NotificationMessage->Topic
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__SetSynchronizationPoint(struct soap* soap, struct _tev__SetSynchronizationPoint *tev__SetSynchronizationPoint, struct _tev__SetSynchronizationPointResponse *tev__SetSynchronizationPointResponse)
{
if(soap->header)
	soap->header->wsa5__Action="http://www.onvif.org/ver10/events/wsdl/PullPointSubscription/SetSynchronizationPointResponse";

fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__GetServiceCapabilities(struct soap* soap, struct _tev__GetServiceCapabilities *tev__GetServiceCapabilities, struct _tev__GetServiceCapabilitiesResponse *tev__GetServiceCapabilitiesResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
tev__GetServiceCapabilitiesResponse->Capabilities=
	(struct tev__Capabilities *)soap_malloc(soap,sizeof(struct tev__Capabilities));

tev__GetServiceCapabilitiesResponse->Capabilities->WSPullPointSupport=
	(enum xsd__boolean *)soap_malloc(soap,sizeof(enum xsd__boolean));
tev__GetServiceCapabilitiesResponse->Capabilities->WSSubscriptionPolicySupport=
	(enum xsd__boolean *)soap_malloc(soap,sizeof(enum xsd__boolean));
tev__GetServiceCapabilitiesResponse->Capabilities->WSPausableSubscriptionManagerInterfaceSupport=
	(enum xsd__boolean *)soap_malloc(soap,sizeof(enum xsd__boolean));

*tev__GetServiceCapabilitiesResponse->Capabilities->WSPullPointSupport=xsd__boolean__true_;
*tev__GetServiceCapabilitiesResponse->Capabilities->WSSubscriptionPolicySupport=xsd__boolean__true_;
*tev__GetServiceCapabilitiesResponse->Capabilities->WSPausableSubscriptionManagerInterfaceSupport=xsd__boolean__true_;



return 0;
}


SOAP_FMAC5 int SOAP_FMAC6 __tev__CreatePullPointSubscription(struct soap* soap, struct _tev__CreatePullPointSubscription *tev__CreatePullPointSubscription, struct _tev__CreatePullPointSubscriptionResponse *tev__CreatePullPointSubscriptionResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
unsigned int ntime= time(NULL);
if(soap->header)
{
soap->header->wsa__Action=NULL;
soap->header->wsa5__Action="http://www.onvif.org/ver10/events/wsdl/EventPortType/CreatePullPointSubscriptionResponse";
}
tev__CreatePullPointSubscriptionResponse->SubscriptionReference.Address=
	(char*)soap_malloc(soap,128);
sprintf(tev__CreatePullPointSubscriptionResponse->SubscriptionReference.Address,
	"http://%s:%d/onvif/Events/PullSubManager_%u",
	g_sys_info.network.ipaddr,ONVIF_EVENT_RORT,ntime);
tev__CreatePullPointSubscriptionResponse->wsnt__CurrentTime=ntime;
tev__CreatePullPointSubscriptionResponse->wsnt__TerminationTime=ntime+600;
//tev__CreatePullPointSubscription->InitialTerminationTime

return 0;
}



SOAP_FMAC5 int SOAP_FMAC6 __tev__GetEventProperties(struct soap* soap, struct _tev__GetEventProperties *tev__GetEventProperties, struct _tev__GetEventPropertiesResponse *tev__GetEventPropertiesResponse)
{

fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);

	tev__GetEventPropertiesResponse->__sizeTopicNamespaceLocation=1;
	tev__GetEventPropertiesResponse->TopicNamespaceLocation=
		(char **)soap_malloc(soap,tev__GetEventPropertiesResponse->__sizeTopicNamespaceLocation*sizeof(char *));

	*tev__GetEventPropertiesResponse->TopicNamespaceLocation=
		(char *)soap_malloc(soap,128*sizeof(char));
	sprintf(*tev__GetEventPropertiesResponse->TopicNamespaceLocation,"http://www.onvif.org/onvif/ver10/topics/topicns.xml");

	tev__GetEventPropertiesResponse->wsnt__FixedTopicSet=xsd__boolean__true_;
	tev__GetEventPropertiesResponse->wstop__TopicSet=
		(struct wstop__TopicSetType *)soap_malloc(soap,sizeof(struct wstop__TopicSetType ));
	tev__GetEventPropertiesResponse->wstop__TopicSet->documentation=
		(struct wstop__Documentation *)soap_malloc(soap,sizeof(struct wstop__Documentation ));

	tev__GetEventPropertiesResponse->wstop__TopicSet->documentation->__mixed=
		(char *)soap_malloc(soap,8*1024*sizeof(char));
	FILE *pfile=fopen(EVENTSETFILE,"rb");
	if(pfile)
	{
		//fprintf(stderr,"%x,event.set\n",pfile);
		fread(tev__GetEventPropertiesResponse->wstop__TopicSet->documentation->__mixed,1,8*1024,pfile);
		fclose(pfile);
	}


	tev__GetEventPropertiesResponse->__sizeTopicExpressionDialect=2;
	tev__GetEventPropertiesResponse->wsnt__TopicExpressionDialect=
		(char **)soap_malloc(soap,tev__GetEventPropertiesResponse->__sizeTopicExpressionDialect*sizeof(char *));
	*tev__GetEventPropertiesResponse->wsnt__TopicExpressionDialect=
		(char *)soap_malloc(soap,128*sizeof(char));
	sprintf(*tev__GetEventPropertiesResponse->wsnt__TopicExpressionDialect,"http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet");
	*(tev__GetEventPropertiesResponse->wsnt__TopicExpressionDialect+1)=
		(char *)soap_malloc(soap,128*sizeof(char));
	sprintf(*(tev__GetEventPropertiesResponse->wsnt__TopicExpressionDialect+1),"http://docs.oasis-open.org/wsn/t-1/TopicExpression/Concrete");

	tev__GetEventPropertiesResponse->__sizeMessageContentFilterDialect=1;
	tev__GetEventPropertiesResponse->MessageContentFilterDialect=
		(char **)soap_malloc(soap,tev__GetEventPropertiesResponse->__sizeMessageContentFilterDialect*sizeof(char *));
	*tev__GetEventPropertiesResponse->MessageContentFilterDialect=
		(char *)soap_malloc(soap,128*sizeof(char));
	sprintf(*tev__GetEventPropertiesResponse->MessageContentFilterDialect,"http://www.onvif.org/ver10/tev/messageContentFilter/ItemFilter");

	tev__GetEventPropertiesResponse->__sizeMessageContentSchemaLocation=1;
	tev__GetEventPropertiesResponse->MessageContentSchemaLocation=
		(char **)soap_malloc(soap,tev__GetEventPropertiesResponse->__sizeMessageContentSchemaLocation*sizeof(char *));
	*tev__GetEventPropertiesResponse->MessageContentSchemaLocation=
		(char *)soap_malloc(soap,128*sizeof(char));
	sprintf(*tev__GetEventPropertiesResponse->MessageContentSchemaLocation,"http://www.onvif.org/onvif/ver10/schema/onvif.xsd");
	if(soap->header)
	{
	soap->header->wsa__Action=NULL;
	soap->header->wsa5__Action="http://www.onvif.org/ver10/events/wsdl/EventPortType/GetEventPropertiesResponse";
	}

return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__Renew(struct soap* soap, struct _wsnt__Renew *wsnt__Renew, struct _wsnt__RenewResponse *wsnt__RenewResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
if(soap->header)
	soap->header->wsa5__Action="http://docs.oasis-open.org/wsn/bw-2/SubscriptionManager/RenewResponse";
unsigned int ntime= time(NULL);
wsnt__RenewResponse->CurrentTime=
	(time_t*)soap_malloc(soap,sizeof(time_t));
*wsnt__RenewResponse->CurrentTime=ntime;
wsnt__RenewResponse->TerminationTime=ntime+600;

return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__Unsubscribe(struct soap* soap, struct _wsnt__Unsubscribe *wsnt__Unsubscribe, struct _wsnt__UnsubscribeResponse *wsnt__UnsubscribeResponse)
{
if(soap->header)
	soap->header->wsa5__Action="http://docs.oasis-open.org/wsn/bw-2/SubscriptionManager/UnsubscribeResponse";

fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);

return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__Subscribe(struct soap* soap, struct _wsnt__Subscribe *wsnt__Subscribe, struct _wsnt__SubscribeResponse *wsnt__SubscribeResponse)
{
if(soap->header)
	soap->header->wsa5__Action="http://docs.oasis-open.org/wsn/bw-2/NotificationProducer/SubscribeResponse";
wsnt__Subscribe->ConsumerReference.Address;
wsnt__Subscribe->InitialTerminationTime;
wsnt__SubscribeResponse->SubscriptionReference;
unsigned int ntime= time(NULL);

wsnt__SubscribeResponse->SubscriptionReference.Address=
	(char*)soap_malloc(soap,128);
sprintf(wsnt__SubscribeResponse->SubscriptionReference.Address,
	"http://%s:%d/onvif/Events/PullSubManager_%u",
	g_sys_info.network.ipaddr,ONVIF_EVENT_RORT,ntime);

wsnt__SubscribeResponse->CurrentTime=
	(time_t*)soap_malloc(soap,sizeof(time_t));
wsnt__SubscribeResponse->TerminationTime=
	(time_t*)soap_malloc(soap,sizeof(time_t));

*wsnt__SubscribeResponse->CurrentTime=ntime;
*wsnt__SubscribeResponse->TerminationTime=ntime+600;


fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__GetCurrentMessage(struct soap* soap, struct _wsnt__GetCurrentMessage *wsnt__GetCurrentMessage, struct _wsnt__GetCurrentMessageResponse *wsnt__GetCurrentMessageResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tev__Notify(struct soap* soap, struct _wsnt__Notify *wsnt__Notify){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tev__GetMessages(struct soap* soap, struct _wsnt__GetMessages *wsnt__GetMessages, struct _wsnt__GetMessagesResponse *wsnt__GetMessagesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tev__DestroyPullPoint(struct soap* soap, struct _wsnt__DestroyPullPoint *wsnt__DestroyPullPoint, struct _wsnt__DestroyPullPointResponse *wsnt__DestroyPullPointResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tev__Notify_(struct soap* soap, struct _wsnt__Notify *wsnt__Notify){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tev__CreatePullPoint(struct soap* soap, struct _wsnt__CreatePullPoint *wsnt__CreatePullPoint, struct _wsnt__CreatePullPointResponse *wsnt__CreatePullPointResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tev__Renew_(struct soap* soap, struct _wsnt__Renew *wsnt__Renew, struct _wsnt__RenewResponse *wsnt__RenewResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tev__Unsubscribe_(struct soap* soap, struct _wsnt__Unsubscribe *wsnt__Unsubscribe, struct _wsnt__UnsubscribeResponse *wsnt__UnsubscribeResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tev__PauseSubscription(struct soap* soap, struct _wsnt__PauseSubscription *wsnt__PauseSubscription, struct _wsnt__PauseSubscriptionResponse *wsnt__PauseSubscriptionResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tev__ResumeSubscription(struct soap* soap, struct _wsnt__ResumeSubscription *wsnt__ResumeSubscription, struct _wsnt__ResumeSubscriptionResponse *wsnt__ResumeSubscriptionResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

#ifdef ONVIF__timg__
SOAP_FMAC5 int SOAP_FMAC6 __timg__GetServiceCapabilities(struct soap* soap, struct _timg__GetServiceCapabilities *timg__GetServiceCapabilities, struct _timg__GetServiceCapabilitiesResponse *timg__GetServiceCapabilitiesResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);

timg__GetServiceCapabilitiesResponse->Capabilities=
	(struct timg__Capabilities*)soap_malloc(soap,sizeof(struct timg__Capabilities));
timg__GetServiceCapabilitiesResponse->Capabilities->ImageStabilization
	=(enum xsd__boolean*)soap_malloc(soap,sizeof(enum xsd__boolean));
*timg__GetServiceCapabilitiesResponse->Capabilities->ImageStabilization=xsd__boolean__false_;


return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__GetImagingSettings(struct soap* soap, struct _timg__GetImagingSettings *timg__GetImagingSettings, struct _timg__GetImagingSettingsResponse *timg__GetImagingSettingsResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
	if(strcmp(timg__GetImagingSettings->VideoSourceToken,VIDEOSOURCETOLEN)!=0)
		return SOAP_ERR;

	CamAEParam aeparams={0};
	CamAWBParam awbparams={0};
	CamExprosureParam Exprosureparams={0};
	CamRGBGains rgbparams={0};
	CamImgEnhanceParams ImgEnhanceparams={0};

	/* get auto exposure params */
	icam_get_ae_params(g_icam_ctrl_handle, &aeparams);
	/* get auto white balance params */
	icam_get_awb_params(g_icam_ctrl_handle, &awbparams);
	icam_get_exposure_params(g_icam_ctrl_handle, &Exprosureparams);
	icam_get_rgb_gains(g_icam_ctrl_handle, &rgbparams);
	icam_get_img_enhance_params(g_icam_ctrl_handle, &ImgEnhanceparams);

	CamImgAdjCfg *onImgAdjCfg=&ImgEnhanceparams.dayCfg;

	timg__GetImagingSettingsResponse->ImagingSettings=
		(struct tt__ImagingSettings20*)soap_malloc(soap,sizeof(struct tt__ImagingSettings20));

	timg__GetImagingSettingsResponse->ImagingSettings->BacklightCompensation=NULL;

	timg__GetImagingSettingsResponse->ImagingSettings->Brightness=
		(float*)soap_malloc(soap,sizeof(float));
	timg__GetImagingSettingsResponse->ImagingSettings->ColorSaturation=
		(float*)soap_malloc(soap,sizeof(float));
	timg__GetImagingSettingsResponse->ImagingSettings->Contrast=
		(float*)soap_malloc(soap,sizeof(float));
	timg__GetImagingSettingsResponse->ImagingSettings->Sharpness=
		(float*)soap_malloc(soap,sizeof(float));

	*timg__GetImagingSettingsResponse->ImagingSettings->Brightness=onImgAdjCfg->brightness;
	*timg__GetImagingSettingsResponse->ImagingSettings->ColorSaturation=onImgAdjCfg->saturation;
	*timg__GetImagingSettingsResponse->ImagingSettings->Contrast=onImgAdjCfg->contrast;
	*timg__GetImagingSettingsResponse->ImagingSettings->Sharpness=onImgAdjCfg->sharpness;

	timg__GetImagingSettingsResponse->ImagingSettings->Exposure=
		(struct tt__Exposure20*)soap_malloc(soap,sizeof(struct tt__Exposure20));
	if(aeparams.flags&CAM_AE_FLAG_AE_EN)
		timg__GetImagingSettingsResponse->ImagingSettings->Exposure->Mode=tt__ExposureMode__AUTO;
	else
		timg__GetImagingSettingsResponse->ImagingSettings->Exposure->Mode=tt__ExposureMode__MANUAL;
	timg__GetImagingSettingsResponse->ImagingSettings->Exposure->Priority=
		(enum tt__ExposurePriority*)soap_malloc(soap,sizeof(enum tt__ExposurePriority));
	*timg__GetImagingSettingsResponse->ImagingSettings->Exposure->Priority=tt__ExposurePriority__FrameRate;
	//tt__ExposurePriority__LowNoise = 0, tt__ExposurePriority__FrameRate 

	timg__GetImagingSettingsResponse->ImagingSettings->Exposure->Window=
		(struct tt__Rectangle*)soap_malloc(soap,sizeof(struct tt__Rectangle));
	timg__GetImagingSettingsResponse->ImagingSettings->Exposure->Window->top=(float*)soap_malloc(soap,sizeof(float));
	timg__GetImagingSettingsResponse->ImagingSettings->Exposure->Window->bottom=(float*)soap_malloc(soap,sizeof(float));
	timg__GetImagingSettingsResponse->ImagingSettings->Exposure->Window->right=(float*)soap_malloc(soap,sizeof(float));
	timg__GetImagingSettingsResponse->ImagingSettings->Exposure->Window->left=(float*)soap_malloc(soap,sizeof(float));
	*timg__GetImagingSettingsResponse->ImagingSettings->Exposure->Window->top=aeparams.roi[0].startY;
	*timg__GetImagingSettingsResponse->ImagingSettings->Exposure->Window->bottom=aeparams.roi[0].endY;
	*timg__GetImagingSettingsResponse->ImagingSettings->Exposure->Window->right=aeparams.roi[0].endX;
	*timg__GetImagingSettingsResponse->ImagingSettings->Exposure->Window->left=aeparams.roi[0].startX;
	timg__GetImagingSettingsResponse->ImagingSettings->Exposure->MinExposureTime=(float*)soap_malloc(soap,sizeof(float));
	timg__GetImagingSettingsResponse->ImagingSettings->Exposure->MaxExposureTime=(float*)soap_malloc(soap,sizeof(float));
	timg__GetImagingSettingsResponse->ImagingSettings->Exposure->MinGain=(float*)soap_malloc(soap,sizeof(float));
	timg__GetImagingSettingsResponse->ImagingSettings->Exposure->MaxGain=(float*)soap_malloc(soap,sizeof(float));
	timg__GetImagingSettingsResponse->ImagingSettings->Exposure->MinIris=(float*)soap_malloc(soap,sizeof(float));
	timg__GetImagingSettingsResponse->ImagingSettings->Exposure->MaxIris=(float*)soap_malloc(soap,sizeof(float));
	timg__GetImagingSettingsResponse->ImagingSettings->Exposure->ExposureTime=(float*)soap_malloc(soap,sizeof(float));
	timg__GetImagingSettingsResponse->ImagingSettings->Exposure->Gain=(float*)soap_malloc(soap,sizeof(float));
	timg__GetImagingSettingsResponse->ImagingSettings->Exposure->Iris=(float*)soap_malloc(soap,sizeof(float));
	*timg__GetImagingSettingsResponse->ImagingSettings->Exposure->MinExposureTime=aeparams.minShutterTime;
	*timg__GetImagingSettingsResponse->ImagingSettings->Exposure->MaxExposureTime=aeparams.maxShutterTime;
	*timg__GetImagingSettingsResponse->ImagingSettings->Exposure->MinGain=aeparams.minGainValue;
	*timg__GetImagingSettingsResponse->ImagingSettings->Exposure->MaxGain=aeparams.maxGainValue;
	*timg__GetImagingSettingsResponse->ImagingSettings->Exposure->MinIris=aeparams.minAperture;
	*timg__GetImagingSettingsResponse->ImagingSettings->Exposure->MaxIris=aeparams.maxAperture;
	*timg__GetImagingSettingsResponse->ImagingSettings->Exposure->ExposureTime=Exprosureparams.shutterTime;
	*timg__GetImagingSettingsResponse->ImagingSettings->Exposure->Gain=Exprosureparams.globalGain;
	*timg__GetImagingSettingsResponse->ImagingSettings->Exposure->Iris=Exprosureparams.reserved;


	timg__GetImagingSettingsResponse->ImagingSettings->Focus=NULL;
	timg__GetImagingSettingsResponse->ImagingSettings->IrCutFilter=NULL;

	timg__GetImagingSettingsResponse->ImagingSettings->WideDynamicRange=
		(struct tt__WideDynamicRange20*)soap_malloc(soap,sizeof(struct tt__WideDynamicRange20));
	if(onImgAdjCfg->flags&CAM_IMG_DRC_EN)
	timg__GetImagingSettingsResponse->ImagingSettings->WideDynamicRange->Mode=tt__WideDynamicMode__ON;
	else
	timg__GetImagingSettingsResponse->ImagingSettings->WideDynamicRange->Mode=tt__WideDynamicMode__OFF;
	//tt__WideDynamicMode__OFF = 0, tt__WideDynamicMode__ON CAM_IMG_DRC_EN
	timg__GetImagingSettingsResponse->ImagingSettings->WideDynamicRange->Level=(float*)soap_malloc(soap,sizeof(float));
	*timg__GetImagingSettingsResponse->ImagingSettings->WideDynamicRange->Level=onImgAdjCfg->drcStrength;

	timg__GetImagingSettingsResponse->ImagingSettings->WhiteBalance=
		(struct tt__WhiteBalance20*)soap_malloc(soap,sizeof(struct tt__WhiteBalance20));
	if(awbparams.flags&AWB_FLAG_EN)
		timg__GetImagingSettingsResponse->ImagingSettings->WhiteBalance->Mode=tt__WhiteBalanceMode__AUTO;//flags
	else
		timg__GetImagingSettingsResponse->ImagingSettings->WhiteBalance->Mode=tt__WhiteBalanceMode__MANUAL;//flags
		//tt__WhiteBalanceMode__AUTO = 0, tt__WhiteBalanceMode__MANUAL
	timg__GetImagingSettingsResponse->ImagingSettings->WhiteBalance->CbGain=(float*)soap_malloc(soap,sizeof(float));
	timg__GetImagingSettingsResponse->ImagingSettings->WhiteBalance->CrGain=(float*)soap_malloc(soap,sizeof(float));
	*timg__GetImagingSettingsResponse->ImagingSettings->WhiteBalance->CbGain=rgbparams.blueGain;
	*timg__GetImagingSettingsResponse->ImagingSettings->WhiteBalance->CrGain=rgbparams.redGain;
	timg__GetImagingSettingsResponse->ImagingSettings->Extension=NULL;
#if 0
	float *Brightness;	/* optional element of type xsd:float */
	float *ColorSaturation; /* optional element of type xsd:float */
	float *Contrast;	/* optional element of type xsd:float */
	struct tt__Exposure20 *Exposure;	/* optional element of type tt:Exposure20 */
	float *Sharpness;	/* optional element of type xsd:float */
	struct tt__WideDynamicRange20 *WideDynamicRange;	/* optional element of type tt:WideDynamicRange20 */
	struct tt__WhiteBalance20 *WhiteBalance;	/* optional element of type tt:WhiteBalance20 */
#endif
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__SetImagingSettings(struct soap* soap, struct _timg__SetImagingSettings *timg__SetImagingSettings, struct _timg__SetImagingSettingsResponse *timg__SetImagingSettingsResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);

	CamAEParam aeparams={0};
	CamAWBParam awbparams={0};
	CamExprosureParam Exprosureparams={0};
	CamRGBGains rgbparams={0};
	CamImgEnhanceParams ImgEnhanceparams={0};
	CamImgAdjCfg *onImgAdjCfg=NULL;

	if(strcmp(timg__SetImagingSettings->VideoSourceToken,VIDEOSOURCETOLEN)!=0)
		return SOAP_ERR;

	if(icam_get_img_enhance_params(g_icam_ctrl_handle, &ImgEnhanceparams))
		return SOAP_ERR;
	onImgAdjCfg=&ImgEnhanceparams.dayCfg;

	if(timg__SetImagingSettings->ImagingSettings->Brightness)
		onImgAdjCfg->brightness=*timg__SetImagingSettings->ImagingSettings->Brightness;
	if(timg__SetImagingSettings->ImagingSettings->ColorSaturation)
		onImgAdjCfg->saturation=*timg__SetImagingSettings->ImagingSettings->ColorSaturation;
	if(timg__SetImagingSettings->ImagingSettings->Contrast)
		onImgAdjCfg->contrast=*timg__SetImagingSettings->ImagingSettings->Contrast;
	if(timg__SetImagingSettings->ImagingSettings->Sharpness)
		onImgAdjCfg->sharpness=*timg__SetImagingSettings->ImagingSettings->Sharpness;

if(timg__SetImagingSettings->ImagingSettings->Exposure)
{
	/* get auto exposure params */
	if(icam_get_ae_params(g_icam_ctrl_handle, &aeparams))
		return SOAP_ERR;

	if(timg__SetImagingSettings->ImagingSettings->Exposure->Mode==tt__ExposureMode__AUTO)
	{
	    aeparams.flags|=CAM_AE_FLAG_AE_EN|CAM_AE_FLAG_AE_EN|CAM_AE_FLAG_AE_EN;
		if(timg__SetImagingSettings->ImagingSettings->Exposure->MinExposureTime)
			aeparams.minShutterTime=*timg__SetImagingSettings->ImagingSettings->Exposure->MinExposureTime;
		if(timg__SetImagingSettings->ImagingSettings->Exposure->MaxExposureTime)
			aeparams.maxShutterTime=*timg__SetImagingSettings->ImagingSettings->Exposure->MaxExposureTime;
		if(timg__SetImagingSettings->ImagingSettings->Exposure->MinGain)
			aeparams.minGainValue=*timg__SetImagingSettings->ImagingSettings->Exposure->MinGain;
		if(timg__SetImagingSettings->ImagingSettings->Exposure->MaxGain)
			aeparams.maxGainValue=*timg__SetImagingSettings->ImagingSettings->Exposure->MaxGain;
		if(timg__SetImagingSettings->ImagingSettings->Exposure->MinIris)
			aeparams.minAperture=*timg__SetImagingSettings->ImagingSettings->Exposure->MinIris;
		if(timg__SetImagingSettings->ImagingSettings->Exposure->MaxIris)
			aeparams.maxAperture=*timg__SetImagingSettings->ImagingSettings->Exposure->MaxIris;
	}else
	{
	     if(icam_get_exposure_params(g_icam_ctrl_handle, &Exprosureparams))
		     return SOAP_ERR;
	     aeparams.flags&=~(CAM_AE_FLAG_AE_EN|CAM_AE_FLAG_AE_EN|CAM_AE_FLAG_AE_EN);
		 if(timg__SetImagingSettings->ImagingSettings->Exposure->ExposureTime)
			Exprosureparams.shutterTime=*timg__SetImagingSettings->ImagingSettings->Exposure->ExposureTime;
		 if(timg__SetImagingSettings->ImagingSettings->Exposure->Gain)
			Exprosureparams.globalGain=*timg__SetImagingSettings->ImagingSettings->Exposure->Gain;
		 if(timg__SetImagingSettings->ImagingSettings->Exposure->Iris)
			Exprosureparams.reserved=*timg__SetImagingSettings->ImagingSettings->Exposure->Iris;
		 if(icam_set_exposure_params(g_icam_ctrl_handle, &Exprosureparams))
		     return SOAP_ERR;
	}
	    if(timg__SetImagingSettings->ImagingSettings->Exposure->Window)
		{
			if(timg__SetImagingSettings->ImagingSettings->Exposure->Window->top
				&&timg__SetImagingSettings->ImagingSettings->Exposure->Window->bottom
				&&timg__SetImagingSettings->ImagingSettings->Exposure->Window->right
				&&timg__SetImagingSettings->ImagingSettings->Exposure->Window->left)
			{
			aeparams.roi[0].startY=*timg__SetImagingSettings->ImagingSettings->Exposure->Window->top;
			aeparams.roi[0].endY=*timg__SetImagingSettings->ImagingSettings->Exposure->Window->bottom;
			aeparams.roi[0].endX=*timg__SetImagingSettings->ImagingSettings->Exposure->Window->right;
			aeparams.roi[0].startX=*timg__SetImagingSettings->ImagingSettings->Exposure->Window->left;

			}
		}
	if(icam_set_ae_params(g_icam_ctrl_handle, &aeparams))
		return SOAP_ERR;
}

if(timg__SetImagingSettings->ImagingSettings->WideDynamicRange)
{
	if(timg__SetImagingSettings->ImagingSettings->WideDynamicRange->Mode==tt__WideDynamicMode__ON)
	{
		onImgAdjCfg->flags|=CAM_IMG_DRC_EN;
		if(timg__SetImagingSettings->ImagingSettings->WideDynamicRange->Level)
		{
		onImgAdjCfg->drcStrength=*timg__SetImagingSettings->ImagingSettings->WideDynamicRange->Level;
		}
	}
	else
		onImgAdjCfg->flags&=~CAM_IMG_DRC_EN;
}

if(timg__SetImagingSettings->ImagingSettings->WhiteBalance)
{
	/* get auto white balance params */
	if(icam_get_awb_params(g_icam_ctrl_handle, &awbparams))
		return SOAP_ERR;

	if(timg__SetImagingSettings->ImagingSettings->WhiteBalance->Mode==tt__WhiteBalanceMode__AUTO)
	{
		awbparams.flags|=AWB_FLAG_EN;
	}
	else
	{
	    onImgAdjCfg->flags&=~AWB_FLAG_EN;
		if(icam_get_rgb_gains(g_icam_ctrl_handle, &rgbparams))
			return SOAP_ERR;
		if(timg__SetImagingSettings->ImagingSettings->WhiteBalance->CbGain)
			rgbparams.blueGain=*timg__SetImagingSettings->ImagingSettings->WhiteBalance->CbGain;
		if(timg__SetImagingSettings->ImagingSettings->WhiteBalance->CrGain)
			rgbparams.redGain=*timg__SetImagingSettings->ImagingSettings->WhiteBalance->CrGain;
		if(icam_set_rgb_gains(g_icam_ctrl_handle, &rgbparams))
			return SOAP_ERR;
	}
	if(icam_set_awb_params(g_icam_ctrl_handle, &awbparams))
		return SOAP_ERR;
}
	if(icam_set_img_enhance_params(g_icam_ctrl_handle, &ImgEnhanceparams))
		return SOAP_ERR;
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__GetOptions(struct soap* soap, struct _timg__GetOptions *timg__GetOptions, struct _timg__GetOptionsResponse *timg__GetOptionsResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
if(strcmp(timg__GetOptions->VideoSourceToken,VIDEOSOURCETOLEN)!=0)
	return SOAP_ERR;
timg__GetOptionsResponse->ImagingOptions=
	(struct tt__ImagingOptions20*)soap_malloc(soap,sizeof(struct tt__ImagingOptions20));
timg__GetOptionsResponse->ImagingOptions->Brightness=
	(struct tt__FloatRange*)soap_malloc(soap,sizeof(struct tt__FloatRange));
timg__GetOptionsResponse->ImagingOptions->ColorSaturation=
	(struct tt__FloatRange*)soap_malloc(soap,sizeof(struct tt__FloatRange));
timg__GetOptionsResponse->ImagingOptions->Contrast=
	(struct tt__FloatRange*)soap_malloc(soap,sizeof(struct tt__FloatRange));
timg__GetOptionsResponse->ImagingOptions->Sharpness=
	(struct tt__FloatRange*)soap_malloc(soap,sizeof(struct tt__FloatRange));
timg__GetOptionsResponse->ImagingOptions->Brightness->Min=0;
timg__GetOptionsResponse->ImagingOptions->Brightness->Max=255;
timg__GetOptionsResponse->ImagingOptions->ColorSaturation->Min=0;
timg__GetOptionsResponse->ImagingOptions->ColorSaturation->Max=255;
timg__GetOptionsResponse->ImagingOptions->Contrast->Min=0;
timg__GetOptionsResponse->ImagingOptions->Contrast->Max=255;
timg__GetOptionsResponse->ImagingOptions->Sharpness->Min=0;
timg__GetOptionsResponse->ImagingOptions->Sharpness->Max=255;

timg__GetOptionsResponse->ImagingOptions->Exposure=
	(struct tt__ExposureOptions20*)soap_malloc(soap,sizeof(struct tt__ExposureOptions20));
timg__GetOptionsResponse->ImagingOptions->Exposure->__sizeMode=2;
timg__GetOptionsResponse->ImagingOptions->Exposure->Mode=
	(enum tt__ExposureMode*)soap_malloc(soap,timg__GetOptionsResponse->ImagingOptions->Exposure->__sizeMode*sizeof(enum tt__ExposureMode));

*timg__GetOptionsResponse->ImagingOptions->Exposure->Mode=tt__ExposureMode__AUTO;
*(timg__GetOptionsResponse->ImagingOptions->Exposure->Mode+1)=tt__ExposureMode__MANUAL;


timg__GetOptionsResponse->ImagingOptions->Exposure->__sizePriority=1;
timg__GetOptionsResponse->ImagingOptions->Exposure->Priority=
	(enum tt__ExposurePriority*)soap_malloc(soap,timg__GetOptionsResponse->ImagingOptions->Exposure->__sizePriority*sizeof(enum tt__ExposurePriority));
*timg__GetOptionsResponse->ImagingOptions->Exposure->Priority=tt__ExposurePriority__FrameRate;

timg__GetOptionsResponse->ImagingOptions->Exposure->MinExposureTime=
	(struct tt__FloatRange*)soap_malloc(soap,sizeof(struct tt__FloatRange));
timg__GetOptionsResponse->ImagingOptions->Exposure->MaxExposureTime=
	(struct tt__FloatRange*)soap_malloc(soap,sizeof(struct tt__FloatRange));
timg__GetOptionsResponse->ImagingOptions->Exposure->MinGain=
	(struct tt__FloatRange*)soap_malloc(soap,sizeof(struct tt__FloatRange));
timg__GetOptionsResponse->ImagingOptions->Exposure->MaxGain=
	(struct tt__FloatRange*)soap_malloc(soap,sizeof(struct tt__FloatRange));
timg__GetOptionsResponse->ImagingOptions->Exposure->MinIris=
	(struct tt__FloatRange*)soap_malloc(soap,sizeof(struct tt__FloatRange));
timg__GetOptionsResponse->ImagingOptions->Exposure->MaxIris=
	(struct tt__FloatRange*)soap_malloc(soap,sizeof(struct tt__FloatRange));
timg__GetOptionsResponse->ImagingOptions->Exposure->ExposureTime=
	(struct tt__FloatRange*)soap_malloc(soap,sizeof(struct tt__FloatRange));
timg__GetOptionsResponse->ImagingOptions->Exposure->Gain=
	(struct tt__FloatRange*)soap_malloc(soap,sizeof(struct tt__FloatRange));
timg__GetOptionsResponse->ImagingOptions->Exposure->Iris=
	(struct tt__FloatRange*)soap_malloc(soap,sizeof(struct tt__FloatRange));

timg__GetOptionsResponse->ImagingOptions->Exposure->MinExposureTime->Min=0;
timg__GetOptionsResponse->ImagingOptions->Exposure->MinExposureTime->Max=60000000;
timg__GetOptionsResponse->ImagingOptions->Exposure->MaxExposureTime->Min=0;
timg__GetOptionsResponse->ImagingOptions->Exposure->MaxExposureTime->Max=60000000;
timg__GetOptionsResponse->ImagingOptions->Exposure->MinGain->Min=1;
timg__GetOptionsResponse->ImagingOptions->Exposure->MinGain->Max=1023;
timg__GetOptionsResponse->ImagingOptions->Exposure->MaxGain->Min=1;
timg__GetOptionsResponse->ImagingOptions->Exposure->MaxGain->Max=1023;
timg__GetOptionsResponse->ImagingOptions->Exposure->MinIris->Min=0;
timg__GetOptionsResponse->ImagingOptions->Exposure->MinIris->Max=255;
timg__GetOptionsResponse->ImagingOptions->Exposure->MaxIris->Min=0;
timg__GetOptionsResponse->ImagingOptions->Exposure->MaxIris->Max=255;
timg__GetOptionsResponse->ImagingOptions->Exposure->ExposureTime->Min=0;
timg__GetOptionsResponse->ImagingOptions->Exposure->ExposureTime->Max=60000000;
timg__GetOptionsResponse->ImagingOptions->Exposure->Gain->Min=0;
timg__GetOptionsResponse->ImagingOptions->Exposure->Gain->Max=1023;
timg__GetOptionsResponse->ImagingOptions->Exposure->Iris->Min=0;
timg__GetOptionsResponse->ImagingOptions->Exposure->Iris->Max=255;



timg__GetOptionsResponse->ImagingOptions->WideDynamicRange=
	(struct tt__WideDynamicRangeOptions20*)soap_malloc(soap,sizeof(struct tt__WideDynamicRangeOptions20));
timg__GetOptionsResponse->ImagingOptions->WideDynamicRange->__sizeMode=2;
timg__GetOptionsResponse->ImagingOptions->WideDynamicRange->Mode=
	(enum tt__WideDynamicMode*)soap_malloc(soap,timg__GetOptionsResponse->ImagingOptions->WideDynamicRange->__sizeMode*sizeof(enum tt__WideDynamicMode));
*timg__GetOptionsResponse->ImagingOptions->WideDynamicRange->Mode=tt__WideDynamicMode__OFF;
*(timg__GetOptionsResponse->ImagingOptions->WideDynamicRange->Mode+1)=tt__WideDynamicMode__ON;
timg__GetOptionsResponse->ImagingOptions->WideDynamicRange->Level=
	(struct tt__FloatRange*)soap_malloc(soap,sizeof(struct tt__FloatRange));
timg__GetOptionsResponse->ImagingOptions->WideDynamicRange->Level->Min=0;
timg__GetOptionsResponse->ImagingOptions->WideDynamicRange->Level->Max=255;


timg__GetOptionsResponse->ImagingOptions->WhiteBalance=
	(struct tt__WhiteBalanceOptions20*)soap_malloc(soap,sizeof(struct tt__WhiteBalanceOptions20));
timg__GetOptionsResponse->ImagingOptions->WhiteBalance->__sizeMode=2;
timg__GetOptionsResponse->ImagingOptions->WhiteBalance->Mode=
	(enum tt__WhiteBalanceMode*)soap_malloc(soap,timg__GetOptionsResponse->ImagingOptions->WhiteBalance->__sizeMode*sizeof(enum tt__WhiteBalanceMode));
*timg__GetOptionsResponse->ImagingOptions->WhiteBalance->Mode=tt__WhiteBalanceMode__AUTO;
*(timg__GetOptionsResponse->ImagingOptions->WhiteBalance->Mode+1)=tt__WhiteBalanceMode__MANUAL;

timg__GetOptionsResponse->ImagingOptions->WhiteBalance->YrGain=
	(struct tt__FloatRange*)soap_malloc(soap,sizeof(struct tt__FloatRange));
timg__GetOptionsResponse->ImagingOptions->WhiteBalance->YrGain->Min=0;
timg__GetOptionsResponse->ImagingOptions->WhiteBalance->YrGain->Max=255;

timg__GetOptionsResponse->ImagingOptions->WhiteBalance->YbGain=
	(struct tt__FloatRange*)soap_malloc(soap,sizeof(struct tt__FloatRange));
timg__GetOptionsResponse->ImagingOptions->WhiteBalance->YbGain->Min=0;
timg__GetOptionsResponse->ImagingOptions->WhiteBalance->YbGain->Max=255;
#if 0
struct tt__FloatRange *Brightness;	/* optional element of type tt:FloatRange */
struct tt__FloatRange *ColorSaturation; /* optional element of type tt:FloatRange */
struct tt__FloatRange *Contrast;	/* optional element of type tt:FloatRange */
struct tt__ExposureOptions20 *Exposure; /* optional element of type tt:ExposureOptions20 */
struct tt__FloatRange *Sharpness;	/* optional element of type tt:FloatRange */
struct tt__WideDynamicRangeOptions20 *WideDynamicRange; /* optional element of type tt:WideDynamicRangeOptions20 */
struct tt__WhiteBalanceOptions20 *WhiteBalance; /* optional element of type tt:WhiteBalanceOptions20 */
#endif
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__Move(struct soap* soap, struct _timg__Move *timg__Move, struct _timg__MoveResponse *timg__MoveResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__Stop(struct soap* soap, struct _timg__Stop *timg__Stop, struct _timg__StopResponse *timg__StopResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__GetStatus(struct soap* soap, struct _timg__GetStatus *timg__GetStatus, struct _timg__GetStatusResponse *timg__GetStatusResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__GetMoveOptions(struct soap* soap, struct _timg__GetMoveOptions *timg__GetMoveOptions, struct _timg__GetMoveOptionsResponse *timg__GetMoveOptionsResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
return 0;
}
#endif


SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetServiceCapabilities(struct soap* soap, struct _tmd__GetServiceCapabilities *tmd__GetServiceCapabilities, struct _tmd__GetServiceCapabilitiesResponse *tmd__GetServiceCapabilitiesResponse)
{
	tmd__GetServiceCapabilitiesResponse->Capabilities=
		(struct tmd__Capabilities*)soap_malloc(soap,sizeof(struct tmd__Capabilities));
	tmd__GetServiceCapabilitiesResponse->Capabilities->VideoSources=1;
	tmd__GetServiceCapabilitiesResponse->Capabilities->VideoOutputs=0;
	tmd__GetServiceCapabilitiesResponse->Capabilities->AudioSources=0;
	tmd__GetServiceCapabilitiesResponse->Capabilities->AudioOutputs=0;
	tmd__GetServiceCapabilitiesResponse->Capabilities->RelayOutputs=RELAY_OUTPUTS_NUM;
	tmd__GetServiceCapabilitiesResponse->Capabilities->SerialPorts=0;
	tmd__GetServiceCapabilitiesResponse->Capabilities->DigitalInputs=INPUT_CONNECTORS_NUM;
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetRelayOutputOptions(struct soap* soap, struct _tmd__GetRelayOutputOptions *tmd__GetRelayOutputOptions, struct _tmd__GetRelayOutputOptionsResponse *tmd__GetRelayOutputOptionsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetAudioSources(struct soap* soap, struct _trt__GetAudioSources *trt__GetAudioSources, struct _trt__GetAudioSourcesResponse *trt__GetAudioSourcesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetAudioOutputs(struct soap* soap, struct _trt__GetAudioOutputs *trt__GetAudioOutputs, struct _trt__GetAudioOutputsResponse *trt__GetAudioOutputsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetVideoSources(struct soap* soap, struct _trt__GetVideoSources *trt__GetVideoSources, struct _trt__GetVideoSourcesResponse *trt__GetVideoSourcesResponse)
{
	fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
trt__GetVideoSourcesResponse->__sizeVideoSources=1;
trt__GetVideoSourcesResponse->VideoSources=
	(struct tt__VideoSource*)soap_malloc(soap,trt__GetVideoSourcesResponse->__sizeVideoSources*sizeof(struct tt__VideoSource));
trt__GetVideoSourcesResponse->VideoSources->token=VIDEOSOURCETOLEN;
trt__GetVideoSourcesResponse->VideoSources->Framerate=25;
trt__GetVideoSourcesResponse->VideoSources->Resolution=
	(struct tt__VideoResolution*)soap_malloc(soap,sizeof(struct tt__VideoResolution));
trt__GetVideoSourcesResponse->VideoSources->Resolution->Width=ONVIF_IMAGE_WIDTH;
trt__GetVideoSourcesResponse->VideoSources->Resolution->Height=ONVIF_IMAGE_HEIGHT;

return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetVideoOutputs(struct soap* soap, struct _tmd__GetVideoOutputs *tmd__GetVideoOutputs, struct _tmd__GetVideoOutputsResponse *tmd__GetVideoOutputsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetVideoSourceConfiguration(struct soap* soap, struct _tmd__GetVideoSourceConfiguration *tmd__GetVideoSourceConfiguration, struct _tmd__GetVideoSourceConfigurationResponse *tmd__GetVideoSourceConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetVideoOutputConfiguration(struct soap* soap, struct _tmd__GetVideoOutputConfiguration *tmd__GetVideoOutputConfiguration, struct _tmd__GetVideoOutputConfigurationResponse *tmd__GetVideoOutputConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetAudioSourceConfiguration(struct soap* soap, struct _tmd__GetAudioSourceConfiguration *tmd__GetAudioSourceConfiguration, struct _tmd__GetAudioSourceConfigurationResponse *tmd__GetAudioSourceConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetAudioOutputConfiguration(struct soap* soap, struct _tmd__GetAudioOutputConfiguration *tmd__GetAudioOutputConfiguration, struct _tmd__GetAudioOutputConfigurationResponse *tmd__GetAudioOutputConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__SetVideoSourceConfiguration(struct soap* soap, struct _tmd__SetVideoSourceConfiguration *tmd__SetVideoSourceConfiguration, struct _tmd__SetVideoSourceConfigurationResponse *tmd__SetVideoSourceConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__SetVideoOutputConfiguration(struct soap* soap, struct _tmd__SetVideoOutputConfiguration *tmd__SetVideoOutputConfiguration, struct _tmd__SetVideoOutputConfigurationResponse *tmd__SetVideoOutputConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__SetAudioSourceConfiguration(struct soap* soap, struct _tmd__SetAudioSourceConfiguration *tmd__SetAudioSourceConfiguration, struct _tmd__SetAudioSourceConfigurationResponse *tmd__SetAudioSourceConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__SetAudioOutputConfiguration(struct soap* soap, struct _tmd__SetAudioOutputConfiguration *tmd__SetAudioOutputConfiguration, struct _tmd__SetAudioOutputConfigurationResponse *tmd__SetAudioOutputConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetVideoSourceConfigurationOptions(struct soap* soap, struct _tmd__GetVideoSourceConfigurationOptions *tmd__GetVideoSourceConfigurationOptions, struct _tmd__GetVideoSourceConfigurationOptionsResponse *tmd__GetVideoSourceConfigurationOptionsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetVideoOutputConfigurationOptions(struct soap* soap, struct _tmd__GetVideoOutputConfigurationOptions *tmd__GetVideoOutputConfigurationOptions, struct _tmd__GetVideoOutputConfigurationOptionsResponse *tmd__GetVideoOutputConfigurationOptionsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetAudioSourceConfigurationOptions(struct soap* soap, struct _tmd__GetAudioSourceConfigurationOptions *tmd__GetAudioSourceConfigurationOptions, struct _tmd__GetAudioSourceConfigurationOptionsResponse *tmd__GetAudioSourceConfigurationOptionsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetAudioOutputConfigurationOptions(struct soap* soap, struct _tmd__GetAudioOutputConfigurationOptions *tmd__GetAudioOutputConfigurationOptions, struct _tmd__GetAudioOutputConfigurationOptionsResponse *tmd__GetAudioOutputConfigurationOptionsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetRelayOutputs(struct soap* soap, struct _tds__GetRelayOutputs *tds__GetRelayOutputs, struct _tds__GetRelayOutputsResponse *tds__GetRelayOutputsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__SetRelayOutputSettings(struct soap* soap, struct _tmd__SetRelayOutputSettings *tmd__SetRelayOutputSettings, struct _tmd__SetRelayOutputSettingsResponse *tmd__SetRelayOutputSettingsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__SetRelayOutputState(struct soap* soap, struct _tds__SetRelayOutputState *tds__SetRelayOutputState, struct _tds__SetRelayOutputStateResponse *tds__SetRelayOutputStateResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetDigitalInputs(struct soap* soap, struct _tmd__GetDigitalInputs *tmd__GetDigitalInputs, struct _tmd__GetDigitalInputsResponse *tmd__GetDigitalInputsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetSerialPorts(struct soap* soap, struct _tmd__GetSerialPorts *tmd__GetSerialPorts, struct _tmd__GetSerialPortsResponse *tmd__GetSerialPortsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetSerialPortConfiguration(struct soap* soap, struct _tmd__GetSerialPortConfiguration *tmd__GetSerialPortConfiguration, struct _tmd__GetSerialPortConfigurationResponse *tmd__GetSerialPortConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__SetSerialPortConfiguration(struct soap* soap, struct _tmd__SetSerialPortConfiguration *tmd__SetSerialPortConfiguration, struct _tmd__SetSerialPortConfigurationResponse *tmd__SetSerialPortConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetSerialPortConfigurationOptions(struct soap* soap, struct _tmd__GetSerialPortConfigurationOptions *tmd__GetSerialPortConfigurationOptions, struct _tmd__GetSerialPortConfigurationOptionsResponse *tmd__GetSerialPortConfigurationOptionsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__SendReceiveSerialCommand(struct soap* soap, struct _tmd__SendReceiveSerialCommand *tmd__SendReceiveSerialCommand, struct _tmd__SendReceiveSerialCommandResponse *tmd__SendReceiveSerialCommandResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

#ifdef ONVIF__tptz__
SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetServiceCapabilities(struct soap* soap, struct _tptz__GetServiceCapabilities *tptz__GetServiceCapabilities, struct _tptz__GetServiceCapabilitiesResponse *tptz__GetServiceCapabilitiesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetConfigurations(struct soap* soap, struct _tptz__GetConfigurations *tptz__GetConfigurations, struct _tptz__GetConfigurationsResponse *tptz__GetConfigurationsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetPresets(struct soap* soap, struct _tptz__GetPresets *tptz__GetPresets, struct _tptz__GetPresetsResponse *tptz__GetPresetsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__SetPreset(struct soap* soap, struct _tptz__SetPreset *tptz__SetPreset, struct _tptz__SetPresetResponse *tptz__SetPresetResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__RemovePreset(struct soap* soap, struct _tptz__RemovePreset *tptz__RemovePreset, struct _tptz__RemovePresetResponse *tptz__RemovePresetResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GotoPreset(struct soap* soap, struct _tptz__GotoPreset *tptz__GotoPreset, struct _tptz__GotoPresetResponse *tptz__GotoPresetResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetStatus(struct soap* soap, struct _tptz__GetStatus *tptz__GetStatus, struct _tptz__GetStatusResponse *tptz__GetStatusResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetConfiguration(struct soap* soap, struct _tptz__GetConfiguration *tptz__GetConfiguration, struct _tptz__GetConfigurationResponse *tptz__GetConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetNodes(struct soap* soap, struct _tptz__GetNodes *tptz__GetNodes, struct _tptz__GetNodesResponse *tptz__GetNodesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetNode(struct soap* soap, struct _tptz__GetNode *tptz__GetNode, struct _tptz__GetNodeResponse *tptz__GetNodeResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__SetConfiguration(struct soap* soap, struct _tptz__SetConfiguration *tptz__SetConfiguration, struct _tptz__SetConfigurationResponse *tptz__SetConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetConfigurationOptions(struct soap* soap, struct _tptz__GetConfigurationOptions *tptz__GetConfigurationOptions, struct _tptz__GetConfigurationOptionsResponse *tptz__GetConfigurationOptionsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GotoHomePosition(struct soap* soap, struct _tptz__GotoHomePosition *tptz__GotoHomePosition, struct _tptz__GotoHomePositionResponse *tptz__GotoHomePositionResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__SetHomePosition(struct soap* soap, struct _tptz__SetHomePosition *tptz__SetHomePosition, struct _tptz__SetHomePositionResponse *tptz__SetHomePositionResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__ContinuousMove(struct soap* soap, struct _tptz__ContinuousMove *tptz__ContinuousMove, struct _tptz__ContinuousMoveResponse *tptz__ContinuousMoveResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__RelativeMove(struct soap* soap, struct _tptz__RelativeMove *tptz__RelativeMove, struct _tptz__RelativeMoveResponse *tptz__RelativeMoveResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__SendAuxiliaryCommand(struct soap* soap, struct _tptz__SendAuxiliaryCommand *tptz__SendAuxiliaryCommand, struct _tptz__SendAuxiliaryCommandResponse *tptz__SendAuxiliaryCommandResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__AbsoluteMove(struct soap* soap, struct _tptz__AbsoluteMove *tptz__AbsoluteMove, struct _tptz__AbsoluteMoveResponse *tptz__AbsoluteMoveResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__Stop(struct soap* soap, struct _tptz__Stop *tptz__Stop, struct _tptz__StopResponse *tptz__StopResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetPresetTours(struct soap* soap, struct _tptz__GetPresetTours *tptz__GetPresetTours, struct _tptz__GetPresetToursResponse *tptz__GetPresetToursResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetPresetTour(struct soap* soap, struct _tptz__GetPresetTour *tptz__GetPresetTour, struct _tptz__GetPresetTourResponse *tptz__GetPresetTourResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetPresetTourOptions(struct soap* soap, struct _tptz__GetPresetTourOptions *tptz__GetPresetTourOptions, struct _tptz__GetPresetTourOptionsResponse *tptz__GetPresetTourOptionsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__CreatePresetTour(struct soap* soap, struct _tptz__CreatePresetTour *tptz__CreatePresetTour, struct _tptz__CreatePresetTourResponse *tptz__CreatePresetTourResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__ModifyPresetTour(struct soap* soap, struct _tptz__ModifyPresetTour *tptz__ModifyPresetTour, struct _tptz__ModifyPresetTourResponse *tptz__ModifyPresetTourResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__OperatePresetTour(struct soap* soap, struct _tptz__OperatePresetTour *tptz__OperatePresetTour, struct _tptz__OperatePresetTourResponse *tptz__OperatePresetTourResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__RemovePresetTour(struct soap* soap, struct _tptz__RemovePresetTour *tptz__RemovePresetTour, struct _tptz__RemovePresetTourResponse *tptz__RemovePresetTourResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}
#endif

#ifdef ONVIF__trc__
SOAP_FMAC5 int SOAP_FMAC6 __trc__GetServiceCapabilities(struct soap* soap, struct _trc__GetServiceCapabilities *trc__GetServiceCapabilities, struct _trc__GetServiceCapabilitiesResponse *trc__GetServiceCapabilitiesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trc__CreateRecording(struct soap* soap, struct _trc__CreateRecording *trc__CreateRecording, struct _trc__CreateRecordingResponse *trc__CreateRecordingResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trc__DeleteRecording(struct soap* soap, struct _trc__DeleteRecording *trc__DeleteRecording, struct _trc__DeleteRecordingResponse *trc__DeleteRecordingResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trc__GetRecordings(struct soap* soap, struct _trc__GetRecordings *trc__GetRecordings, struct _trc__GetRecordingsResponse *trc__GetRecordingsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trc__SetRecordingConfiguration(struct soap* soap, struct _trc__SetRecordingConfiguration *trc__SetRecordingConfiguration, struct _trc__SetRecordingConfigurationResponse *trc__SetRecordingConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trc__GetRecordingConfiguration(struct soap* soap, struct _trc__GetRecordingConfiguration *trc__GetRecordingConfiguration, struct _trc__GetRecordingConfigurationResponse *trc__GetRecordingConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trc__CreateTrack(struct soap* soap, struct _trc__CreateTrack *trc__CreateTrack, struct _trc__CreateTrackResponse *trc__CreateTrackResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trc__DeleteTrack(struct soap* soap, struct _trc__DeleteTrack *trc__DeleteTrack, struct _trc__DeleteTrackResponse *trc__DeleteTrackResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trc__GetTrackConfiguration(struct soap* soap, struct _trc__GetTrackConfiguration *trc__GetTrackConfiguration, struct _trc__GetTrackConfigurationResponse *trc__GetTrackConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trc__SetTrackConfiguration(struct soap* soap, struct _trc__SetTrackConfiguration *trc__SetTrackConfiguration, struct _trc__SetTrackConfigurationResponse *trc__SetTrackConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trc__CreateRecordingJob(struct soap* soap, struct _trc__CreateRecordingJob *trc__CreateRecordingJob, struct _trc__CreateRecordingJobResponse *trc__CreateRecordingJobResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trc__DeleteRecordingJob(struct soap* soap, struct _trc__DeleteRecordingJob *trc__DeleteRecordingJob, struct _trc__DeleteRecordingJobResponse *trc__DeleteRecordingJobResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trc__GetRecordingJobs(struct soap* soap, struct _trc__GetRecordingJobs *trc__GetRecordingJobs, struct _trc__GetRecordingJobsResponse *trc__GetRecordingJobsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trc__SetRecordingJobConfiguration(struct soap* soap, struct _trc__SetRecordingJobConfiguration *trc__SetRecordingJobConfiguration, struct _trc__SetRecordingJobConfigurationResponse *trc__SetRecordingJobConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trc__GetRecordingJobConfiguration(struct soap* soap, struct _trc__GetRecordingJobConfiguration *trc__GetRecordingJobConfiguration, struct _trc__GetRecordingJobConfigurationResponse *trc__GetRecordingJobConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trc__SetRecordingJobMode(struct soap* soap, struct _trc__SetRecordingJobMode *trc__SetRecordingJobMode, struct _trc__SetRecordingJobModeResponse *trc__SetRecordingJobModeResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trc__GetRecordingJobState(struct soap* soap, struct _trc__GetRecordingJobState *trc__GetRecordingJobState, struct _trc__GetRecordingJobStateResponse *trc__GetRecordingJobStateResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}
#endif

#ifdef ONVIF__trp__
SOAP_FMAC5 int SOAP_FMAC6 __trp__GetServiceCapabilities(struct soap* soap, struct _trp__GetServiceCapabilities *trp__GetServiceCapabilities, struct _trp__GetServiceCapabilitiesResponse *trp__GetServiceCapabilitiesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trp__GetReplayUri(struct soap* soap, struct _trp__GetReplayUri *trp__GetReplayUri, struct _trp__GetReplayUriResponse *trp__GetReplayUriResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trp__GetReplayConfiguration(struct soap* soap, struct _trp__GetReplayConfiguration *trp__GetReplayConfiguration, struct _trp__GetReplayConfigurationResponse *trp__GetReplayConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trp__SetReplayConfiguration(struct soap* soap, struct _trp__SetReplayConfiguration *trp__SetReplayConfiguration, struct _trp__SetReplayConfigurationResponse *trp__SetReplayConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

#endif
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetServiceCapabilities(struct soap* soap, struct _trt__GetServiceCapabilities *trt__GetServiceCapabilities, struct _trt__GetServiceCapabilitiesResponse *trt__GetServiceCapabilitiesResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
trt__GetServiceCapabilitiesResponse->Capabilities=
	(struct trt__Capabilities*)soap_malloc(soap,sizeof(struct trt__Capabilities));
trt__GetServiceCapabilitiesResponse->Capabilities->ProfileCapabilities=
	(struct trt__ProfileCapabilities*)soap_malloc(soap,sizeof(struct trt__ProfileCapabilities));
trt__GetServiceCapabilitiesResponse->Capabilities->ProfileCapabilities->MaximumNumberOfProfiles=
	(int*)soap_malloc(soap,sizeof(int));
*trt__GetServiceCapabilitiesResponse->Capabilities->ProfileCapabilities->MaximumNumberOfProfiles=1;

trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities=
	(struct trt__StreamingCapabilities*)soap_malloc(soap,sizeof(struct trt__StreamingCapabilities));
trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->RTPMulticast=
	(enum xsd__boolean*)soap_malloc(soap,sizeof(enum xsd__boolean));
trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->RTP_USCORETCP=
	(enum xsd__boolean*)soap_malloc(soap,sizeof(enum xsd__boolean));
trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->RTP_USCORERTSP_USCORETCP=
	(enum xsd__boolean*)soap_malloc(soap,sizeof(enum xsd__boolean));
trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->NonAggregateControl=
	(enum xsd__boolean*)soap_malloc(soap,sizeof(enum xsd__boolean));

*trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->RTPMulticast=xsd__boolean__true_;
*trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->RTP_USCORETCP=xsd__boolean__true_;
*trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->RTP_USCORERTSP_USCORETCP=xsd__boolean__true_;
*trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->NonAggregateControl=xsd__boolean__false_;

trt__GetServiceCapabilitiesResponse->Capabilities->SnapshotUri=
	(enum xsd__boolean*)soap_malloc(soap,sizeof(enum xsd__boolean));
trt__GetServiceCapabilitiesResponse->Capabilities->Rotation=
	(enum xsd__boolean*)soap_malloc(soap,sizeof(enum xsd__boolean));
*trt__GetServiceCapabilitiesResponse->Capabilities->SnapshotUri=xsd__boolean__false_;
*trt__GetServiceCapabilitiesResponse->Capabilities->Rotation=xsd__boolean__false_;

return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSources(struct soap* soap, struct _trt__GetVideoSources *trt__GetVideoSources, struct _trt__GetVideoSourcesResponse *trt__GetVideoSourcesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSources(struct soap* soap, struct _trt__GetAudioSources *trt__GetAudioSources, struct _trt__GetAudioSourcesResponse *trt__GetAudioSourcesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputs(struct soap* soap, struct _trt__GetAudioOutputs *trt__GetAudioOutputs, struct _trt__GetAudioOutputsResponse *trt__GetAudioOutputsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__CreateProfile(struct soap* soap, struct _trt__CreateProfile *trt__CreateProfile, struct _trt__CreateProfileResponse *trt__CreateProfileResponse)
{
	fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);	
	return soap_sender_fault_subcode(soap, "ter:Action/ter:MaxNVTProfiles", NULL, NULL);
	return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetProfile(struct soap* soap, struct _trt__GetProfile *trt__GetProfile, struct _trt__GetProfileResponse *trt__GetProfileResponse)
{
	fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
	if(strcmp(trt__GetProfile->ProfileToken,IPCPROFILE_1)!=0)
		return SOAP_ERR;

	if(!trt__GetProfileResponse->Profile)
		trt__GetProfileResponse->Profile=
		(struct tt__Profile*)soap_malloc(soap,sizeof(struct tt__Profile));
	trt__GetProfileResponse->Profile->Name=IPCPROFILE_1;
	trt__GetProfileResponse->Profile->token=IPCPROFILE_1;


	struct _trt__GetVideoSourceConfiguration trt__GetVideoSourceConfiguration;
	struct _trt__GetVideoSourceConfigurationResponse trt__GetVideoSourceConfigurationResponse;
	trt__GetVideoSourceConfiguration.ConfigurationToken=VIDEOSOURCECONFIG;
	trt__GetProfileResponse->Profile->VideoSourceConfiguration=
		(struct tt__VideoSourceConfiguration*)soap_malloc(soap,sizeof(struct tt__VideoSourceConfiguration));
	trt__GetVideoSourceConfigurationResponse.Configuration=trt__GetProfileResponse->Profile->VideoSourceConfiguration;
	__trt__GetVideoSourceConfiguration(soap,&trt__GetVideoSourceConfiguration,&trt__GetVideoSourceConfigurationResponse);



	trt__GetProfileResponse->Profile->VideoEncoderConfiguration=
		(struct tt__VideoEncoderConfiguration*)soap_malloc(soap,sizeof(struct tt__VideoEncoderConfiguration));
	struct _trt__GetVideoEncoderConfiguration trt__GetVideoEncoderConfiguration;
	trt__GetVideoEncoderConfiguration.ConfigurationToken=VIDEOENCODER1;
	struct _trt__GetVideoEncoderConfigurationResponse trt__GetVideoEncoderConfigurationResponse;
	trt__GetVideoEncoderConfigurationResponse.Configuration=trt__GetProfileResponse->Profile->VideoEncoderConfiguration;
	__trt__GetVideoEncoderConfiguration(soap, &trt__GetVideoEncoderConfiguration, &trt__GetVideoEncoderConfigurationResponse);

	trt__GetProfileResponse->Profile->fixed=(enum xsd__boolean*)soap_malloc(soap,sizeof(enum xsd__boolean));
	*trt__GetProfileResponse->Profile->fixed=xsd__boolean__true_;
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetProfiles(struct soap* soap, struct _trt__GetProfiles *trt__GetProfiles, struct _trt__GetProfilesResponse *trt__GetProfilesResponse)
{
	fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
	trt__GetProfilesResponse->__sizeProfiles=1;
	trt__GetProfilesResponse->Profiles=(struct tt__Profile*)soap_malloc(soap,trt__GetProfilesResponse->__sizeProfiles*sizeof(struct tt__Profile));
	struct _trt__GetProfile trt__GetProfile;
	struct _trt__GetProfileResponse trt__GetProfileResponse;
	trt__GetProfile.ProfileToken=IPCPROFILE_1;
	trt__GetProfileResponse.Profile=trt__GetProfilesResponse->Profiles;
	__trt__GetProfile(soap,&trt__GetProfile, &trt__GetProfileResponse);

return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddVideoEncoderConfiguration(struct soap* soap, struct _trt__AddVideoEncoderConfiguration *trt__AddVideoEncoderConfiguration, struct _trt__AddVideoEncoderConfigurationResponse *trt__AddVideoEncoderConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddVideoSourceConfiguration(struct soap* soap, struct _trt__AddVideoSourceConfiguration *trt__AddVideoSourceConfiguration, struct _trt__AddVideoSourceConfigurationResponse *trt__AddVideoSourceConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioEncoderConfiguration(struct soap* soap, struct _trt__AddAudioEncoderConfiguration *trt__AddAudioEncoderConfiguration, struct _trt__AddAudioEncoderConfigurationResponse *trt__AddAudioEncoderConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioSourceConfiguration(struct soap* soap, struct _trt__AddAudioSourceConfiguration *trt__AddAudioSourceConfiguration, struct _trt__AddAudioSourceConfigurationResponse *trt__AddAudioSourceConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddPTZConfiguration(struct soap* soap, struct _trt__AddPTZConfiguration *trt__AddPTZConfiguration, struct _trt__AddPTZConfigurationResponse *trt__AddPTZConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddVideoAnalyticsConfiguration(struct soap* soap, struct _trt__AddVideoAnalyticsConfiguration *trt__AddVideoAnalyticsConfiguration, struct _trt__AddVideoAnalyticsConfigurationResponse *trt__AddVideoAnalyticsConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddMetadataConfiguration(struct soap* soap, struct _trt__AddMetadataConfiguration *trt__AddMetadataConfiguration, struct _trt__AddMetadataConfigurationResponse *trt__AddMetadataConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioOutputConfiguration(struct soap* soap, struct _trt__AddAudioOutputConfiguration *trt__AddAudioOutputConfiguration, struct _trt__AddAudioOutputConfigurationResponse *trt__AddAudioOutputConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioDecoderConfiguration(struct soap* soap, struct _trt__AddAudioDecoderConfiguration *trt__AddAudioDecoderConfiguration, struct _trt__AddAudioDecoderConfigurationResponse *trt__AddAudioDecoderConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveVideoEncoderConfiguration(struct soap* soap, struct _trt__RemoveVideoEncoderConfiguration *trt__RemoveVideoEncoderConfiguration, struct _trt__RemoveVideoEncoderConfigurationResponse *trt__RemoveVideoEncoderConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveVideoSourceConfiguration(struct soap* soap, struct _trt__RemoveVideoSourceConfiguration *trt__RemoveVideoSourceConfiguration, struct _trt__RemoveVideoSourceConfigurationResponse *trt__RemoveVideoSourceConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioEncoderConfiguration(struct soap* soap, struct _trt__RemoveAudioEncoderConfiguration *trt__RemoveAudioEncoderConfiguration, struct _trt__RemoveAudioEncoderConfigurationResponse *trt__RemoveAudioEncoderConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioSourceConfiguration(struct soap* soap, struct _trt__RemoveAudioSourceConfiguration *trt__RemoveAudioSourceConfiguration, struct _trt__RemoveAudioSourceConfigurationResponse *trt__RemoveAudioSourceConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemovePTZConfiguration(struct soap* soap, struct _trt__RemovePTZConfiguration *trt__RemovePTZConfiguration, struct _trt__RemovePTZConfigurationResponse *trt__RemovePTZConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveVideoAnalyticsConfiguration(struct soap* soap, struct _trt__RemoveVideoAnalyticsConfiguration *trt__RemoveVideoAnalyticsConfiguration, struct _trt__RemoveVideoAnalyticsConfigurationResponse *trt__RemoveVideoAnalyticsConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveMetadataConfiguration(struct soap* soap, struct _trt__RemoveMetadataConfiguration *trt__RemoveMetadataConfiguration, struct _trt__RemoveMetadataConfigurationResponse *trt__RemoveMetadataConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioOutputConfiguration(struct soap* soap, struct _trt__RemoveAudioOutputConfiguration *trt__RemoveAudioOutputConfiguration, struct _trt__RemoveAudioOutputConfigurationResponse *trt__RemoveAudioOutputConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioDecoderConfiguration(struct soap* soap, struct _trt__RemoveAudioDecoderConfiguration *trt__RemoveAudioDecoderConfiguration, struct _trt__RemoveAudioDecoderConfigurationResponse *trt__RemoveAudioDecoderConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__DeleteProfile(struct soap* soap, struct _trt__DeleteProfile *trt__DeleteProfile, struct _trt__DeleteProfileResponse *trt__DeleteProfileResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceConfigurations(struct soap* soap, struct _trt__GetVideoSourceConfigurations *trt__GetVideoSourceConfigurations, struct _trt__GetVideoSourceConfigurationsResponse *trt__GetVideoSourceConfigurationsResponse)
{
	fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
	struct _trt__GetVideoSourceConfiguration trt__GetVideoSourceConfiguration;
	struct _trt__GetVideoSourceConfigurationResponse trt__GetVideoSourceConfigurationResponse;
	int ret;
	trt__GetVideoSourceConfigurationsResponse->__sizeConfigurations=1;
	trt__GetVideoSourceConfigurationsResponse->Configurations=
		(struct tt__VideoSourceConfiguration*)soap_malloc(soap,trt__GetVideoSourceConfigurationsResponse->__sizeConfigurations*sizeof(struct tt__VideoSourceConfiguration));
	trt__GetVideoSourceConfiguration.ConfigurationToken=VIDEOSOURCECONFIG;
	trt__GetVideoSourceConfigurationResponse.Configuration=trt__GetVideoSourceConfigurationsResponse->Configurations;
	ret=__trt__GetVideoSourceConfiguration(soap,&trt__GetVideoSourceConfiguration,&trt__GetVideoSourceConfigurationResponse);

return ret;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoEncoderConfigurations(struct soap* soap, struct _trt__GetVideoEncoderConfigurations *trt__GetVideoEncoderConfigurations, struct _trt__GetVideoEncoderConfigurationsResponse *trt__GetVideoEncoderConfigurationsResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
trt__GetVideoEncoderConfigurationsResponse->__sizeConfigurations=1;
trt__GetVideoEncoderConfigurationsResponse->Configurations=
	(struct tt__VideoEncoderConfiguration*)soap_malloc(soap,trt__GetVideoEncoderConfigurationsResponse->__sizeConfigurations*sizeof(struct tt__VideoEncoderConfiguration));
struct _trt__GetVideoEncoderConfiguration trt__GetVideoEncoderConfiguration;
trt__GetVideoEncoderConfiguration.ConfigurationToken=VIDEOENCODER1;
struct _trt__GetVideoEncoderConfigurationResponse trt__GetVideoEncoderConfigurationResponse;
trt__GetVideoEncoderConfigurationResponse.Configuration=trt__GetVideoEncoderConfigurationsResponse->Configurations;
return __trt__GetVideoEncoderConfiguration(soap, &trt__GetVideoEncoderConfiguration, &trt__GetVideoEncoderConfigurationResponse);

return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSourceConfigurations(struct soap* soap, struct _trt__GetAudioSourceConfigurations *trt__GetAudioSourceConfigurations, struct _trt__GetAudioSourceConfigurationsResponse *trt__GetAudioSourceConfigurationsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioEncoderConfigurations(struct soap* soap, struct _trt__GetAudioEncoderConfigurations *trt__GetAudioEncoderConfigurations, struct _trt__GetAudioEncoderConfigurationsResponse *trt__GetAudioEncoderConfigurationsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoAnalyticsConfigurations(struct soap* soap, struct _trt__GetVideoAnalyticsConfigurations *trt__GetVideoAnalyticsConfigurations, struct _trt__GetVideoAnalyticsConfigurationsResponse *trt__GetVideoAnalyticsConfigurationsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetMetadataConfigurations(struct soap* soap, struct _trt__GetMetadataConfigurations *trt__GetMetadataConfigurations, struct _trt__GetMetadataConfigurationsResponse *trt__GetMetadataConfigurationsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputConfigurations(struct soap* soap, struct _trt__GetAudioOutputConfigurations *trt__GetAudioOutputConfigurations, struct _trt__GetAudioOutputConfigurationsResponse *trt__GetAudioOutputConfigurationsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioDecoderConfigurations(struct soap* soap, struct _trt__GetAudioDecoderConfigurations *trt__GetAudioDecoderConfigurations, struct _trt__GetAudioDecoderConfigurationsResponse *trt__GetAudioDecoderConfigurationsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceConfiguration(struct soap* soap, struct _trt__GetVideoSourceConfiguration *trt__GetVideoSourceConfiguration, struct _trt__GetVideoSourceConfigurationResponse *trt__GetVideoSourceConfigurationResponse)
{
	fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
	if(strcmp(trt__GetVideoSourceConfiguration->ConfigurationToken,VIDEOSOURCECONFIG)!=0)
	{
		return SOAP_ERR;
	}
	if(!trt__GetVideoSourceConfigurationResponse->Configuration)
		trt__GetVideoSourceConfigurationResponse->Configuration=
			(struct tt__VideoSourceConfiguration*)soap_malloc(soap,sizeof(struct tt__VideoSourceConfiguration));
	trt__GetVideoSourceConfigurationResponse->Configuration->Name=VIDEOSOURCECONFIG;
	trt__GetVideoSourceConfigurationResponse->Configuration->token=VIDEOSOURCECONFIG;
	trt__GetVideoSourceConfigurationResponse->Configuration->UseCount=1;
	trt__GetVideoSourceConfigurationResponse->Configuration->SourceToken=VIDEOSOURCETOLEN;
	trt__GetVideoSourceConfigurationResponse->Configuration->Bounds=
		(struct tt__IntRectangle*)soap_malloc(soap,sizeof(	struct tt__IntRectangle));
	trt__GetVideoSourceConfigurationResponse->Configuration->Bounds->x=0;
	trt__GetVideoSourceConfigurationResponse->Configuration->Bounds->y=0;
	trt__GetVideoSourceConfigurationResponse->Configuration->Bounds->width=ONVIF_IMAGE_WIDTH;
	trt__GetVideoSourceConfigurationResponse->Configuration->Bounds->height=ONVIF_IMAGE_HEIGHT;
	
	return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoEncoderConfiguration(struct soap* soap, struct _trt__GetVideoEncoderConfiguration *trt__GetVideoEncoderConfiguration, struct _trt__GetVideoEncoderConfigurationResponse *trt__GetVideoEncoderConfigurationResponse)
{
    fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);

	int encw,ench;
	CamH264Params *encparams=
		(CamH264Params*)soap_malloc(soap,sizeof(CamH264Params));

	if(icam_get_h264_params(g_icam_ctrl_handle,encparams)!=E_NO)
		return SOAP_ERR;

	switch(encparams->resolution)
	{
	case H264_RES_1920X1080:
		encw=1920;
		ench=1080;
		break;
	case H264_RES_1280X720:
		encw=1280;
		ench=720;
		break;
	case H264_RES_1600X1200:
		encw=1600;
		ench=1200;
		break;
	case H264_RES_720X480:
		encw=720;
		ench=480;
		break;
		default:
			return SOAP_ERR;
		
	}

	if(strcmp(trt__GetVideoEncoderConfiguration->ConfigurationToken,VIDEOENCODER1)!=0)
		return SOAP_ERR;
	if(!trt__GetVideoEncoderConfigurationResponse->Configuration)
	trt__GetVideoEncoderConfigurationResponse->Configuration=
		(struct tt__VideoEncoderConfiguration*)soap_malloc(soap,sizeof(struct tt__VideoEncoderConfiguration));
	trt__GetVideoEncoderConfigurationResponse->Configuration->Name=VIDEOENCODER1;
	trt__GetVideoEncoderConfigurationResponse->Configuration->UseCount=1;
	trt__GetVideoEncoderConfigurationResponse->Configuration->token=VIDEOENCODER1;
	trt__GetVideoEncoderConfigurationResponse->Configuration->Encoding=tt__VideoEncoding__H264;
	trt__GetVideoEncoderConfigurationResponse->Configuration->Resolution=
	  ( struct tt__VideoResolution*)soap_malloc(soap,sizeof(struct tt__VideoResolution));
	trt__GetVideoEncoderConfigurationResponse->Configuration->Resolution->Width=encw;
	trt__GetVideoEncoderConfigurationResponse->Configuration->Resolution->Height=ench;
	trt__GetVideoEncoderConfigurationResponse->Configuration->Quality=1;
	trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl=
	  ( struct tt__VideoRateControl*)soap_malloc(soap,sizeof(struct tt__VideoRateControl));
	trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->FrameRateLimit=encparams->frameRate;
	trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->EncodingInterval=1;
	trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->BitrateLimit= encparams->bitRate;

	if(trt__GetVideoEncoderConfigurationResponse->Configuration->Encoding==tt__VideoEncoding__H264)
	{
	trt__GetVideoEncoderConfigurationResponse->Configuration->H264=
		( struct tt__H264Configuration*)soap_malloc(soap,sizeof(struct tt__H264Configuration));
	trt__GetVideoEncoderConfigurationResponse->Configuration->H264->GovLength=encparams->IPRatio;
	trt__GetVideoEncoderConfigurationResponse->Configuration->H264->H264Profile=tt__H264Profile__High;
	}
	trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast=
		(struct tt__MulticastConfiguration*)soap_malloc(soap,sizeof(struct tt__MulticastConfiguration));
	trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address=
		(struct tt__IPAddress*)soap_malloc(soap,sizeof(struct tt__IPAddress));
	trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address->Type=tt__IPType__IPv4;
	trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address->IPv4Address="255.255.0.0";

return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSourceConfiguration(struct soap* soap, struct _trt__GetAudioSourceConfiguration *trt__GetAudioSourceConfiguration, struct _trt__GetAudioSourceConfigurationResponse *trt__GetAudioSourceConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioEncoderConfiguration(struct soap* soap, struct _trt__GetAudioEncoderConfiguration *trt__GetAudioEncoderConfiguration, struct _trt__GetAudioEncoderConfigurationResponse *trt__GetAudioEncoderConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoAnalyticsConfiguration(struct soap* soap, struct _trt__GetVideoAnalyticsConfiguration *trt__GetVideoAnalyticsConfiguration, struct _trt__GetVideoAnalyticsConfigurationResponse *trt__GetVideoAnalyticsConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetMetadataConfiguration(struct soap* soap, struct _trt__GetMetadataConfiguration *trt__GetMetadataConfiguration, struct _trt__GetMetadataConfigurationResponse *trt__GetMetadataConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputConfiguration(struct soap* soap, struct _trt__GetAudioOutputConfiguration *trt__GetAudioOutputConfiguration, struct _trt__GetAudioOutputConfigurationResponse *trt__GetAudioOutputConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioDecoderConfiguration(struct soap* soap, struct _trt__GetAudioDecoderConfiguration *trt__GetAudioDecoderConfiguration, struct _trt__GetAudioDecoderConfigurationResponse *trt__GetAudioDecoderConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleVideoEncoderConfigurations(struct soap* soap, struct _trt__GetCompatibleVideoEncoderConfigurations *trt__GetCompatibleVideoEncoderConfigurations, struct _trt__GetCompatibleVideoEncoderConfigurationsResponse *trt__GetCompatibleVideoEncoderConfigurationsResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
	if(strcmp(trt__GetCompatibleVideoEncoderConfigurations->ProfileToken,IPCPROFILE_1)!=0)
		return SOAP_ERR;
	trt__GetCompatibleVideoEncoderConfigurationsResponse->__sizeConfigurations=1;
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations=
		(struct tt__VideoEncoderConfiguration*)soap_malloc(soap,trt__GetCompatibleVideoEncoderConfigurationsResponse->__sizeConfigurations*sizeof(struct tt__VideoEncoderConfiguration));
	struct _trt__GetVideoEncoderConfiguration trt__GetVideoEncoderConfiguration;
	trt__GetVideoEncoderConfiguration.ConfigurationToken=VIDEOENCODER1;
	struct _trt__GetVideoEncoderConfigurationResponse trt__GetVideoEncoderConfigurationResponse;
	trt__GetVideoEncoderConfigurationResponse.Configuration=trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations;
	return __trt__GetVideoEncoderConfiguration(soap, &trt__GetVideoEncoderConfiguration, &trt__GetVideoEncoderConfigurationResponse);
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleVideoSourceConfigurations(struct soap* soap, struct _trt__GetCompatibleVideoSourceConfigurations *trt__GetCompatibleVideoSourceConfigurations, struct _trt__GetCompatibleVideoSourceConfigurationsResponse *trt__GetCompatibleVideoSourceConfigurationsResponse)
{
	if(strcmp(trt__GetCompatibleVideoSourceConfigurations->ProfileToken,IPCPROFILE_1)!=0)
		return SOAP_ERR;
	struct _trt__GetVideoSourceConfiguration trt__GetVideoSourceConfiguration;
	struct _trt__GetVideoSourceConfigurationResponse trt__GetVideoSourceConfigurationResponse;
	int ret;
	trt__GetCompatibleVideoSourceConfigurationsResponse->__sizeConfigurations=1;
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations=
		(struct tt__VideoSourceConfiguration*)soap_malloc(soap,trt__GetCompatibleVideoSourceConfigurationsResponse->__sizeConfigurations*sizeof(struct tt__VideoSourceConfiguration));
	trt__GetVideoSourceConfiguration.ConfigurationToken=VIDEOSOURCECONFIG;
	trt__GetVideoSourceConfigurationResponse.Configuration=trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations;
	ret=__trt__GetVideoSourceConfiguration(soap,&trt__GetVideoSourceConfiguration,&trt__GetVideoSourceConfigurationResponse);

fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
return ret;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioEncoderConfigurations(struct soap* soap, struct _trt__GetCompatibleAudioEncoderConfigurations *trt__GetCompatibleAudioEncoderConfigurations, struct _trt__GetCompatibleAudioEncoderConfigurationsResponse *trt__GetCompatibleAudioEncoderConfigurationsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioSourceConfigurations(struct soap* soap, struct _trt__GetCompatibleAudioSourceConfigurations *trt__GetCompatibleAudioSourceConfigurations, struct _trt__GetCompatibleAudioSourceConfigurationsResponse *trt__GetCompatibleAudioSourceConfigurationsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleVideoAnalyticsConfigurations(struct soap* soap, struct _trt__GetCompatibleVideoAnalyticsConfigurations *trt__GetCompatibleVideoAnalyticsConfigurations, struct _trt__GetCompatibleVideoAnalyticsConfigurationsResponse *trt__GetCompatibleVideoAnalyticsConfigurationsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleMetadataConfigurations(struct soap* soap, struct _trt__GetCompatibleMetadataConfigurations *trt__GetCompatibleMetadataConfigurations, struct _trt__GetCompatibleMetadataConfigurationsResponse *trt__GetCompatibleMetadataConfigurationsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioOutputConfigurations(struct soap* soap, struct _trt__GetCompatibleAudioOutputConfigurations *trt__GetCompatibleAudioOutputConfigurations, struct _trt__GetCompatibleAudioOutputConfigurationsResponse *trt__GetCompatibleAudioOutputConfigurationsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioDecoderConfigurations(struct soap* soap, struct _trt__GetCompatibleAudioDecoderConfigurations *trt__GetCompatibleAudioDecoderConfigurations, struct _trt__GetCompatibleAudioDecoderConfigurationsResponse *trt__GetCompatibleAudioDecoderConfigurationsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}




SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoSourceConfiguration(struct soap* soap, struct _trt__SetVideoSourceConfiguration *trt__SetVideoSourceConfiguration, struct _trt__SetVideoSourceConfigurationResponse *trt__SetVideoSourceConfigurationResponse)
{
	if(strcmp(trt__SetVideoSourceConfiguration->Configuration->token,VIDEOSOURCECONFIG)!=0)
		return SOAP_ERR;
	if(strcmp(trt__SetVideoSourceConfiguration->Configuration->SourceToken,VIDEOSOURCETOLEN)!=0)
		return SOAP_ERR;

if(trt__SetVideoSourceConfiguration->Configuration->Bounds)
{
	if(trt__SetVideoSourceConfiguration->Configuration->Bounds->width>ONVIF_IMAGE_WIDTH
		||trt__SetVideoSourceConfiguration->Configuration->Bounds->height>ONVIF_IMAGE_HEIGHT)
	{
	return soap_sender_fault_subcode(soap, "ter:InvalidArgVal/ter:ConfigModify", NULL, NULL); 
	}
}
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoEncoderConfiguration(struct soap* soap, struct _trt__SetVideoEncoderConfiguration *trt__SetVideoEncoderConfiguration, struct _trt__SetVideoEncoderConfigurationResponse *trt__SetVideoEncoderConfigurationResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);

	if(strcmp(trt__SetVideoEncoderConfiguration->Configuration->token,VIDEOENCODER1)!=0)
		return SOAP_ERR;
	//trt__SetVideoEncoderConfiguration->ForcePersistence

	//trt__SetVideoEncoderConfiguration->Configuration
	//icam_set_h264_params(IN ICamCtrlHandle hCtrl, IN const CamH264Params *params)
	CamH264Params *params=(CamH264Params*)soap_malloc(soap,sizeof(CamH264Params));
	if(icam_get_h264_params(g_icam_ctrl_handle, params))
		return SOAP_ERR;
	if(trt__SetVideoEncoderConfiguration->Configuration->Encoding!=tt__VideoEncoding__H264)
		return SOAP_ERR;
	if(trt__SetVideoEncoderConfiguration->Configuration->Resolution)
	{
		switch(trt__SetVideoEncoderConfiguration->Configuration->Resolution->Width)
		{
		case 1920:
			params->resolution=H264_RES_1920X1080;
			break;
		case 1280:	
			params->resolution=H264_RES_1280X720;
			break;
		case 1600:
			params->resolution=H264_RES_1600X1200;
			break;
		case 720:
			params->resolution=H264_RES_720X480;
			break;
			default:
				return SOAP_ERR;
		}
	}

	if(trt__SetVideoEncoderConfiguration->Configuration->RateControl)
	{
		params->frameRate=trt__SetVideoEncoderConfiguration->Configuration->RateControl->FrameRateLimit;
		params->bitRate=trt__SetVideoEncoderConfiguration->Configuration->RateControl->BitrateLimit;
	}

	if(trt__SetVideoEncoderConfiguration->Configuration->H264)
	{
		params->IPRatio=trt__SetVideoEncoderConfiguration->Configuration->H264->GovLength;
	}

	if(icam_set_h264_params(g_icam_ctrl_handle, params))
	   return SOAP_ERR;
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioSourceConfiguration(struct soap* soap, struct _trt__SetAudioSourceConfiguration *trt__SetAudioSourceConfiguration, struct _trt__SetAudioSourceConfigurationResponse *trt__SetAudioSourceConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioEncoderConfiguration(struct soap* soap, struct _trt__SetAudioEncoderConfiguration *trt__SetAudioEncoderConfiguration, struct _trt__SetAudioEncoderConfigurationResponse *trt__SetAudioEncoderConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoAnalyticsConfiguration(struct soap* soap, struct _trt__SetVideoAnalyticsConfiguration *trt__SetVideoAnalyticsConfiguration, struct _trt__SetVideoAnalyticsConfigurationResponse *trt__SetVideoAnalyticsConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetMetadataConfiguration(struct soap* soap, struct _trt__SetMetadataConfiguration *trt__SetMetadataConfiguration, struct _trt__SetMetadataConfigurationResponse *trt__SetMetadataConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioOutputConfiguration(struct soap* soap, struct _trt__SetAudioOutputConfiguration *trt__SetAudioOutputConfiguration, struct _trt__SetAudioOutputConfigurationResponse *trt__SetAudioOutputConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioDecoderConfiguration(struct soap* soap, struct _trt__SetAudioDecoderConfiguration *trt__SetAudioDecoderConfiguration, struct _trt__SetAudioDecoderConfigurationResponse *trt__SetAudioDecoderConfigurationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceConfigurationOptions(struct soap* soap, struct _trt__GetVideoSourceConfigurationOptions *trt__GetVideoSourceConfigurationOptions, struct _trt__GetVideoSourceConfigurationOptionsResponse *trt__GetVideoSourceConfigurationOptionsResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
//if(strcmp(trt__GetVideoSourceConfigurationOptions->ProfileToken,IPCPROFILE_1)!=0)
 //  return SOAP_ERR;
//if(strcmp(trt__GetVideoSourceConfigurationOptions->ConfigurationToken,VIDEOSOURCECONFIG)!=0)
 //  return SOAP_ERR;

trt__GetVideoSourceConfigurationOptionsResponse->Options=
	(struct tt__VideoSourceConfigurationOptions*)soap_malloc(soap,sizeof(struct tt__VideoSourceConfigurationOptions));

trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange=
	(struct tt__IntRectangleRange*)soap_malloc(soap,sizeof(struct tt__IntRectangleRange));
trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->XRange=
	(struct tt__IntRange*)soap_malloc(soap,sizeof(struct tt__IntRange));
trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->YRange=
		(struct tt__IntRange*)soap_malloc(soap,sizeof(struct tt__IntRange));
trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->WidthRange=
	(struct tt__IntRange*)soap_malloc(soap,sizeof(struct tt__IntRange));
trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->HeightRange=
	(struct tt__IntRange*)soap_malloc(soap,sizeof(struct tt__IntRange));
	
	trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->XRange->Min=0;
	trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->XRange->Max=0;
	trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->YRange->Min=0;
	trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->YRange->Max=0;
	trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->WidthRange->Min=ONVIF_IMAGE_WIDTH;
	trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->WidthRange->Max=ONVIF_IMAGE_WIDTH;
	trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->HeightRange->Min=ONVIF_IMAGE_HEIGHT;
	trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->HeightRange->Max=ONVIF_IMAGE_HEIGHT;
	trt__GetVideoSourceConfigurationOptionsResponse->Options->__sizeVideoSourceTokensAvailable=1;
	trt__GetVideoSourceConfigurationOptionsResponse->Options->VideoSourceTokensAvailable=
		(char**)soap_malloc(soap,trt__GetVideoSourceConfigurationOptionsResponse->Options->__sizeVideoSourceTokensAvailable*sizeof(char *));
	*trt__GetVideoSourceConfigurationOptionsResponse->Options->VideoSourceTokensAvailable=VIDEOSOURCETOLEN;
return 0;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoEncoderConfigurationOptions(struct soap* soap, struct _trt__GetVideoEncoderConfigurationOptions *trt__GetVideoEncoderConfigurationOptions, struct _trt__GetVideoEncoderConfigurationOptionsResponse *trt__GetVideoEncoderConfigurationOptionsResponse)
{

	struct tt__VideoResolution *ResolutionsAvailable;
	//trt__GetVideoEncoderConfigurationOptions->ProfileToken
	//trt__GetVideoEncoderConfigurationOptions->ConfigurationToken
	trt__GetVideoEncoderConfigurationOptionsResponse->Options=
		(struct tt__VideoEncoderConfigurationOptions*)soap_malloc(soap,sizeof(struct tt__VideoEncoderConfigurationOptions));

	trt__GetVideoEncoderConfigurationOptionsResponse->Options->QualityRange=
			(struct tt__IntRange*)soap_malloc(soap,sizeof(struct tt__IntRange));
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->QualityRange->Min=0;
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->QualityRange->Max=5;

	trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264=
			(struct tt__H264Options*)soap_malloc(soap,sizeof(struct tt__H264Options));

	trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeResolutionsAvailable=4;
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable=
		(struct tt__VideoResolution*)soap_malloc(soap,trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeResolutionsAvailable*sizeof(struct tt__VideoResolution));
	ResolutionsAvailable=trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable;

	ResolutionsAvailable->Width=1920;
	ResolutionsAvailable->Height=1080;
	ResolutionsAvailable++;
	ResolutionsAvailable->Width=1280;
	ResolutionsAvailable->Height=720;
	ResolutionsAvailable++;
	ResolutionsAvailable->Width=1600;
	ResolutionsAvailable->Height=1200;
	ResolutionsAvailable++;
	ResolutionsAvailable->Width=720;
	ResolutionsAvailable->Height=480;

	trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeH264ProfilesSupported=1;
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported=
		(enum tt__H264Profile*)soap_malloc(soap,trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeH264ProfilesSupported*sizeof(enum tt__H264Profile));
	*trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported=tt__H264Profile__High;

	trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->GovLengthRange=
			(struct tt__IntRange*)soap_malloc(soap,sizeof(struct tt__IntRange));
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->GovLengthRange->Max=200;
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->GovLengthRange->Min=1;

	trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->FrameRateRange=
			(struct tt__IntRange*)soap_malloc(soap,sizeof(struct tt__IntRange));
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->FrameRateRange->Max=25;
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->FrameRateRange->Min=1;

	trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->EncodingIntervalRange=
			(struct tt__IntRange*)soap_malloc(soap,sizeof(struct tt__IntRange));

	trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->EncodingIntervalRange->Max=12;
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->EncodingIntervalRange->Min=1;
	fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
	return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSourceConfigurationOptions(struct soap* soap, struct _trt__GetAudioSourceConfigurationOptions *trt__GetAudioSourceConfigurationOptions, struct _trt__GetAudioSourceConfigurationOptionsResponse *trt__GetAudioSourceConfigurationOptionsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioEncoderConfigurationOptions(struct soap* soap, struct _trt__GetAudioEncoderConfigurationOptions *trt__GetAudioEncoderConfigurationOptions, struct _trt__GetAudioEncoderConfigurationOptionsResponse *trt__GetAudioEncoderConfigurationOptionsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetMetadataConfigurationOptions(struct soap* soap, struct _trt__GetMetadataConfigurationOptions *trt__GetMetadataConfigurationOptions, struct _trt__GetMetadataConfigurationOptionsResponse *trt__GetMetadataConfigurationOptionsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputConfigurationOptions(struct soap* soap, struct _trt__GetAudioOutputConfigurationOptions *trt__GetAudioOutputConfigurationOptions, struct _trt__GetAudioOutputConfigurationOptionsResponse *trt__GetAudioOutputConfigurationOptionsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioDecoderConfigurationOptions(struct soap* soap, struct _trt__GetAudioDecoderConfigurationOptions *trt__GetAudioDecoderConfigurationOptions, struct _trt__GetAudioDecoderConfigurationOptionsResponse *trt__GetAudioDecoderConfigurationOptionsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetGuaranteedNumberOfVideoEncoderInstances(struct soap* soap, struct _trt__GetGuaranteedNumberOfVideoEncoderInstances *trt__GetGuaranteedNumberOfVideoEncoderInstances, struct _trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse *trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetStreamUri(struct soap* soap, struct _trt__GetStreamUri *trt__GetStreamUri, struct _trt__GetStreamUriResponse *trt__GetStreamUriResponse)
{
    if(strcmp(trt__GetStreamUri->ProfileToken,IPCPROFILE_1)!=0)
    {
	return soap_sender_fault_subcode(soap, "ter:InvalidArgVal/ter:NoProfile", NULL, NULL); 
    }
	trt__GetStreamUriResponse->MediaUri=
		(struct tt__MediaUri*)soap_malloc(soap,sizeof(struct tt__MediaUri));
	trt__GetStreamUriResponse->MediaUri->Uri=(char*)soap_malloc(soap,128*sizeof(char));

   snprintf(trt__GetStreamUriResponse->MediaUri->Uri,128,"rtsp://%s:%d/%s",g_sys_info.network.ipaddr,g_sys_info.rtpParamsbuf.rtspSrvPort,g_sys_info.rtpParamsbuf.streamName);
	//Int32 icam_get_rtp_params(IN ICamCtrlHandle hCtrl, OUT CamRtpParams *buf)
	fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
	return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__StartMulticastStreaming(struct soap* soap, struct _trt__StartMulticastStreaming *trt__StartMulticastStreaming, struct _trt__StartMulticastStreamingResponse *trt__StartMulticastStreamingResponse)
{
    if(strcmp(trt__StartMulticastStreaming->ProfileToken,IPCPROFILE_1)!=0)
    {
	return soap_sender_fault_subcode(soap, "ter:InvalidArgVal/ter:NoProfile", NULL, NULL); 
    }
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__StopMulticastStreaming(struct soap* soap, struct _trt__StopMulticastStreaming *trt__StopMulticastStreaming, struct _trt__StopMulticastStreamingResponse *trt__StopMulticastStreamingResponse)
{
    if(strcmp(trt__StopMulticastStreaming->ProfileToken,IPCPROFILE_1)!=0)
    {
	return soap_sender_fault_subcode(soap, "ter:InvalidArgVal/ter:NoProfile", NULL, NULL); 
    }


fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetSynchronizationPoint(struct soap* soap, struct _trt__SetSynchronizationPoint *trt__SetSynchronizationPoint, struct _trt__SetSynchronizationPointResponse *trt__SetSynchronizationPointResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetSnapshotUri(struct soap* soap, struct _trt__GetSnapshotUri *trt__GetSnapshotUri, struct _trt__GetSnapshotUriResponse *trt__GetSnapshotUriResponse)
{
fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
if(strcmp(trt__GetSnapshotUri->ProfileToken,IPCPROFILE_1)!=0)
	return SOAP_ERR;
trt__GetSnapshotUriResponse->MediaUri=
	(struct tt__MediaUri*)soap_malloc(soap,sizeof(struct tt__MediaUri));
trt__GetSnapshotUriResponse->MediaUri->Uri=(char*)soap_malloc(soap,128*sizeof(char));
snprintf(trt__GetSnapshotUriResponse->MediaUri->Uri,128,"http://%s:%d/%s",g_sys_info.network.ipaddr,g_sys_info.httpParamsbuf.rtspSrvPort,g_sys_info.httpParamsbuf.streamName);

return 0;
}

#ifdef ONVIF__tse__
SOAP_FMAC5 int SOAP_FMAC6 __tse__GetServiceCapabilities(struct soap* soap, struct _tse__GetServiceCapabilities *tse__GetServiceCapabilities, struct _tse__GetServiceCapabilitiesResponse *tse__GetServiceCapabilitiesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetRecordingSummary(struct soap* soap, struct _tse__GetRecordingSummary *tse__GetRecordingSummary, struct _tse__GetRecordingSummaryResponse *tse__GetRecordingSummaryResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetRecordingInformation(struct soap* soap, struct _tse__GetRecordingInformation *tse__GetRecordingInformation, struct _tse__GetRecordingInformationResponse *tse__GetRecordingInformationResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetMediaAttributes(struct soap* soap, struct _tse__GetMediaAttributes *tse__GetMediaAttributes, struct _tse__GetMediaAttributesResponse *tse__GetMediaAttributesResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tse__FindRecordings(struct soap* soap, struct _tse__FindRecordings *tse__FindRecordings, struct _tse__FindRecordingsResponse *tse__FindRecordingsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetRecordingSearchResults(struct soap* soap, struct _tse__GetRecordingSearchResults *tse__GetRecordingSearchResults, struct _tse__GetRecordingSearchResultsResponse *tse__GetRecordingSearchResultsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tse__FindEvents(struct soap* soap, struct _tse__FindEvents *tse__FindEvents, struct _tse__FindEventsResponse *tse__FindEventsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetEventSearchResults(struct soap* soap, struct _tse__GetEventSearchResults *tse__GetEventSearchResults, struct _tse__GetEventSearchResultsResponse *tse__GetEventSearchResultsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tse__FindPTZPosition(struct soap* soap, struct _tse__FindPTZPosition *tse__FindPTZPosition, struct _tse__FindPTZPositionResponse *tse__FindPTZPositionResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetPTZPositionSearchResults(struct soap* soap, struct _tse__GetPTZPositionSearchResults *tse__GetPTZPositionSearchResults, struct _tse__GetPTZPositionSearchResultsResponse *tse__GetPTZPositionSearchResultsResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetSearchState(struct soap* soap, struct _tse__GetSearchState *tse__GetSearchState, struct _tse__GetSearchStateResponse *tse__GetSearchStateResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tse__EndSearch(struct soap* soap, struct _tse__EndSearch *tse__EndSearch, struct _tse__EndSearchResponse *tse__EndSearchResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tse__FindMetadata(struct soap* soap, struct _tse__FindMetadata *tse__FindMetadata, struct _tse__FindMetadataResponse *tse__FindMetadataResponse){fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);return 0;}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetMetadataSearchResults(struct soap* soap, struct _tse__GetMetadataSearchResults *tse__GetMetadataSearchResults, struct _tse__GetMetadataSearchResultsResponse *tse__GetMetadataSearchResultsResponse)
{
	fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
return 0;
}
#endif


void wsdd_event_Hello(struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, const char *EndpointReference, const char *Types, const char *Scopes, const char *MatchBy, const char *XAddrs, unsigned int MetadataVersion)
{
	fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);

	return ;
}
void wsdd_event_Bye(struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, const char *EndpointReference, const char *Types, const char *Scopes, const char *MatchBy, const char *XAddrs, unsigned int *MetadataVersion)
{
	fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);

	return ;
}

soap_wsdd_mode wsdd_event_Probe(struct soap *soap, const char *MessageID, const char *ReplyTo, const char *Types, const char *Scopes, const char *MatchBy, struct wsdd__ProbeMatchesType *matches)
{
	fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);

	if(g_sys_info.WS_DiscoveryMode==tt__DiscoveryMode__NonDiscoverable)
	{
	soap->error=SOAP_ERR;
	return SOAP_WSDD_MANAGED;
	}
	
	char *XAddr=(char *)soap_malloc(soap,128*sizeof(char));
	snprintf(XAddr,128,"http://%s:%d/onvif/device_service",g_sys_info.network.ipaddr,ONVIF_SERVER_RORT);
	char *EndpointReferencetemp=(char *)soap_malloc(soap,128*sizeof(char));
	snprintf(EndpointReferencetemp,128,"urn:uuid:464A4854-4656-5242-4530-%s",g_sys_info.network.mac);
	char *EndpointReference= (char *)soap_malloc(soap, 1024);
	sprintf(EndpointReference,EndpointReferencetemp);	
	soap_wsdd_init_ProbeMatches(soap,matches); 

	//printf("Types=%s MatchBy=%s\n",Types,MatchBy);
//	if(MatchBy&&strstr(MatchBy,"http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/ldap")==NULL)
	//{
	//	soap->error=SOAP_ERR;
	//	soap->errnum=1;
	//	return SOAP_WSDD_MANAGED;
	//}

	//printf("Scopes=%s \n",Scopes);
	if(Scopes!=NULL)
	{
	  if(Scopes[0]!=0&&strstr(Scopes,"onvif://www.onvif.org/")==NULL)
	  {
	    soap->error=SOAP_ERR;
	    return SOAP_WSDD_MANAGED;
	  }
	}
	if(Types==NULL)
	{
	    soap->error=SOAP_ERR;
	    return SOAP_WSDD_MANAGED;
	}
	if(Types[0]!=0&&(strstr(Types,"Device")==NULL&&strstr(Types,"NetworkVideoTransmitter")==NULL))
	  {
	    soap->error=SOAP_ERR;
	    return SOAP_WSDD_MANAGED;
	  }
//	printf("Types=%s, Scopes=%s %x\n",Types, Scopes,Scopes[0]);
	soap_wsdd_add_ProbeMatch(soap,matches,
			 EndpointReferencetemp,
			 "tdn:NetworkVideoTransmitter",
			 g_ipc_scopes,
			 NULL,
			 XAddr,1);	

	return SOAP_WSDD_MANAGED;
}

soap_wsdd_mode wsdd_event_Resolve(struct soap *soap, const char *MessageID, const char *ReplyTo, const char *EndpointReference, struct wsdd__ResolveMatchType *match)
{
	fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);
	//soap_wsdd_mode wsdd_mode;
	//return wsdd_mode;
}



void wsdd_event_ProbeMatches(struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, struct wsdd__ProbeMatchesType *matches)
{
	fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);

	return ;
}

void wsdd_event_ResolveMatches(struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, struct wsdd__ResolveMatchType *match)
{
	fprintf(stderr,"%s,%s,%d\n",__FILE__,__func__,__LINE__);

	return ;
}



SOAP_FMAC5 int SOAP_FMAC6 soap_send___wsdd__Hello(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct wsdd__HelloType *wsdd__Hello)
{	struct __wsdd__Hello soap_tmp___wsdd__Hello; 
	if (soap_action == NULL)
		soap_action = "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/Hello";
	soap->encodingStyle = NULL;
	soap_tmp___wsdd__Hello.wsdd__Hello = wsdd__Hello;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___wsdd__Hello(soap, &soap_tmp___wsdd__Hello);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___wsdd__Hello(soap, &soap_tmp___wsdd__Hello, "-wsdd:Hello", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
 
	if (soap_connect(soap, NULL, soap_action))	
		{
		return SOAP_ERR;///soap_closesock(soap);	
		} 		  
		soap_set_endpoint(soap, soap_endpoint);	 	
		struct sockaddr_in peer;
		peer.sin_family=AF_INET; 
		peer.sin_port=htons(3702);
		peer.sin_addr.s_addr=inet_addr("239.255.255.250");	
		memcpy(&soap->peer, &peer, sizeof(struct sockaddr_in));	
		soap->peerlen = sizeof(peer); 	
	if(soap_envelope_begin_out(soap)	
		|| soap_putheader(soap)			
		|| soap_body_begin_out(soap)	
		|| soap_put___wsdd__Hello(soap, &soap_tmp___wsdd__Hello, "-wsdd:Hello", NULL)
		|| soap_body_end_out(soap) 	
		|| soap_envelope_end_out(soap)		
		|| soap_end_send(soap))			
		return soap_closesock(soap);
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 soap_send___wsdd__Bye(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct wsdd__ByeType *wsdd__Bye)
{	struct __wsdd__Bye soap_tmp___wsdd__Bye;
	if (soap_action == NULL)
		soap_action = "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/Bye";
	soap->encodingStyle = NULL;
	soap_tmp___wsdd__Bye.wsdd__Bye = wsdd__Bye;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___wsdd__Bye(soap, &soap_tmp___wsdd__Bye);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___wsdd__Bye(soap, &soap_tmp___wsdd__Bye, "-wsdd:Bye", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;

	if (soap_connect(soap, NULL, soap_action))	
		{
		return SOAP_ERR;///soap_closesock(soap);	
		} 		  
		soap_set_endpoint(soap, soap_endpoint);	 	
		struct sockaddr_in peer;
		peer.sin_family=AF_INET; 
		peer.sin_port=htons(3702);
		peer.sin_addr.s_addr=inet_addr("239.255.255.250");	
		memcpy(&soap->peer, &peer, sizeof(struct sockaddr_in));	
		soap->peerlen = sizeof(peer); 	
	if(soap_envelope_begin_out(soap)	
		|| soap_putheader(soap)			
		|| soap_body_begin_out(soap)	
		|| soap_put___wsdd__Bye(soap, &soap_tmp___wsdd__Bye, "-wsdd:Bye", NULL)
		|| soap_body_end_out(soap) 	
		|| soap_envelope_end_out(soap)		
		|| soap_end_send(soap))			
		return soap_closesock(soap);
	return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 soap_send___wsdd__Probe(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct wsdd__ProbeType *wsdd__Probe)
{	struct __wsdd__Probe soap_tmp___wsdd__Probe;
	if (soap_action == NULL)
		soap_action = "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/Probe";
	soap->encodingStyle = NULL;
	soap_tmp___wsdd__Probe.wsdd__Probe = wsdd__Probe;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___wsdd__Probe(soap, &soap_tmp___wsdd__Probe);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___wsdd__Probe(soap, &soap_tmp___wsdd__Probe, "-wsdd:Probe", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___wsdd__Probe(soap, &soap_tmp___wsdd__Probe, "-wsdd:Probe", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 soap_recv___wsdd__ProbeMatches(struct soap *soap, struct __wsdd__ProbeMatches *_param_5)
{
	soap_default___wsdd__ProbeMatches(soap, _param_5);
	soap_begin(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	soap_get___wsdd__ProbeMatches(soap, _param_5, "-wsdd:ProbeMatches", NULL);
	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
		soap->error = SOAP_NO_METHOD;
	if (soap->error
	 || soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_send___wsdd__Resolve(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct wsdd__ResolveType *wsdd__Resolve)
{	struct __wsdd__Resolve soap_tmp___wsdd__Resolve;
	if (soap_action == NULL)
		soap_action = "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/Resolve";
	soap->encodingStyle = NULL;
	soap_tmp___wsdd__Resolve.wsdd__Resolve = wsdd__Resolve;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___wsdd__Resolve(soap, &soap_tmp___wsdd__Resolve);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___wsdd__Resolve(soap, &soap_tmp___wsdd__Resolve, "-wsdd:Resolve", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___wsdd__Resolve(soap, &soap_tmp___wsdd__Resolve, "-wsdd:Resolve", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 soap_recv___wsdd__ResolveMatches(struct soap *soap, struct __wsdd__ResolveMatches *_param_7)
{
	soap_default___wsdd__ResolveMatches(soap, _param_7);
	soap_begin(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	soap_get___wsdd__ResolveMatches(soap, _param_7, "-wsdd:ResolveMatches", NULL);
	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
		soap->error = SOAP_NO_METHOD;
	if (soap->error
	 || soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_send___wsdd__ProbeMatches(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct wsdd__ProbeMatchesType *wsdd__ProbeMatches)
{	struct __wsdd__ProbeMatches soap_tmp___wsdd__ProbeMatches;
	if (soap_action == NULL)
		soap_action = "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/ProbeMatches";
	soap->encodingStyle = NULL;
	soap_tmp___wsdd__ProbeMatches.wsdd__ProbeMatches = wsdd__ProbeMatches;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___wsdd__ProbeMatches(soap, &soap_tmp___wsdd__ProbeMatches);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___wsdd__ProbeMatches(soap, &soap_tmp___wsdd__ProbeMatches, "-wsdd:ProbeMatches", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___wsdd__ProbeMatches(soap, &soap_tmp___wsdd__ProbeMatches, "-wsdd:ProbeMatches", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 soap_send___wsdd__ResolveMatches(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct wsdd__ResolveMatchesType *wsdd__ResolveMatches)
{	struct __wsdd__ResolveMatches soap_tmp___wsdd__ResolveMatches;
	if (soap_action == NULL)
		soap_action = "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/ResolveMatches";
	soap->encodingStyle = NULL;
	soap_tmp___wsdd__ResolveMatches.wsdd__ResolveMatches = wsdd__ResolveMatches;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___wsdd__ResolveMatches(soap, &soap_tmp___wsdd__ResolveMatches);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___wsdd__ResolveMatches(soap, &soap_tmp___wsdd__ResolveMatches, "-wsdd:ResolveMatches", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___wsdd__ResolveMatches(soap, &soap_tmp___wsdd__ResolveMatches, "-wsdd:ResolveMatches", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	return SOAP_OK;
}

 
 static int set_ip_mreq_socket_udp(int server_udp)	
 {	
	 unsigned char one = 1;  
	 int sock_opt = 1;	
   
	 /* reuse socket addr */  
	 if ((setsockopt(server_udp, SOL_SOCKET, SO_REUSEADDR, (void *) &sock_opt,	
					 sizeof (sock_opt))) == -1) {  
		 printf("setsockopt\n");  
	 }	
	 if ((setsockopt(server_udp, IPPROTO_IP, IP_MULTICAST_LOOP,  
						&one, sizeof (unsigned char))) == -1) {  
		 printf("setsockopt\n");  
	 }	

	 struct ip_mreq mreq;  
	 mreq.imr_multiaddr.s_addr = inet_addr("239.255.255.250");	
	 mreq.imr_interface.s_addr = htonl(INADDR_ANY);  
   
	 if(setsockopt(server_udp,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq))==-1){  
		 perror("memberchip error\n");	
	 }	
   
	 return 0;  
 }	



int soap_wsdd_Hello_send(struct soap *soap)
{
	char *XAddr=(char *)soap_malloc(soap,128*sizeof(char));
	snprintf(XAddr,128,"http://%s:%d/onvif/device_service",g_sys_info.network.ipaddr,ONVIF_SERVER_RORT);
	char *EndpointReferencetemp=(char *)soap_malloc(soap,128*sizeof(char));
	snprintf(EndpointReferencetemp,128,"urn:uuid:464A4854-4656-5242-4530-%s",g_sys_info.network.mac);
	char *EndpointReference= (char *)soap_malloc(soap, 1024);
	sprintf(EndpointReference,EndpointReferencetemp); 
	soap->error=0;
	soap_wsdd_Hello(soap, SOAP_WSDD_MANAGED, "urn:schemas-xmlsoap-org:ws:2005:04:discovery", 
		 soap_wsa_rand_uuid(soap), NULL,EndpointReferencetemp,"tdn:NetworkVideoTransmitter", 
		g_ipc_scopes, 
		NULL, XAddr, 1);  
}

int soap_wsdd_Bye_send(struct soap *soap)
{
	char *XAddr=(char *)soap_malloc(soap,128*sizeof(char));
	snprintf(XAddr,128,"http://%s:%d/onvif/device_service",g_sys_info.network.ipaddr,ONVIF_SERVER_RORT);
	char *EndpointReferencetemp=(char *)soap_malloc(soap,128*sizeof(char));
	snprintf(EndpointReferencetemp,128,"urn:uuid:464A4854-4656-5242-4530-%s",g_sys_info.network.mac);
	char *EndpointReference= (char *)soap_malloc(soap, 1024);
	sprintf(EndpointReference,EndpointReferencetemp); 
	soap->error=0;
	soap_wsdd_Bye(soap, SOAP_WSDD_MANAGED, "urn:schemas-xmlsoap-org:ws:2005:04:discovery", 
		soap_wsa_rand_uuid(soap),EndpointReferencetemp,
		"tdn:NetworkVideoTransmitter","onvif://www.onvif.org/type/NetworkVideoTransmitter onvif://www.onvif.org/location/js onvif://www.onvif.org/hardware/js",
		NULL,XAddr, 1);
}




void *soap_discovery_Probe()  
{  
    int ret=0;
	int i=0;
	int relaystatus=0;
	int relayreset=0;
	struct soap*wsdd_soap;
	wsdd_soap=soap_new2(SOAP_IO_UDP|SOAP_IO_FLUSH,SOAP_IO_UDP|SOAP_IO_FLUSH);
	soap_set_namespaces(wsdd_soap, namespaces);
	if (!soap_valid_socket(soap_bind(wsdd_soap, NULL, 3702, 100)))  
	 {      
	    soap_print_fault(wsdd_soap, stderr);  
	 }  
	set_ip_mreq_socket_udp(wsdd_soap->master);
	if(g_sys_info.WS_DiscoveryMode==tt__DiscoveryMode__Discoverable)
		soap_wsdd_Hello_send(wsdd_soap);
	while(g_onvif_status.running)
	{
	if(g_sys_info.WS_DiscoveryMode==tt__DiscoveryMode__Discoverable)
	{
		ret=soap_wsdd_listen(wsdd_soap,1);
		if(g_onvif_status.sendbyeflag)
		{
		soap_wsdd_Bye_send(wsdd_soap);
		g_onvif_status.sendbyeflag=0;
		}
	}
	else
		sleep(1);	
	if(g_onvif_status.relayreset)
	{ 
	   relayreset=0xFF;
	   relaystatus=0;
		for(i=0;i<8;i++)
		{
			if(g_onvif_status.alarmout[i].DelayTime)
			{
				g_onvif_status.alarmout[i].DelayTime-=1;
				if(g_onvif_status.alarmout[i].DelayTime==0)
				{
				relaystatus|=1<<i;
				}
			}
			else
			  relayreset&=~(1<<i);
 		}
		if(!relayreset)
		{
		g_onvif_status.relayreset=0;
		}
		else if(relaystatus)
		{
		   CamIoCfg params;
		    if(icam_get_io_config(g_icam_ctrl_handle,&params))
				continue;
			for(i=0;i<8;i++)
			{
			   if(relaystatus&(1<<i))
				{
				  if(g_onvif_status.alarmout[i].IdleState==tt__RelayIdleState__open)
					params.status &=~(1<<i);
				  else
				  	params.status |=(1<<i);
				}
			}
			printf("relaystatus =%x\n",params.status);  
			icam_set_io_config(g_icam_ctrl_handle,&params);
		}	
    }
	//printf("soap_wsdd_listen ret=%d-----wsdd_soap->error=%d\n",ret,wsdd_soap->error);  
    }
	if(g_sys_info.WS_DiscoveryMode==tt__DiscoveryMode__Discoverable)
		soap_wsdd_Bye_send(wsdd_soap);
return ;
}  


void *soap_event()  
{  
		int m, s;	 
		struct soap add_soap;	 
		int sock_opt = 1;  	
		soap_init(&add_soap);
		soap_set_namespaces(&add_soap, namespaces); 
		m = soap_bind(&add_soap, NULL, ONVIF_EVENT_RORT, 100);    
		while (m < 0) 
		{	
		ONVIF_EVENT_RORT+=1;
		m = soap_bind(&add_soap, NULL, ONVIF_EVENT_RORT, 100);	  
		sleep(1);
		}
		printf("soap_bind:  <event_server_port> %d  n",ONVIF_EVENT_RORT);   
				
		if (m < 0) 
		{	 printf("soap_bind:  <event_server_port> %d  faild\n",ONVIF_EVENT_RORT);   
				soap_print_fault(&add_soap, stderr);	
				return;	 
		}
		 /* reuse socket addr */  
		 if ((setsockopt(add_soap.master, SOL_SOCKET, SO_REUSEADDR, (void *) &sock_opt, 
						 sizeof (sock_opt))) == -1) {  
			 printf("setsockopt\n");  
		 }	
		 
		while(g_onvif_status.running) 
			{    
				s = soap_accept(&add_soap);    
				if (s < 0) {	
					soap_print_fault(&add_soap, stderr);	
					return ;
				}	 
				fprintf(stderr, "Socket connection successful: slave socket = %d\n", s);	
				soap_serve(&add_soap);	  
				soap_end(&add_soap);	
			}	 
return ;
}  


#if 0
void *http_snap()  
{  
		int m, s;	 
		struct soap add_soap;	 
		int sock_opt = 1;  	
		soap_init(&add_soap);
		soap_set_namespaces(&add_soap, namespaces); 
		m = soap_bind(&add_soap, NULL, ONVIF_HTTP_SNAP_RORT, 100);    
		if (m < 0) 
		{	
				soap_print_fault(&add_soap, stderr);	
				return;	 
		}
		 /* reuse socket addr */  
		 if ((setsockopt(add_soap.master, SOL_SOCKET, SO_REUSEADDR, (void *) &sock_opt, 
						 sizeof (sock_opt))) == -1) {  
			 printf("setsockopt\n");  
		 }	
		 
		while(g_onvif_status.running) {    
				s = soap_accept(&add_soap);    
				if (s < 0) {	
					soap_print_fault(&add_soap, stderr);	
					exit(-1);	 
				}	 
				fprintf(stderr, "Socket connection successful: slave socket = %d\n", s);	
				soap_serve(&add_soap);	  
				soap_end(&add_soap);	
			}	 
return ;
}  
#endif

int main(int argc, char **argv)    
{    
    int m, s;    
	g_onvif_status.running=1;
    struct soap add_soap;    
	int sock_opt = 1;  
	g_icam_ctrl_handle=icam_ctrl_create(ICAME_CTRL_FILR, 0, 5);
	init_sys_config();


//printf("g_network_info.ipaddr=%s\n",g_network_info.ipaddr);
    //bind_server_udp1(server_udp);   
    pthread_t thrHello;  
    pthread_t thrProbe;  
    //pthread_create(&thrHello,NULL,main_Hello,server_udp);   
    //sleep(2);   

	
	soap_init1(&add_soap,SOAP_ENC_MTOM);
    soap_set_namespaces(&add_soap, namespaces); 
	
	
	 
    m = soap_bind(&add_soap, NULL, ONVIF_SERVER_RORT, 100);    
	while (m < 0)
	{
			ONVIF_SERVER_RORT+=1;
			m = soap_bind(&add_soap, NULL, ONVIF_SERVER_RORT, 100); 
			sleep(1);
	}
   if (m < 0) 
   	{    
        soap_print_fault(&add_soap, stderr);   
		printf("soap_bind:  <server_port> %d  faild\n",ONVIF_SERVER_RORT);    
            exit(-1);    
    }    
    	/* reuse socket addr */  
	 if ((setsockopt(add_soap.master, SOL_SOCKET, SO_REUSEADDR, (void *) &sock_opt,	
					 sizeof (sock_opt))) == -1)
	 {  
		 printf("setsockopt\n");  
	 }


		pthread_create(&thrProbe,NULL,soap_discovery_Probe,NULL);  
		// soap_init(&add_soap);
		
#ifdef ONVIF__TEST__
		if(ONVIF_EVENT_RORT!=ONVIF_SERVER_RORT)
			pthread_create(&thrProbe,NULL,soap_event,NULL);  
#endif

        fprintf(stderr, "soap serve start successful: ONVIF_SERVER_RORT = %d\n", ONVIF_SERVER_RORT);    
        while(g_onvif_status.running)
		{    
            s = soap_accept(&add_soap);    
            if (s < 0)
			{    
                soap_print_fault(&add_soap, stderr);    
                exit(-1);    
            }    
            fprintf(stderr, "Socket connection successful: slave socket = %d\n", s);    
            soap_serve(&add_soap);    
            soap_end(&add_soap);    
        }    
 
	if(g_icam_ctrl_handle)
		icam_ctrl_delete(g_icam_ctrl_handle);
	g_icam_ctrl_handle=NULL;
    return 0;    
}   


