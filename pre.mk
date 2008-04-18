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

ifeq ("$(PLAT)","")
  PLAT=generic
endif

# include the pre.mk for the specific platform
ifeq ("$(FROMPLUGIN)","1")
  ROOT=..
else
  ROOT=.
endif
include $(ROOT)/$(PLAT)/pre.mk