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
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/Analysis/Filters/SourceFilter.h>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Core::Utils;

//-----------------------------------------------------------------------------
void SourceFilter::addOutput(Output::Id id, OutputSPtr output)
{
  if (output->filter() != this)
  {
    auto what = QObject::tr("Assigned unexpected filter output.");
    auto details = QObject::tr("SourceFilter::addOutput() -> Assigned unexpected filter output.");

    throw EspinaException(what, details);
  }

  m_outputs[id] = output;
}

//----------------------------------------------------------------------------
void SourceFilter::addOutputData(Output::Id id, DataSPtr data)
{
  if (!m_outputs.contains(id))
  {
    m_outputs[id] = std::make_shared<Output>(this, id, data->bounds().spacing());
  }

  m_outputs[id]->setData(data);
}

//----------------------------------------------------------------------------
void SourceFilter::setInput(InputSPtr input)
{
  m_inputs << input;
}

//----------------------------------------------------------------------------
bool SourceFilter::needUpdate(Output::Id id) const
{
  if (!m_outputs.contains(id))
  {
    auto what = QObject::tr("Unknown output id, id: %1.").arg(id);
    auto details = QObject::tr("SourceFilter::needUpdate() -> Unknown output id, id: %1.").arg(id);

    throw EspinaException(what, details);
  }

  return m_outputs.isEmpty() || !validOutput(id) || ignoreStorageContent();
}

//----------------------------------------------------------------------------
void SourceFilter::execute(Output::Id id)
{
  Q_ASSERT(m_inputs.size() == 1);

  if (!m_outputs.contains(id))
  {
    auto what = QObject::tr("Unknown output id, id: %1.").arg(id);
    auto details = QObject::tr("SourceFilter::execute(id) -> Unknown output id, id: %1.").arg(id);

    throw EspinaException(what, details);
  }

  if (m_inputs.size() != 0)
  {
    auto what = QObject::tr("Invalid number of inputs, number: %1").arg(m_inputs.size());
    auto details = QObject::tr("SourceFilter::execute(id) -> Invalid number of inputs, number: %1").arg(m_inputs.size());

    throw EspinaException(what, details);
  }

  // do nothing
}
