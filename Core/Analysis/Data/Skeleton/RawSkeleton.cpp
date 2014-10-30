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
  static QString RAWSKELETON_DATAFILE               = QString("%1_SkeletonData.vtp");
  static QString RAWSKELETON_EDITEDREGIONS_DATAFILE = QString("%1_EditedRegionSkeletonData.vtp");

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
    bool isValid = true;
    isValid &= (m_skeleton.Get() != nullptr);

    if(isEdited())
    {
      isValid &= (m_editedRegionsSkeleton != nullptr);
    }

    return isValid;
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

  //----------------------------------------------------------------------------
  bool RawSkeleton::fetchData(const TemporalStorageSPtr storage, const QString& path, const QString& id)
  {
    bool dataFetched = false;

    QString fileName = storage->absoluteFilePath(path + QString(RAWSKELETON_DATAFILE).arg(id));

    QFileInfo file(fileName);

    if(file.exists())
    {
      m_skeleton = PolyDataUtils::readPolyDataFromFile(fileName);
      dataFetched = true;
    }

    return dataFetched;
  }

  //----------------------------------------------------------------------------
  Snapshot RawSkeleton::snapshot(TemporalStorageSPtr storage, const QString& path, const QString& id) const
  {
    QString fileName = path + QString(RAWSKELETON_DATAFILE).arg(id);
    Snapshot snapshot;

    storage->makePath(path);

    if (m_skeleton)
    {
      snapshot << SnapshotData(fileName, PolyDataUtils::savePolyDataToBuffer(m_skeleton));
    }

    return snapshot;
  }

  //----------------------------------------------------------------------------
  Snapshot RawSkeleton::editedRegionsSnapshot(TemporalStorageSPtr storage, const QString& path, const QString& id) const
  {
    Snapshot snapshot;

    if(isEdited())
    {
      QString fileName = path + QString(RAWSKELETON_EDITEDREGIONS_DATAFILE).arg(id);
      snapshot << SnapshotData(fileName, PolyDataUtils::savePolyDataToBuffer(m_editedRegionsSkeleton));
    }

    return snapshot;
  }

  //----------------------------------------------------------------------------
  void RawSkeleton::restoreEditedRegions(TemporalStorageSPtr storage, const QString& path, const QString& id)
  {
    QString fileName = storage->absoluteFilePath(path + QString(RAWSKELETON_EDITEDREGIONS_DATAFILE).arg(id));

    QFileInfo file(fileName);

    if(file.exists())
    {
      m_editedRegionsSkeleton = PolyDataUtils::readPolyDataFromFile(fileName);

      double bounds[6];
      m_editedRegionsSkeleton->GetBounds(bounds);
      clearEditedRegions();
      addEditedRegion(Bounds{bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]});
    }
  }

} // namespace EspINA
