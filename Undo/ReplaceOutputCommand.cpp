/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// ESPINA
#include "ReplaceOutputCommand.h"

using namespace ESPINA;

//-----------------------------------------------------------------------------
ReplaceOutputCommand::ReplaceOutputCommand(SegmentationAdapterPtr segmentation,
                                           InputSPtr              input,
                                           QUndoCommand*          parent)
: QUndoCommand  {parent}
, m_segmentation{segmentation}
, m_input       {input}
{
}

//-----------------------------------------------------------------------------
void ReplaceOutputCommand::redo()
{
  swapInputs();
}

//-----------------------------------------------------------------------------
void ReplaceOutputCommand::undo()
{
  swapInputs();
}

//-----------------------------------------------------------------------------
void ReplaceOutputCommand::swapInputs()
{
  auto swap = m_segmentation->asInput();

  m_segmentation->changeOutput(m_input);

  m_input = swap;

  m_segmentation->invalidateRepresentations();
}

