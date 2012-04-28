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
// An interface to the WIS GO7007 capture device.
// C++ header

#ifndef _ICAM_INPUT_HH
#define _ICAM_INPUT_HH

#include <MediaSink.hh>
#include "media_server.h"

#define AUDIO_SAMPLE_FREQ  	16000
#define AUDIO_NUM_CHANS		1

class ICamInput: public Medium {
public:
  static ICamInput* createNew(UsageEnvironment& env, int vType);

  FramedSource* videoSource();
  FramedSource* audioSource();
  int videoWrite(MediaFrame *frame);
  int audioWrite(MediaFrame *frame);
  virtual ~ICamInput();

private:
  ICamInput(UsageEnvironment& env, int vType); // called only by createNew()
  

//  Boolean initialize(UsageEnvironment& env);
//  Boolean openFiles(UsageEnvironment& env);
//  static Boolean initALSA(UsageEnvironment& env);
//  static Boolean initV4L(UsageEnvironment& env);
//  static unsigned long getFrame(UsageEnvironment& env, unsigned char *addr, unsigned long len);
//  static void listVideoInputDevices(UsageEnvironment& env);

private:
  friend class VideoOpenFileSource;
  friend class AudioOpenFileSource;
  int videoType;
//  int fVideoFileNo;
  FramedSource* fOurVideoSource;
//  static int fOurAudioFileNo;
  FramedSource* fOurAudioSource;
};

// Functions to set the optimal buffer size for RTP sink objects.
// These should be called before each RTPSink is created.
#define AUDIO_MAX_FRAME_SIZE 20480
#define VIDEO_MAX_FRAME_SIZE 1200000
inline void setAudioRTPSinkBufferSize() { OutPacketBuffer::maxSize = AUDIO_MAX_FRAME_SIZE; }
inline void setVideoRTPSinkBufferSize() { OutPacketBuffer::maxSize = VIDEO_MAX_FRAME_SIZE; }

#endif
