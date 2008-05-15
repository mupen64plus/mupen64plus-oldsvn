# pre.mk - Dynamic Makefile setup script
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

ifeq ("$(ROOT)", "")
  echo "Root Path not set! Treating as a plugin..."
  ROOT=..
endif

ifeq ("$(PLAT)", "")
  UNAME_OS = $(shell uname -o)
  ifeq ("$(UNAME_OS)","GNU/Linux")
    PLAT=linux
  endif
  ifeq ("$(UNAME_OS)","Linux")
    PLAT=linux
  endif
  ifeq ("$(UNAME_OS)","Msys")
    PLAT=win32
  endif
  ifeq ("$(UNAME_OS)","CYGWIN_NT-5.0")
    PLAT=cygwin
  endif
  ifeq ("$(UNAME_OS)","Windows_NT")
    PLAT=mks
  endif
endif

include $(ROOT)/platform/$(PLAT)/pre.mk
