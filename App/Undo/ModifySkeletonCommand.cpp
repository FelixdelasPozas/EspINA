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
#include <GUI/Model/ModelAdapter.h>

using namespace ESPINA;
using namespace ESPINA::GUI;

//-----------------------------------------------------------------------------
ModifySkeletonCommand::ModifySkeletonCommand(SegmentationAdapterSPtr      segmentation,
                                             vtkSmartPointer<vtkPolyData> skeletonPolyData,
                                             ConnectionList               connections)
: m_segmentation  {segmentation}
, m_newSkeleton   {skeletonPolyData}
, m_oldSkeleton   {readLockSkeleton(segmentation->output())->skeleton()}
, m_editedRegions {readLockSkeleton(segmentation->output())->editedRegions()}
, m_connections   {connections}
, m_oldConnections{segmentation->model()->connections(segmentation)}
{
}

//-----------------------------------------------------------------------------
ModifySkeletonCommand::~ModifySkeletonCommand()
{
}

//-----------------------------------------------------------------------------
void ModifySkeletonCommand::redo()
{
  m_segmentation->setBeingModified(true);

  auto model = m_segmentation->model();
  if(!m_oldConnections.isEmpty()) model->deleteConnections(m_oldConnections);
  if(!m_connections.isEmpty())    model->addConnections(m_connections);

  // set skeleton modifies edited regions accordingly
  writeLockSkeleton(m_segmentation->output())->setSkeleton(m_newSkeleton);

  m_segmentation->setBeingModified(false);

  m_segmentation->invalidateRepresentations();
}

//-----------------------------------------------------------------------------
void ModifySkeletonCommand::undo()
{
  m_segmentation->setBeingModified(true);

  auto model = m_segmentation->model();
  if(!m_connections.isEmpty())    model->deleteConnections(m_connections);
  if(!m_oldConnections.isEmpty()) model->addConnections(m_oldConnections);

  auto data = writeLockSkeleton(m_segmentation->output());
  data->setSkeleton(m_oldSkeleton);
  // original skeleton edited regions can be empty, must restore the original state,
  // as setSkeleton() modifies edited regions.
  data->setEditedRegions(m_editedRegions);

  m_segmentation->setBeingModified(false);

  m_segmentation->invalidateRepresentations();
}

