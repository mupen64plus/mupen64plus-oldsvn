#***************************************************************************
#                            wiinput64 Makefile
#                            -------------------
# Copyright (C) 2009 Lionel Fuentes (Funto)
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

# local CFLAGS and PLUGIN_LDFLAGS
CFLAGS += -fpic -DPIC -D__LINUX__ -DX86_ASM -D_ENABLE_TILT -D_ENABLE_FORCE -Ilibcwiimote/src -Wall
PLUGIN_LDFLAGS += -Llibcwiimote -lcwiimote -lbluetooth -lm -lpthread

# GTK version (TODO : use the variables given by ../pre.mk and make the "if" work ^^)
# if asked on the command line, append the GTK flags
#ifeq ($GUI,GTK2)
CFLAGS += `pkg-config gtk+-2.0 --cflags` -DUSE_GTK
PLUGIN_LDFLAGS += `pkg-config gtk+-2.0 --libs`
#endif

# list of object files to generate
OBJECTS = wiinput64.o gui_gtk2.o gui_none.o

# build targets
all: ../plugins/wiinput64.so

clean:
	rm -f *.o *.so
	@(cd libcwiimote && $(MAKE) clean)

# build rules
../plugins/wiinput64.so: $(OBJECTS) libcwiimote/libcwiimote.a
	$(CXX) $(OBJECTS) $(LDFLAGS) $(ASM_LDFLAGS) $(PLUGIN_LDFLAGS) -o $@
	$(STRIP) $@

%.o: %.c
	$(CXX) $(CFLAGS) -c -o $@ $<

libcwiimote/libcwiimote.a:
	@(cd libcwiimote && $(MAKE))
