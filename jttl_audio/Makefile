# Makefile for Mupen64

# include pre-make file with a bunch of definitions
include ../pre.mk

# test for presence of libsamplerate
ifneq ($(strip $(shell pkg-config samplerate --modversion 2> /dev/null)),)
  ifneq ($(NO_RESAMP), 1)
    # set libsamplerate flags and libraries
    SRC_FLAGS	= `pkg-config samplerate --cflags` -DUSE_SRC
    SRC_LIBS	= `pkg-config samplerate --libs`
  else
    # clear libsamplerate flags and libraries
    SRC_FLAGS	= 
    SRC_LIBS	= 
  endif
else
  # warn user
  $(warning No libsamplerate development libraries found.  jttl_audio will be built without Best Quality SINC resampler.)
  # clear libsamplerate flags and libraries
  SRC_FLAGS	= 
  SRC_LIBS	= 
endif

# local CFLAGS, LIBS, and LDFLAGS
CFLAGS += -fpic -DPIC

# list of object files to generate
OBJECTS = main.o volume.o

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

