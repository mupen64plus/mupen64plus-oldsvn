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

#include <KGlobal>
#include <KLocale>
#include <KGlobalSettings>

#include <cstdio>

#include "rommodel.h"
#include "globals.h"
#include "settings.h"

namespace core {
    extern "C" {
        #include "../rom.h"
        #include "../mupenIniApi.h"
        #include "../../memory/memory.h" // for sl()
        #include "../main.h"
    }
}

RomModel::RomModel(QObject* parent)
    : QAbstractItemModel(parent)
    , m_showFullPath(Settings::showFullPathsInFilenames())
    , m_romDirectories(Settings::romDirectories())
{
    m_columnHeaders << i18n("Flag")
                    << i18n("Good Name")
                    << i18n("Country")
                    << i18n("Size")
                    << i18n("Comments")
                    << i18n("File Name");

    QPixmap paustralia(core::get_iconpath("australia.png"));
    QPixmap peurope(core::get_iconpath("europe.png"));
    QPixmap pfrance(core::get_iconpath("france.png"));
    QPixmap pgermany(core::get_iconpath("germany.png"));
    QPixmap pitaly(core::get_iconpath("italy.png"));
    QPixmap pjapan(core::get_iconpath("japan.png"));
    QPixmap pspain(core::get_iconpath("spain.png"));
    QPixmap pusa(core::get_iconpath("usa.png"));
    QPixmap pjapanusa(core::get_iconpath("japanusa.png"));
    QPixmap pn64cart(core::get_iconpath("n64cart.xpm"));
    
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
    
    m_countryInfo[0] = demo;
    m_countryInfo['7'] = beta;
    m_countryInfo[0x41] = japanusa;
    m_countryInfo[0x44] = germany;
    m_countryInfo[0x45] = usa;
    m_countryInfo[0x46] = france;
    m_countryInfo['I'] = italy;
    m_countryInfo[0x4A] = japan;
    m_countryInfo['S'] = spain;
    m_countryInfo[0x55] = australia;
    m_countryInfo[0x59] = australia;
    m_countryInfo[0x50] = europe;
    m_countryInfo[0x58] = europe;
    m_countryInfo[0x20] = europe;
    m_countryInfo[0x21] = europe;
    m_countryInfo[0x38] = europe;
    m_countryInfo[0x70] = europe;
    m_countryInfo['?'] = unknown;

    update();
}

K_GLOBAL_STATIC(RomModel, instance);
RomModel* RomModel::self()
{
    return instance;
}

void RomModel::update()
{
    KUrl url;
    QString abspath;
    char crc[BUF_MAX];
    
    m_romList.clear();

    foreach(QString directory, m_romDirectories) {
        foreach(QString romFile, QDir(directory).entryList(RomExtensions)) {
            url = directory;
            url.addPath(romFile);
            abspath = url.path();
            int size = core::fill_header(abspath.toLocal8Bit());
            if (size > 0) {
                RomEntry entry;
                entry.size = size;
                entry.fileName = abspath;
                std::snprintf(crc, BUF_MAX, "%08X-%08X-C%02X",
                                sl(core::ROM_HEADER->CRC1),
                                sl(core::ROM_HEADER->CRC2),
                                core::ROM_HEADER->Country_code);
                entry.cCountry = core::ROM_HEADER->Country_code;
                core::mupenEntry* iniEntry = core::ini_search_by_CRC(crc);
                if (iniEntry) {
                    entry.comments = iniEntry->comments;
                    entry.goodName = iniEntry->goodname;
                    // do we need to free inEntry? the gtk code seems to but
                    // we crash if we attempt it...
                    // free(iniEntry);
                } else {
                    entry.goodName = romFile;
                    entry.comments = i18n("No INI Entry");
                }
                m_romList << entry;
            }
        }
    }
    
    reset();
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
                    data = countryName(entry.cCountry);
                    break;
                case Size:
                    data = i18n("%0 Mbit").arg((entry.size * 8) / 1024 / 1024);
                    break;
                case Comments:
                    data = entry.comments;
                    break;
                case FileName:
                    {
                        if (Settings::showFullPathsInFilenames())
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
                    data = countryFlag(entry.cCountry);
                    break;
            }
        } else if (role == Sort) {
            switch(index.column()) {
                case GoodName:
                    data = entry.goodName;
                    break;
                case Flag:
                case Country:
                    data = countryName(entry.cCountry);
                    break;
                case Size:
                    data = entry.size;
                    break;
                case Comments:
                    data = entry.comments;
                    break;
                case FileName:
                    {
                        if (Settings::showFullPathsInFilenames())
                            data = entry.fileName;
                        else
                            data = KUrl(entry.fileName).fileName();
                    }
                    break;
            }
        }
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
    if (Settings::romDirectories() != m_romDirectories) {
        m_romList.clear();
        m_romDirectories = Settings::romDirectories();
        update();
    }
    if (m_showFullPath != Settings::showFullPathsInFilenames()) {
        m_showFullPath = Settings::showFullPathsInFilenames();
        emit dataChanged(
            createIndex(0, FileName),
            createIndex(columnCount() - 1, FileName)
        );
    }
}

QPixmap RomModel::countryFlag(unsigned char c) const
{
    if (!m_countryInfo.contains(c)) {
        c = '?';
    }
    return m_countryInfo[c].second;
}

QString RomModel::countryName(unsigned char c) const
{
    if (!m_countryInfo.contains(c)) {
        c = '?';
    }
    return m_countryInfo[c].first;
}

#include "rommodel.moc"
