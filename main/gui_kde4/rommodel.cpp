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

#include <QStringList>
#include <QDir>
#include <QFont>

#include <KUrl>
#include <KGlobal>
#include <KLocale>
#include <KGlobalSettings>

#include <cstdio>

#include "rommodel.h"
#include "globals.h"

namespace core {
    extern "C" {
        #include "../rom.h"
        #include "../romcache.h"
        #include "../main.h"
        #include "../config.h"
    }
}

RomModel::RomModel(QObject* parent)
    : QAbstractItemModel(parent)
    , m_showFullPath(core::config_get_bool("RomBrowserShowFullPaths", FALSE))
    , m_romDirectories(romDirectories())
{
    m_columnHeaders << i18n("Flag")
                    << i18n("Good Name")
                    << i18n("Country")
                    << i18n("Size")
                    << i18n("Comments")
                    << i18n("File Name");

    QString file;
    char* iconpath = core::get_iconspath();
    QPixmap paustralia(file.sprintf("%s%s", iconpath, "australia.png"));
    QPixmap peurope(file.sprintf("%s%s", iconpath, "europe.png"));
    QPixmap pfrance(file.sprintf("%s%s", iconpath, "france.png"));
    QPixmap pgermany(file.sprintf("%s%s", iconpath, "germany.png"));
    QPixmap pitaly(file.sprintf("%s%s", iconpath, "italy.png"));
    QPixmap pjapan(file.sprintf("%s%s", iconpath, "japan.png"));
    QPixmap pspain(file.sprintf("%s%s", iconpath, "spain.png"));
    QPixmap pusa(file.sprintf("%s%s", iconpath, "usa.png"));
    QPixmap pjapanusa(file.sprintf("%s%s", iconpath, "japanusa.png"));
    QPixmap pn64cart(file.sprintf("%s%s", iconpath, "mupen64cart.png"));

    QPair<QString, QPixmap> demo(i18n("Demo"), pn64cart);
    QPair<QString, QPixmap> beta(i18n("Beta"), pn64cart);
    QPair<QString, QPixmap> japanusa(i18n("Japan/USA"), pjapanusa);
    QPair<QString, QPixmap> usa(i18n("USA"), pusa);
    QPair<QString, QPixmap> germany(i18n("Germany"), pgermany);
    QPair<QString, QPixmap> france(i18n("France"), pfrance);
    QPair<QString, QPixmap> italy(i18n("Italy"), pitaly);
    QPair<QString, QPixmap> japan(i18n("Japan"), pjapan);
    QPair<QString, QPixmap> spain(i18n("Spain"), pspain);
    QPair<QString, QPixmap> australia(i18n("Australia"), paustralia);
    QPair<QString, QPixmap> europe(i18n("Europe"), peurope);
    QPair<QString, QPixmap> unknown(i18n("Unknown"), pn64cart);

    m_countryInfo[char(0)] = demo;
    m_countryInfo['7'] = beta;
    m_countryInfo[char(0x41)] = japanusa;
    m_countryInfo[char(0x44)] = germany;
    m_countryInfo[char(0x45)] = usa;
    m_countryInfo[char(0x46)] = france;
    m_countryInfo['I'] = italy;
    m_countryInfo[char(0x4A)] = japan;
    m_countryInfo['S'] = spain;
    m_countryInfo[char(0x55)] = australia;
    m_countryInfo[char(0x59)] = australia;
    m_countryInfo[char(0x50)] = europe;
    m_countryInfo[char(0x58)] = europe;
    m_countryInfo[char(0x20)] = europe;
    m_countryInfo[char(0x21)] = europe;
    m_countryInfo[char(0x38)] = europe;
    m_countryInfo[char(0x70)] = europe;
    m_countryInfo['?'] = unknown;
}

K_GLOBAL_STATIC(RomModel, instance);
RomModel* RomModel::self()
{
    return instance;
}

void RomModel::update(unsigned int roms, unsigned short clear)
{
    //If clear flag is set, clear the GUI rombrowser.
    if(clear)
        { m_romList.clear(); }

    int arrayroms = m_romList.count();

    //If there are currently more ROMs in cache than GUI rombrowser, add them.
    if(roms>arrayroms)
        {
        core::cache_entry *entry;
        entry = core::g_romcache.top;
        unsigned int romcounter;

        //Advance cache pointer.
        for ( romcounter=0; romcounter < arrayroms; ++romcounter )
            {
            entry = entry->next;
            if((entry==NULL))
                { return; }
            }

        RomEntry modelentry;
        for ( romcounter=0; (romcounter<roms)&&(entry!=NULL); ++romcounter )
            {
            modelentry.size = entry->romsize;
            modelentry.goodName = entry->inientry->goodname;
            modelentry.country = entry->countrycode;
            modelentry.fileName = entry->filename;
            modelentry.comments = entry->usercomments;
            modelentry.archivefile = entry->archivefile;

            //Actually add entries to RomModel
            m_romList << modelentry;
            //printf("Added: %s\n", entry->inientry->goodname);
            entry = entry->next;
            }
        reset();
        }
}

QModelIndex RomModel::index(int row, int column,
                             const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return createIndex(row, column);
}
 
QModelIndex RomModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int RomModel::rowCount(const QModelIndex& parent) const
{
    int retval = 0;

    if (!parent.isValid())
        return m_romList.count();

    return retval;
}

int RomModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_columnHeaders.count();
}

QVariant RomModel::data(const QModelIndex& index, int role) const
{
    QVariant data;
    if (index.isValid()) {
        const RomEntry& entry = m_romList[index.row()];
        if (role == Qt::DisplayRole) {
            switch(index.column()) {
                case Flag:
                    // Country flag only
                    break;
                case GoodName:
                    data = entry.goodName;
                    break;
                case Country:
                    data = countryName(entry.country);
                    break;
                case Size:
                    data = i18n("%0 Mbit").arg((entry.size * 8) / 1024 / 1024);
                    break;
                case Comments:
                    data = entry.comments;
                    break;
                case FileName:
                    {
                        if (core::config_get_bool("RomBrowserShowFullPaths", FALSE))
                            data = entry.fileName;
                        else
                            data = KUrl(entry.fileName).fileName();
                    }
                    break;
                default:
                    data = i18n("Internal error");
                    break;
            }
        } else if (role == Qt::FontRole) {
            switch(index.column()) {
                case Size:
                    data = KGlobalSettings::fixedFont();
                    break;
            }
        } else if (role == Qt::TextAlignmentRole) {
            switch(index.column()) {
                case Size:
                    data = Qt::AlignRight;
                    break;
            }
        } else if (role == Qt::DecorationRole) {
            switch(index.column()) {
                case Flag:
                    data = countryFlag(entry.country);
                    break;
            }
        } else if (role == Sort) {
            switch(index.column()) {
                case GoodName:
                    data = entry.goodName;
                    break;
                case Flag:
                case Country:
                    data = countryName(entry.country);
                    break;
                case Size:
                    data = entry.size;
                    break;
                case Comments:
                    data = entry.comments;
                    break;
                case FileName:
                    {
                        if (core::config_get_bool("RomBrowserShowFullPaths", FALSE))
                            data = entry.fileName;
                        else
                            data = KUrl(entry.fileName).fileName();
                    }
                    break;
            }
        }
        //Assign Role enums here to retrive information.
        else if (role == FullPath) 
            { data = entry.fileName; }
        else if (role == ArchiveFile) 
            { data = entry.archivefile; }
    }
    return data;
}

QVariant RomModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    QVariant data;
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            data = m_columnHeaders[section];
        }
    }
    return data;
}

void RomModel::settingsChanged()
{
    if (romDirectories() != m_romDirectories) {
        m_romDirectories = romDirectories();
        core::g_romcache.rcstask = core::RCS_RESCAN;
    }
    if (m_showFullPath != core::config_get_bool("RomBrowserShowFullPaths", FALSE)) {
        m_showFullPath = core::config_get_bool("RomBrowserShowFullPaths", FALSE);
        emit dataChanged(
            createIndex(0, FileName),
            createIndex(columnCount() - 1, FileName)
        );
    }
}

QPixmap RomModel::countryFlag(QChar c) const
{
    if (!m_countryInfo.contains(c)) {
        c = '?';
    }
    return m_countryInfo[c].second;
}

QString RomModel::countryName(QChar c) const
{
    if (!m_countryInfo.contains(c)) {
        c = '?';
    }
    return m_countryInfo[c].first;
}

#include "rommodel.moc"
