# Sun @ 2011.10.11

include Rules.make

SUBDIRS = log buffer net_utils ftp alg jpg_enc h264_enc osd capture msg img_convert crc cir_buf media_server

CLEAN_SUBDIRS = $(addsuffix .clean, $(SUBDIRS))
INSTALL_SUBDIRS = $(addsuffix .install, $(SUBDIRS))
LIB_SUBDIRS = $(addsuffix .lib, $(SUBDIRS))

.PHONY: $(SUBDIRS) $(CLEAN_SUBDIRS) $(INSTALL_SUBDIRS)

all: $(SUBDIRS)

$(SUBDIRS):
	@echo
	@echo Making all in subdirectory $@...
	@$(MAKE) -C $@

install: $(INSTALLSUBDIRS)

$(INSTALL_SUBDIRS):
	@echo
	@echo Executing make install in subdirectory $(basename $@)...
	@cd $(basename $@) ; $(MAKE) install

clean:	$(CLEAN_SUBDIRS)

$(CLEAN_SUBDIRS):
	@cd $(basename $@) ; $(MAKE) clean

libs:	$(LIB_SUBDIRS)

$(LIB_SUBDIRS):
	@cd $(basename $@) ; $(MAKE) lib

tags ctags:
	-rm ./tags
	ctags -w `find $(SUBDIRS) $(TAG_SUBDIRS) \
						-name '*.[ch]' -print`
cscope:
	find $(SUBDIRS) $(TAG_SUBDIRS) -name '*.[ch]' -print \
						> cscope.files
	cscope -b -q -k


