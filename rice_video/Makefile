# Makefile for RiceVideoLinux, in Mupen64Plus

# include pre-make file with a bunch of definitions
include ../pre.mk

# local CFLAGS, LIBS, and LDFLAGS
CFLAGS += -DUSE_GTK `sdl-config --cflags` $(GTK_FLAGS) -fpic -DPIC
LDFLAGS	+= -L/usr/X11R6/lib `sdl-config --libs` -lGL -shared -Wl,-Bsymbolic

# set options

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
	rm -rf ricevideo.so $(OBJECTS)

rebuild: clean $(ALL)

# build rules

.cpp.o:
	$(CXX) -o $*.o $(CFLAGS) -c $<

.c.o:
	$(CC) -o $*.o $(CFLAGS) -c $<


