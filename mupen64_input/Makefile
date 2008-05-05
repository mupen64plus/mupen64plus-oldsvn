# Makefile for Mupen64

# include pre-make file with a bunch of definitions
FROMPLUGIN=1
include ../pre.mk

# local CFLAGS, LIBS, and LDFLAGS
CFLAGS += -fpic -DPIC

# list of object files to generate
OBJECTS = main.o

PLUGIN_NAME=mupen64_input$(LIBSUFFIX)

# build targets
all: $(PLUGIN_NAME)

clean:
	rm -f *.o *$(LIBSUFFIX)

# build rules
.c.o:
	$(CC) -o $@ $(CFLAGS) $(SDL_FLAGS) $(GTK_FLAGS) -DUSE_GTK -c $<

$(PLUGIN_NAME): $(OBJECTS)
	$(CC) $^ $(LDFLAGS) $(PLUGIN_LDFLAGS) $(GTK_LIBS) -o $@
	$(STRIP) $@

