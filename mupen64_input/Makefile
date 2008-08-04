# Makefile for Mupen64 Input in Mupen64plus.

# include pre-make file with a bunch of definitions
USES_GTK2 = true 
include ../pre.mk

# local CFLAGS, LIBS, and LDFLAGS
CFLAGS += -fpic -DPIC

ifeq ($(GUI),GTK2)
  CFLAGS += $(GTK_FLAGS) -DUSE_GTK
  PLUGIN_LDFLAGS += $(GTK_LIBS)
endif

# list of object files to generate
OBJECTS = main.o

# build targets
all: mupen64_input.so

clean:
	rm -f *.o *.so

# build rules
.c.o:
	$(CC) -o $@ $(CFLAGS) $(SDL_FLAGS) -c $<

mupen64_input.so: $(OBJECTS)
	$(CC) $^ $(SDL_LIBS) $(PLUGIN_LDFLAGS) -o $@
	$(STRIP) $@

