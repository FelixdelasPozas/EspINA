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
#include "MorphologicalEditionFilter.h"
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>
#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.hxx>
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>

// Qt
#include <QDebug>

using namespace ESPINA;

const unsigned int LABEL_VALUE = 255;


//-----------------------------------------------------------------------------
MorphologicalEditionFilter::MorphologicalEditionFilter(InputSList          inputs,
                                                       const Filter::Type &type,
                                                       SchedulerSPtr       scheduler)
: Filter         {inputs, type, scheduler}
, m_radius       {0}
, m_prevRadius   {m_radius}
, m_isOutputEmpty{true}
{
}

//-----------------------------------------------------------------------------
MorphologicalEditionFilter::~MorphologicalEditionFilter()
{
}

//-----------------------------------------------------------------------------
void MorphologicalEditionFilter::restoreState(const State& state)
{
  for (auto token : state.split(';'))
  {
    QStringList tokens = token.split('=');

    if (tokens.size() == 2)
    {
      if ("Radius" == tokens[0])
      {
        m_prevRadius = m_radius = tokens[1].toInt();
      }
    }
  }
}

//-----------------------------------------------------------------------------
State MorphologicalEditionFilter::state() const
{
  State state;

  state = QString("Radius=%1").arg(m_radius);

  return state;
}


//-----------------------------------------------------------------------------
Snapshot MorphologicalEditionFilter::saveFilterSnapshot() const
{
  Snapshot snapshot;

  return snapshot;
}

//-----------------------------------------------------------------------------
bool MorphologicalEditionFilter::needUpdate() const
{
  return needUpdate(0);
}


//-----------------------------------------------------------------------------
bool MorphologicalEditionFilter::needUpdate(Output::Id id) const
{
  if (id != 0) throw Undefined_Output_Exception();

  return m_outputs.isEmpty() || !validOutput(id) || ignoreStorageContent();
}

//-----------------------------------------------------------------------------
bool MorphologicalEditionFilter::ignoreStorageContent() const
{
  return m_prevRadius != m_radius;
}

//-----------------------------------------------------------------------------
bool MorphologicalEditionFilter::areEditedRegionsInvalidated()
{
  return false;
}

//-----------------------------------------------------------------------------
void MorphologicalEditionFilter::finishExecution(itkVolumeType::Pointer output)
{
  m_isOutputEmpty = true;

  auto bounds     = minimalBounds<itkVolumeType>(output, SEG_BG_VALUE);
  m_isOutputEmpty = !bounds.areValid();

  if (!m_isOutputEmpty)
  {
    NmVector3 spacing = m_inputs[0]->output()->spacing();

    if (!m_outputs.contains(0))
    {
      m_outputs[0] = std::make_shared<Output>(this, 0, spacing);
    }

    auto volume = std::make_shared<SparseVolume<itkVolumeType>>(bounds, spacing);
    volume->draw(output, bounds);

    m_outputs[0]->setData(volume);
    m_outputs[0]->setData(std::make_shared<MarchingCubesMesh<itkVolumeType>>(m_outputs[0].get()));
    m_outputs[0]->setSpacing(spacing); // it may change after re-execution

    m_prevRadius = m_radius;
  }
  else
  {
    qWarning() << "MorphologicalEditionFilter: Empty Output;";
  }
}
