# Makefile for Mupen64

# include pre-make file with a bunch of definitions
include ../pre.mk

# local CFLAGS, LIBS, and LDFLAGS
CFLAGS += -fpic -DPIC

# list of object files to generate
OBJECTS = main.o

# build targets
all: mupen64_audio.so

clean:
	rm -f *.o *.so

# build rules
.c.o:
	$(CC) -o $@ $(CFLAGS) $(GTK_FLAGS) -DUSE_GTK -c $<

mupen64_audio.so: $(OBJECTS)
	$(CC) $^ $(LDFLAGS) $(PLUGIN_LDFLAGS) $(GTK_LIBS) -lpthread -o $@
	$(STRIP) $@

