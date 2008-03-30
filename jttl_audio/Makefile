# Makefile for Mupen64

# include pre-make file with a bunch of definitions
include ../pre.mk

# local CFLAGS, LIBS, and LDFLAGS
CFLAGS += -fpic -DPIC -g

# list of object files to generate
OBJECTS = main.o

# build targets
all: jttl_audio.so

clean:
	rm -f *.o *.so

# build rules
.c.o:
	$(CC) $(CFLAGS) $(SDL_FLAGS) $(GTK_FLAGS) $(SRC_FLAGS) -DUSE_GTK -c -o $@ $<

jttl_audio.so: $(OBJECTS)
	$(CC) $^ $(LDFLAGS) $(PLUGIN_LDFLAGS) $(SDL_LIBS) $(GTK_LIBS) $(SRC_LIBS) -o $@
	$(STRIP) $@

