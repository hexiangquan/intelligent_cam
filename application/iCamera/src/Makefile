#
# Makefile for making the iStream application
#
# Copyright (C) 2010  Incorporated  
# 
# 
#  Redistribution and use in source and binary forms, with or without 
#  modification, are permitted provided that the following conditions 
#  are met:
#
#    Redistributions of source code must retain the above copyright 
#    notice, this list of conditions and the following disclaimer.
#
#    Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the 
#    documentation and/or other materials provided with the   
#    distribution.
#
#    Neither the name of Texas Instruments Incorporated nor the names of
#    its contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
#  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
#  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
#  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

APP_ROOT = ../..

-include $(APP_ROOT)/Rules.make

TARGET = $(notdir $(CURDIR))
STATIC ?= 0

# Executable target name and src
OBJ_DIR ?= ../build/obj
BIN_DIR ?= ../build

EXEC ?= $(BIN_DIR)/iCamera 
EXEC_SRC = $(wildcard *.c)
EXEC_OBJ = $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(basename $(EXEC_SRC))))
EXEC_INSTALL_PATH := $(TARGET_EXEC_DIR)

# dirs for libs
LIB_DIR = ./ctrl_inf

# Headers depended
APP_INC = ../include
HEADERS = $(wildcard *.h) $(wildcard $(APP_INC)/*.h) $(wildcard $(SDK_INC_DIR)/*.h)
INC_DIR := -I. -I$(SDK_INC_DIR) -I$(APP_INC) -I$(APP_INC_DIR)
COMMON_HEADER = $(APP_INC)/cam_*.h

# Complie options
C_FLAGS += -Wall -DDM365 -O2 $(INC_DIR)
CPP_FLAGS ?= 

# Link libs and options
LD_FLAGS += -L$(SDK_LIB_DIR)
LD_LIBS := -lpthread -lm -lbuf -lalg -lcapture -lcrc -lmsg -limgconv
LD_LIBS += -ljpgenc -llog -losd -lh264enc -lnetproto -lnetutils -lftp
LD_LIBS += -lmediasrv -lcirbuf -ldisplay -lntp -lsyscommu -lfileutils
ifeq ($(STATIC), 1)
LD_LIBS += $(XDC_LFILE)
endif

# Comment this out if you want to see full compiler and linker output.
#override VERBOSE = @

# Complier, archive and link commands
COMPILE = $(VERBOSE) $(CSTOOL_PREFIX)gcc $(CPP_FLAGS) $(C_FLAGS) $(CPP_FLAGS) -c
LINK = $(VERBOSE) $(CSTOOL_PREFIX)g++ $(LD_FLAGS)
AR = $(VERBOSE) $(CSTOOL_PREFIX)ar

.PHONY: clean install exe rebuild

all: exe install

rebuild: clean all

exe: $(EXEC)

$(EXEC): $(EXEC_OBJ)
	$(LINK) -o $@ $^ $(LD_LIBS)

$(EXEC_OBJ): $(OBJ_DIR)/%.o: %.c $(HEADERS)
	@-mkdir -p $(OBJ_DIR)
	$(COMPILE) -o $@ $<

lib:
	make -C $(LIB_DIR)

lib_install:
	make -C $(LIB_DIR) install

lib_clean:
	make -C $(LIB_DIR) clean

install: 
	-cp -f $(EXEC) $(EXEC_INSTALL_PATH)
	-cp -f $(COMMON_HEADER) $(APP_INC_DIR)

clean: 
	@echo Removing generated files..
	$(VERBOSE) -rm -rf $(EXEC) $(OBJ_DIR) *~ *.d .dep
