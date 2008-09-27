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

#ifndef MUPEN64_STAR_LABEL_H
#define MUPEN64_STAR_LABEL_H

#include <QLabel>

class StarLabel : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(int max READ max WRITE setMax)
    public:
        StarLabel(QWidget* parent = 0);

        int max() const;
        void setMax(int max);

    protected:
        void paintEvent(QPaintEvent* event);

    private:
        int m_max;
};

#endif // MUPEN64_STAR_LABEL_H

