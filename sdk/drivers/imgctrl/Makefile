#
#  Copyright 2012 by S.K. Sun
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; version 2 of the License.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>
#
#

ROOTDIR = ../..

# We conditionally include $(ROOTDIR)/Rules.make because when the
# kernel make system invokes us, it won't be found... but that's okay.
-include $(ROOTDIR)/Rules.make

MOD_NAME = img_ctrl

TARGET = $(MOD_NAME).ko
PWD := $(shell pwd)

obj-m += $(MOD_NAME).o
#$(MOD_NAME)-objs := img_ctrl.o

EXTRA_CFLAGS +=	-DUSE_UDEV=$(USE_UDEV)
EXEC_DIR ?= $(TARGET_MOD_DIR)/drivers/dsp
INC_DIR = -I$(PWD)/../include
MAKE_ENV = ARCH=arm CROSS_COMPILE=$(CSTOOL_PREFIX)

# For module test
EXEC = imgCtrlTest
SRC = $(MOD_NAME)_test.c
HEADERS := $(PWD)/../include/img_ctrl.h
CC = $(CSTOOL_PREFIX)gcc
EXEC_INSTALL_PATH ?= $(TARGET_EXEC_DIR)

EXTRA_CFLAGS += $(INC_DIR)

INC_DIR += -I$(SDK_INC_DIR) -I$(LINUXKERNEL_INSTALL_DIR)/include

C_FLAGS += -Wall -I. -g
C_FLAGS += $(INC_DIR)
C_FLAGS += -D_DEBUG

LD_FLAGS := -L$(LIB_PATH) -L$(SDK_LIB_DIR)
LD_FLAGS += -lpthread

debug: EXTRA_CFLAGS += -D_DEBUG 


.PHONY: clean debug release install default test
	
default: debug test install

docs:

release:
	make -C $(LINUXKERNEL_INSTALL_DIR) M=`pwd` $(MAKE_ENV) \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" modules

debug:
	make -C $(LINUXKERNEL_INSTALL_DIR) M=`pwd` $(MAKE_ENV) \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" modules

test: $(EXEC)

$(EXEC): $(HEADERS) $(SRC)
	$(CC) $(C_FLAGS) -o $@ $^ $(LD_FLAGS) 

install:
	install -d $(EXEC_DIR)
	install $(TARGET) $(EXEC_DIR)
	-cp -p $(EXEC) $(EXEC_INSTALL_PATH)
	-cp -p $(HEADERS) $(SDK_INC_DIR)

clean:
	-rm -rf *.o *mod* *.ko .cmem* .tmp*

rebuild: clean default

#
#  @(#) ti.sdo.linuxutils.cmem; 2, 2, 0,142; 11-30-2010 18:31:30; /db/atree/library/trees/linuxutils/linuxutils-j02x/src/ xlibrary

#

