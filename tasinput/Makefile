# Makefile for Mupen64

# include pre-make file with a bunch of definitions
include ../pre.mk

# local CFLAGS, LIBS, and LDFLAGS
CFLAGS += -fpic -DPIC

# list of object files to generate
OBJECTS = ConfigGtk2.o Dl.o DefSDL.cpp MacroDlg.o

# build targets
all: tasinput.so

clean:
	rm -f *.o *.so

# build rules
.c.o:
	$(CC) -o $@ $(CFLAGS) $(SDL_FLAGS) $(GTK_FLAGS) -DUSE_GTK -c $<

tasinput.so: $(OBJECTS)
	$(CC) $^ $(LDFLAGS) $(PLUGIN_LDFLAGS) $(GTK_LIBS) -o $@
	$(STRIP) $@

