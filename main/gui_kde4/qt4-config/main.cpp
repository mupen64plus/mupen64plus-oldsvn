/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - main.cpp                                                *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2008 Louai Al-Khanji                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <iostream>
#include <QtCore>

using std::cerr;
using std::cout;

void usage(const char* binaryName)
{
    cerr << binaryName << " <argument>\n";
    cerr << "Arguments: include, lib, bin\n";
    exit(1);
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    QStringList args = app.arguments();
    if (args.count() != 2) {
        usage(argv[0]);
    } else {
        QString arg = args.at(1);
        QLibraryInfo::LibraryLocation res = QLibraryInfo::PrefixPath;
        if (arg == "include") {
            res = QLibraryInfo::HeadersPath;
        } else if (arg == "lib") {
            res = QLibraryInfo::LibrariesPath;
        } else if (arg == "bin") {
            res = QLibraryInfo::BinariesPath;
        } else {
            usage(argv[0]);
        }
        cout << qPrintable(QLibraryInfo::location(res)) << "\n";
    }
    return 0;
}
