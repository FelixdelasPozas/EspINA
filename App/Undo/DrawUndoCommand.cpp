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
#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.hxx>
#include <Core/Utils/SignalBlocker.h>
#include <Undo/DrawUndoCommand.h>

using namespace ESPINA;

//-----------------------------------------------------------------------------
DrawUndoCommand::DrawUndoCommand(SegmentationAdapterSPtr seg, BinaryMaskSPtr<unsigned char> mask)
: m_segmentation(seg)
, m_mask(mask)
, m_image{nullptr}
, m_hasVolume(hasVolumetricData(seg->output()))
{
  if(m_hasVolume)
  {
    auto volume = readLockVolume(seg->output());
    auto bounds = intersection(volume->bounds(), mask->bounds().bounds());

    m_bounds = volume->bounds();
    m_image  = volume->itkImage(bounds);
  }
  else
  {
    m_bounds = mask->bounds().bounds();
    m_image  = mask->itkImage();
  }
}

//-----------------------------------------------------------------------------
void DrawUndoCommand::redo()
{
  auto output = m_segmentation->output();

  if (m_hasVolume)
  {
    auto volume = writeLockVolume(output);
    SignalBlocker<Output::WriteLockData<DefaultVolumetricData>> blockSignals(volume);
    expandAndDraw(volume, m_mask);
  }
  else
  {
    auto strokeSpacing = m_segmentation->output()->spacing();
    auto volume = std::make_shared<SparseVolume<itkVolumeType>>(m_bounds, strokeSpacing);
    volume->draw(m_image);

    output->setData(volume);
    output->setData(std::make_shared<MarchingCubesMesh<itkVolumeType>>(output.get()));
  }

  m_segmentation->invalidateRepresentations();
}

//-----------------------------------------------------------------------------
void DrawUndoCommand::undo()
{
  if (m_hasVolume)
  {
    auto volume = writeLockVolume(m_segmentation->output());
    SignalBlocker<Output::WriteLockData<DefaultVolumetricData>> blockSignals(volume);
    volume->draw(m_image);
    volume->resize(m_bounds);
  }
  else
  {
    m_segmentation->output()->removeData(VolumetricData<itkVolumeType>::TYPE);
  }

  m_segmentation->invalidateRepresentations();
}
