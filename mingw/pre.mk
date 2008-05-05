# MinGW Compilation Script
# (C) 2008 The Mupen64Plus Project

# Set MinGW specific variables
MGWDIR  = /usr/mingw32
MGWPRE  = mingw32-
MGWBIN  = /bin
MGWLIB  = /lib

# Set base program pointers and flags
CC      = $(MGWDIR)$(MGWBIN)/$(MGWPRE)gcc
CXX     = $(MGWDIR)$(MGWBIN)/$(MGWPRE)g++
LD      = $(MGWDIR)$(MGWBIN)/$(MGWPRE)g++
STRIP   = $(MGWDIR)$(MGWBIN)/$(MGWPRE)strip --strip-all
PROF    = $(MGWDIR)$(MGWBIN)/$(MGWPRE)gprof
RM      = rm
MV      = mv
CP      = cp
MD      = mkdir
FIND    = find
INSTALL = ginstall
BINSUFFIX = .exe
LIBSUFFIX = .dll

# Check for SDL
ifeq ("$(shell which $(MGWDIR)$(MGWBIN)/sdl-config 2>&1 | head -c 9)", "which: no")
  # If it is not found, throw an error.
  $(error No SDL development libraries found!)
endif

# Check for GTK+2.0
ifeq ("$(shell which $(MGWDIR)$(MGWBIN)/pkg-config 2>&1 | head -c 9)", "which: no")
  # If pkg-config is not installed, throw an error.
  $(error No GTK 2.x development libraries found!)
endif
ifneq ("$(shell PKG_CONFIG_PATH=$(MGWDIR)$(MGWLIB)/pkgconfig $(MGWDIR)$(MGWBIN)/pkg-config gtk+-2.0 --modversion | head -c 2)", "2.")
  # If we do find pkg-config, make sure it shows GTK+2.0 is install, and if not, throw an error.
  $(error No GTK 2.x development libraries found!)
endif

# Set GTK flags and libraries via pkg-config
GTK_FLAGS    = `PKG_CONFIG_PATH=$(MGWDIR)$(MGWLIB)/pkgconfig $(MGWDIR)$(MGWBIN)/pkg-config gtk+-2.0 --cflags` -D_GTK2
GTK_LIBS     = `PKG_CONFIG_PATH=$(MGWDIR)$(MGWLIB)/pkgconfig $(MGWDIR)$(MGWBIN)/pkg-config gtk+-2.0 --libs`
GTHREAD_LIBS = `PKG_CONFIG_PATH=$(MGWDIR)$(MGWLIB)/pkgconfig $(MGWDIR)$(MGWBIN)/pkg-config gthread-2.0 --libs`

# Set base CFLAGS and LDFLAGS for all systems
CFLAGS = -pipe -ffast-math -funroll-loops -fexpensive-optimizations -fno-strict-aliasing -DUSEWIN32 -I$(ROOT)/main/win32 -mconsole
LDFLAGS =


# Set special flags per-system
ifeq ($(CPU), X86)
  ifeq ($(ARCH), 64BITS)
    CFLAGS += -march=athlon64
    $(error There is not yet support for expieremental MinGW 64bit. Sorry.)
  else
    CFLAGS += -march=i686 -mtune=pentium-m -mmmx -msse
    ifneq ($(PROFILE), 1)
      CFLAGS += -fomit-frame-pointer
    endif
  endif
  # Tweak flags for 32-bit build on 64-bit system
  ifeq ($(ARCH), 64BITS_32)
    CFLAGS += -m32
    LDFLAGS += -m32 -m i386pe
  endif
endif
ifeq ($(CPU), PPC)
  $(error MinGW does not support PowerPC. Mupen64Plus would not work on PowerPC WinNT anyways.)
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

SDL_FLAGS   = `$(MGWDIR)$(MGWBIN)/sdl-config --cflags`
SDL_LIBS    = `$(MGWDIR)$(MGWBIN)/sdl-config --libs`

ifeq ($(VCR), 1)
  # Test for presence of avifile
  ifneq ($(shell $(MGWDIR)$(MGWBIN)/avifile-config --version), 0)
    # throw error
    $(error VCR support requires avifile library)
  else
    AVIFILE_FLAGS   = `$(MGWDIR)$(MGWBIN)/avifile-config --cflags`
    AVIFILE_LIBS    = `$(MGWDIR)$(MGWBIN)/avifile-config --libs`
  endif
endif

FREETYPE_LIBS   = `$(MGWDIR)$(MGWBIN)/freetype-config --libs`
FREETYPE_FLAGS  = `$(MGWDIR)$(MGWBIN)/freetype-config --cflags`

PLUGIN_LDFLAGS  = -Wl,-Bsymbolic -shared

LIBGL_LIBS  = -L/usr/X11R6/lib -lopengl32 -lpng
PLATFORM = win32
PLATFORMSPECLIB = -lwinmm -lkernel32 -liconv -lshell32
