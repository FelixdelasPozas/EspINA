/*
    
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#include <Core/Analysis/Output.h>
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/Analysis/Data/Volumetric/SparseVolumeUtils.h>
#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.h>
#include <Core/Utils/SignalBlocker.h>
#include <Undo/DrawUndoCommand.h>

using namespace ESPINA;

//-----------------------------------------------------------------------------
DrawUndoCommand::DrawUndoCommand(SegmentationAdapterSPtr seg,
                                 BinaryMaskSPtr<unsigned char> mask)
: m_segmentation{seg}
, m_mask        {mask}
, m_image       {nullptr}
, m_hasVolume   {hasVolumetricData(seg->output())}
{
  if(m_hasVolume)
  {
    auto volume = readLockVolume(seg->output());
    m_bounds = volume->bounds();

    if(intersect(m_bounds, mask->bounds()))
    {
      auto bounds = intersection(m_bounds, mask->bounds());
      m_image  = volume->itkImage(bounds);
    }
  }
  else
  {
    m_bounds = mask->bounds();
    m_image  = mask->itkImage();
  }
}

//-----------------------------------------------------------------------------
void DrawUndoCommand::redo()
{
  m_segmentation->setBeingModified(true);

  auto output = m_segmentation->output();
  SignalBlocker<OutputSPtr> blocker(output);

  if (m_hasVolume)
  {
    auto volume = writeLockVolume(output);
    expandAndDraw(volume, m_mask);
  }
  else
  {
    auto strokeSpacing = m_segmentation->output()->spacing();
    auto volume = std::make_shared<SparseVolume<itkVolumeType>>(m_bounds, strokeSpacing);
    volume->draw(m_image);

    output->setData(volume);
  }

  {
    auto mesh = std::make_shared<MarchingCubesMesh>(output.get());
    output->setData(mesh);
  }

  m_segmentation->setBeingModified(false);

  m_segmentation->invalidateRepresentations();
}

//-----------------------------------------------------------------------------
void DrawUndoCommand::undo()
{
  m_segmentation->setBeingModified(true);

  auto output = m_segmentation->output();
  SignalBlocker<OutputSPtr> blocker(output);

  if (m_hasVolume)
  {
    {
      auto volume = writeLockVolume(output);
      volume->resize(m_bounds);

      if(m_image != nullptr)
      {
        volume->draw(m_image);
      }
    }

    {
      auto mesh = std::make_shared<MarchingCubesMesh>(output.get());
      output->setData(mesh);
    }

    auto mesh = std::make_shared<MarchingCubesMesh>(output.get());
    output->setData(mesh);
    mesh->mesh();
  }
  else
  {
    output->removeData(VolumetricData<itkVolumeType>::TYPE);
    output->removeData(MarchingCubesMesh::TYPE);
  }

  m_segmentation->setBeingModified(false);

  m_segmentation->invalidateRepresentations();
}
