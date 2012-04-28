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
// A "ServerMediaSubsession" subclass for on-demand unicast streaming
// of PCM audio from a WIS GO7007 capture device.
// Implementation

#include "WISPCMAudioServerMediaSubsession.hh"
//#include "Options.hh"
#include "AudioRTPCommon.hh"
#include <liveMedia.hh>
#include "ICamInput.hh"


WISPCMAudioServerMediaSubsession* WISPCMAudioServerMediaSubsession
::createNew(UsageEnvironment& env, ICamInput& input) {
  return new WISPCMAudioServerMediaSubsession(env, input);
}

WISPCMAudioServerMediaSubsession
::WISPCMAudioServerMediaSubsession(UsageEnvironment& env, ICamInput& input)
  : WISServerMediaSubsession(env, input,
			     AUDIO_SAMPLE_FREQ * 8 * AUDIO_NUM_CHANS) {
}

WISPCMAudioServerMediaSubsession::~WISPCMAudioServerMediaSubsession() {
}

FramedSource* WISPCMAudioServerMediaSubsession
::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate) {
  estBitrate = fEstimatedKbps;
  return fWISInput.audioSource();
}

RTPSink* WISPCMAudioServerMediaSubsession
::createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic,
		   FramedSource* /*inputSource*/) {
  setVideoRTPSinkBufferSize();
 
	return SimpleRTPSink::createNew(envir(), rtpGroupsock, 96,
					 AUDIO_SAMPLE_FREQ, "audio", "PCMU", AUDIO_NUM_CHANS);
}

