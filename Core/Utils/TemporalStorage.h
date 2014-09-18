/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// Qt
#include <QPair>
#include <QDir>
#include <QUuid>

// C++
#include <memory>
#include <cstdint>

namespace ESPINA
{
  using SnapshotData = QPair<QString, QByteArray>;
  using Snapshot     = QList<SnapshotData>;

  class EspinaCore_EXPORT TemporalStorage
  {
  public:
    enum class Mode: std::int8_t { Recursive = 1, NoRecursive = 2 };

  public:
    /* \brief TemporalStorage class constructor.
     * \param[in] parent, parent dir of the storage.
     *
     */
    explicit TemporalStorage(const QDir* parent = nullptr);

    /* \brief TemporalStorage class destructor.
     *
     */
    ~TemporalStorage();

    /** \brief Write snapshot data to storage destination.
     *
     *  This version uses disk as storage destination.
     */
    void saveSnapshot(SnapshotData data);

    /* \brief Returns file absolute path if found in any storage created in the session.
     * \param[in] fileName File name to search for.
     */
    QString findFile(const QString &fileName) const;

    /* \brief Returns the contents of the specified file contained in the storage.
     * \param[in] descriptor, file name.
     *
     */
    QByteArray snapshot(const QString& descriptor) const;

    /* \brief Returns the content of a directory in the storage as a Snapshot object.
     * \param[in] relativePath, path relative to storage parent directory.
     * \param[in] mode,
     *
     */
    Snapshot snapshots(const QString& relativePath, Mode mode) const;

    /* \brief Makes a path in the storage parent directory.
     * \param[in] path, path relative to storage parent directory.
     *
     */
    void makePath(const QString& path)
    { m_storageDir.mkpath(path); }

    /* \brief Returns the absolute file path of the specified path in the storage.
     * \param[in] path, path relative to storage parent directory.
     *
     */
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
  bool EspinaCore_EXPORT removeTemporalDirectory();
}

#endif // ESPINA_TEMPORAL_STORAGE_H
