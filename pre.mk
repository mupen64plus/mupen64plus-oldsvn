#/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
# *   Mupen64plus - pre.mk                                                  *
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

ARCH = 32BITS
CPU = X86
OS = LINUX

SDL_FLAGS	= -IC:\SDL-1.2.13\include\SDL -IC:\SDL-1.2.13\include
SDL_LIBS	= -LC:\SDL-1.2.13\lib -lSDL

FREETYPE_LIBS	= -LC:\freetype-2.3.5-1\lib -lfreetype
FREETYPE_FLAGS	= -IC:\freetype-2.3.5-1\include -IC:\freetype-2.3.5-1\include\freetype2

# detect GUI options
ifeq ($(GUI),)
  GUI = GTK2
endif

# set GTK2 flags and libraries
# ideally we don't always do this, only when using the Gtk GUI,
# but too many plugins require it...

# test for presence of GTK 2.0
#ifeq ($(shell which pkg-config 2>/dev/null),)
  #$(error pkg-config not installed!)
#endif
#ifneq ("$(shell pkg-config gtk+-2.0 --modversion | head -c 2)", "2.")
  #$(error No GTK 2.x development libraries found!)
#endif
# set GTK flags and libraries
#GTK_FLAGS	= $(shell pkg-config gtk+-2.0 --cflags)
#GTK_LIBS	= $(shell pkg-config gtk+-2.0 --libs)
#GTHREAD_LIBS 	= $(shell pkg-config gthread-2.0 --libs)

# set Qt flags and libraries
ifeq ($(GUI), QT4)
   ifneq ($(USES_QT4),)
    MOC         = moc #$(shell which moc 2>/dev/null)
    ifeq ($(MOC),)
      $(error moc from Qt not found! Make sure the Qt binaries are in your PATH)
    endif
    UIC         = uic #$(shell which uic 2>/dev/null)
    ifeq ($(UIC),)
      $(error uic from Qt not found! Make sure the Qt binaries are in your PATH)
    endif
    QT_FLAGS    = -IC:\Qt\4.4.1\include -IC:\Qt\4.4.1\include\QtCore -IC:\Qt\4.4.1\include\QtGui #$(shell pkg-config QtCore QtGui --cflags)
    QT_LIBS     = -LC:\Qt\4.4.1\lib -lQtGui4 -lQtCore4 $(shell pkg-config QtCore QtGui --libs)
    # define Gtk flags when using Qt4 gui so it can load plugins, etc.
  endif
endif

# set base program pointers and flags
CC      = gcc
CXX     = g++
LD      = g++
ifeq ($(OS),LINUX)
STRIP	= strip -s
endif
ifeq ($(OS),OSX)
STRIP	= strip 
endif
RM      = del
MV      = move
CP      = copy
MD      = mkdir
FIND    = find
PROF    = gprof
INSTALL = ginstall

# create SVN version defines
MUPEN_RELEASE = 1.4
RELEASE = 0.1

ifneq ($(RELEASE),)
  MUPEN_VERSION = $(MUPEN_RELEASE)
  PLUGIN_VERSION = $(MUPEN_RELEASE)
else 
  ifeq ($(shell svn info ./ 2>/dev/null),)
    MUPEN_VERSION = $(MUPEN_RELEASE)"-development"
    PLUGIN_VERSION = $(MUPEN_RELEASE)"-development"
  else
    SVN_REVISION = $(shell svn info ./ 2>/dev/null | sed -n '/^Revision: /s/^Revision: //p')
    SVN_BRANCH = $(shell svn info ./ 2>/dev/null | sed -n '/^URL: /s/.*mupen64plus.//1p')
    SVN_DIFFHASH = $(shell svn diff ./ 2>/dev/null | md5sum | sed '/.*/s/ -//;/^d41d8cd98f00b204e9800998ecf8427e/d')
    MUPEN_VERSION = $(MUPEN_RELEASE)-$(SVN_BRANCH)-r$(SVN_REVISION) $(SVN_DIFFHASH)
    PLUGIN_VERSION = $(MUPEN_RELEASE)-$(SVN_BRANCH)-r$(SVN_REVISION)
  endif
endif

# set base CFLAGS and LDFLAGS for all systems
CFLAGS = -pipe -O3 -ffast-math -funroll-loops -fexpensive-optimizations -fno-strict-aliasing 
LDFLAGS =

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
  CFLAGS += -mcpu=powerpc -D_BIG_ENDIAN
endif

# set CFLAGS, LIBS, and LDFLAGS for external dependencies
ifeq ($(OS),LINUX)
  PLUGIN_LDFLAGS	= -Wl,-Bsymbolic -shared
endif
ifeq ($(OS),OSX)
  PLUGIN_LDFLAGS	= -bundle
endif

ifeq ($(OS),LINUX)
  LIBGL_LIBS	= # -L/usr/X11R6/lib -lGL -lGLU
endif
ifeq ($(OS),OSX)
  LIBGL_LIBS	= -framework OpenGL
endif

# set flags for compile options.

# set CFLAGS macro for no assembly language if required
ifeq ($(NO_ASM), 1)
  CFLAGS += -DNO_ASM
endif

# set variables for profiling
ifeq ($(PROFILE), 1)
  CFLAGS += -pg -g
  LDFLAGS += -pg
  STRIP = true
else   # set variables for debugging symbols
  ifeq ($(DBGSYM), 1)
    CFLAGS += -g
    STRIP = true
  endif
endif