# Makefile for Mupen64

# include pre-make file with a bunch of definitions
include ../pre.mk

# local CFLAGS, LIBS, and LDFLAGS
CFLAGS += -fpic -DPIC

# list of object files to generate
OBJECTS = main.o rsp.o vi_SDL.o vi.o rdp.o tx.o rs.o tf.o cc.o bl.o

# build targets
all: mupen64_soft_gfx.so

clean:
	rm -f *.o *.so

# build rules
.cpp.o:
	$(CXX) -o $@ $(CFLAGS) $(SDL_FLAGS) -c $<

mupen64_soft_gfx.so: $(OBJECTS)
	$(CXX) $^ $(LDFLAGS) $(PLUGIN_LDFLAGS) $(SDL_LIBS) -o $@
	$(STRIP) $@

