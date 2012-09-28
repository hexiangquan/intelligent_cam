/*
 * Copyright (C) 2012-2018 S.K. Sun
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and the associated README documentation file (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
// A group of "C" interface for multi-media server
// using a built-in RTSP server.

#include <signal.h>
#include <BasicUsageEnvironment.hh>
#include <getopt.h>
#include <liveMedia.hh>
#include "Err.hh"
#include "WISJPEGVideoServerMediaSubsession.hh"
#include "WISMPEG4VideoServerMediaSubsession.hh"
#include "WISH264VideoServerMediaSubsession.hh"
#include "WISPCMAudioServerMediaSubsession.hh"
#include <sys/time.h>
#include <sys/resource.h>
#include <GroupsockHelper.hh>
#include "WISJPEGStreamSource.hh"
#include "ICamInput.hh"
#include "media_server.h"
#include "log.h"
#include <pthread.h>

#define MEDIA_AUDIO_EN		1

/* Objects used for multicast streaming: */
typedef struct {
	Groupsock* 		rtpGroupsockAudio;
	Groupsock* 		rtcpGroupsockAudio;
	Groupsock* 		rtpGroupsockVideo;
	Groupsock* 		rtcpGroupsockVideo;
	FramedSource* 	sourceAudio;
	RTPSink* 		sinkAudio;
	RTCPInstance* 	rtcpAudio;
	FramedSource* 	sourceVideo;
	RTPSink* 		sinkVideo;
	RTCPInstance* 	rtcpVideo;
}MulticastObj;

/* struct for media sub session */
typedef struct MediaSubSessionObj {
	ICamInput 		*inputDev;
	MulticastObj	*multicastResource;
}MediaSrvSubSession;

/* struct for media sessions */
typedef struct MediaSessionObj{
	StreamingMode 		mode;
	ServerMediaSession	*sms;
	Int32				subSessionNum;
	MediaSrvSubSession	subSessions[MEDIA_SRV_MAX_SUB_SESSION_NUM];
}MediaSrvSession;


/* Obj for media server */
struct MediaSrvObj {
	TaskScheduler		*scheduler;
	UsageEnvironment	*env;
	RTSPServer			*rtspSrv;
	Int32				sessionNum;
	pthread_t			pid;
	MediaSrvSession		sessions[MEDIA_SRV_MAX_SESSION_NUM];
	volatile char		exit;
};


/* we are using C interfaces */
extern "C" {

static void media_srv_sig_handler(int sig)
{
	/* do nothing */
	return;
} 


/*****************************************************************************
 Prototype    : media_srv_create
 Description  : create media server
 Input        : Uint16 rtspSrvPort  
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/26
    Author       : Sun
    Modification : Created function

*****************************************************************************/
MediaSrvHandle media_srv_create(Uint16 rtspSrvPort)
{
	MediaSrvHandle hMediaSrv;

	/* alloc mem */
	hMediaSrv = (MediaSrvHandle)calloc(1, sizeof(struct MediaSrvObj));
	if(!hMediaSrv) {
		ERR("alloc mem failed");
		return NULL;
	}

	/* Begin by setting up our usage environment: */
	hMediaSrv->scheduler = BasicTaskScheduler::createNew();
	hMediaSrv->env = BasicUsageEnvironment::createNew(*hMediaSrv->scheduler);

	if(!hMediaSrv->scheduler || !hMediaSrv->env) {
		ERR("create usage env failed.");
		goto exit;
	}

	/* Create the RTSP server: */
	hMediaSrv->rtspSrv = RTSPServer::createNew(*hMediaSrv->env, rtspSrvPort, NULL);
	if (!hMediaSrv->rtspSrv) {
		ERR("Failed to create RTSP server: %s", hMediaSrv->env->getResultMsg());
		goto exit;
	}

	/* catch signals */
	signal(SIGUSR2, media_srv_sig_handler);
	
	/* all done */
	return hMediaSrv;

exit:
	/* error occured, delete and return NULL */
	media_srv_delete(hMediaSrv);

	return NULL;
}

/*****************************************************************************
 Prototype    : multicast_sub_session_delete
 Description  : delete multicast subsession
 Input        : MediaSubSessionHandle media  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/26
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 multicast_sub_session_delete(MediaSubSessionHandle media)
{
	MulticastObj	*multicast = media->multicastResource;

	if(!multicast) {
		return E_INVAL;
	}

#ifdef MEDIA_AUDIO_EN
	Medium::close(multicast->rtcpAudio);
	Medium::close(multicast->sinkAudio);
	Medium::close(multicast->sourceAudio);
	delete multicast->rtpGroupsockAudio;
	delete multicast->rtcpGroupsockAudio;
#endif

	Medium::close(multicast->rtcpVideo);
	Medium::close(multicast->sinkVideo);
	Medium::close(multicast->sourceVideo);
	delete multicast->rtpGroupsockVideo;
	delete multicast->rtcpGroupsockVideo;

	free(multicast);

	media->multicastResource = NULL;

	return E_NO;
}


/*****************************************************************************
 Prototype    : media_srv_delete
 Description  : delete media server
 Input        : MediaSrvHandle hSrv  
 Output       : None
 Return Value : extern
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/26
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 media_srv_delete(MediaSrvHandle hSrv)
{
	if(!hSrv)
		return E_INVAL;

	if(hSrv->pid > 0) {
		/* wait thread exit */
		hSrv->exit = 1;
		pthread_kill(hSrv->pid, SIGUSR2);
		//DBG("wait thread exit: %d...", hSrv->exit);
		pthread_join(hSrv->pid, NULL);
	}

	/* close rtsp server */
	if(hSrv->rtspSrv) {
		Medium::close(hSrv->rtspSrv); // will also reclaim "sms" and its "ServerMediaSubsession"s
		Int32 i, j;
		/* release resources closed by sessions and sub sessions */
		for(i = 0; i < hSrv->sessionNum; i++) {
			MediaSessionHandle hSession = &hSrv->sessions[i];
			for(j = 0; j < hSession->subSessionNum; j++) {
				MediaSubSessionHandle hSubSession = &hSession->subSessions[j];
				if(hSubSession->multicastResource) 
					multicast_sub_session_delete(hSubSession);
				if(hSubSession->inputDev)
					Medium::close(hSubSession->inputDev);
			}
		}
	}
	
	/* stop env */
	if(hSrv->env)
		hSrv->env->reclaim();

	/* delete task scheduler */
  	delete hSrv->scheduler;

	/* free mem */
	free(hSrv);

	return E_NO;
}

/*****************************************************************************
 Prototype    : media_srv_create_session
 Description  : create new media session
 Input        : MediaSrvHandle hSrv  
                const char *name     
                const char *desp     
                StreamingMode mode   
 Output       : None
 Return Value : MediaSessionHandle
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/26
    Author       : Sun
    Modification : Created function

*****************************************************************************/
MediaSessionHandle media_srv_create_session(MediaSrvHandle hSrv, const char *name, const char *desp, StreamingMode mode)
{
	MediaSessionHandle hSession;

	if(!hSrv)
		return NULL;

	if(hSrv->sessionNum >= MEDIA_SRV_MAX_SESSION_NUM) {
		ERR("can't add more sessions.");
		return NULL;
	}

	hSession = &hSrv->sessions[hSrv->sessionNum];

	/* create new media session */
	ServerMediaSession* sms = NULL;

 	sms = ServerMediaSession::createNew(*hSrv->env, name, name, desp, mode == STREAMING_MULTICAST_SSM);
	if(!sms) {
		ERR("create media session failed: %s", hSrv->env->getResultMsg());
		goto exit;
	}

	/* record params */
	hSession->mode = mode;
	hSession->sms = sms;
	hSrv->sessionNum++;

	return hSession;

exit:
	if(sms)
		delete sms;

	return NULL;
}

/*****************************************************************************
 Prototype    : media_srv_add_unicast_sub_session
 Description  : create sub session for unicast
 Input        : ServerMediaSession *sms  
                ICamInput *input         
                MediaType type           
                Uint32 bitRate           
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/26
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 media_srv_add_unicast_sub_session(MediaSessionHandle hSession, ICamInput *input, MediaType type, Uint32 bitRate)
{
	ServerMediaSession *sms = hSession->sms;
	WISServerMediaSubsession *subSession = NULL;
	Int32 ret = E_IO;

	switch(type) {
	case MEDIA_TYPE_H264:
		subSession = WISH264VideoServerMediaSubsession::createNew(sms->envir(), *input, bitRate);
		break;
	case MEDIA_TYPE_MJPEG:
		subSession = WISJPEGVideoServerMediaSubsession::createNew(sms->envir(), *input, bitRate);
		break;
	case MEDIA_TYPE_MPEG4:
		subSession = WISMPEG4VideoServerMediaSubsession::createNew(sms->envir(), *input, bitRate);
		break;
	default: 
		ERR("unsupport media type: %d", (int)type);
		break;
	}

	if(!subSession)
		return ret;

	sms->addSubsession(subSession);
	sms->addSubsession(WISPCMAudioServerMediaSubsession::createNew(sms->envir(), *input));

	return E_NO;
	
}

/*****************************************************************************
 Prototype    : media_srv_add_multicast_sub_session
 Description  : add multicast sub session and playing
 Input        : MediaSessionHandle hSession  
                ICamInput *input             
                MediaType type               
                Uint16 videoPort             
                Uint16 audioPort             
                Uint32 bitRate               
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/26
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 media_srv_add_multicast_sub_session(MediaSessionHandle hSession, ICamInput *input, MediaType type, Uint16 videoPort, Uint16 audioPort, Uint32 bitRate)
{
	Int32 ret = E_IO;
	ServerMediaSession *sms = hSession->sms;
	MediaSubSessionHandle media = &hSession->subSessions[hSession->subSessionNum];
	MulticastObj	*multicast;
	
	multicast = (MulticastObj *)calloc(1, sizeof(MulticastObj));
	if(!multicast) {
		ERR("alloc mem for multicast resource failed");
		return E_NOMEM;
	}

	media->multicastResource = multicast;
	
	netAddressBits multicastAddress = 0;
	if(hSession->mode == STREAMING_MULTICAST_SSM) {
		if(multicastAddress == 0)
			multicastAddress = chooseRandomIPv4SSMAddress(sms->envir());
	} else if (multicastAddress != 0) {
		hSession->mode = STREAMING_MULTICAST_ASM;
	}

	struct in_addr dest;
	Uint8 ttl = 255;
	dest.s_addr = multicastAddress;

	// Create 'groupsocks' for RTP and RTCP:
    const Port rtpPortVideo(videoPort);
    const Port rtcpPortVideo(videoPort + 1);

    multicast->rtpGroupsockVideo = new Groupsock(sms->envir(), dest, rtpPortVideo, ttl);
    multicast->rtcpGroupsockVideo = new Groupsock(sms->envir(), dest, rtcpPortVideo, ttl);
    if(hSession->mode == STREAMING_MULTICAST_SSM) {
		multicast->rtpGroupsockVideo->multicastSendOnly();
		multicast->rtcpGroupsockVideo->multicastSendOnly();
    }
	
	setVideoRTPSinkBufferSize();

	multicast->sourceAudio = input->audioSource();
	
	switch(type) {
	case MEDIA_TYPE_H264:
		multicast->sourceVideo = H264VideoStreamFramer
			::createNew(sms->envir(), input->videoSource());
		multicast->sinkVideo = H264VideoRTPSink
			::createNew(sms->envir(), multicast->rtpGroupsockVideo, 96, 0x42, "h264");
		break;
	case MEDIA_TYPE_MJPEG:
		multicast->sourceVideo = WISJPEGStreamSource
			::createNew(input->videoSource());
		multicast->sinkVideo = JPEGVideoRTPSink
			::createNew(sms->envir(), multicast->rtpGroupsockVideo);
		break;
	case MEDIA_TYPE_MPEG4:
		multicast->sourceVideo = MPEG4VideoStreamDiscreteFramer
			::createNew(sms->envir(), input->videoSource());
		multicast->sinkVideo = MPEG4ESVideoRTPSink
			::createNew(sms->envir(), multicast->rtpGroupsockVideo, 97);
		break;
	default: 
		ERR("unsupport media type: %d", (int)type);
		break;
	}

	if(!multicast->sinkVideo || !multicast->sourceVideo) {
		ret = multicast_sub_session_delete(media);
		return E_IO;
	}

	setVideoRTPSinkBufferSize();
	
	/* VIDEO Channel initial */
	char CNAME[101];
	bzero(CNAME, sizeof(CNAME));
	gethostname(CNAME, sizeof(CNAME));
	
	// Create (and start) a 'RTCP instance' for this RTP sink:
	unsigned totalSessionBandwidthVideo = (bitRate + 500)/1000; // in kbps; for RTCP b/w share

	multicast->rtcpVideo = RTCPInstance::createNew(sms->envir(), multicast->rtcpGroupsockVideo,
								totalSessionBandwidthVideo, (Uint8 *)CNAME,
								multicast->sinkVideo, NULL /* we're a server */ ,
								hSession->mode == STREAMING_MULTICAST_SSM);
    // Note: This starts RTCP running automatically
	sms->addSubsession(PassiveServerMediaSubsession::createNew(*multicast->sinkVideo, multicast->rtcpVideo));

	// Start streaming:
	multicast->sinkVideo->startPlaying(*multicast->sourceVideo, NULL, NULL);

#ifdef MEDIA_AUDIO_EN
	/* AUDIO Channel initial */

	// there's a separate RTP stream for audio
	// Create 'groupsocks' for RTP and RTCP:
	const Port rtpPortAudio(audioPort);
	const Port rtcpPortAudio(audioPort + 1);

	multicast->rtpGroupsockAudio = new Groupsock(sms->envir(), dest, rtpPortAudio, ttl);
	multicast->rtcpGroupsockAudio = new Groupsock(sms->envir(), dest, rtcpPortAudio, ttl);

	if(hSession->mode == STREAMING_MULTICAST_SSM) {
		multicast->rtpGroupsockAudio->multicastSendOnly();
		multicast->rtcpGroupsockAudio->multicastSendOnly();
	}
	
	multicast->sinkAudio = 
		SimpleRTPSink::createNew(sms->envir(), 
			multicast->rtpGroupsockAudio, 96, AUDIO_SAMPLE_FREQUENCY, "audio", "PCMU", 1);
	

	// Create (and start) a 'RTCP instance' for this RTP sink:
	unsigned totalSessionBandwidthAudio = (AUDIO_OUT_BITRATE + 500)/1000; // in kbps; for RTCP b/w share
	multicast->rtcpAudio = RTCPInstance::createNew(sms->envir(), multicast->rtcpGroupsockAudio,
							  totalSessionBandwidthAudio, (Uint8 *)CNAME,
							  multicast->sinkAudio, NULL /* we're a server */,
							  hSession->mode == STREAMING_MULTICAST_SSM);
	// Note: This starts RTCP running automatically
	sms->addSubsession(PassiveServerMediaSubsession
			::createNew(*multicast->sinkAudio, multicast->rtcpAudio));

	// Start streaming:
	multicast->sinkAudio->startPlaying(*multicast->sourceAudio, NULL, NULL);
#endif

	return E_NO;
	
}


/*****************************************************************************
 Prototype    : media_srv_add_sub_session
 Description  : add sub session
 Input        : MediaSessionHandle hSession  
                MediaType type               
                Uint16 videoPort             
                Uint16 audioPort             
                Uint32 bitRate               
 Output       : None
 Return Value : MediaSubSessionHandle
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/26
    Author       : Sun
    Modification : Created function

*****************************************************************************/
MediaSubSessionHandle media_srv_add_sub_session(MediaSessionHandle hSession, MediaType type, Uint16 videoPort, Uint16 audioPort, Uint32 bitRate)
{
	if(!hSession)
		return NULL;

	if(hSession->subSessionNum >= MEDIA_SRV_MAX_SUB_SESSION_NUM) {
		ERR("can't add more sub sessions");
		return NULL;
	}

	Int32 ret = E_IO;
	ServerMediaSession *sms = hSession->sms;
	ICamInput *inputDevice = ICamInput::createNew(sms->envir(), type);
	MediaSubSessionHandle media = &hSession->subSessions[hSession->subSessionNum];

	if(!inputDevice) {
		goto exit;
	}

	if(hSession->mode == STREAMING_UNICAST) {
		/* unicast playing */
		ret = media_srv_add_unicast_sub_session(hSession, inputDevice, type, bitRate);
	} else {
		/* multicast playing */
		ret = media_srv_add_multicast_sub_session(hSession, inputDevice, type, videoPort, audioPort, bitRate);
	}

	if(ret)
		goto exit;

	media->inputDev = inputDevice;
	hSession->subSessionNum++;
	return media;
	
exit:

	if(inputDevice)
		Medium::close(inputDevice);

	return NULL;
}

//static char g_exit = 0;

/*****************************************************************************
 Prototype    : media_srv_thread
 Description  : thread for ansync running
 Input        : void *arg  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/26
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static void *media_srv_thread(void *arg)
{
	assert(arg != NULL);
	MediaSrvHandle	hSrv = (MediaSrvHandle)arg;
	Int32			i;

	for(i = 0; i < hSrv->sessionNum; i++) {
		ServerMediaSession *sms = hSrv->sessions[i].sms;
		hSrv->rtspSrv->addServerMediaSession(sms);

		char *url = hSrv->rtspSrv->rtspURL(sms);
		INFO("Play this stream using the URL: %s\n\t", url);
		delete[] url;
	}

	// Begin the LIVE555 event loop:
	hSrv->env->taskScheduler().doEventLoop((char *)&hSrv->exit); // does not return

	DBG("media srv thread exit...");
	pthread_exit(0);
}

/*****************************************************************************
 Prototype    : media_srv_run
 Description  : run server
 Input        : MediaSrvHandle hSrv  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/26
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 media_srv_run(MediaSrvHandle hSrv)
{
	if(!hSrv)
		return E_INVAL;

	if(!hSrv->sessionNum) {
		ERR("you should add at least one session before running.");
		return E_MODE;
	}

	if(pthread_create(&hSrv->pid, NULL, media_srv_thread, hSrv) < 0) {
		ERRSTR("create thread failed");
		return E_IO;
	}

	return E_NO;
}

/*****************************************************************************
 Prototype    : media_stream_in
 Description  : input new frame for a sub session
 Input        : MediaSubSessionHandle hSubSession  
                MediaFrame *frame                  
                Bool isVideo                       
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/4/28
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 media_stream_in(MediaSubSessionHandle hSubSession, MediaFrame *frame, Bool isVideo)
{
	if(!hSubSession || !frame)
		return E_INVAL;

	if(!hSubSession->inputDev) {
		ERR("input dev is not created.");
		return E_MODE;
	}

	if(isVideo)
		return hSubSession->inputDev->videoWrite(frame);
	else
		return hSubSession->inputDev->audioWrite(frame);
}

/*****************************************************************************
 Prototype    : media_get_link_status
 Description  : Check link status
 Input        : MediaSubSessionHandle hSubSession  
 Output       : None
 Return Value : TRUE -- data link is established, FALSE -- not linked
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/8/10
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Bool media_get_link_status(MediaSubSessionHandle hSubSession)
{
	if(!hSubSession)
		return FALSE;

	return hSubSession->inputDev->isOpened();
}

} /* end of extern "C" */

