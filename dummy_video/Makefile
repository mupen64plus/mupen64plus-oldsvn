# Makefile for Mupen64

# include pre-make file with a bunch of definitions
FROMPLUGIN=1
include ../pre.mk

# local CFLAGS, LIBS, and LDFLAGS
CFLAGS += -fpic -DPIC -D__LINUX__ -DX86_ASM

# list of object files to generate
OBJECTS = \
	video.o \
	
PLUGIN_NAME = dummyvideo$(LIBSUFFIX)

# build targets
all: $(PLUGIN_NAME)

clean:
	rm -f *.o *$(LIBSUFFIX)

# build rules
.cpp.o:
	$(CXX) -o $@ $(CFLAGS) $(SDL_FLAGS) $(GTK_FLAGS) -c $<

$(PLUGIN_NAME): $(OBJECTS)
	$(CXX) $^ $(LDFLAGS) $(ASM_LDFLAGS) $(PLUGIN_LDFLAGS) $(SDL_LIBS) $(GTK_LIBS) $(GTHREAD_LIBS) $(LIBGL_LIBS) -o $@
	$(STRIP) $@

video.o: video.cpp
	$(CXX) $(CFLAGS) $(SDL_FLAGS) -DMAINDEF -c -o $@ $<

