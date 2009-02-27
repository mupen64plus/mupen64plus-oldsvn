#/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
# *   Mupen64plus - guidbg_qt4.pro                                          *
# *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
# *   Copyright (C) 2009 olejl                                              *
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

TEMPLATE = lib
CONFIG += staticlib
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += sdl
}

# Input
HEADERS += debuggerwidget.h
HEADERS += registerwidget.h
HEADERS += breakpointswidget.h
HEADERS += hexspinboxdialog.h
HEADERS += tablelistmodel.h
HEADERS += hexspinbox.h
HEADERS += memeditmodel.h
HEADERS += memeditwidget.h
FORMS += debuggerwidget.ui
FORMS += registerwidget.ui
FORMS += breakpointswidget.ui
FORMS += hexspinboxdialog.ui
FORMS += memeditwidget.ui
SOURCES += debuggerwidget.cpp
SOURCES += registerwidget.cpp
SOURCES += breakpointswidget.cpp
SOURCES += hexspinboxdialog.cpp
SOURCES += tablelistmodel.cpp
SOURCES += hexspinbox.cpp
SOURCES += memeditmodel.cpp
SOURCES += memeditwidget.cpp
