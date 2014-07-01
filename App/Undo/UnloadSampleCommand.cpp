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


#include "UnloadSampleCommand.h"
#include <Core/Model/EspinaModel.h>


using namespace EspINA;


//-----------------------------------------------------------------------------
UnloadSampleCommand::UnloadSampleCommand(SampleSPtr    sample,
                                         EspinaModel  *model,
                                         QUndoCommand *parent)
: QUndoCommand(parent)
, m_model     (model )
, m_sample    (sample)
{
  Q_ASSERT(sample->relatedItems(EspINA::RELATION_INOUT).isEmpty());
}

//-----------------------------------------------------------------------------
void UnloadSampleCommand::redo()
{
  m_model->removeSample(m_sample);
}

//-----------------------------------------------------------------------------
void UnloadSampleCommand::undo()
{
  m_model->addSample(m_sample);
}


