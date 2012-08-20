# Sun @ 2011.10.11
ROOT_DIR = ../

include Rules.make

.PHONY: rebuild install exe

all: install

rebuild: clean all

install: 
	cp -rp etc/* $(TARGET_FILESYS_DIR)/etc
	cp -rp home/root/* $(TARGET_FILESYS_DIR)/home/root
	@mkdir -p $(TARGET_FILESYS_DIR)/home/root/backup
	@mkdir -p $(TARGET_FILESYS_DIR)/home/root/update

#	cd $(TARGET_FILESYS_DIR)/etc/rc5.d
#	-ln -s ../rc.local S99rclocal

clean:
	-rm *~
	@-rm home/root/*~
	@-rm etc/*~
	@-rm etc/init.d/*~
exe:


