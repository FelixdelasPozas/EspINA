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
#include "ModifySkeletonCommand.h"
#include <Core/Analysis/Data/SkeletonData.h>
#include <GUI/Model/SegmentationAdapter.h>

using namespace ESPINA;

//-----------------------------------------------------------------------------
ModifySkeletonCommand::ModifySkeletonCommand(SegmentationAdapterPtr segmentation, vtkSmartPointer<vtkPolyData> skeletonPolyData)
: m_segmentation {segmentation}
, m_newSkeleton  {skeletonPolyData}
, m_oldSkeleton  {readLockSkeleton(segmentation->output())->skeleton()}
, m_editedRegions{readLockSkeleton(segmentation->output())->editedRegions()}
{
}

//-----------------------------------------------------------------------------
ModifySkeletonCommand::~ModifySkeletonCommand()
{
}

//-----------------------------------------------------------------------------
void ModifySkeletonCommand::redo()
{
  // set skeleton modifies edited regions accordingly
  writeLockSkeleton(m_segmentation->output())->setSkeleton(m_newSkeleton);
}

//-----------------------------------------------------------------------------
void ModifySkeletonCommand::undo()
{
  auto data = writeLockSkeleton(m_segmentation->output());
  data->setSkeleton(m_oldSkeleton);
  // original skeleton edited regions can be empty, must restore the original state,
  // as setSkeleton() modifies edited regions.
  data->setEditedRegions(m_editedRegions);
}

