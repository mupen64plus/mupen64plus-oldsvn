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
  NO_ASM = 1
endif
ifeq ("$(UNAME)","ppc64")
  CPU = PPC
  ARCH = 64BITS
  NO_ASM = 1
endif


# test for presence of SDL
ifeq ($(shell which sdl-config 2>/dev/null),)
  # throw error
  $(error No SDL development libraries found!)
endif

# test for presence of GTK 2.0
ifeq ($(shell which pkg-config 2>/dev/null),)
  # throw error
  $(error pkg-config not installed!)
endif
ifneq ("$(shell pkg-config gtk+-2.0 --modversion | head -c 2)", "2.")
  # throw error
  $(error No GTK 2.x development libraries found!)
endif

# set GTK flags and libraries
GTK_FLAGS	= $(shell pkg-config gtk+-2.0 --cflags) -D_GTK2
GTK_LIBS	= $(shell pkg-config gtk+-2.0 --libs)
GTHREAD_LIBS	= $(shell pkg-config gthread-2.0 --libs)

# set KDE flags and libraries
ifeq ($(GUI), KDE4)
  ifneq ($(USES_KDE4),)
    KDE_CONFIG=$(shell which kde4-config 2>/dev/null)
    ifeq ($(KDE_CONFIG),)
      $(error kde4-config not found, try the GTK2 GUI!)
    endif

    KCONFIG_COMPILER = $(shell $(KDE_CONFIG) --prefix)/bin/kconfig_compiler
    MOC         = $(shell which moc 2>/dev/null)
    UIC         = $(shell which uic 2>/dev/null)
    ifeq ($(MOC),)
      $(error moc from Qt not found, make sure the Qt binaries are in your PATH)
    endif
    ifeq ($(UIC),)
      $(error uic from Qt not found, make sure the Qt binaries are in your PATH)
    endif
    QT_FLAGS    = $(shell pkg-config QtCore QtGui --cflags)
    KDE_FLAGS   = -I$(shell $(KDE_CONFIG) --path include) -I$(shell $(KDE_CONFIG) --path include)/KDE $(QT_FLAGS)
    QT_LIBS     = $(shell pkg-config QtCore QtGui --libs)
    KDE_LIBRARY_PATHS = $(shell kde4-config --path lib | sed s/:/" -L"/g | sed s:^:-L:)
    KDE_LIBS    = -lkdecore -lkdeui -lkio $(KDE_LIBRARY_PATHS)
  endif
endif # end KDE4 GUI




# set base program pointers and flags
CC      = gcc
CXX     = g++
LD      = g++
STRIP   = strip --strip-all
RM      = rm
MV      = mv
CP      = cp
MD      = mkdir
FIND    = find
PROF    = gprof
INSTALL = ginstall

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

# find installed assembler: yasm or nasm
ifeq ($(NO_ASM), 1)
  CFLAGS += -DNO_ASM
else
  ifneq ("$(shell which yasm 2>&1 | head -c 9)", "which: no")
    ASM=yasm
  else
    ifneq ("$(shell which nasm 2>&1 | head -c 9)", "which: no")
      ASM=nasm
    else
      # throw error
      $(error No yasm or nasm found!)
    endif
  endif
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

# set CFLAGS, LIBS, and LDFLAGS for external dependencies

SDL_FLAGS	= $(shell sdl-config --cflags)
SDL_LIBS	= $(shell sdl-config --libs)

ifeq ($(VCR), 1)
  # test for presence of avifile
  ifneq ($(shell avifile-config --version), 0)
    # throw error
    $(error VCR support requires avifile library)
  else
    AVIFILE_FLAGS	= $(shell avifile-config --cflags)
    AVIFILE_LIBS	= $(shell avifile-config --libs)
  endif
endif

FREETYPE_LIBS	= $(shell freetype-config --libs)
FREETYPE_FLAGS	= $(shell freetype-config --cflags)

PLUGIN_LDFLAGS	= -Wl,-Bsymbolic -shared

LIBGL_LIBS	= -L/usr/X11R6/lib -lGL -lGLU

