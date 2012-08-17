#
# Makefile for making the encode demos on DM365 platform
#
# Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/ 
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

ROOTDIR = ..
TARGET = $(notdir $(CURDIR))

-include $(ROOTDIR)/Rules.make

# Static lib or dynamic linked lib
STATIC ?= 0

# Lib src and path
LIB_NAME = imgconv
LIB_PATH = ./lib
OBJ_DIR = ./obj
LIB_SRC_PATH = ./
LIB_SRC = img_convert.c resize.c previewer.c
LIB_OBJ = $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(basename $(LIB_SRC))))
LIB_HEADER = img_convert.h
LIB_INSTALL_DIR := .

# Executable target name and src
EXEC = $(LIB_NAME)Test
EXEC_SRC = img_convert_test.c
EXEC_OBJ = $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(basename $(EXEC_SRC))))
EXEC_INSTALL_PATH ?= $(TARGET_EXEC_DIR)
TEST_INPUT = $(wildcard *.yuv)

# Headers of this module
HEADERS = $(wildcard *.h) $(wildcard ../*.h)
INC_DIR := -I. -I$(SDK_INC_DIR) -I$(SDK_LIB_INC_DIR) -I$(LINUXKERNEL_INSTALL_DIR)/include

C_FLAGS += -Wall -DDM365 -O2 $(INC_DIR)
CPP_FLAGS ?= 
AR_FLAGS += -rcs 

ifeq ($(STATIC), 1)
LIB=$(LIB_PATH)/lib$(LIB_NAME).a
else
LIB=$(LIB_PATH)/lib$(LIB_NAME).so
C_FLAGS+=-fPIC
LIB_INSTALL_DIR = $(TARGET_LDLIB_DIR)
endif

# Comment this out if you want to see full compiler and linker output.
#override VERBOSE = @

# File for XDC link
XDC_LFILE ?= $(ALG_XDC_LFILE)
XDC_CFLAGS ?= $(ALG_XDC_CFLAGS)

# Link libs and flags
LD_LIBS += -l$(LIB_NAME) -lpthread -lasound -lalg -lcapture -ljpgenc -lbuf -lm -lmsg $(XDC_LFILE)
LD_FLAGS += -L$(LINUXLIBS_INSTALL_DIR)/lib -L$(LIB_PATH) -L$(SDK_LIB_DIR)

# Complier, archive and link commands
COMPILE = $(VERBOSE) $(CSTOOL_PREFIX)gcc $(CPP_FLAGS) $(C_FLAGS) $(CPP_FLAGS) -c
LINK = $(VERBOSE) $(CSTOOL_PREFIX)gcc $(LD_FLAGS)
AR = $(VERBOSE) $(CSTOOL_PREFIX)ar

.PHONY: clean install lib test rebuild 

all: lib test install

rebuild: clean all

lib: $(LIB)

test: $(EXEC) $(LIB)

#install: $(if $(wildcard $(TARGET)), install_$(TARGET))

ifeq ($(STATIC), 1)
$(LIB): $(LIB_OBJ)
	@echo
	@echo Linking $@ from $^..
	@mkdir -p $(LIB_PATH)
	$(AR) $(AR_FLAGS) $@ $^
else
$(LIB): $(LIB_OBJ)
	@echo
	@echo Linking $@ from $^..
	@mkdir -p $(LIB_PATH)
	$(LINK) -shared -o $@ $^
endif

$(LIB_OBJ):	$(OBJ_DIR)/%.o: %.c $(HEADERS)
	@echo Compiling $@ from $<..
	@mkdir -p $(OBJ_DIR)
	$(COMPILE) -o $@ $<

$(EXEC): $(EXEC_OBJ) $(LIB)
	$(LINK) -o $@ $^ $(LD_LIBS)

$(EXEC_OBJ): $(OBJ_DIR)/%.o: %.c $(HEADERS)
	$(COMPILE) -o $@ $<

#install_$(TARGET):
install:
	-cp -p $(LIB) $(LIB_INSTALL_DIR)
	-cp -p $(LIB) $(SDK_LIB_DIR)
	-cp -p $(LIB_HEADER) $(SDK_INC_DIR)
	-cp -p $(EXEC) $(EXEC_INSTALL_PATH)
	#-cp -p $(TEST_INPUT) $(EXEC_INSTALL_PATH)

clean:
	@echo Removing generated files..
	$(VERBOSE) -$(RM) -rf $(EXEC) $(OBJ_DIR) $(LIB_PATH) *~ *.d .dep
