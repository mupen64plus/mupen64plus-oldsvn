# Makefile for Mupen64

# include pre-make file with a bunch of definitions
USES_KDE4 = true
include ../pre.mk

# local CFLAGS, LIBS, and LDFLAGS
CFLAGS += -fpic -DPIC -D__LINUX__ -DX86_ASM -Wall 

ifeq ($(GUI), KDE4)
  LDFLAGS += $(KDE_LIBS) $(GTHREAD_LIBS)
  OBJECTS = Config_kde4.o gln64settings.o
else
  LDFLAGS += $(GTK_LIBS) $(GTHREAD_LIBS)
  OBJECTS = Config_gtk2.o
endif

# list of object files to generate
OBJECTS += glN64.o \
	OpenGL.o \
	N64.o \
	RSP.o \
	VI.o \
	Textures.o \
	FrameBuffer.o \
	Combiner.o \
	gDP.o \
	gSP.o \
	GBI.o \
	DepthBuffer.o \
	CRC.o \
	2xSAI.o \
	NV_register_combiners.o \
	texture_env.o \
	texture_env_combine.o \
	RDP.o \
	F3D.o \
	F3DEX.o \
	F3DEX2.o \
	L3D.o \
	L3DEX.o \
	L3DEX2.o \
	S2DEX.o \
	S2DEX2.o \
	F3DPD.o \
	F3DDKR.o \
	F3DWRUS.o

# build targets
all: glN64.so

clean:
	rm -f *.o *.so gln64settings.cpp gln64settings.h* ui_gln64config.*

# build rules
.cpp.o:
	$(CXX) -o $@ $(CFLAGS) $(SDL_FLAGS) -c $<

gln64settings.o: gln64settings.cpp
	$(CXX) -o $@ $(CFLAGS) ${KDE_FLAGS} $(SDL_FLAGS) -c $<


ui_gln64config.h: gln64config.ui
	$(UIC) $< -o $@

gln64settings.h: gln64.kcfg gln64settings.kcfgc
	$(KCONFIG_COMPILER) gln64.kcfg gln64settings.kcfgc

glN64.so: $(OBJECTS)
	$(CXX) $^ $(LDFLAGS) $(ASM_LDFLAGS) $(PLUGIN_LDFLAGS) $(SDL_LIBS) $(LIBGL_LIBS) -o $@
	$(STRIP) $@

glN64.o: glN64.cpp
	$(CXX) $(CFLAGS) $(SDL_FLAGS) -DMAINDEF -c -o $@ $<

Config_kde4.o: Config_kde4.cpp gln64settings.h ui_gln64config.h
	$(CXX) $(CFLAGS) $(SDL_FLAGS) $(KDE_FLAGS) -c -o $@ $<

Config_gtk2.o: Config_gtk2.cpp
	$(CXX) $(CFLAGS) $(GTK_FLAGS) $(SDL_FLAGS) -c -o $@ $<

GBI.o: GBI.cpp
	$(CXX) $(CFLAGS) -DUSE_GTK $(GTK_FLAGS) $(SDL_FLAGS) -c -o $@ $<
