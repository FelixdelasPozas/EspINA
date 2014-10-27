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
#include "BrushUndoCommand.h"
#include <Core/Analysis/Output.h>
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/Analysis/Data/Volumetric/SparseVolumeUtils.h>
#include <Core/Utils/ChangeSignalDelayer.h>

using namespace ESPINA;

//-----------------------------------------------------------------------------
DrawUndoCommand::DrawUndoCommand(SegmentationAdapterSPtr seg, BinaryMaskSPtr<unsigned char> mask)
: m_segmentation(seg)
, m_mask(mask)
{
  m_hasVolumetricData = seg->output()->hasData(VolumetricData<itkVolumeType>::TYPE);
}

//-----------------------------------------------------------------------------
void DrawUndoCommand::redo()
{
  SparseVolumeSPtr volume = nullptr;

  if(m_hasVolumetricData)
  {
    volume = std::dynamic_pointer_cast<SparseVolume<itkVolumeType>>(volumetricData(m_segmentation->output()));
  }
  else
  {
    volume = SparseVolumeSPtr{new SparseVolume<itkVolumeType>(m_mask->bounds().bounds(), m_mask->spacing(), m_mask->origin())};
    m_segmentation->output()->setData(volume);
  }
  ChangeSignalDelayer inhibitor(volume);

  m_bounds = volume->bounds();
  expandAndDraw(volume, m_mask);
}

//-----------------------------------------------------------------------------
void DrawUndoCommand::undo()
{
  if(m_hasVolumetricData)
  {
    auto volume = volumetricData(m_segmentation->output());
    ChangeSignalDelayer inhibitor(volume);
    volume->undo();
    volume->resize(m_bounds);
  }
  else
  {
    m_segmentation->output()->removeData(VolumetricData<itkVolumeType>::TYPE);
  }
}
