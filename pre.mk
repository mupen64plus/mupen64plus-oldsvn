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

SDL_FLAGS	= `sdl-config --cflags`
SDL_LIBS	= `sdl-config --libs`

ifeq ($(VCR), 1)
  # test for presence of avifile
  ifneq ($(shell avifile-config --version), 0)
    # throw error
    $(error VCR support requires avifile library)
  else
    AVIFILE_FLAGS	= `avifile-config --cflags`
    AVIFILE_LIBS	= `avifile-config --libs`
  endif
endif

FREETYPE_LIBS	= `freetype-config --libs`
FREETYPE_FLAGS	= `freetype-config --cflags`

PLUGIN_LDFLAGS	= -Wl,-Bsymbolic -shared

LIBGL_LIBS	= -L/usr/X11R6/lib -lGL

