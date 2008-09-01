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
CFLAGS += -DGUI_SDL -fpic -DPIC

# list of object files to generate
OBJ_BLIGHT = \
	plugin.o \
	arial.ttf.o \
	configdialog_sdl.o \
	pad.o

# build targets
all: version.h blight_input.so

clean:
	rm -f *.o *.so ttftoh arial.ttf.c

# build rules
version.h: .svn/entries
	@sed 's|@MUPEN_VERSION@|\"$(MUPEN_VERSION)\"| ; s|@PLUGIN_VERSION@|\"$(PLUGIN_VERSION)\"|' \
        ../main/version.template > version.h
	@$(MV) version.h ../main/version.h

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

