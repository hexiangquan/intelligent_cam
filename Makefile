# Sun @ 2011.10.11

-include Rules.make

SUBDIRS = sdk application rootfs

CLEAN_SUBDIRS = $(addsuffix .clean, $(SUBDIRS))
INSTALL_SUBDIRS = $(addsuffix .install, $(SUBDIRS))
EXE_SUBDIRS = $(addsuffix .exe, $(SUBDIRS))

.PHONY: $(SUBDIRS) $(CLEAN_SUBDIRS) $(INSTALL_SUBDIRS) $(EXE_SUBDIRS) exe tags ctags cscope

all: $(SUBDIRS)

rebuild: clean all

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
	@-rm *~

$(CLEAN_SUBDIRS):
	@cd $(basename $@) ; $(MAKE) clean

exe:	$(EXE_SUBDIRS)

$(EXE_SUBDIRS):
	@cd $(basename $@) ; $(MAKE)

tags ctags:
	@-rm ./tags
	ctags -w `find $(SUBDIRS) $(TAG_SUBDIRS) \
						-name '*.[ch]' -print`
cscope:
	find $(SUBDIRS) $(TAG_SUBDIRS) -name '*.[ch]' -print \
						> cscope.files
	cscope -b -q -k


