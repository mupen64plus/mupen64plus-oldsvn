#/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
# *   Mupen64plus - Makefile                                                *
# *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
# *   Copyright (C) 2007-2008 DarkJeztr Tillin9 Richard42                   *
# *                                                                         *
# *   This program is free software; you can redistribute it and/or modify  *
# *   it under the terms of the GNU General Public License as published by  *
# *   the Free Software Foundation; either version 2 of the License, or     *
# *   (at your option) any later version.                                   *
# *                                                                         *
# *   This program is distributed in the hope that it will be useful,       *
# *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
# *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
# *   GNU General Public License for more details.                          *
# *                                                                         *
# *   You should have received a copy of the GNU General Public License     *
# *   along with this program; if not, write to the                         *
# *   Free Software Foundation, Inc.,                                       *
# *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
# * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

# include pre-make file with a bunch of definitions
USES_QT4 = true
USES_GTK2 = true
include ./pre.mk

# local CFLAGS, LIBS, and LDFLAGS
LDFLAGS += -lz -lm -lpng -lfreetype

ifeq ($(OS), LINUX)
  LDFLAGS += -Wl,-export-dynamic
endif

# set executable stack as a linker option for X86 architecture, for dynamic recompiler
ifeq ($(CPU), X86)
  ifeq ($(OS), LINUX)
    LDFLAGS += -z execstack
  endif
endif

# set options
ifeq ($(DBG), 1)
  CFLAGS += -DDBG
endif
ifeq ($(DBG_COMPARE), 1)
  CFLAGS += -DCOMPARE_CORE
endif
ifeq ($(DBG_CORE), 1)
  CFLAGS += -DCORE_DBG
endif
ifeq ($(DBG_COUNT), 1)
  CFLAGS += -DCOUNT_INSTR
endif
ifeq ($(DBG_PROFILE), 1)
  CFLAGS += -DPROFILE_R4300
endif
ifeq ($(LIRC), 1)
  CFLAGS += -DWITH_LIRC
endif
ifeq ($(GUI), NONE)
  CFLAGS += -DNO_GUI
else
  ifeq ($(GUI), QT4)
  CFLAGS += $(QT_FLAGS) $(GTK_FLAGS)
  LDFLAGS += $(QT_LIBS)
    ifeq ($(DBG), 1)
      CFLAGS += $(GTK_FLAGS)
    endif
  else
    ifeq ($(GUI), GTK2)
      CFLAGS += $(GTK_FLAGS)
    endif
  endif
endif

# set installation options
ifeq ($(PREFIX),)
  PREFIX := /usr/local
endif
ifeq ($(SHAREDIR),)
  SHAREDIR := $(PREFIX)/share/mupen64plus
endif
ifeq ($(BINDIR),)
  BINDIR := $(PREFIX)/bin
endif
ifeq ($(LIBDIR),)
  LIBDIR := $(SHAREDIR)/plugins
endif
ifeq ($(MANDIR),)
  MANDIR := $(PREFIX)/man/man1
endif

INSTALLOPTS := $(PREFIX) $(SHAREDIR) $(BINDIR) $(LIBDIR) $(MANDIR)

# set Freetype flags
FREETYPEINC = $(shell pkg-config --cflags freetype2)
CFLAGS += $(FREETYPEINC)

# list of object files to generate
OBJ_CORE = \
	main/main.o \
	main/romcache.o \
	main/util.o \
	main/translate.o \
	main/cheat.o \
	main/config.o \
	main/adler32.o \
	main/md5.o \
	main/plugin.o \
	main/rom.o \
	main/savestates.o \
	main/zip/ioapi.o \
	main/zip/unzip.o \
	main/bzip2/bzlib.o \
	main/bzip2/crctable.o \
	main/bzip2/decompress.o \
	main/bzip2/compress.o \
	main/bzip2/randtable.o \
	main/bzip2/huffman.o \
	main/bzip2/blocksort.o \
	main/lzma/buffer.o \
	main/lzma/io.o \
	main/lzma/main.o \
	main/7zip/7zAlloc.o \
	main/7zip/7zBuffer.o \
	main/7zip/7zCrc.o \
	main/7zip/7zDecode.o \
	main/7zip/7zExtract.o \
	main/7zip/7zHeader.o \
	main/7zip/7zIn.o \
	main/7zip/7zItem.o \
	main/7zip/7zMain.o \
	main/7zip/LzmaDecode.o \
	main/7zip/BranchX86.o \
	main/7zip/BranchX86_2.o \
	memory/dma.o \
	memory/flashram.o \
	memory/memory.o \
	memory/pif.o \
	memory/tlb.o \
	r4300/r4300.o \
	r4300/bc.o \
	r4300/compare_core.o \
	r4300/cop0.o \
	r4300/cop1.o \
	r4300/cop1_d.o \
	r4300/cop1_l.o \
	r4300/cop1_s.o \
	r4300/cop1_w.o \
	r4300/exception.o \
	r4300/interupt.o \
	r4300/profile.o \
	r4300/pure_interp.o \
	r4300/recomp.o \
	r4300/special.o \
	r4300/regimm.o \
	r4300/tlb.o

# handle dynamic recompiler objects
ifneq ($(NO_ASM), 1)
  ifeq ($(CPU), X86)
    ifeq ($(ARCH), 64BITS)
      DYNAREC = x86_64
    else
      DYNAREC = x86
    endif
  endif
  OBJ_DYNAREC = \
      r4300/$(DYNAREC)/assemble.o \
      r4300/$(DYNAREC)/debug.o \
      r4300/$(DYNAREC)/gbc.o \
      r4300/$(DYNAREC)/gcop0.o \
      r4300/$(DYNAREC)/gcop1.o \
      r4300/$(DYNAREC)/gcop1_d.o \
      r4300/$(DYNAREC)/gcop1_l.o \
      r4300/$(DYNAREC)/gcop1_s.o \
      r4300/$(DYNAREC)/gcop1_w.o \
      r4300/$(DYNAREC)/gr4300.o \
      r4300/$(DYNAREC)/gregimm.o \
      r4300/$(DYNAREC)/gspecial.o \
      r4300/$(DYNAREC)/gtlb.o \
      r4300/$(DYNAREC)/regcache.o \
      r4300/$(DYNAREC)/rjump.o
else
  OBJ_DYNAREC = r4300/empty_dynarec.o
endif

OBJ_LIRC = \
	main/lirc.o

OBJ_OPENGL = \
	opengl/OGLFT.o \
	opengl/osd.o \
	opengl/screenshot.o

OBJ_GTK_GUI = \
	main/gui_gtk/main_gtk.o \
	main/gui_gtk/aboutdialog.o \
	main/gui_gtk/cheatdialog.o \
	main/gui_gtk/configdialog.o \
	main/gui_gtk/rombrowser.o \
	main/gui_gtk/romproperties.o

OBJ_QT_GUI = \
	main/gui_qt4/main.o \
	main/gui_qt4/mainwidget.o \
	main/gui_qt4/mainwindow.o \
	main/gui_qt4/romdirectorieslistwidget.o \
	main/gui_qt4/rommodel.o \
	main/gui_qt4/settingsdialog.o \
	main/gui_qt4/globals.o

OBJ_QT_MOC = \
	main/gui_qt4/mainwidget.moc \
	main/gui_qt4/mainwindow.moc \
	main/gui_qt4/romdirectorieslistwidget.moc \
	main/gui_qt4/settingsdialog.moc \
	main/gui_qt4/rommodel.moc

OBJ_QT_HEADERS = \
	main/gui_qt4/ui_romdirectorieslistwidget.h \
	main/gui_qt4/ui_settingsdialog.h \
	main/gui_qt4/ui_mainwindow.h \
    main/gui_qt4/ui_mainwidget.h

OBJ_DBG = \
	debugger/debugger.o \
	debugger/decoder.o \
	debugger/memory.o \
	debugger/breakpoints.o

OBJ_GTK_DBG_GUI = \
	main/gui_gtk/debugger/debugger.o \
	main/gui_gtk/debugger/breakpoints.o \
	main/gui_gtk/debugger/desasm.o \
	main/gui_gtk/debugger/memedit.o \
	main/gui_gtk/debugger/varlist.o \
	main/gui_gtk/debugger/registers.o \
	main/gui_gtk/debugger/regGPR.o \
	main/gui_gtk/debugger/regCop0.o \
	main/gui_gtk/debugger/regSpecial.o \
	main/gui_gtk/debugger/regCop1.o \
	main/gui_gtk/debugger/regAI.o \
	main/gui_gtk/debugger/regPI.o \
	main/gui_gtk/debugger/regRI.o \
	main/gui_gtk/debugger/regSI.o \
	main/gui_gtk/debugger/regVI.o \
	main/gui_gtk/debugger/regTLB.o \
	main/gui_gtk/debugger/ui_clist_edit.o \
	main/gui_gtk/debugger/ui_disasm_list.o

PLUGINS	= plugins/blight_input.so \
          plugins/dummyaudio.so \
          plugins/dummyvideo.so \
          plugins/glN64.so \
          plugins/ricevideo.so \
          plugins/glide64.so \
          plugins/jttl_audio.so \
          plugins/mupen64_hle_rsp_azimer.so \
          plugins/mupen64_input.so

SHARE = $(shell grep CONFIG_PATH config.h | cut -d '"' -f 2)

# set primary objects and libraries for all outputs
ALL = mupen64plus $(PLUGINS)
OBJECTS = $(OBJ_CORE) $(OBJ_DYNAREC) $(OBJ_OPENGL)
LIBS = $(SDL_LIBS) $(LIBGL_LIBS)

# add extra objects and libraries for selected options
ifeq ($(DBG), 1)
  OBJECTS +=  $(OBJ_DBG) $(OBJ_GTK_DBG_GUI)
  LIBS += -lopcodes -lbfd
endif
ifeq ($(LIRC), 1)
  OBJECTS += $(OBJ_LIRC)
  LDFLAGS += -llirc_client
endif
ifeq ($(GUI), QT4)
  OBJECTS += $(OBJ_QT_GUI)
  LIBS += $(QT_LIBS) $(GTK_LIBS)
else
  ifneq ($(GUI), NONE)
    OBJECTS += $(OBJ_GTK_GUI)
    LIBS += $(GTK_LIBS) $(GTHREAD_LIBS)
  endif
endif

# build targets
targets:
	@echo "Mupen64Plus makefile. "
	@echo "  Targets:"
	@echo "    all           == Build Mupen64Plus and all plugins"
	@echo "    clean         == remove object files"
	@echo "    rebuild       == clean and re-build all"
	@echo "    install       == Install Mupen64Plus and all plugins"
	@echo "    uninstall     == Uninstall Mupen64Plus and all plugins"
	@echo "  Options:"
	@echo "    BITS=32       == build 32-bit binaries on 64-bit machine"
	@echo "    LIRC=1        == enable LIRC support"
	@echo "    NO_RESAMP=1   == disable libsamplerate support in jttl_audio"
	@echo "    NO_ASM=1      == build without assembly (no dynamic recompiler or MMX/SSE code)"
	@echo "    GUI=NONE      == build without GUI support"
	@echo "    GUI=GTK2      == build with GTK2 GUI support (default)"
	@echo "    GUI=QT4       == build with QT4 GUI support"
	@echo "  Install Options:"
	@echo "    PREFIX=path   == install/uninstall prefix (default: /usr/local/)"
	@echo "    SHAREDIR=path == path to install shared data (default: PREFIX/share/mupen64plus/)"
	@echo "    BINDIR=path   == path to install mupen64plus binary (default: PREFIX/bin/)"
	@echo "    LIBDIR=path   == path to install plugin libraries (default: SHAREDIR/plugins/)"
	@echo "    MANDIR=path   == path to install manual files (default: PREFIX/man/man1/)"
	@echo "  Debugging Options:"
	@echo "    PROFILE=1     == build gprof instrumentation into binaries for profiling"
	@echo "    DBGSYM=1      == add debugging symbols to binaries"
	@echo "    DBG=1         == build graphical debugger"
	@echo "    DBG_CORE=1    == print debugging info in r4300 core"
	@echo "    DBG_COUNT=1   == print R4300 instruction count totals (64-bit dynarec only)"
	@echo "    DBG_COMPARE=1 == enable core-synchronized r4300 debugging"
	@echo "    DBG_PROFILE=1 == dump profiling data for r4300 dynarec to data file"
#	@echo "    RELEASE=1     == inhibit SVN info from version strings"
# The RELEASE flag is hidden from view as it should only be used internally.  It only affects
# the version strings

all: version.h $(ALL)

mupen64plus: $(OBJECTS)
	$(CXX) $^ $(LDFLAGS) $(LIBS) -lpthread -ldl -o $@
	$(STRIP) $@

install:
	./install.sh $(INSTALLOPTS)

uninstall:
	./uninstall.sh $(INSTALLOPTS)

clean:
	$(MAKE) -C blight_input clean
	$(MAKE) -C dummy_audio clean
	$(MAKE) -C dummy_video clean
	$(MAKE) -C glN64 clean
	$(MAKE) -C rice_video clean
	$(MAKE) -C glide64 clean
	$(MAKE) -C jttl_audio clean
	$(MAKE) -C rsp_hle clean
	$(MAKE) -C mupen64_input clean
	$(RM) -f ./r4300/*.o ./r4300/x86/*.o ./r4300/x86_64/*.o ./memory/*.o ./debugger/*.o ./opengl/*.o
	$(RM) -f ./main/*.o ./main/zip/*.o ./main/bzip2/*.o ./main/lzma/*.o ./main/7zip/*.o ./main/gui_gtk/*.o ./main/gui_gtk/debugger/*.o
	$(RM) -f mupen64plus
	$(RM) -f plugins/mupen64_input.so blight_input/arial.ttf.c blight_input/ttftoh plugins/blight_input.so plugins/mupen64_hle_rsp_azimer.so
	$(RM) -f plugins/dummyaudio.so plugins/dummyvideo.so plugins/jttl_audio.so plugins/glN64.so plugins/ricevideo.so plugins/glide64.so
	$(RM) -f main/gui_qt4/settings.cpp main/gui_qt4/settings.h main/gui_qt4/*.moc main/gui_qt4/ui_*.h main/gui_qt4/*.o

rebuild: clean all

# build rules
version.h: .svn/entries
	@sed 's|@MUPEN_VERSION@|\"$(MUPEN_VERSION)\"| ; s|@PLUGIN_VERSION@|\"$(PLUGIN_VERSION)\"|' \
        main/version.template > version.h
	@$(MV) version.h main/version.h

.cpp.o:
	$(CXX) -o $@ $(CFLAGS) $(SDL_FLAGS) -c $<

# I have no idea why this is needed, but apparently it is.
main/gui_qt4/settings.o: main/gui_qt4/settings.cpp
	$(CXX) -o $@ $(CFLAGS) $(SDL_FLAGS) -c $<

.c.o:
	$(CC) -o $@ $(CFLAGS) $(SDL_FLAGS) -c $<

plugins/blight_input.so: FORCE
	$(MAKE) -C blight_input all
	@$(CP) ./blight_input/blight_input.so ./plugins/blight_input.so

plugins/dummyaudio.so: FORCE
	$(MAKE) -C dummy_audio all
	@$(CP) ./dummy_audio/dummyaudio.so ./plugins/dummyaudio.so

plugins/dummyvideo.so: FORCE
	$(MAKE) -C dummy_video all
	@$(CP) ./dummy_video/dummyvideo.so ./plugins/dummyvideo.so

plugins/glN64.so: FORCE
	$(MAKE) -C glN64 all
	@$(CP) ./glN64/glN64.so ./plugins/glN64.so

plugins/ricevideo.so: FORCE
	$(MAKE) -C rice_video all
	@$(CP) ./rice_video/ricevideo.so ./plugins/ricevideo.so

plugins/glide64.so: FORCE
	$(MAKE) -C glide64 all
	@$(CP) ./glide64/glide64.so ./plugins/glide64.so

plugins/jttl_audio.so: FORCE
	$(MAKE) -C jttl_audio all
	@$(CP) ./jttl_audio/jttl_audio.so ./plugins/jttl_audio.so

plugins/mupen64_hle_rsp_azimer.so: FORCE
	$(MAKE) -C rsp_hle all
	@$(CP) ./rsp_hle/mupen64_hle_rsp_azimer.so ./plugins/mupen64_hle_rsp_azimer.so

plugins/mupen64_input.so: FORCE
	$(MAKE) -C mupen64_input all
	@$(CP) ./mupen64_input/mupen64_input.so ./plugins/mupen64_input.so

# QT4 build rules
main/gui_qt4/ui_%.h: main/gui_qt4/%.ui
	$(UIC) $< -o $@

main/gui_qt4/%.moc: main/gui_qt4/%.h
	$(MOC) -i $< -o $@

$(OBJ_QT_GUI): $(OBJ_QT_MOC) $(OBJ_QT_HEADERS)

# This is used to force the plugin builds
FORCE:

