# Generic Compilation Script
# (C) 2008 The Mupen64Plus Project

# Set base program pointers and flags
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

# Check for SDL
ifeq ("$(shell which sdl-config 2>&1 | head -c 9)", "which: no")
  # If it is not found, throw an error.
  $(error No SDL development libraries found!)
endif

# Check for GTK+2.0
ifeq ("$(shell which pkg-config 2>&1 | head -c 9)", "which: no")
  # If pkg-config is not installed, throw an error.
  $(error No GTK 2.x development libraries found!)
endif
ifneq ("$(shell pkg-config gtk+-2.0 --modversion | head -c 2)", "2.")
  # If we do find pkg-config, make sure it shows GTK+2.0 is install, and if not, throw an error.
  $(error No GTK 2.x development libraries found!)
endif

# Set GTK flags and libraries via pkg-config
GTK_FLAGS   = `pkg-config gtk+-2.0 --cflags` -D_GTK2
GTK_LIBS    = `pkg-config gtk+-2.0 --libs`
GTHREAD_LIBS    = `pkg-config gthread-2.0 --libs`

# Set base CFLAGS and LDFLAGS for all systems
CFLAGS = -pipe -ffast-math -funroll-loops -fexpensive-optimizations -fno-strict-aliasing -DUSEPOSIX
LDFLAGS =


# Set special flags per-system
ifeq ($(CPU), X86)
  ifeq ($(ARCH), 64BITS)
    CFLAGS += -march=athlon64
  else
    CFLAGS += -march=i686 -mtune=pentium-m -mmmx -msse
    ifneq ($(PROFILE), 1)
      CFLAGS += -fomit-frame-pointer
    endif
  endif
  # Tweak flags for 32-bit build on 64-bit system
  ifeq ($(ARCH), 64BITS_32)
    CFLAGS += -m32
    LDFLAGS += -m32 -m elf_i386
  endif
endif
ifeq ($(CPU), PPC)
  CFLAGS += -mcpu=powerpc -D_BIG_ENDIAN
endif

# Find installed assembler: yasm or nasm
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

# Set variables for profiling
ifeq ($(PROFILE), 1)
  CFLAGS += -pg -g
  LDFLAGS += -pg
  STRIP = true
else # Set variables for debugging symbols
  ifeq ($(DBGSYM), 1)
    CFLAGS += -g
    STRIP = true
  endif
endif

# set CFLAGS, LIBS, and LDFLAGS for external dependencies

SDL_FLAGS   = `sdl-config --cflags`
SDL_LIBS    = `sdl-config --libs`

ifeq ($(VCR), 1)
  # test for presence of avifile
  ifneq ($(shell avifile-config --version), 0)
    # throw error
    $(error VCR support requires avifile library)
  else
    AVIFILE_FLAGS   = `avifile-config --cflags`
    AVIFILE_LIBS    = `avifile-config --libs`
  endif
endif

FREETYPE_LIBS   = `freetype-config --libs`
FREETYPE_FLAGS  = `freetype-config --cflags`

PLUGIN_LDFLAGS  = -Wl,-Bsymbolic -shared

LIBGL_LIBS  = -L/usr/X11R6/lib -lGL
PLATFORM = posix

PLATFORMSPECLIB = -lpthread -ldl

# set executable stack as a linker option for X86 architecture, for dynamic recompiler
ifeq ($(CPU), X86)
  LDFLAGS += -z execstack
endif
