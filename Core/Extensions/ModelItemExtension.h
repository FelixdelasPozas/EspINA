/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef MODELITEMEXTENSION_H
#define MODELITEMEXTENSION_H

#include "Core/Model/ModelItem.h"

#include <QDir>
#include <QStringList>
#include <QVariant>

#include <quazipfile.h>

namespace EspINA
{

  class ModelItem::Extension
  : public QObject
  {
    Q_OBJECT
  public:
    typedef QList<QPair<QString, QByteArray> > CacheList;

  public:
    virtual ~Extension(){}

    virtual ModelItem::ExtId id() = 0;
    /// List of extension names which need to be loaded to use the extension
    virtual ModelItem::ExtIdList dependencies() const  = 0;

    virtual void initialize(ModelItem::Arguments args = ModelItem::Arguments()) = 0;

    virtual bool isCacheFile(const QString &file) const = 0;

    virtual bool loadCache(QuaZipFile &file, const QDir &tmpDir, EspinaModel *model) = 0;

    virtual bool saveCache(CacheList &cacheList) = 0;

  public slots:
    virtual void invalidate() {};//TODO: Change to pure abstract

  protected:
    Extension() : m_init(false) {}
    mutable bool m_init;
  };

} // namespace EspINA

#endif // MODELITEMEXTENSION_H
