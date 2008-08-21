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
# Makefile for glN64 in Mupen64plus.

# include pre-make file with a bunch of definitions
USES_KDE4 = true
USES_GTK2 = true
include ../pre.mk

# local CFLAGS, LIBS, and LDFLAGS
CFLAGS += -fpic -DPIC -D__LINUX__ -Wall
LDFLAGS += $(GTHREAD_LIBS)

ifeq ($(GUI), NONE)
  OBJECTS = Config_nogui.o
else
  ifeq ($(GUI), KDE4)
    LDFLAGS += $(KDE_LIBS)
    CFLAGS += $(KDE_FLAGS)
    OBJECTS = Config_kde4.o gln64settings.o
  else
    LDFLAGS += $(GTK_LIBS) 
    CFLAGS += -DUSE_GTK $(GTK_FLAGS)
    OBJECTS = Config_gtk2.o
  endif
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
all: version.h glN64.so

clean:
	rm -f *.o *.so gln64settings.cpp gln64settings.h* ui_gln64config.*

# build rules
version.h: .svn/entries
	@sed 's|@MUPEN_VERSION@|\"$(MUPEN_VERSION)\"| ; s|@PLUGIN_VERSION@|\"$(PLUGIN_VERSION)\"|' \
        ../main/version.template > version.h
	@$(MV) version.h ../main/version.h

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
	$(CXX) $(CFLAGS) $(SDL_FLAGS) -c -o $@ $<

Config_gtk2.o: Config_gtk2.cpp
	$(CXX) $(CFLAGS) $(SDL_FLAGS) -c -o $@ $<

GBI.o: GBI.cpp
	$(CXX) $(CFLAGS) $(SDL_FLAGS) -c -o $@ $<

