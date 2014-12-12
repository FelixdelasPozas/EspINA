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
#include <vtkPoints.h>

// C++
#include <memory>

namespace ESPINA
{
  //----------------------------------------------------------------------------
  RawSkeleton::RawSkeleton(OutputSPtr output)
  : m_skeleton{nullptr}
  {
    if(output != nullptr)
    {
      this->setOutput(output.get());
    }
  }

  //----------------------------------------------------------------------------
  RawSkeleton::RawSkeleton(vtkSmartPointer<vtkPolyData> skeleton,
                           const NmVector3 &spacing,
                           OutputSPtr output)
  : m_skeleton{skeleton}
  {
    if(output != nullptr)
    {
      this->setOutput(output.get());
    }
  }

  //----------------------------------------------------------------------------
  size_t RawSkeleton::memoryUsage() const
  {
    if (m_skeleton)
    {
      return m_skeleton->GetActualMemorySize();
    }

    return 0;
  }

  //----------------------------------------------------------------------------
  void RawSkeleton::setSpacing(const NmVector3 &newSpacing)
  {
    if(m_skeleton != nullptr && spacing() != newSpacing)
    {
      auto oldSpacing = spacing();
      Q_ASSERT(newSpacing[0] != 0 && newSpacing[1] != 0 && newSpacing[2] != 0);
      NmVector3 ratio{newSpacing[0]/oldSpacing[0], newSpacing[1]/oldSpacing[1], newSpacing[2]/oldSpacing[2]};

      PolyDataUtils::scalePolyData(m_skeleton, ratio);
      updateModificationTime();
    }
  }

  //----------------------------------------------------------------------------
  void RawSkeleton::setSkeleton(vtkSmartPointer<vtkPolyData> skeleton)
  {
    m_skeleton = skeleton;

    BoundsList editedRegions;
    if (m_skeleton)
    {
      editedRegions << bounds();
    }

    setEditedRegions(editedRegions);
    updateModificationTime();
  }

  //----------------------------------------------------------------------------
  NmVector3 RawSkeleton::spacing() const
  {
    NmVector3 result;

    if(m_output != nullptr)
    {
      result = m_output->spacing();
    }

    return result;
  }

  //----------------------------------------------------------------------------
  RawSkeletonSPtr rawSkeleton(OutputSPtr output)
  {
    auto skeletonData = std::dynamic_pointer_cast<RawSkeleton>(output->data(SkeletonData::TYPE));
    Q_ASSERT(skeletonData);
    return skeletonData;
  }

} // namespace EspINA
