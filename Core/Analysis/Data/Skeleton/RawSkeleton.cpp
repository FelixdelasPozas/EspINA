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
#include <Core/Analysis/Data/SkeletonDataUtils.h>
#include <Core/Utils/vtkPolyDataUtils.h>

// VTK
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkStringArray.h>

// C++
#include <memory>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::PolyDataUtils;

//--------------------------------------------------------------------
RawSkeleton::RawSkeleton(const NmVector3 &spacing, const NmVector3 &origin)
: m_skeleton{nullptr}
{
  m_bounds = VolumeBounds{Bounds(), spacing, origin};
}

//----------------------------------------------------------------------------
RawSkeleton::RawSkeleton(vtkSmartPointer<vtkPolyData> skeleton, const NmVector3 &spacing, const NmVector3 &origin)
: m_skeleton{skeleton}
{
  if(m_skeleton)
  {
    m_bounds = PolyDataUtils::polyDataVolumeBounds(skeleton, spacing, origin);
  }
}

//----------------------------------------------------------------------------
size_t RawSkeleton::memoryUsage() const
{
  QMutexLocker lock(&m_lock);

  return (m_skeleton ? m_skeleton->GetActualMemorySize() * 1024 : 0);
}

//----------------------------------------------------------------------------
void RawSkeleton::setSpacing(const NmVector3 &newSpacing)
{
  QMutexLocker lock(&m_lock);

  auto oldSpacing = m_bounds.spacing();

  if (m_skeleton != nullptr && oldSpacing != newSpacing)
  {
    Q_ASSERT(newSpacing[0] != 0 && newSpacing[1] != 0 && newSpacing[2] != 0);

    auto oldOrigin  = m_bounds.origin();

    NmVector3 ratio{newSpacing[0] / oldSpacing[0], newSpacing[1] / oldSpacing[1], newSpacing[2] / oldSpacing[2] };

    PolyDataUtils::scalePolyData(m_skeleton, ratio);

    m_bounds = polyDataVolumeBounds(m_skeleton, newSpacing, oldOrigin * ratio);
    updateModificationTime();
  }
}

//----------------------------------------------------------------------------
void RawSkeleton::setSkeleton(vtkSmartPointer<vtkPolyData> skeleton)
{
  {
    QMutexLocker lock(&m_lock);

    m_skeleton = skeleton;

    BoundsList editedRegions;
    if (m_skeleton)
    {
      m_bounds = polyDataVolumeBounds(m_skeleton, m_bounds.spacing(), m_bounds.origin());

      editedRegions << bounds();

      setEditedRegions(editedRegions);
    }
  }

  updateModificationTime();
}

//----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> RawSkeleton::skeleton() const
{
  QMutexLocker lock(&m_lock);

  vtkSmartPointer<vtkPolyData> skeleton = nullptr;

  if(m_skeleton)
  {
    skeleton = vtkSmartPointer<vtkPolyData>::New();
    skeleton->DeepCopy(m_skeleton);
  }

  return skeleton;
}

//----------------------------------------------------------------------------
bool RawSkeleton::isValid() const
{
  QMutexLocker lock(&m_lock);

  return m_bounds.areValid() && !needFetch();
}

//----------------------------------------------------------------------------
bool RawSkeleton::isEmpty() const
{
  QMutexLocker lock(&m_lock);

  return !m_skeleton || m_skeleton->GetNumberOfCells() == 0;
}
