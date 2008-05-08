# Makefile for Mupen64

# include pre-make file with a bunch of definitions
include ../pre.mk

# list of object files to generate
OBJ_BLIGHT = \
	plugin.o

OBJ_GTK_GUI = \
	gui_gtk/gui.o

OBJ_SDL_GUI = \
	gui_sdl/arial.ttf.o \
	gui_sdl/gui.o \
	gui_sdl/pad.o

# local CFLAGS, LIBS, and LDFLAGS
ifeq ($(GUI), GTK)
  CFLAGS += $(GTK_FLAGS)
  OBJ_BLIGHT += $(OBJ_GTK_GUI)
  LIBS += $(GTK_LIBS)
else
  CFLAGS += $(SDL_FLAGS)
  OBJ_BLIGHT += $(OBJ_SDL_GUI)
  LIBS += -lSDL_ttf $(SDL_LIBS)
endif

# build targets
all: blight_input.so

clean:
	rm -f *.so *.o gui_sdl/*.o gui_sdl/ttftoh gui_sdl/arial.ttf.c gui_gtk/*.o

# build rules
.c.o:
	$(CC) -o $@ $(CFLAGS) -c $<

blight_input.so: $(OBJ_BLIGHT)
	$(CC) $^ $(LDFLAGS) $(PLUGIN_LDFLAGS) $(LIBS) -o $@
	$(STRIP) $@

gui_sdl/arial.ttf.c: gui_sdl/ttftoh
	./gui_sdl/ttftoh ./font/arial.ttf
	$(MV) ./font/arial.ttf.h ./gui_sdl/arial.ttf.c

gui_sdl/ttftoh: gui_sdl/ttftoh.o
	$(CC) $^ -o $@
	$(STRIP) $@

gui_sdl/ttftoh.o: gui_sdl/ttftoh.c
	$(CC) -o $@ -O3 -c $<

