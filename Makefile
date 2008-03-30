# Makefile for Mupen64Plus

# include pre-make file with a bunch of definitions
include ./pre.mk

# local CFLAGS, LIBS, and LDFLAGS
LDFLAGS += -lz -lm -lpng

# set executable stack as a linker option for X86 architecture, for dynamic recompiler
ifeq ($(CPU), X86)
  LDFLAGS += -z execstack
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
ifeq ($(VCR), 1)
  CFLAGS += -DVCR_SUPPORT
endif
ifeq ($(LIRC), 1)
  CFLAGS += -DWITH_LIRC
endif
ifeq ($(NOGUI_ONLY), 1)
  CFLAGS += -DNOGUI_ONLY
else
  CFLAGS += $(GTK_FLAGS)
endif
ifndef PREFIX
  PREFIX := /usr/local
endif

CFLAGS += -DPREFIX=\"$(PREFIX)\"

# list of object files to generate
OBJ_CORE = \
	main/main.o \
	main/util.o \
	main/translate.o \
	main/guifuncs.o \
	main/config.o \
	main/adler32.o \
	main/ioapi.o \
	main/md5.o \
	main/mupenIniApi.o \
	main/plugin.o \
	main/rom.o \
	main/savestates.o \
	main/unzip.o \
	main/volume.o \
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

ifeq ($(CPU), X86)
  ifeq ($(ARCH), 64BITS)
    DYNAREC = x86_64
  else
    DYNAREC = x86
  endif
  OBJ_X86 = \
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
  OBJ_X86 =
endif

OBJ_VCR	= \
	main/vcr.o \
	main/vcr_compress.o \
	main/vcr_resample.o \
	main/gui_gtk/vcrcomp_dialog.o

OBJ_LIRC = \
	main/lirc.o

OBJ_GTK_GUI = \
	main/gui_gtk/main_gtk.o \
	main/gui_gtk/aboutdialog.o \
	main/gui_gtk/configdialog.o \
	main/gui_gtk/rombrowser.o \
	main/gui_gtk/romproperties.o \
	main/gui_gtk/dirbrowser.o

OBJ_DBG = \
        debugger/debugger.o \
		debugger/breakpoints.o \
		debugger/desasm.o \
		debugger/decoder.o \
		debugger/registers.o \
		debugger/regGPR.o \
		debugger/regCop0.o \
		debugger/regSpecial.o \
		debugger/regCop1.o \
		debugger/regAI.o \
		debugger/regPI.o \
		debugger/regRI.o \
		debugger/regSI.o \
		debugger/regVI.o \
		debugger/regTLB.o \
		debugger/ui_clist_edit.o

PLUGINS	= plugins/blight_input.so \
          plugins/dummyaudio.so \
          plugins/glN64.so \
          plugins/ricevideo.so \
          plugins/glide64.so \
          plugins/jttl_audio.so \
          plugins/mupen64_audio.so \
          plugins/mupen64_hle_rsp_azimer.so \
          plugins/mupen64_input.so

SHARE = $(shell grep CONFIG_PATH config.h | cut -d '"' -f 2)

# set primary objects and libraries for all outputs
ALL = mupen64plus $(PLUGINS)
OBJECTS = $(OBJ_CORE) $(OBJ_X86)
LIBS = $(SDL_LIBS) $(LIBGL_LIBS)

# add extra objects and libraries for selected options
ifeq ($(DBG), 1)
  ALL += mupen64plus_dbg
  OBJECTS += $(OBJ_DBG)
endif
ifeq ($(VCR), 1)
  OBJECTS += $(OBJ_VCR)
  LIBS += $(AVIFILE_LIBS)
endif
ifeq ($(LIRC), 1)
  OBJECTS += $(OBJ_LIRC)
  LDFLAGS += -llirc_client
endif
ifneq ($(NOGUI_ONLY), 1)
  OBJECTS += $(OBJ_GTK_GUI)
  LIBS += $(GTK_LIBS)
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
	@echo "    VCR=1         == enable video recording"
	@echo "    LIRC=1        == enable LIRC support"
	@echo "    NOGUI_ONLY=1  == build without GUI support"
	@echo "    PREFIX=path   == specify install/uninstall prefix (default: /usr/local)"
	@echo "  Debugging Options:"
	@echo "    PROFILE=1     == build gprof instrumentation into binaries for profiling"
	@echo "    DBGSYM=1      == add debugging symbols to binaries"
	@echo "    DBG=1         == build graphical debugger"
	@echo "    DBG_CORE=1    == print debugging info in r4300 core"
	@echo "    DBG_COUNT=1   == print R4300 instruction count totals (64-bit dynarec only)"
	@echo "    DBG_COMPARE=1 == enable core-synchronized r4300 debugging"
	@echo "    DBG_PROFILE=1 == dump profiling data for r4300 dynarec to data file"

all: $(ALL)

mupen64plus: $(OBJECTS)
	$(CC) $^ $(LDFLAGS) $(LIBS) -Wl,-export-dynamic -lpthread -ldl -o $@
	$(STRIP) $@

mupen64plus_dbg: $(OBJECTS) main/main_gtk.o
	$(CC) $^ $(LDFLAGS) $(LIBS) -Wl,-export-dynamic -lpthread -ldl -o $@

install:
	./install.sh $(PREFIX)

uninstall:
	./uninstall.sh $(PREFIX)

clean:
	$(MAKE) -C blight_input clean
	$(MAKE) -C dummy_audio clean
	$(MAKE) -C glN64 clean
	$(MAKE) -C rice_video clean
	$(MAKE) -C glide64 clean
	$(MAKE) -C jttl_audio clean
	$(MAKE) -C mupen64_audio clean
	$(MAKE) -C rsp_hle clean
	$(MAKE) -C mupen64_input clean
	$(RM) -f ./r4300/*.o ./r4300/x86/*.o ./r4300/x86_64/*.o ./memory/*.o ./main/*.o ./main/gui_gtk/*.o ./debugger/*.o
	$(RM) -f mupen64plus mupen64plus_dbg 
	$(RM) -f plugins/mupen64_input.so blight_input/arial.ttf.c blight_input/ttftoh plugins/blight_input.so plugins/mupen64_hle_rsp_azimer.so 
	$(RM) -f plugins/dummyaudio.so plugins/mupen64_audio.so plugins/jttl_audio.so plugins/glN64.so plugins/ricevideo.so plugins/glide64.so

rebuild: clean all

# build rules
.cpp.o:
	$(CXX) -o $@ $(CFLAGS) $(SDL_FLAGS) -c $<

.c.o:
	$(CC) -o $@ $(CFLAGS) $(SDL_FLAGS) -c $<

main/vcr_compress.o: main/vcr_compress.cpp
	$(CXX) -o $@ $(CFLAGS) $(AVIFILE_FLAGS) -c $<

plugins/blight_input.so: FORCE
	$(MAKE) -C blight_input all
	@$(CP) ./blight_input/blight_input.so ./plugins/blight_input.so

plugins/dummyaudio.so: FORCE
	$(MAKE) -C dummy_audio all
	@$(CP) ./dummy_audio/dummyaudio.so ./plugins/dummyaudio.so

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

plugins/mupen64_audio.so: FORCE
	$(MAKE) -C mupen64_audio all
	@$(CP) ./mupen64_audio/mupen64_audio.so ./plugins/mupen64_audio.so

plugins/mupen64_hle_rsp_azimer.so: FORCE
	$(MAKE) -C rsp_hle all
	@$(CP) ./rsp_hle/mupen64_hle_rsp_azimer.so ./plugins/mupen64_hle_rsp_azimer.so

plugins/mupen64_input.so: FORCE
	$(MAKE) -C mupen64_input all
	@$(CP) ./mupen64_input/mupen64_input.so ./plugins/mupen64_input.so

FORCE:

