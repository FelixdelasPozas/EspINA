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

namespace EspINA
{
  using SnapshotData = QPair<QString, QByteArray>;
  using Snapshot     = QList<SnapshotData>;

  class EspinaCore_EXPORT TemporalStorage
  {
  public:
    enum class Mode
    {
      Recursive
    , NoRecursive
    };

  public:
    explicit TemporalStorage(const QDir* parent = nullptr);
    ~TemporalStorage();

    /** \brief Write snapshot data to storage destination
     *
     *  This version uses disk as storage destination
     */
    void saveSnapshot(SnapshotData data);

    /* \brief Returns file absolute path if found in any storage created in the session.
     * \param[in] fileName File name to search for
     */
    QString findFile(const QString &fileName) const;

    QByteArray snapshot(const QString& descriptor) const;

    Snapshot snapshots(const QString& relativePath, Mode mode) const;

    void makePath(const QString& path)
    { m_storageDir.mkpath(path); }

    QString absoluteFilePath(const QString &path) const
    { return m_storageDir.absoluteFilePath(path); }

    /* \brief Returns true if final given as argument exists in this storage.
     *
     */
    bool exists(const QString &name);

  private:
    QUuid m_uuid;
    QDir  m_storageDir;
    static QList<TemporalStorage *> s_Storages;
  };

  using TemporalStorageSPtr = std::shared_ptr<TemporalStorage>;

  /* \brief Removes all files inside the "espina" folder in the OS temporal directory.
   *
   */
  bool removeTemporalDirectory();
}

#endif // ESPINA_TEMPORAL_STORAGE_H
