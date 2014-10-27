/*

 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

 ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include "RawSkeleton.h"
#include <Core/Utils/vtkPolyDataUtils.h>

// VTK
#include <vtkPolyData.h>

// Qt
#include <QDir>

// C++
#include <memory>

namespace ESPINA
{
  static QString RAWSKELETON_DATAFILE = QString("%1_SkeletonData.vtp");

  //----------------------------------------------------------------------------
  RawSkeleton::RawSkeleton(OutputSPtr output)
  : m_skeleton{nullptr}
  {
    this->setOutput(output.get());
  }

  //----------------------------------------------------------------------------
  RawSkeleton::RawSkeleton(vtkSmartPointer<vtkPolyData> skeleton,
                           const NmVector3 &spacing,
                           OutputSPtr output)
  : m_skeleton{skeleton}
  {
    this->setOutput(output.get());
    m_output->setSpacing(spacing);
  }

  //----------------------------------------------------------------------------
  bool RawSkeleton::fetchData(const TemporalStorageSPtr storage, const QString& prefix)
  {
    bool dataFetched = false;

    QString fileName = storage->absoluteFilePath(prefix + QString(RAWSKELETON_DATAFILE).arg(m_output->id()));

    QFileInfo file(fileName);

    if(file.exists())
    {
      m_skeleton = PolyDataUtils::readPolyDataFromFile(fileName);
      dataFetched = true;
    }

    return dataFetched;
  }

  //----------------------------------------------------------------------------
  Snapshot RawSkeleton::snapshot(TemporalStorageSPtr storage, const QString &prefix) const
  {
    QString fileName = prefix + QString(RAWSKELETON_DATAFILE).arg(m_output->id());
    Snapshot snapshot;

    storage->makePath(prefix);

    if (m_skeleton)
    {
      snapshot << SnapshotData(fileName, PolyDataUtils::savePolyDataToBuffer(m_skeleton));
    }

    return snapshot;
  }

  //----------------------------------------------------------------------------
  vtkSmartPointer<vtkPolyData> RawSkeleton::skeleton() const
  {
    return m_skeleton;
  }

  //----------------------------------------------------------------------------
  size_t RawSkeleton::memoryUsage() const
  {
    if (m_skeleton)
      return m_skeleton->GetActualMemorySize();

    return 0;
  }

  //----------------------------------------------------------------------------
  bool RawSkeleton::isValid() const
  {
    return (m_skeleton.Get() != nullptr);
  }

  //----------------------------------------------------------------------------
  bool RawSkeleton::isEmpty() const
  {
    return !isValid();
  }

  //----------------------------------------------------------------------------
  bool RawSkeleton::setInternalData(SkeletonDataSPtr rhs)
  {
    m_skeleton = rhs->skeleton();
    return true;
  }

  //----------------------------------------------------------------------------
  RawSkeletonSPtr rawSkeleton(OutputSPtr output)
  {
    RawSkeletonSPtr data = std::dynamic_pointer_cast<RawSkeleton>(output->data(SkeletonData::TYPE));
    return data;
  }

} // namespace EspINA
