#***************************************************************************
#                           dummy_audio Makefile
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
CFLAGS += -fpic -DPIC

# list of object files to generate
OBJECTS = audio.o

# build targets
all: dummyaudio.so

clean:
	rm -f *.o *.so

# build rules
.c.o:
	$(CC) -o $@ $(CFLAGS) -c $<

dummyaudio.so: $(OBJECTS)
	$(CC) $^ $(LDFLAGS) $(PLUGIN_LDFLAGS) -o $@
	$(STRIP) $@

