# Makefile for Glide64, in Mupen64Plus

# include pre-make file with a bunch of definitions
include ../pre.mk

# set BITS
ifeq ("$(ARCH)","64BITS")
  BITS=64
  MOPT=-m amd64
else
  BITS=32
  MOPT=
endif

# local CFLAGS, LIBS, and LDFLAGS
CFLAGS = -O3 -Wall -g -DGCC -DUSE_GTK $(SDL_CFLAGS) $(GTK_FLAGS) -Iwrapper/ -ffast-math -funroll-loops
LDFLAGS	= -shared -Wl,-Bsymbolic -lGL -lGLU -L/usr/X11R6/lib $(SDL_LIBS)

# set special flags per-system
ifeq ($(CPU), X86)
  ifeq ($(ARCH), 64BITS)
    CFLAGS += -march=athlon64 -fPIC
  else
    CFLAGS += -march=i686 -mtune=pentium-m -mmmx -msse
    ifneq ($(PROFILE), 1)
    ifneq ($(VPDEBUG), 1)
      CFLAGS += -fomit-frame-pointer
    endif
    endif
  endif
  # tweak flags for 32-bit build on 64-bit system
  ifeq ($(ARCH), 64BITS_32)
    CFLAGS += -m32
    LDFLAGS += -m32 -m elf_i386
  endif
endif
ifeq ($(CPU), PPC)
  CFLAGS += -mcpu=powerpc
endif

# set options
ifeq ($(PROFILE), 1)
  CFLAGS += -pg -g
  LDFLAGS += -pg
  STRIP = true
else
  ifeq ($(DBGSYM), 1)
    CFLAGS += -g
    STRIP = true
  endif
endif
ifeq ($(VPDEBUG), 1)
  CFLAGS  += -DVPDEBUG
  LDFLAGS += -lIL
endif
ifeq ($(RDP_LOG_ERR), 1)
  CFLAGS += -DRDP_LOGGING -DRDP_ERROR_LOG
else
  ifeq ($(RDP_LOG), 1)
    CFLAGS += -DRDP_LOGGING
  endif
endif

# list of object files to generate
OBJECTS = \
	Main.o \
	rdp.o \
	Ini.o \
	TexCache.o \
	Debugger.o \
	Util.o \
	CRC.o \
	Combine.o \
	TexBuffer.o \
	Config.o \
	3dmath.o \
	DepthBufferRender.o

ifeq ($(NO_ASM), 1)
  CFLAGS += -DNO_ASM
else
  OBJECTS += Tmem_nasm.o
endif

WRAPPEROBJECTS = \
	wrapper/combiner.o \
	wrapper/textures.o \
	wrapper/main.o \
	wrapper/geometry.o \
	wrapper/config.o \
	wrapper/filter.o \
	wrapper/2xsai.o \
	wrapper/hq2x.o \
	wrapper/hq4x.o

GTKOBJECTS = \
	support.o \
	messagebox.o

OBJECTS	+= $(GTKOBJECTS) $(WRAPPEROBJECTS)
TARGET	= glide64.so

CFLAGS += -MMD -MP -MQ $@
DEPS := $(OBJECTS:.o=.d)
ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif

# build targets
targets:
	@echo "Glide64 Video Plugin + wrapper makefile. "
	@echo "  Targets:"
	@echo "    all        == Build Glide64 video plugin"
	@echo "    clean      == remove object files"
	@echo "    rebuild    == clean and re-build all"
	@echo "  Options:"
	@echo "    BITS=32       == build 32-bit binaries on 64-bit machine"
	@echo "    PROFILE=1     == build gprof instrumentation into binaries for profiling"
	@echo "    DBGSYM=1      == add debugging symbols to binaries"
	@echo "    VPDEBUG=1     == some kind of debugging"
	@echo "    RDP_LOG=1     == Dump out RDP instruction data"
	@echo "    RDP_LOG_ERR=1 == Dump out RDP instruction data + errors"

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(LD) $(OBJECTS) $(GTK_LIBS) $(LDFLAGS) -o $@
	$(STRIP) --strip-all $@

Main.o: font.h cursor.h
font.h:	compiletex
	./compiletex font.tex font.h font

cursor.h: compiletex
	./compiletex cursor.tex cursor.h cursor

compiletex: compiletex.c
	@rm -f compiletex compiletex.o
	$(CC) -o compiletex.o -c $<
	$(LD) -o $@ compiletex.o

Tmem_nasm.o: Tmem_nasm.asm
	$(ASM) -f elf $(MOPT) -DBITS=$(BITS) $< -o $@

rdp.o: ucode06.h

clean:
	$(RM) -rf $(OBJECTS) $(TARGET) $(DEPS) compiletex compiletex.o font.h cursor.h
	$(RM) -rf *.ii *.i *.s

rebuild: clean $(TARGET)


# build rules

.SUFFIXES: .o .cpp .c
.cpp.o:
# in case this was invoked because a header changed and the compilation fails,
# the record of dependency in the .d is gone, but not the .o
	@rm -f $@
	$(CXX) -o $@ $(CFLAGS) -c $<

.c.o:
	@rm -f $@
	$(CC) -o $@ $(CFLAGS) -c $<

.PHONY: all clean rebuild

