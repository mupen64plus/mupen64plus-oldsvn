# Makefile for Mupen64

# include pre-make file with a bunch of definitions
FROMPLUGIN=1
include ../pre.mk

# local CFLAGS, LIBS, and LDFLAGS
CFLAGS += -fpic -DPIC

# list of object files to generate
OBJECTS = audio.o
PLUGIN_NAME = dummyaudio$(LIBSUFFIX)

# build targets
all: $(PLUGIN_NAME)

clean:
	rm -f *.o *$(LIBSUFFIX)

# build rules
.c.o:
	$(CC) -o $@ $(CFLAGS) -c $<

$(PLUGIN_NAME): $(OBJECTS)
	$(CC) $^ $(LDFLAGS) $(PLUGIN_LDFLAGS) -o $@
	$(STRIP) $@

