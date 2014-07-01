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

// EspINA
#include "BrushUndoCommand.h"
#include <Core/Analysis/Data/Volumetric/SparseVolume.h>
#include <Core/Analysis/Data/Volumetric/SparseVolumeUtils.h>
#include <Core/Utils/ChangeSignalDelayer.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
DrawUndoCommand::DrawUndoCommand(SegmentationAdapterSPtr seg, BinaryMaskSPtr<unsigned char> mask)
: m_segmentation(seg)
, m_mask(mask)
{
}

//-----------------------------------------------------------------------------
void DrawUndoCommand::redo()
{
  auto volume = std::dynamic_pointer_cast<SparseVolume<itkVolumeType>>(volumetricData(m_segmentation->output()));
  ChangeSignalDelayer inhibitor(volume);

  m_bounds = volume->bounds();
  expandAndDraw(volume, m_mask->bounds().bounds(), m_mask); // CHECK
}

//-----------------------------------------------------------------------------
void DrawUndoCommand::undo()
{
  auto volume = volumetricData(m_segmentation->output());
  ChangeSignalDelayer inhibitor(volume);

  volume->undo();
  volume->resize(m_bounds);
}
