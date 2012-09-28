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
// Implementation

#include "ICamInput.hh"
//#include "Options.hh"
#include "Err.hh"
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/poll.h>
#include <linux/soundcard.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "log.h"
#include "circular_buf.h"

//#define READ_TEST_FILE	1
#define TEST_FILE_NAME	"420.h264"

#define CIR_RD_TIME_PRD	5		//ms
#define CIR_WR_TIMEOUT	2000	// 2s
#define CIR_RD_TIMEOUT	1000	// 1s
#define CIR_MAGIC_CODE	0x51BADDAD


////////// OpenFileSource definition //////////

// A common "FramedSource" subclass, used for reading from an open file:

class OpenFileSource: public FramedSource {
public:
  int  uSecsToDelay;
  int  uSecsToDelayMax;
  int  srcType;
protected:
  OpenFileSource(UsageEnvironment& env, ICamInput& input);
  virtual ~OpenFileSource();

  virtual int readFromFile() = 0;
  
private: // redefined virtual functions:
  virtual void doGetNextFrame();

private:
  static void incomingDataHandler(OpenFileSource* source);
  void incomingDataHandler1();

protected:
  ICamInput& fInput;
  
};


////////// VideoOpenFileSource definition //////////

class VideoOpenFileSource: public OpenFileSource {
public:
  VideoOpenFileSource(UsageEnvironment& env, ICamInput& input);
  virtual ~VideoOpenFileSource();
  void freeSource();
  int writeToBuf(MediaFrame *frame);

protected: // redefined virtual functions:
  virtual int readFromFile();
  
private:
#ifdef READ_TEST_FILE
	FILE *fp;
#else
	Bool isOpened;
	static CirBufHandle  hCirBuf;
	Int32 errCnt;
#endif
};



////////// AudioOpenFileSource definition //////////

class AudioOpenFileSource: public OpenFileSource {
private:
	Bool isOpened;
	static CirBufHandle  hCirBuf;

public:
  AudioOpenFileSource(UsageEnvironment& env, ICamInput& input);
  virtual ~AudioOpenFileSource();
  void freeSource();
  int writeToBuf(MediaFrame *frame);

protected: // redefined virtual functions:
  virtual int readFromFile();
};

////////// ICamInput implementation //////////

ICamInput* ICamInput::createNew(UsageEnvironment& env, int vType) {
  return new ICamInput(env, vType);
}

FramedSource* ICamInput::videoSource() {

  if (fOurVideoSource == NULL) {
    fOurVideoSource = new VideoOpenFileSource(envir(), *this);
  }

  return fOurVideoSource;
}

FramedSource* ICamInput::audioSource() {

  if (fOurAudioSource == NULL) {
    fOurAudioSource = new AudioOpenFileSource(envir(), *this);
  }

  return fOurAudioSource;
}

ICamInput::ICamInput(UsageEnvironment& env, int vType)
  : Medium(env), videoType(vType), fOurVideoSource(NULL), fOurAudioSource(NULL) {
}

ICamInput::~ICamInput() {
 if( fOurVideoSource ) {
 	((VideoOpenFileSource *)fOurVideoSource)->freeSource();
 	delete (VideoOpenFileSource *)fOurVideoSource;
 	fOurVideoSource = NULL;
 }
 
 if( fOurAudioSource ) {
 	((AudioOpenFileSource *)fOurAudioSource)->freeSource();
 	delete (AudioOpenFileSource *)fOurAudioSource;
 	fOurAudioSource = NULL;
 }	
}

int ICamInput::videoWrite(MediaFrame * frame) {
	if(!fOurVideoSource)
		return E_MODE;

	return ((VideoOpenFileSource *)fOurVideoSource)->writeToBuf(frame);
}

int ICamInput::audioWrite(MediaFrame * frame) {
	if(!fOurAudioSource)
		return E_MODE;

	return ((AudioOpenFileSource *)fOurAudioSource)->writeToBuf(frame);
}

int ICamInput::isOpened() {
	return fOurVideoSource ? 1 : 0;
}

////////// OpenFileSource implementation //////////

OpenFileSource
::OpenFileSource(UsageEnvironment& env, ICamInput& input)
  : FramedSource(env), fInput(input) {
		return;
}

OpenFileSource::~OpenFileSource() {
	return;
}

void OpenFileSource::doGetNextFrame() {
	incomingDataHandler(this);
}

void OpenFileSource
::incomingDataHandler(OpenFileSource* source) {
  source->incomingDataHandler1();
}

void OpenFileSource::incomingDataHandler1() {
	int ret;

	if (!isCurrentlyAwaitingData()) return; // we're not ready for the data yet

	ret = readFromFile();
	if (ret < 0) {
		handleClosure(this);
		DBG("In Grab Image, the source stops being readable!!!!\n");
	} else if (ret == 0)  {

		if( uSecsToDelay >= uSecsToDelayMax ) {
			uSecsToDelay = uSecsToDelayMax;
		} else {
			uSecsToDelay *= 2; 
		}
		nextTask() = envir().taskScheduler().scheduleDelayedTask(uSecsToDelay,
			      (TaskFunc*)incomingDataHandler, this);
	} else {

		nextTask() = envir().taskScheduler().scheduleDelayedTask(0, (TaskFunc*)afterGetting, this);
	}
}


////////// VideoOpenFileSource implementation //////////
CirBufHandle VideoOpenFileSource::hCirBuf = NULL;

VideoOpenFileSource
::VideoOpenFileSource(UsageEnvironment& env, ICamInput& input)
  : OpenFileSource(env, input)
{

	DBG("video file source opened...");
	uSecsToDelay = 5000;
	uSecsToDelayMax = 33000;
	srcType = 0;
	errCnt = 0;

	#ifdef READ_TEST_FILE
	fp = fopen(TEST_FILE_NAME, "rb");
	if(!fp) {
		ERR("open %s failed", TEST_FILE_NAME);
	}
	#else
	if(!hCirBuf) {
		//DBG("create new cir buf");
		hCirBuf = circular_buf_create(VIDEO_MAX_FRAME_SIZE * 3, CIR_RD_TIME_PRD);
	} else {
		//DBG("flush cir buffer");
		circular_buf_flush(hCirBuf, -1);	//flush previous write data
	}
	isOpened = TRUE;
	#endif
	
}

VideoOpenFileSource::~VideoOpenFileSource() {
	fInput.fOurVideoSource = NULL;
	DBG("video file source closed.");
	#ifdef READ_TEST_FILE
	if(fp)
		fclose(fp);
	fp = NULL;
	#else
	isOpened = FALSE;
	#endif
}

void VideoOpenFileSource::freeSource()
{
	if(hCirBuf)
		circular_buf_delete(hCirBuf);
	hCirBuf = NULL;
}

#ifdef READ_TEST_FILE
int NAL_Search(FILE *fp, int offset) {
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

int GetFileFrame(FILE *fp, void *buf, unsigned *size) {
	static int offset = 0;
	int offset1 = 0;
	int offset2 = 0;
	int framesize = 0;

	if( fp == NULL )
		return -1;
	
	fseek(fp, offset, SEEK_SET);
	
	offset1 = NAL_Search(fp, offset);
	if(offset1 < 0) {
		/* got the end of the file */
		fseek(fp, 0, SEEK_SET);
		offset = 0;
		return -1;
	}
	offset2 = NAL_Search(fp, offset1+4);
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
#endif

static Int32 mediaFrameWrite(CirBufHandle hCirBuf, MediaFrame *frame)
{
	/* wait enough buffer availabe */
	int err;

	if(!hCirBuf || !frame)
		return E_INVAL;
	
	err = circular_buf_wait_ready(hCirBuf, FALSE, frame->dataLen + sizeof(MediaFrame), 
				CIR_WR_TIMEOUT);
	if(err)
		return err;
	
	/* write to circular buffer */
	frame->reserved = CIR_MAGIC_CODE;
	circular_buf_write(hCirBuf, frame, sizeof(MediaFrame), CIR_WR_TIMEOUT);
	err = circular_buf_write(hCirBuf, frame->data, frame->dataLen, CIR_WR_TIMEOUT);

	return err;
}

int VideoOpenFileSource::readFromFile() {
#ifdef READ_TEST_FILE
	int ret;

	ret = GetFileFrame(fp, fTo, &fFrameSize);
	if(ret < 0)
		return 0;
	
	if (fFrameSize > fMaxSize) {
		DBG("Frame Truncated");
		DBG("fFrameSize = %d\n",fFrameSize);
		DBG("fMaxSize = %d\n",fMaxSize);
		fNumTruncatedBytes = fFrameSize - fMaxSize;
		fFrameSize = fMaxSize;
	} else {
		fNumTruncatedBytes = 0;
	}
			
	// Note the timestamp and size:
	gettimeofday(&fPresentationTime, NULL);
	usleep(30000);
	return 1;
#else
	int 		err;
	MediaFrame 	frame;

	err = circular_buf_wait_ready(hCirBuf, TRUE, sizeof(MediaFrame), CIR_WR_TIMEOUT);
	if(err) {
		return ((++errCnt > 5) ? (-1) : 0);
	}
	
	/* read frame header */
	err = circular_buf_read(hCirBuf, &frame, sizeof(MediaFrame), CIR_WR_TIMEOUT);
	if(err || frame.reserved != CIR_MAGIC_CODE) {
		ERR("read frame header failed.");
		++errCnt;
		return 0;
	}

	//Int32 rdLen;

	/* read data */
	if ((unsigned )frame.dataLen > fMaxSize) {
		DBG("Frame Truncated");
		DBG("fFrameSize = %d",fFrameSize);
		DBG("fMaxSize = %d",fMaxSize);
		fNumTruncatedBytes = frame.dataLen - fMaxSize;
		fFrameSize = fMaxSize;
		//rdLen = ROUND_UP(fFrameSize, CIR_LEN_ALIGN);
		err = circular_buf_read(hCirBuf, fTo, fFrameSize, CIR_WR_TIMEOUT);
		/* clear left data */
		char *buf = (char *)malloc(fNumTruncatedBytes);
		circular_buf_read(hCirBuf, buf, fNumTruncatedBytes, 0);
		free(buf);
	} else {
		fNumTruncatedBytes = 0;
		fFrameSize = frame.dataLen;
		//rdLen = ROUND_UP(frame.dataLen, CIR_LEN_ALIGN);
		err = circular_buf_read(hCirBuf, fTo, fFrameSize, CIR_WR_TIMEOUT);
	}

	fPresentationTime = frame.timestamp;
	errCnt = 0;
	return err ? 0 : 1;
#endif	
	
}

int VideoOpenFileSource::writeToBuf(MediaFrame * frame)
{
	/* playing is not started yet, just return */
	if(!isOpened)
		return E_MODE;

	int err = mediaFrameWrite(hCirBuf, frame);
	return err;
}

////////// AudioOpenFileSource implementation //////////
CirBufHandle AudioOpenFileSource::hCirBuf = NULL;

AudioOpenFileSource
::AudioOpenFileSource(UsageEnvironment& env, ICamInput& input)
  : OpenFileSource(env, input) {
	uSecsToDelay = 5000;
	uSecsToDelayMax = 125000;
	srcType = 1;
	isOpened = TRUE;
	//if(!hCirBuf)
		//circular_buf_create(AUDIO_MAX_FRAME_SIZE * 3, CIR_RD_TIME_PRD);
}

AudioOpenFileSource::~AudioOpenFileSource() {
  fInput.fOurAudioSource = NULL;
}

int AudioOpenFileSource::readFromFile() {
#ifdef READ_TEST_FILE
	int timeinc;

	// Read available audio data:
	int ret = 0;

	if (ret <= 0) return 0;
	if (ret < 0) ret = 0;
	fFrameSize = (unsigned)ret;
	fNumTruncatedBytes = 0;

	/* PR#2665 fix from Robin
	* Assuming audio format = AFMT_S16_LE
	* Get the current time
	* Substract the time increment of the audio oss buffer, which is equal to
	* buffer_size / channel_number / sample_rate / sample_size ==> 400+ millisec
	*/
	timeinc = fFrameSize * 1000 / AUDIO_NUM_CHANS / (AUDIO_SAMPLE_FREQ/1000) ;/// 2;
	while (fPresentationTime.tv_usec < timeinc) {
		fPresentationTime.tv_sec -= 1;
		timeinc -= 1000000;
	}

	fPresentationTime.tv_usec -= timeinc;
#endif
	return 0;
}

void AudioOpenFileSource::freeSource() {
	if(hCirBuf)
		circular_buf_delete(hCirBuf);
	hCirBuf = NULL;
}

int AudioOpenFileSource::writeToBuf(MediaFrame * frame) {
	/* playing is not started yet, just return */
	if(!isOpened)
		return 0;

	int err = mediaFrameWrite(this->hCirBuf, frame);
	return err;
}


