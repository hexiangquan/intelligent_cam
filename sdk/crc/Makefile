# Makefile for buffer module
# Created by Sun @ 2012.1.10

include ../Rules.make

STATIC=0
#PLATFORM=PC

LIB_PATH = lib
LIB_NAME = crc
LIB_SRC = crc16.c
LIB_HEADER = $(addsuffix .h, $(basename $(LIB_SRC)))
LIB_OBJ=$(addprefix $(LIB_PATH)/, $(addsuffix .o, $(basename $(LIB_SRC))))

INC_DIR := $(SDK_INC_DIR)
LIB_INSTALL_DIR := $(SDK_LIB_DIR)

EXEC = $(LIB_NAME)Test
SRC = $(LIB_NAME)_test.o

ifeq ($(PLATFORM), dm368)
CROSS_COMPILE ?= $(CSTOOL_PREFIX)
EXEC_INSTALL_PATH ?= $(TARGET_EXEC_DIR)
else
CROSS_COMPILE ?=
EXEC_INSTALL_PATH ?= .
endif

CC = $(CROSS_COMPILE)gcc

ifeq ($(STATIC), 1)
LIB=$(LIB_PATH)/lib$(LIB_NAME).a
else
LIB=$(LIB_PATH)/lib$(LIB_NAME).so
CFLAGS+=-fPIC
LIB_INSTALL_DIR = $(TARGET_LDLIB_DIR)
endif

CFLAGS += -Wall -I. -g
LDFLAGS := -L$(LIB_PATH) -L$(LIB_INSTALL_DIR) -L$(SDK_LIB_DIR)
LDFLAGS += -lpthread -l$(LIB_NAME)

CFLAGS += -I$(INC_DIR)

all: $(LIB) install
	
lib: $(LIB)

test: $(EXEC)

$(LIB_PATH)/%.o: %.c
	-mkdir -p $(LIB_PATH)
	$(CC) $(CFLAGS) -o $@ -c $<

ifeq ($(STATIC), 1)
$(LIB): $(LIB_OBJ)
	$(AR) -rcs $@ $^
else
$(LIB): $(LIB_OBJ)
	$(CC) -shared -o $@ $^
endif

clean:
	@rm -vf $(LIB) $(EXEC) *.o *~ $(LIB_PATH)/*.o

install:
	cp -p $(LIB) $(LIB_INSTALL_DIR)
	cp -p $(LIB_HEADER) $(SDK_INC_DIR)
	cp -p $(LIB) $(SDK_LIB_DIR)

