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
#include "SourceFilter.h"
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>

using namespace ESPINA;

//-----------------------------------------------------------------------------
void SourceFilter::addOutput(Output::Id id, OutputSPtr output)
{
  m_outputs[id] = output;
}

//----------------------------------------------------------------------------
void SourceFilter::addOutputData(Output::Id id, DataSPtr data)
{
  if (!m_outputs.contains(id))
  {
    m_outputs[id] = std::make_shared<Output>(this, id, data->spacing());
  }

  m_outputs[id]->setData(data);
}

//----------------------------------------------------------------------------
bool SourceFilter::needUpdate(Output::Id id) const
{
  if (!m_outputs.contains(id)) throw Undefined_Output_Exception();

  return m_outputs.isEmpty() || !validOutput(id) || ignoreStorageContent();
}

//----------------------------------------------------------------------------
void SourceFilter::execute(Output::Id id)
{
  if (!m_outputs.contains(id)) throw Undefined_Output_Exception();

  if (m_inputs.size() != 0) throw Invalid_Number_Of_Inputs_Exception();
}
