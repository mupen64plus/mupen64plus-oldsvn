#***************************************************************************
#                           blight_input Makefile
#                            -------------------
# Copyright (C) 2007-2008 Richard Goedeken (Richard42)
#***************************************************************************

#***************************************************************************
#*                                                                         *
#*   This program is free software; you can redistribute it and/or modify  *
#*   it under the terms of the GNU General Public License as published by  *
#*   the Free Software Foundation; either version 2 of the License, or     *
#*   (at your option) any later version.                                   *
#*                                                                         *
#***************************************************************************

# include pre-make file with a bunch of definitions
include ../pre.mk

# local CFLAGS, LIBS, and LDFLAGS
SDLTTF_LIBS	= -lSDL_ttf
ifneq ($(OS), IRIX)
  CFLAGS += -DGUI_SDL -fpic -DPIC
else
  CFLAGS += -DGUI_SDL -LANG:anonymous_unions=ON
  LDFLAGS += -shared
endif

# list of object files to generate
OBJ_BLIGHT = \
	plugin.o \
	arial.ttf.o \
	configdialog_sdl.o \
	pad.o

# build targets
all: blight_input.so

clean:
	rm -f *.o *.so ttftoh arial.ttf.c

# build rules
.c.o:
	$(CC) -o $@ $(CFLAGS) $(SDL_FLAGS) $(FREETYPE_FLAGS) -c $<

blight_input.so: $(OBJ_BLIGHT)
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

