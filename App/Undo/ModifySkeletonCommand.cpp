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
#include <Extensions/SkeletonInformation/SynapseInformation.h>
#include <Extensions/BasicInformation/BasicSegmentationInformation.h>

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::Extensions;

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
void ModifySkeletonCommand::redo()
{
  auto model = m_segmentation->model();
  if(!m_oldConnections.isEmpty()) model->deleteConnections(m_oldConnections);
  std::for_each(m_oldConnections.constBegin(), m_oldConnections.constEnd(), [&](const Connection& c) { invalidateSynapseExtensions(c); });

  if(!m_connections.isEmpty())    model->addConnections(m_connections);
  std::for_each(m_connections.constBegin(), m_connections.constEnd(), [&](const Connection& c) { invalidateSynapseExtensions(c); });

  // set skeleton modifies edited regions accordingly
  writeLockSkeleton(m_segmentation->output())->setSkeleton(m_newSkeleton);

  m_segmentation->invalidateRepresentations();
}

//-----------------------------------------------------------------------------
void ModifySkeletonCommand::undo()
{
  auto model = m_segmentation->model();
  if(!m_connections.isEmpty())    model->deleteConnections(m_connections);
  std::for_each(m_connections.constBegin(), m_connections.constEnd(), [&](const Connection& c) { invalidateSynapseExtensions(c); });

  if(!m_oldConnections.isEmpty()) model->addConnections(m_oldConnections);
  std::for_each(m_oldConnections.constBegin(), m_oldConnections.constEnd(), [&](const Connection& c) { invalidateSynapseExtensions(c); });

  auto data = writeLockSkeleton(m_segmentation->output());
  data->setSkeleton(m_oldSkeleton);
  // original skeleton edited regions can be empty, must restore the original state,
  // as setSkeleton() modifies edited regions.
  data->setEditedRegions(m_editedRegions);

  m_segmentation->invalidateRepresentations();
}

//-----------------------------------------------------------------------------
void ModifySkeletonCommand::invalidateSynapseExtensions(const Connection& connection)
{
  auto segmentation = connection.item2;
  if(segmentation->category()->classificationName().startsWith("Synapse"))
  {
    auto extensions = segmentation->extensions();

    // We could use output()->updateModificationTime() to invalidate all extensions, but
    // its better to just invalidate connection related extensions and not the rest.
    if(extensions->hasExtension(SynapseConnectionInformation::TYPE))
    {
      extensions->get<SynapseConnectionInformation>()->invalidate();
    }

    if(extensions->hasExtension(BasicSegmentationInformationExtension::TYPE))
    {
      extensions->get<BasicSegmentationInformationExtension>()->invalidate();
    }
  }
}
