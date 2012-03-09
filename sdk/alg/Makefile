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
LIB_NAME = $(notdir $(CURDIR))
LIB_PATH = ./lib
OBJ_DIR = ./obj
LIB_SRC_PATH = ./
LIB_SRC = alg.c
LIB_OBJ = $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(basename $(LIB_SRC))))
LIB_HEADER =$(LIB_NAME).h
LIB_INSTALL_DIR := $(SDK_LIB_DIR)

# Executable target name and src
EXEC = $(LIB_NAME)_test
EXEC_SRC = $(LIB_NAME)_test.c
EXEC_OBJ = $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(basename $(EXEC_SRC))))
EXEC_INSTALL_PATH ?= $(TARGET_EXEC_DIR)

# Headers of this module
HEADERS = $(wildcard *.h) $(wildcard ../*.h)
INC_DIR := -I. -I$(SDK_INC_DIR) -I$(SDK_LIB_INC_DIR)

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

# Package path for the XDC tools
FCTOOLS_INSTALL_DIR = $(FC_INSTALL_DIR)/fctools
XDC_PATH = $(CODEC_INSTALL_DIR)/packages;$(FC_INSTALL_DIR)/packages;$(FCTOOLS_INSTALL_DIR)/packages;$(CE_INSTALL_DIR)/packages;$(XDAIS_INSTALL_DIR)/packages;$(CMEM_INSTALL_DIR)/packages;$(CETOOLS_INSTALL_DIR)/packages;$(CE_INSTALL_DIR)/examples

# Where to output configuration files
XDC_CFG		= $(TARGET)_config

# Output compiler options
XDC_CFLAGS	= $(XDC_CFG)/compiler.opt

# Output linker file
XDC_LFILE	= $(XDC_CFG)/linker.cmd

# Input configuration file
XDC_CFGFILE	= $(TARGET).cfg

# Platform (board) to build for
XDC_PLATFORM = ti.platforms.evmDM365

# Target tools
XDC_TARGET = gnu.targets.arm.GCArmv5T

export CSTOOL_DIR

# The XDC configuration tool command line
CONFIGURO = $(XDC_INSTALL_DIR)/xs xdc.tools.configuro
CONFIG_BLD := config.bld

# Link libs and flags
LD_LIBS += -l$(LIB_NAME) -lpthread -lasound
LD_FLAGS += -L$(LINUXLIBS_INSTALL_DIR)/lib -L$(LIB_PATH)

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
$(LIB): $(LIB_OBJ) $(XDC_LFILE)
	@echo
	@echo Linking $@ from $^..
	$(AR) $(AR_FLAGS) $@ $^
else
$(LIB): $(LIB_OBJ) $(XDC_LFILE)
	@echo
	@echo Linking $@ from $^..
	$(LINK) -shared -o $@ $^
endif

$(LIB_OBJ):	$(OBJ_DIR)/%.o: %.c $(HEADERS) $(XDC_CFLAGS)
	@echo Compiling $@ from $<..
	$(COMPILE) $(shell cat $(XDC_CFLAGS)) -o $@ $<

$(EXEC): $(EXEC_OBJ) $(XDC_LFILE)
	$(LINK) -o $@ $^ $(LD_LIBS)

$(EXEC_OBJ): $(OBJ_DIR)/%.o: %.c $(HEADERS)
	$(COMPILE) $(shell cat $(XDC_CFLAGS)) -o $@ $<

$(XDC_LFILE) $(XDC_CFLAGS):	$(XDC_CFGFILE)
	@echo
	@echo ======== Building $(TARGET) ========
	@echo Configuring application using $<
	@echo
	$(VERBOSE) XDCPATH="$(XDC_PATH)" $(CONFIGURO) -o $(XDC_CFG) -t $(XDC_TARGET) -p $(XDC_PLATFORM) -b $(CONFIG_BLD) $(XDC_CFGFILE)
	@mkdir -p $(LIB_PATH)
	@mkdir -p $(OBJ_DIR)

#install_$(TARGET):
install:
	-cp -p $(LIB) $(LIB_INSTALL_DIR)
	-cp -p $(LIB) $(SDK_LIB_DIR)
	-cp -p $(LIB_HEADER) $(SDK_INC_DIR)
	-cp -p $(EXEC) $(EXEC_INSTALL_PATH)

clean:
	@echo Removing generated files..
	$(VERBOSE) -$(RM) -rf $(XDC_CFG) $(EXEC) $(OBJ_DIR) $(LIB_PATH) *~ *.d .dep
