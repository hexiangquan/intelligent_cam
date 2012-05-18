# Makefile for buffer module
# Created by Sun @ 2012.1.10

include ../Rules.make

STATIC ?= 0
#PLATFORM=PC

LIB_PATH = lib
LIB_NAME = buf
LIB_SRC = buffer.c buffer_pool.c
LIB_HEADER = buffer.h
LIB_OBJ=$(addprefix $(LIB_PATH)/, $(addsuffix .o, $(basename $(LIB_SRC))))

INC_DIR := -I$(SDK_INC_DIR) -I$(SDK_LIB_INC_DIR)
LIB_INSTALL_DIR := $(SDK_LIB_DIR)

EXEC = bufTest
SRC = buffer_test.o
LDLIBS :=

ifeq ($(PLATFORM), dm368)
CROSS_COMPILE ?= $(CSTOOL_PREFIX)
EXEC_INSTALL_PATH ?= $(TARGET_EXEC_DIR)
else
CROSS_COMPILE ?=
EXEC_INSTALL_PATH ?= .
endif

CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar

ifeq ($(STATIC), 1)
LIB=$(LIB_PATH)/lib$(LIB_NAME).a
else
LIB=$(LIB_PATH)/lib$(LIB_NAME).so
CFLAGS+=-fPIC
LIB_INSTALL_DIR = $(TARGET_LDLIB_DIR)
endif

CFLAGS += -Wall -I. -O2
LDFLAGS := -L$(LIB_PATH) -L$(SDK_LIB_DIR) -l$(LIB_NAME)
LDFLAGS += -lpthread
LDLIBS += $(LIB_PATH)/cmem.a470MV

CFLAGS += $(INC_DIR)

all: $(LIB) $(EXEC) install
	
lib: $(LIB) $(LIB_HEADER)

test: $(EXEC)

$(LIB_PATH)/%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

ifeq ($(STATIC), 1)
$(LIB): $(LIB_OBJ)
	$(AR) -rcs $@ $^ $(LDLIBS)
else
$(LIB): $(LIB_OBJ)
	$(CC) -shared -o $@ $^ $(LDLIBS)
endif

$(EXEC): $(SRC) 
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) 
	
clean:
	@rm -vf $(LIB) $(EXEC) *.o *~ $(LIB_PATH)/*.o

install:
	cp -p $(LIB) $(LIB_INSTALL_DIR)
	cp -p $(LIB_HEADER) $(SDK_INC_DIR)
	cp -p $(EXEC) $(EXEC_INSTALL_PATH)
	cp -p $(LIB) $(SDK_LIB_DIR)

