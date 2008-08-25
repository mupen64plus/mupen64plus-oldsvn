/*
* Copyright (C) 2008 Louai Al-Khanji
*
* This program is free software; you can redistribute it and/
* or modify it under the terms of the GNU General Public Li-
* cence as published by the Free Software Foundation; either
* version 2 of the Licence, or any later version.
*
* This program is distributed in the hope that it will be use-
* ful, but WITHOUT ANY WARRANTY; without even the implied war-
* ranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public Licence for more details.
*
* You should have received a copy of the GNU General Public
* Licence along with this program; if not, write to the Free
* Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
* USA.
*
*/

#ifndef MUPEN64_KDE4_GLOBALS_H
#define MUPEN64_KDE4_GLOBALS_H

#include <QStringList>
#include <QIcon>

const int BUF_MAX = 512;
const QStringList RomExtensions = QStringList() << "*.rom"
                                                << "*.v64"
                                                << "*.z64"
                                                << "*.gz"
                                                << "*.zip"
                                                << "*.n64"
                                                << "*.bz2"
                                                << "*.lzma"
                                                << "*.7z";
QStringList romDirectories();
QIcon icon(QString iconName);
QPixmap pixmap(QString name, QString subdirectory = QString());

#endif // MUPEN64_KDE4_GLOBALS_H
