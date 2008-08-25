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

#ifndef MUPEN_KDE4_ROMMODEL_H
#define MUPEN_KDE4_ROMMODEL_H

#include <QAbstractItemModel>
#include <QStringList>
#include <QPixmap>
#include <QPair>
#include <QChar>

extern "C" {
    namespace core {
        #include "../romcache.h"
    }
}

class RomModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_ENUMS(Columns)
    Q_ENUMS(Role)
    public:
        enum Columns {
            Country = 0,
            GoodName,
            Status,
            Size,
            FileName,
            InternalName,
            Comments,
            MD5,
            CRC1,
            CRC2,
            SaveType,
            Players,
            Rumble,
            CompressionType,
            ImageType,
            CIC,
            TimeStamp,
            COLUMNS_SENTINEL, // keep this as the last entry
            LAST_VISIBLE_COLUMN = FileName // except this one may come after!
        };
        enum Role {
            Sort = Qt::UserRole,
            FullPath,
            ArchiveFile
        };

        RomModel(QObject* parent = 0);

        static RomModel* self();
        void update(unsigned int roms, unsigned short clear);

        // Model method implementations
        virtual QModelIndex index(int row, int column,
                           const QModelIndex& parent = QModelIndex()) const;
        virtual QModelIndex parent(const QModelIndex& index) const;
        virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
        virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
        virtual QVariant data(const QModelIndex& index,
                               int role = Qt::DisplayRole) const;
        virtual QVariant headerData(int section, Qt::Orientation orientation,
                                     int role = Qt::DisplayRole) const;
        void settingsChanged();

    private:
        QPixmap countryFlag(QChar c) const;
        QString countryName(QChar c) const;

        QStringList m_columnHeaders;
        QList<core::cache_entry*> m_romList;

        bool m_showFullPath;
        QStringList m_romDirectories;
        QMap<QChar, QPair<QString, QPixmap> > m_countryInfo;
};

#endif // MUPEN_KDE4_ROMMODEL_H
