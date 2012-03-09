# Makefile for log module
# Created by Sun @ 2012.1.7

include ../Rules.make

STATIC=0
#PLATFORM=PC

LIB_PATH = lib
LIB_NAME = log
LIB_SRC = log.c err_str.c
LIB_HEADER = log.h
LIB_OBJ=$(addprefix $(LIB_PATH)/, $(addsuffix .o, $(basename $(LIB_SRC))))

INC_DIR := $(SDK_INC_DIR)
LIB_INSTALL_DIR := $(SDK_LIB_DIR)

EXEC = logTest
SRC = log_test.c

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

CFLAGS += -Wall -I. -O2
LDFLAGS := -L$(LIB_PATH) -L. -l$(LIB_NAME)
LDFLAGS += -lpthread

CFLAGS += -I$(INC_DIR)

all: $(LIB) $(EXEC) install
	
lib: $(LIB)

test: $(EXEC)

$(LIB_PATH)/%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

ifeq ($(STATIC), 1)
$(LIB): $(LIB_OBJ)
	$(AR) -rcs $@ $^
else
$(LIB): $(LIB_OBJ)
	$(CC) -shared -o $@ $^
endif

$(EXEC): $(SRC) 
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) 
	
clean:
	@rm -vf $(LIB) $(EXEC) *.o *~ $(LIB_PATH)/*.o

install:
	cp -p $(LIB) $(LIB_INSTALL_DIR)
	cp -p $(LIB_HEADER) $(SDK_INC_DIR)
	cp -p $(EXEC) $(EXEC_INSTALL_PATH)

