/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_TEMPORAL_STORAGE_H
#define ESPINA_TEMPORAL_STORAGE_H

#include "Core/EspinaCore_Export.h"

#include <QPair>
#include <QDir>
#include <QUuid>

#include <memory>

namespace EspINA {

  using SnapshotData = QPair<QString, QByteArray>;
  using Snapshot     = QList<SnapshotData>;

  class EspinaCore_EXPORT TemporalStorage
  {
  public:
    explicit TemporalStorage(const QDir& parent);
    ~TemporalStorage();

    /** \brief Write snapshot data to storage destination
     *
     *  This version uses disk as storage destination
     */
    void saveSnapshot(SnapshotData data);

    QByteArray snapshot(const QString& descriptor) const;

    void makePath(const QString& path)
    { m_storageDir.mkpath(path); }

    QString absoluteFilePath(const QString &path) const
    { return m_storageDir.absoluteFilePath(path); }

  private:
    QUuid m_uuid;
    QDir  m_storageDir;
  };

  using TemporalStorageSPtr = std::shared_ptr<TemporalStorage>;
}

#endif // ESPINA_TEMPORAL_STORAGE_H