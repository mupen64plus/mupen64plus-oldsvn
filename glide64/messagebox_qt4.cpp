/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*   Mupen64plus - messagebox_qt4.cpp                                      *
*   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
*   Copyright (C) 2008 slougi                                             *
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

#include <QPushButton>
#include <QMessageBox>
#include <stdarg.h>

#include "icons/messagebox-error.xpm"
#include "icons/messagebox-info.xpm"
#include "icons/messagebox-quest.xpm"
#include "icons/messagebox-warn.xpm"

static const int MAX = 2048;

extern "C" {

#include "messagebox.h"

int messagebox(const char *title, int flags, const char *fmt, ...)
{
    va_list ap;
    char buf[MAX];

    va_start( ap, fmt );
    vsnprintf( buf, MAX, fmt, ap );
    va_end( ap );

    QMessageBox mb;
    mb.setWindowTitle(title);
    mb.setText(buf);

    switch( flags & 0x00000F00 )
    {
        case MB_ICONWARNING:
            mb.setIconPixmap(QPixmap(messagebox_warn_xpm));
            break;

        case MB_ICONINFORMATION:
            mb.setIconPixmap(QPixmap(messagebox_info_xpm));
            break;

        case MB_ICONQUESTION:
            mb.setIconPixmap(QPixmap(messagebox_quest_xpm));
            break;

        case MB_ICONERROR:
            mb.setIconPixmap(QPixmap(messagebox_error_xpm));
            break;
    }

    QAbstractButton *button1, *button2, *button3;
    button1 = button2 = button3 = 0;

    switch( flags & 0x000000FF )
    {
    case MB_ABORTRETRYIGNORE:
        button1 = mb.addButton(QWidget::tr("Abort"), QMessageBox::RejectRole);
        button2 = mb.addButton(QWidget::tr("Retry"), QMessageBox::AcceptRole);
        button3 = mb.addButton(QWidget::tr("Ignore"), QMessageBox::AcceptRole);
        break;

    case MB_CANCELTRYCONTINUE:
        button1 = mb.addButton(QWidget::tr("Cancel"), QMessageBox::RejectRole);
        button2 = mb.addButton(QWidget::tr("Retry"), QMessageBox::AcceptRole);
        button3 = mb.addButton(QWidget::tr("Continue"), QMessageBox::AcceptRole);
        break;

    case MB_OKCANCEL:
        button1 = mb.addButton(QWidget::tr("OK"), QMessageBox::AcceptRole);
        button2 = mb.addButton(QWidget::tr("Cancel"), QMessageBox::RejectRole);
        break;

    case MB_RETRYCANCEL:
        button1 = mb.addButton(QWidget::tr("Retry"), QMessageBox::AcceptRole);
        button2 = mb.addButton(QWidget::tr("Cancel"), QMessageBox::RejectRole);
        break;

    case MB_YESNO:
        button1 = mb.addButton(QWidget::tr("Yes"), QMessageBox::YesRole);
        button2 = mb.addButton(QWidget::tr("No"), QMessageBox::NoRole);
        break;

    case MB_YESNOCANCEL:
        button1 = mb.addButton(QWidget::tr("Yes"), QMessageBox::YesRole);
        button2 = mb.addButton(QWidget::tr("No"), QMessageBox::NoRole);
        button3 = mb.addButton(QWidget::tr("Cancel"), QMessageBox::RejectRole);
        break;

    case MB_OK:
    default:
        button1 = mb.addButton(QWidget::tr("OK"), QMessageBox::AcceptRole);
    }

    mb.exec();

    if (button1 == mb.clickedButton()) {
        return 1;
    } else if (button2 == mb.clickedButton()) {
        return 2;
    } else if (button3 == mb.clickedButton()) {
        return 3;
    }
}

} // extern "C"
