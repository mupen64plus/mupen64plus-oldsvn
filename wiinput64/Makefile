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
USES_GTK2 = true
include ../pre.mk

# local CFLAGS and PLUGIN_LDFLAGS
CFLAGS += -fpic -DPIC -D__LINUX__ -DX86_ASM -D_ENABLE_TILT -D_ENABLE_FORCE -Ilibcwiimote/src -Wall
PLUGIN_LDFLAGS += -Llibcwiimote -lcwiimote -lbluetooth -lm -lpthread

# if asked on the command line, append the GTK flags
ifeq ($(GUI),GTK2)
  CFLAGS += $(GTK_FLAGS) -DUSE_GTK
  PLUGIN_LDFLAGS += $(GTK_LIBS)
endif

# list of object files to generate
OBJECTS = wiinput64.o gui_gtk2.o gui_none.o

# build targets
all: wiinput64.so

clean:
	rm -f *.o *.so
	@(cd libcwiimote && $(MAKE) clean)

# build rules
wiinput64.so: $(OBJECTS) libcwiimote/libcwiimote.a
	$(CXX) $(OBJECTS) $(LDFLAGS) $(ASM_LDFLAGS) $(PLUGIN_LDFLAGS) -o $@
	$(STRIP) $@

%.o: %.c
	$(CXX) $(CFLAGS) -c -o $@ $<

libcwiimote/libcwiimote.a:
	@(cd libcwiimote && $(MAKE))
