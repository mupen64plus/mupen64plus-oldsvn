# Makefile for Mupen64

# include pre-make file with a bunch of definitions
FROMPLUGIN=1
include ../pre.mk

# local CFLAGS, LIBS, and LDFLAGS
SDLTTF_LIBS	= -lSDL_ttf
CFLAGS += -DGUI_SDL -fpic -DPIC

# list of object files to generate
OBJ_BLIGHT = \
	plugin.o \
	arial.ttf.o \
	configdialog_sdl.o \
	pad.o

# build targets
PLUGIN_NAME = blight_input$(LIBSUFFIX)
all: $(PLUGIN_NAME)

clean:
	rm -f *.o *$(LIBSUFFIX) ttftoh arial.ttf.c

# build rules
.c.o:
	$(CC) -o $@ $(CFLAGS) $(SDL_FLAGS) $(FREETYPE_FLAGS) -c $<

$(PLUGIN_NAME): $(OBJ_BLIGHT)
	$(CC) $^ $(LDFLAGS) $(PLUGIN_LDFLAGS) $(SDL_LIBS) $(SDLTTF_LIBS) $(FREETYPE_LIBS) -o $@
	$(STRIP) $@

configdialog_sdl.o: configdialog_sdl.c
	$(CC) -o $@ $(CFLAGS) $(SDL_FLAGS) -c $<

plugin.o: plugin.c
	$(CC) -o $@ $(CFLAGS) $(SDL_FLAGS) -c $<

arial.ttf.c: ttftoh
	./ttftoh ./font/arial.ttf
	$(MV) ./font/arial.ttf.h ./arial.ttf.c

ttftoh: ttftoh.o
	$(CC) $^ -o $@
	$(STRIP) $@

ttftoh.o: ttftoh.c
	$(CC) -o $@ -O3 -c $<

