# Makefile for buffer module
# Created by Sun @ 2012.1.10

include ../Rules.make

EXEC = bin
SRC_DIR = ntp-4.2.6p5

ifeq ($(PLATFORM), dm368)
CROSS_COMPILE ?= $(CSTOOL_PREFIX)
EXEC_INSTALL_PATH ?= $(TARGET_FILESYS_DIR)/usr/bin
HOST=$(CSTOOL_DIR)/bin/arm-none-linux-gnueabi
else
CROSS_COMPILE ?=
EXEC_INSTALL_PATH=$(PRJ_DIR)/sdk/ntp
HOST=
endif

CC = $(CROSS_COMPILE)gcc

all: $(EXEC) install

$(EXEC):
	./(SRC_DIR)/configue --prefix=$(EXEC_INSTALL_PATH) --exec-prefix=$(EXEC_INSTALL_PATH) --host=$(HOST)
	make -C$(SRC_DIR) CC=$(CC)
	
clean:
	make -C$(SRC_DIR) clean
	make -C$(SRC_DIR) distclean

install:
	make -C$(SRC_DIR) install
