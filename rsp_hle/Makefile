#/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
# *   Mupen64plus - Makefile                                                *
# *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
# *   Copyright (C) 2007-2008 Pyromanik DarkJeztr Tillin9 Richard42         *
# *                                                                         *
# *   This program is free software; you can redistribute it and/or modify  *
# *   it under the terms of the GNU General Public License as published by  *
# *   the Free Software Foundation; either version 2 of the License, or     *
# *   (at your option) any later version.                                   *
# *                                                                         *
# *   This program is distributed in the hope that it will be useful,       *
# *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
# *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
# *   GNU General Public License for more details.                          *
# *                                                                         *
# *   You should have received a copy of the GNU General Public License     *
# *   along with this program; if not, write to the                         *
# *   Free Software Foundation, Inc.,                                       *
# *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
# * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
# Makefile for Mupen64 HLE RSP plugin in Mupen64plus.

# include pre-make file with a bunch of definitions
USES_QT4 = true
USES_GTK2 = true 
include ../pre.mk

# local CFLAGS, LIBS, and LDFLAGS
CFLAGS += -fpic -DPIC

ifeq ($(GUI), NONE)
  OBJECTS = gui_none.o
else
  ifeq ($(GUI), QT4)
    PLUGIN_LDFLAGS += $(QT_LIBS)
    CXXFLAGS += $(QT_FLAGS)
    OBJECTS = gui_qt4.o moc_ConfigDialog.o qrc_messagebox.o
  else
    CFLAGS += $(GTK_FLAGS)
    PLUGIN_LDFLAGS += $(GTK_LIBS)
    OBJECTS = gui_gtk2.o
  endif
endif

# list of object files to generate
OBJECTS += main.o disasm.o jpeg.o ucode3.o ucode2.o ucode1.o ucode3mp3.o config.o extension.o

# build targets
all: version.h mupen64_hle_rsp_azimer.so

clean:
	rm -f *.o *.so ui_*.h qrc_*.cpp moc_*.cpp

# build rules
version.h: .svn/entries
	@sed 's|@MUPEN_VERSION@|\"$(MUPEN_VERSION)\"| ; s|@PLUGIN_VERSION@|\"$(PLUGIN_VERSION)\"|' \
        ../main/version.template > version.h
	@$(MV) version.h ../main/version.h

.c.o:
	$(CC) -o $@ $(CFLAGS) -c $<

.cpp.o:
	$(CXX) -o $@ $(CFLAGS) -c $<

moc_ConfigDialog.cpp: ui_config.h ConfigDialog.h
	$(MOC) $(CXXFLAGS) ConfigDialog.h -o moc_ConfigDialog.cpp

qrc_messagebox.cpp: messagebox.qrc
	$(RCC) -name messagebox messagebox.qrc -o qrc_messagebox.cpp

ui_config.h: config.ui
	$(UIC) config.ui -o ui_config.h

gui_qt4.o: gui_qt4.cpp ConfigDialog.h ui_config.h gui.h wintypes.h config.h extension.h
	$(CXX) -c $(CXXFLAGS) -o gui_qt4.o gui_qt4.cpp

moc_ConfigDialog.o: moc_ConfigDialog.cpp 
	$(CXX) -c $(CXXFLAGS) -o moc_ConfigDialog.o moc_ConfigDialog.cpp

qrc_messagebox.o: qrc_messagebox.cpp 
	$(CXX) -c $(CXXFLAGS) -o qrc_messagebox.o qrc_messagebox.cpp

mupen64_hle_rsp_azimer.so: $(OBJECTS)
	$(CXX) $^ $(LDFLAGS) $(PLUGIN_LDFLAGS) -o $@
	$(STRIP) $@

