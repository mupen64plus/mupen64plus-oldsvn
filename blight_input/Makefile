# Makefile for Mupen64

# include pre-make file with a bunch of definitions
include ../pre.mk

# local CFLAGS, LIBS, and LDFLAGS
SDLTTF_LIBS	= -lSDL_ttf
CFLAGS += -DGUI_SDL -fpic -DPIC

ifeq ($(GUI), SDL)
  CFLAGS += -DGUI_SDL
  OBJ_BLIGHT += $(OBJ_SDL_GUI)
  LIBS += -lSDL_ttf
endif

# list of object files to generate
OBJ_BLIGHT = \
	plugin.o \
	pad.o

OBJ_GTK_GUI = \
	gui_gtk/configdialog.o

OBJ_SDL_GUI = \
	gui_sdl/arial.ttf.o \
	gui_sdl/configdialog_sdl.o

# build targets
all: blight_input.so

clean:
	rm -f *.o *.so ttftoh arial.ttf.c

# build rules
.c.o:
	$(CC) -o $@ $(CFLAGS) $(SDL_FLAGS) $(FREETYPE_FLAGS) -c $<

blight_input.so: $(OBJ_BLIGHT)
	$(CC) $^ $(LDFLAGS) $(LIBS) -o $@
	$(STRIP) $@

gui_sdl/configdialog_sdl.o: configdialog_sdl.c
	$(CC) -o $@ $(CFLAGS) $(SDL_FLAGS) -c $<

plugin.o: plugin.c
	$(CC) -o $@ $(CFLAGS) $(SDL_FLAGS) -c $<

arial.ttf.c: ttftoh
	./ttftoh ./font/arial.ttf
	$(MV) ./font/arial.ttf.h ./arial.ttf.c

gui_sdl/ttftoh: ttftoh.o
	$(CC) $^ -o $@
	$(STRIP) $@

gui_sdl/ttftoh.o: ttftoh.c
	$(CC) -o $@ -O3 -c $<

