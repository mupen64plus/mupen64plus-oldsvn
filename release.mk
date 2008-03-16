# Makefile for building Mupen64Plus releases

# check version number of this build
ifeq ("$(VER)", "")
  $(error Must give version parameter, ie: make VER=1.5.1)
else
  MODVER = $(shell echo "$(VER)" | sed 's/\./-/g')
endif

# create folder/zip names
SRCNAME = Mupen64Plus-$(MODVER)-src
BINNAME_32 = Mupen64Plus-$(MODVER)-bin-32
BINNAME_64 = Mupen64Plus-$(MODVER)-bin-64

# set primary build objects
ALL = src bin-32 bin-64

# build targets
targets:
	@echo "Mupen64Plus release makefile. "
	@echo "  Targets:"
	@echo "    all       == Build Mupen64Plus source zip, plus 32-bit and 64-bit binary zips"
	@echo "    src       == build Mupen64Plus source package (zip format)"
	@echo "    bin-32    == build 32-bit binary package (zip format)"
	@echo "    bin-64    == build 64-bit binary package (zip format)"
	@echo "  Options:"
	@echo "    VER=<ver> == (required) Sets version number of build (VER=1.0.3)"

all: $(ALL)

src: FORCE
	# clean objects from source tree
	$(MAKE) -f ./Makefile clean
	# remove source tree and zip file if they exist, then make empty directory for source tree
	rm -rf ../$(SRCNAME)
	mkdir ../$(SRCNAME)
	# get list of directiories in mupen64 source tree, excluding SVN stuff
	rm -f ../dirs.tmp
	find . -mindepth 1 -type d -a ! -regex '.*\.svn.*' > ../dirs.tmp
	# create the directories
	cat ../dirs.tmp | while read dirname; do relpath=$${dirname#*/}; mkdir "../$(SRCNAME)/$$relpath"; done
	rm ../dirs.tmp
	# get list of files in mupen64 source tree, excluding SVN stuff
	rm -f ../files.tmp
	find . -mindepth 1 -type f -a ! -regex '.*\.svn.*' > ../files.tmp
	# copy the files
	cat ../files.tmp | while read filename; do relpath=$${filename#*/}; cp "./$$relpath" "../$(SRCNAME)/$$relpath"; done
	rm ../files.tmp
	# delete some unnecessary stuff if present
	rm -f ../$(SRCNAME)/plugins/*.so
	rm -f ../$(SRCNAME)/plugins/*~
	rm -rf ../$(SRCNAME)/save/*
	rm -f ../$(SRCNAME)/*~
	rm -f ../$(SRCNAME)/*.cache
	# zip it up and delete the directory
	cd .. ; zip -r $(SRCNAME).zip $(SRCNAME)
	rm -rf ../$(SRCNAME)

bin-32: FORCE
	$(MAKE) -f ./Makefile clean
	$(MAKE) -f ./Makefile all BITS=32
	# remove binary tree and zip file if they exist, then make empty directory for binary tree
	rm -rf ../$(BINNAME_32)
	rm -f ../$(BINNAME_32).zip
	mkdir ../$(BINNAME_32)
	mkdir ../$(BINNAME_32)/icons
	mkdir ../$(BINNAME_32)/plugins
	mkdir ../$(BINNAME_32)/save
	# copy files into binary structure
	cp ./*.conf ../$(BINNAME_32)/
	cp ./*.TXT ../$(BINNAME_32)/
	cp ./mupen64plus.ini ../$(BINNAME_32)/
	cp ./mupen64plus ../$(BINNAME_32)/
	cp ./icons/* ../$(BINNAME_32)/icons
	cp ./plugins/*.so ../$(BINNAME_32)/plugins
	cp ./plugins/RiceVideo*.* ../$(BINNAME_32)/plugins
	cp ./plugins/Glide64.ini ../$(BINNAME_32)/plugins
	cp ./README ../$(BINNAME_32)/README
	cp ./RELEASE ../$(BINNAME_32)/RELEASE
	# zip it up and delete the directory
	cd .. ; zip -r $(BINNAME_32).zip $(BINNAME_32)
	rm -rf ../$(BINNAME_32)

bin-64: FORCE
	$(MAKE) -f ./Makefile clean
	$(MAKE) -f ./Makefile all
	# remove binary tree and zip file if they exist, then make empty directory for binary tree
	rm -rf ../$(BINNAME_64)
	rm -f ../$(BINNAME_64).zip
	mkdir ../$(BINNAME_64)
	mkdir ../$(BINNAME_64)/icons
	mkdir ../$(BINNAME_64)/plugins
	mkdir ../$(BINNAME_64)/save
	# copy files into binary structure
	cp ./*.conf ../$(BINNAME_64)/
	cp ./*.TXT ../$(BINNAME_64)/
	cp ./mupen64plus.ini ../$(BINNAME_64)/
	cp ./mupen64plus ../$(BINNAME_64)/
	cp ./icons/* ../$(BINNAME_64)/icons
	cp ./plugins/*.so ../$(BINNAME_64)/plugins
	cp ./plugins/RiceVideo*.* ../$(BINNAME_64)/plugins
	cp ./plugins/Glide64.ini ../$(BINNAME_64)/plugins
	cp ./README ../$(BINNAME_64)/README
	cp ./RELEASE ../$(BINNAME_64)/RELEASE
	# zip it up and delete the directory
	cd .. ; zip -r $(BINNAME_64).zip $(BINNAME_64)
	rm -rf ../$(BINNAME_64)

FORCE:

