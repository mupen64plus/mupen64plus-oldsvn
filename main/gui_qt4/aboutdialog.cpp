/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - aboutdialog.cpp                                         *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2009 Louai Al-Khanji                                    *
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

#include <QtGlobal>
#include "aboutdialog.h"
#include "../version.h"

namespace core {
    extern "C" {
        #include "../main.h"
    }
}

AboutDialog::AboutDialog(QWidget* parent)
: QDialog(parent)
{
    setupUi(this);
    QString text;
    text = tr(
        "<html>"

        "<p><center>"
        "<a href=\"http://code.google.com/p/mupen64plus/\">"
        "<img src=\"%1mupen64logo.png\"></a>"
        "<br/>"
        "Version %2<br/>"
        "<a href=\"http://code.google.com/p/mupen64plus/\">"
        "Home Page</a>"
        "<br/>"
        "<a href=\"http://www.emutalk.net/forumdisplay.php?f=113\">"
        "Forum on EmuTalk.net</a>"
        "</center></p>"

        "<p><center>"
        "Copyright &copy; 2007-2009 The Mupen64Plus Team<br/>"
        "<a href=\"http://www.gnu.org/licenses/old-licenses/gpl-2.0.html\">"
        "Licensing: GNU GPL version 2 or later</a>"
        "</center></p>"

        "</html>");
    text = text.arg(core::get_iconspath()).arg(MUPEN_VERSION);
    textBrowser->setHtml(text);
}
