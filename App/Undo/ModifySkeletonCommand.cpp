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

#include "ModifySkeletonCommand.h"

namespace ESPINA
{
  //-----------------------------------------------------------------------------
  ModifySkeletonCommand::ModifySkeletonCommand(SkeletonDataSPtr data, vtkSmartPointer<vtkPolyData> skeletonPolyData)
  : m_skeletonData{data}
  , m_newSkeleton {skeletonPolyData}
  , m_oldSkeleton {data->skeleton()}
  , m_editedRegions{data->editedRegions()}
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
    m_skeletonData->setSkeleton(m_newSkeleton);
  }

  //-----------------------------------------------------------------------------
  void ModifySkeletonCommand::undo()
  {
    m_skeletonData->setSkeleton(m_oldSkeleton);
    // original skeleton edited regions can be empty, must restore the original state,
    // as setSkeleton() modifies edited regions.
    m_skeletonData->setEditedRegions(m_editedRegions);
  }

} // namespace EspINA
