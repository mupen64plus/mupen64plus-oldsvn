# Makefile for Mupen64

# include pre-make file with a bunch of definitions
FROMPLUGIN=1
include ../pre.mk

# local CFLAGS, LIBS, and LDFLAGS
CFLAGS += -fpic -DPIC

# list of object files to generate
OBJECTS = main.o disasm.o jpeg.o ucode3.o ucode2.o ucode1.o ucode3mp3.o

# build targets
all: mupen64_hle_rsp_azimer.so

clean:
	rm -f *.o *.so

# build rules
.c.o:
	$(CC) -o $@ $(CFLAGS) $(GTK_FLAGS) -DUSE_GTK -c $<
.cpp.o:
	$(CXX) -o $@ $(CFLAGS) -c $<

mupen64_hle_rsp_azimer.so: $(OBJECTS)
	$(CXX) $^ $(LDFLAGS) $(PLUGIN_LDFLAGS) $(GTK_LIBS) -o $@
	$(STRIP) $@

