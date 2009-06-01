#/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
# *   Mupen64plus - Makefile                                                *
# *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
# *   Copyright (C) 2009 slougi                                             *
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
# Makefile for tr64_ogl in Mupen64plus.

# include pre-make file with a bunch of definitions
USES_QT4 = true
USES_GTK2 = true
ifeq ($(WIN32),1)
  include ../pre.mk.win32
else
  include ../pre.mk
endif

# local CFLAGS, LIBS, and LDFLAGS
CFLAGS += -DPIC -D__LINUX__ -Wall
LDFLAGS += $(GTHREAD_LIBS)

ifneq ($(OS), WINDOWS)
  CFLAGS += -fpic
  OBJECTS = nixgl.o
else
  OBJECTS = wingl.o
endif

# list of object files to generate
OBJECTS += 3dmath.o \
    combine1.o \
    display.o \
    driver.o \
    render.o \
    texture.o \
    autodet.o \
    crc_ucode.o \
    debug.o \
    lighting.o \
    rdp.o \
    rdp_gl.o

# build targets
all: tr64_ogl.$(SO_EXTENSION)

clean:
	rm -f *.o *.$(SO_EXTENSION)

# build rules
.c.o:
	$(CC) -o $@ $(CFLAGS) $(SDL_FLAGS) -c $<

tr64_ogl.$(SO_EXTENSION): $(OBJECTS)
	$(CC) $^ $(LDFLAGS) $(ASM_LDFLAGS) $(PLUGIN_LDFLAGS) $(SDL_LIBS) $(LIBGL_LIBS) -shared -o $@
	$(STRIP) $@

