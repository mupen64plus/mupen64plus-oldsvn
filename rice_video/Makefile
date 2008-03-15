# detect system architecture: i386, x86_64, or PPC/PPC64
UNAME = $(shell uname -m)
ifeq ("$(UNAME)","x86_64")
  CPU = X86
  ifeq ("$(BITS)", "32")
    ARCH = 64BITS_32
  else
    ARCH = 64BITS
  endif
endif
ifneq ("$(filter i%86,$(UNAME))","")
  CPU = X86
  ARCH = 32BITS
endif
ifeq ("$(UNAME)","ppc")
  CPU = PPC
  ARCH = 32BITS
endif
ifeq ("$(UNAME)","ppc64")
  CPU = PPC
  ARCH = 64BITS
endif

# test for presence of SDL
ifeq ("$(shell which sdl-config 2>&1 | head -c 9)", "which: no")
  # throw error
  $(error No SDL development libraries found!)
endif

# test for presence of GTK 2.0
ifeq ("$(shell which pkg-config 2>&1 | head -c 9)", "which: no")
  # throw error
  $(error No GTK 2.x development libraries found!)
endif
ifneq ("$(shell pkg-config gtk+-2.0 --modversion | head -c 2)", "2.")
  # throw error
  $(error No GTK 2.x development libraries found!)
endif

# set GTK flags and libraries
GTK_FLAGS	= `pkg-config gtk+-2.0 --cflags` -D_GTK2
GTK_LIBS	= `pkg-config gtk+-2.0 --libs`
GTHREAD_LIBS	= `pkg-config gthread-2.0 --libs`

# set base program pointers and flags
CC = gcc
CXX = g++
LD = g++
STRIP = strip

# set base CFLAGS and LDFLAGS for all systems
CFLAGS = -DUSE_GTK `sdl-config --cflags` $(GTK_FLAGS) -fpic -DPIC -O3 -ffast-math -funroll-loops
LDFLAGS	= -L/usr/X11R6/lib `sdl-config --libs` -lGL -shared -Wl,-Bsymbolic

# set special flags per-system
ifeq ($(CPU), X86)
  ifeq ($(ARCH), 64BITS)
    CFLAGS += -march=athlon64
  else
    CFLAGS += -march=i686 -mtune=pentium-m -mmmx -msse
    ifneq ($(PROFILE), 1)
      CFLAGS += -fomit-frame-pointer
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

ifeq ($(DBG), 1)
  CFLAGS += -D_DEBUG
endif

# list of object files to generate
OBJECTS = OGLGraphicsContext.o \
    Debugger.o \
	Video.o \
	Config.o \
	GraphicsContext.o \
	TextureFilters.o \
	DeviceBuilder.o \
	Render.o \
	RSP_Parser.o \
	TextureManager.o \
	RenderBase.o \
	FrameBuffer.o \
	messagebox.o \
	Texture.o \
	TextureFilters_2xsai.o \
	TextureFilters_hq2x.o \
	TextureFilters_hq4x.o \
	liblinux/bmp.o \
	liblinux/BMGImage.o \
	liblinux/pngrw.o \
	OGLRender.o \
	OGLTexture.o \
	OGLFragmentShaders.o \
	OGLCombinerNV.o \
	OGLCombinerTNT2.o \
	OGLExtCombiner.o \
	OGLCombiner.o \
	OGLExtRender.o \
	blender.o \
	math.o \
	Combiner.o \
	RSP_S2DEX.o \
	RenderExt.o \
	ConvertImage.o \
	ConvertImage16.o \
	DecodedMux.o \
	RenderTexture.o \
	support.o \
	liblinux/BMGUtils.o \
	liblinux/pngread.o \
	liblinux/png.o \
	liblinux/pngget.o \
	liblinux/pngrtran.o \
	liblinux/pngtrans.o \
	liblinux/pngwrite.o \
	liblinux/pngset.o \
	liblinux/pngmem.o \
	OGLDecodedMux.o \
	CNvTNTCombiner.o \
	GeneralCombiner.o \
	DirectXDecodedMux.o \
	liblinux/pngerror.o \
	liblinux/inflate.o \
	liblinux/pngrio.o \
	liblinux/pngrutil.o \
	liblinux/crc32.o \
	liblinux/pngwutil.o \
	liblinux/pngwio.o \
	liblinux/pngwtran.o \
	liblinux/deflate.o \
	CombinerTable.o \
	liblinux/infblock.o \
	liblinux/zutil.o \
	liblinux/adler32.o \
	liblinux/trees.o \
	liblinux/infcodes.o \
	liblinux/infutil.o \
	liblinux/inftrees.o \
	liblinux/inffast.o \
	OGLRenderExt.o

# build targets
targets:
	@echo "RiceVideoLinux N64 Video plugin makefile. "
	@echo "  Targets:"
	@echo "    all        == Build RiceVideoLinux plugin"
	@echo "    clean      == remove object files"
	@echo "    rebuild    == clean and re-build all"
	@echo "  Options:"
	@echo "    BITS=32    == build 32-bit binaries on 64-bit machine"
	@echo "    PROFILE=1  == build gprof instrumentation into binaries for profiling"
	@echo "    DBG=1      == turn on debugging functions"
	@echo "    DBGSYM=1   == add debugging symbols to binaries"

all: ricevideo.so instruction

ricevideo.so: $(OBJECTS)
	$(LD) $(OBJECTS) $(GTK_LIBS) $(LDFLAGS) -o $@
	$(STRIP) --strip-all $@

instruction:
	$(warning please copy ricevideo.so AND RiceVideoLinux.ini in the plugins/ folder of the emulator)

clean:
	rm -rf $(OBJECTS) $(ALL)

rebuild: clean $(ALL)

# build rules

.cpp.o:
	$(CXX) -o $*.o $(CFLAGS) -c $<

.c.o:
	$(CC) -o $*.o $(CFLAGS) -c $<


