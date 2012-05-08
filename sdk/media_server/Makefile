ROOTDIR = ..
include $(ROOTDIR)/Rules.make

LIVE_DIR = $(ROOTDIR)/open/live
EXEC = mediaSrvTest
LIB_NAME = mediasrv
LIB_HEADER = media_server.h
#wis-streamer
#PLATFORM=PC
TEST_FILE = 420.h264
STATIC=0

ifeq ($(PLATFORM), dm368)
CROSS_COMPILE =	$(CSTOOL_PREFIX)
EXEC_DIR = $(TARGET_EXEC_DIR)
else
CROSS_COMPILE ?=
endif

CC = $(CROSS_COMPILE)gcc
CPLUSPLUS = $(CROSS_COMPILE)c++
PUBLIC_INCLUDE_DIR = $(ROOTDIR)/../common/include

INCLUDES = -I . \
	-I$(LIVE_DIR)/BasicUsageEnvironment/include \
	-I$(LIVE_DIR)/UsageEnvironment/include \
	-I$(LIVE_DIR)/groupsock/include \
	-I$(LIVE_DIR)/liveMedia/include \
	-I$(PUBLIC_INCLUDE_DIR)

CFLAGS = $(INCLUDES) -D_LINUX -g -Wall -O3

LIBS = \
	-L$(LIVE_DIR)/BasicUsageEnvironment -lBasicUsageEnvironment \
	-L$(LIVE_DIR)/UsageEnvironment -lUsageEnvironment \
	-L$(LIVE_DIR)/groupsock -lgroupsock \
	-L$(LIVE_DIR)/liveMedia -lliveMedia 

#wis-streamer.o
LIB_OBJS =  Err.o ICamInput.o \
	WISServerMediaSubsession.o \
	WISJPEGStreamSource.o \
	WISJPEGVideoServerMediaSubsession.o \
	WISMPEG4VideoServerMediaSubsession.o \
	WISH264VideoServerMediaSubsession.o \
	WISPCMAudioServerMediaSubsession.o \
	media_server.o

EXEC_OBJS = media_server_test.o

ifeq ($(STATIC), 1)
LIB=lib$(LIB_NAME).a
LIB_INSTALL_DIR = .
else
LIB=lib$(LIB_NAME).so
CFLAGS+=-fPIC
LIB_INSTALL_DIR = $(TARGET_LDLIB_DIR)
endif

all: $(LIB)	$(EXEC)

ifeq ($(STATIC), 1)
$(LIB): $(LIB_OBJS)
	$(AR) -rcs $@ $^ $(LIBS)
else
$(LIB): $(LIB_OBJS)
	$(CC) -shared -o $@ $^ $(LIBS)
endif

EXEC_LIBS = -L. -l$(LIB_NAME) \
			-L$(SDK_LIB_DIR) -lcirbuf \
			-lpthread

$(EXEC): $(EXEC_OBJS)
	$(CPLUSPLUS) $(CFLAGS) -o $(EXEC) $(EXEC_OBJS) $(EXEC_LIBS)

media_server.cpp:		media_server.h
media_server_test.c:	media_server.h

#wis-streamer.cpp:		Err.hh

Err.cpp:				Err.hh

ICamInput.cpp:			ICamInput.hh Err.hh

WISServerMediaSubsession.cpp:		WISServerMediaSubsession.hh

WISServerMediaSubsession.hh:		ICamInput.hh
WISMPEG4VideoServerMediaSubsession.hh:	WISServerMediaSubsession.hh

WISMPEG4VideoServerMediaSubsession.cpp:	WISMPEG4VideoServerMediaSubsession.hh

WISH264VideoServerMediaSubsession.hh:	WISServerMediaSubsession.hh

WISH264VideoServerMediaSubsession.cpp:	WISH264VideoServerMediaSubsession.hh

WISJPEGStreamSource.cpp:		WISJPEGStreamSource.hh
WISPCMAudioServerMediaSubsession.cpp:	WISPCMAudioServerMediaSubsession.hh

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

.cpp.o:
	$(CPLUSPLUS) -c $(CFLAGS) $< -o $@


install:	$(EXEC)
	install $(EXEC) $(EXEC_DIR)
	install $(LIB) $(LIB_INSTALL_DIR)
	install $(LIB_HEADER) $(SDK_INC_DIR)
	install $(LIB) $(SDK_LIB_DIR)
	-cp -p $(TEST_FILE) $(EXEC_DIR) 

clean:
	rm -f *.o *~
	rm -f $(EXEC)
