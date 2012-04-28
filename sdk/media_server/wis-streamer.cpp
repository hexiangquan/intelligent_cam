/*
 * Copyright (C) 2005-2006 WIS Technologies International Ltd.
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
// An application that streams audio/video captured by a WIS GO7007,
// using a built-in RTSP server.
// main program

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

enum  StreamingMode
{
	STREAMING_UNICAST,
	STREAMING_UNICAST_THROUGH_DARWIN,
	STREAMING_MULTICAST_ASM,
	STREAMING_MULTICAST_SSM
};


portNumBits rtspServerPortNum = 554;
char const* MjpegStreamName = "mjpeg";
char const* Mpeg4StreamName = "mpeg4";
char const* H264StreamName = "h264";
char const* streamDescription = "RTSP/RTP stream from IPNC";

int MjpegVideoBitrate = 4000000;
int Mpeg4VideoBitrate = 4000000;
int H264VideoBitrate = 4000000;
int audioOutputBitrate = 128000;


unsigned audioSamplingFrequency = 16000;
unsigned audioNumChannels = 1;
int audio_enable = 1;
char watchVariable = 0;
char videoType = -1;
char IsAudioAlarm = 0;



int qid = 0;

void sigterm(int dummy)
{
	printf("caught SIGTERM: shutting down\n");
	
	exit(1);
}

void sigint(int dummy)
{
	printf("caught SIGINT: shutting down\n");
	//printf("watchVariable = %d\n",watchVariable);
	//printf("videoType = %d\n",videoType);
	watchVariable = 1;
	alarm(1);
	//ApproInterfaceExit();
	//exit(1);
}

void init_signals(void)
{
	struct sigaction sa;

	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGTERM);
	sigaddset(&sa.sa_mask, SIGINT);
	sigaddset(&sa.sa_mask, SIGALRM);

	sa.sa_flags = 0;
	sa.sa_handler = sigterm;
	sigaction(SIGTERM, &sa, NULL);

	sa.sa_flags = 0;
	sa.sa_handler = sigint;
	sigaction(SIGINT, &sa, NULL);

	sa.sa_flags = 0;
	sa.sa_handler = sigint;
	sigaction( SIGALRM, &sa, NULL );
}

int main(int argc, char** argv) {

	init_signals();
	setpriority(PRIO_PROCESS, 0, 0);

	audioOutputBitrate = 64000;
	audioSamplingFrequency = 8000;

		// Begin by setting up our usage environment:
	TaskScheduler* scheduler = BasicTaskScheduler::createNew();
	UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);
	
	ICamInput* H264InputDevice = NULL;
	
	StreamingMode streamingMode = STREAMING_UNICAST;
	netAddressBits multicastAddress = 0;//our_inet_addr("224.1.4.6");
	portNumBits videoRTPPortNum = 0;
	portNumBits audioRTPPortNum = 0;

	rtspServerPortNum = 8557;
	H264VideoBitrate = 4000000;
	videoRTPPortNum = 6016;
	audioRTPPortNum = 6018;
	streamingMode = STREAMING_UNICAST;
	videoType = 0;

	if(argc > 1) {
		if(strcmp(argv[1], "-m" )== 0) {
			streamingMode = STREAMING_MULTICAST_SSM;
		}
	}
	

	// Objects used for multicast streaming:
	static Groupsock* rtpGroupsockAudio = NULL;
	static Groupsock* rtcpGroupsockAudio = NULL;
	static Groupsock* rtpGroupsockVideo = NULL;
	static Groupsock* rtcpGroupsockVideo = NULL;
	static FramedSource* sourceAudio = NULL;
	static RTPSink* sinkAudio = NULL;
	static RTCPInstance* rtcpAudio = NULL;
	static FramedSource* sourceVideo = NULL;
	static RTPSink* sinkVideo = NULL;
	static RTCPInstance* rtcpVideo = NULL;

	*env << "Initializing...\n";

	// Initialize the WIS input device:
	H264InputDevice = ICamInput::createNew(*env, videoType);
	if(H264InputDevice == NULL) {
		err(*env) << "Failed to create H264 input device\n";
		exit(1);
	}


	// Create the RTSP server:
	RTSPServer* rtspServer = NULL;
	// Normal case: Streaming from a built-in RTSP server:
	rtspServer = RTSPServer::createNew(*env, rtspServerPortNum, NULL);
	if (rtspServer == NULL) {
		*env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
		exit(1);
	}

	*env << "...done initializing\n";



	if( streamingMode == STREAMING_UNICAST ) {
		// Create a record describing the media to be streamed:
	  
		ServerMediaSession* sms
		  = ServerMediaSession::createNew(*env, H264StreamName, H264StreamName, streamDescription,streamingMode == STREAMING_MULTICAST_SSM);
		sms->addSubsession(WISH264VideoServerMediaSubsession
				 ::createNew(sms->envir(), *H264InputDevice, H264VideoBitrate));
		sms->addSubsession(WISPCMAudioServerMediaSubsession::createNew(sms->envir(), *H264InputDevice));
		rtspServer->addServerMediaSession(sms);

		char *url = rtspServer->rtspURL(sms);
		*env << "Play this stream using the URL:\n\t" << url << "\n";
		delete[] url;
	
	}else {

		// Multicast
		if (streamingMode == STREAMING_MULTICAST_SSM)
		{
			if (multicastAddress == 0)
				multicastAddress = chooseRandomIPv4SSMAddress(*env);
		} else if (multicastAddress != 0) {
			streamingMode = STREAMING_MULTICAST_ASM;
		}

		struct in_addr dest; dest.s_addr = multicastAddress;
		const unsigned char ttl = 255;

		// For RTCP:
		const unsigned maxCNAMElen = 100;
		unsigned char CNAME[maxCNAMElen + 1];
		gethostname((char *) CNAME, maxCNAMElen);
		CNAME[maxCNAMElen] = '\0';      // just in case

		ServerMediaSession* sms;

 		sms = ServerMediaSession::createNew(*env, H264StreamName, H264StreamName, streamDescription,streamingMode == STREAMING_MULTICAST_SSM);

		sourceAudio = H264InputDevice->audioSource();
		sourceVideo = H264VideoStreamFramer::createNew(*env, H264InputDevice->videoSource());

		// Create 'groupsocks' for RTP and RTCP:
	    const Port rtpPortVideo(videoRTPPortNum);
	    const Port rtcpPortVideo(videoRTPPortNum+1);
	    rtpGroupsockVideo = new Groupsock(*env, dest, rtpPortVideo, ttl);
	    rtcpGroupsockVideo = new Groupsock(*env, dest, rtcpPortVideo, ttl);
	    if (streamingMode == STREAMING_MULTICAST_SSM) {
			rtpGroupsockVideo->multicastSendOnly();
			rtcpGroupsockVideo->multicastSendOnly();
	    }
		setVideoRTPSinkBufferSize();
		sinkVideo = H264VideoRTPSink::createNew(*env, rtpGroupsockVideo,96, 0x42, "h264");
	
		/* VIDEO Channel initial */
	
		// Create (and start) a 'RTCP instance' for this RTP sink:
		unsigned totalSessionBandwidthVideo = (Mpeg4VideoBitrate+500)/1000; // in kbps; for RTCP b/w share
		rtcpVideo = RTCPInstance::createNew(*env, rtcpGroupsockVideo,
						totalSessionBandwidthVideo, CNAME,
						sinkVideo, NULL /* we're a server */ ,
						streamingMode == STREAMING_MULTICAST_SSM);
	    // Note: This starts RTCP running automatically
		sms->addSubsession(PassiveServerMediaSubsession::createNew(*sinkVideo, rtcpVideo));

		// Start streaming:
		sinkVideo->startPlaying(*sourceVideo, NULL, NULL);
	
		/* AUDIO Channel initial */
	
		// there's a separate RTP stream for audio
		// Create 'groupsocks' for RTP and RTCP:
		const Port rtpPortAudio(audioRTPPortNum);
		const Port rtcpPortAudio(audioRTPPortNum+1);

		rtpGroupsockAudio = new Groupsock(*env, dest, rtpPortAudio, ttl);
		rtcpGroupsockAudio = new Groupsock(*env, dest, rtcpPortAudio, ttl);

		if (streamingMode == STREAMING_MULTICAST_SSM) {
			rtpGroupsockAudio->multicastSendOnly();
			rtcpGroupsockAudio->multicastSendOnly();
		}
		if( audioSamplingFrequency == 16000 ) {
			sinkAudio = SimpleRTPSink::createNew(*env, rtpGroupsockAudio, 96, audioSamplingFrequency, "audio", "PCMU", 1);
		}else{
			sinkAudio = SimpleRTPSink::createNew(*env, rtpGroupsockAudio, 0, audioSamplingFrequency, "audio", "PCMU", 1);
		}

		// Create (and start) a 'RTCP instance' for this RTP sink:
		unsigned totalSessionBandwidthAudio = (audioOutputBitrate+500)/1000; // in kbps; for RTCP b/w share
		rtcpAudio = RTCPInstance::createNew(*env, rtcpGroupsockAudio,
					  totalSessionBandwidthAudio, CNAME,
					  sinkAudio, NULL /* we're a server */,
					  streamingMode == STREAMING_MULTICAST_SSM);
		// Note: This starts RTCP running automatically
		sms->addSubsession(PassiveServerMediaSubsession::createNew(*sinkAudio, rtcpAudio));

		// Start streaming:
		sinkAudio->startPlaying(*sourceAudio, NULL, NULL);
		rtspServer->addServerMediaSession(sms);
	
		char *url = rtspServer->rtspURL(sms);
		//char *url2 = inet_ntoa(dest);
		*env << "Mulicast Play this stream using the URL:\n\t" << url << "\n";
		//*env << "2 Mulicast addr:\n\t" << url2 << "\n";
		delete[] url;
	}


	// Begin the LIVE555 event loop:
	env->taskScheduler().doEventLoop(&watchVariable); // does not return


	if( streamingMode!= STREAMING_UNICAST ) {
		Medium::close(rtcpAudio);
		Medium::close(sinkAudio);
		Medium::close(sourceAudio);
		delete rtpGroupsockAudio;
		delete rtcpGroupsockAudio;

		Medium::close(rtcpVideo);
		Medium::close(sinkVideo);
		Medium::close(sourceVideo);
		delete rtpGroupsockVideo;
		delete rtcpGroupsockVideo;
	}

  	Medium::close(rtspServer); // will also reclaim "sms" and its "ServerMediaSubsession"s

	if( H264InputDevice != NULL ) {
		Medium::close(H264InputDevice);
	}

  	env->reclaim();

  	delete scheduler;

	return 0; // only to prevent compiler warning

}


