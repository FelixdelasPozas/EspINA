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
MorphologicalEditionFilter::MorphologicalEditionFilter(InputSList    inputs,
                                                       Filter::Type  type,
                                                       SchedulerSPtr scheduler)
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
    if (tokens.size() != 2)
      continue;

    if ("Radius" == tokens[0])
    {
      m_radius = tokens[1].toInt();
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

  // TODO: When input exists, check its timeStamp
  return m_outputs.isEmpty() || !validOutput(id) || ignoreStorageContent();
//   bool update = Filter::needUpdate(oId);
//
//   if (!update)
//   {
//     Q_ASSERT(m_outputs.size() == 1);
//
//     VolumetricDataSPtr<T> outputVolume = segmentationVolume(m_outputs[0]);
//     Q_ASSERT(outputVolume.get());
//     Q_ASSERT(outputVolume->toITK().IsNotNull());
//     if (!m_inputs.isEmpty())
//     {
//       Q_ASSERT(m_inputs.size() == 1);
//       SegmentationVolumeSPtr inputVolume = segmentationVolume(m_inputs[0]);
//       update = outputVolume->timeStamp() < inputVolume->timeStamp();
//     }
//   }
//
//   return update;
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

  auto bounds = minimalBounds<itkVolumeType>(output, SEG_BG_VALUE);
  m_isOutputEmpty = !bounds.areValid();

  if (!m_isOutputEmpty)
  {
    if (!m_outputs.contains(0))
    {
      m_outputs[0] = OutputSPtr(new Output(this, 0));
    }

    NmVector3 spacing = m_inputs[0]->output()->spacing();

    DefaultVolumetricDataSPtr volume{new SparseVolume<itkVolumeType>(bounds, spacing)};
    volume->draw(output, bounds);

    MeshDataSPtr mesh{new MarchingCubesMesh<itkVolumeType>(volume)};

    m_outputs[0]->setData(volume);
    m_outputs[0]->setData(mesh);

    m_outputs[0]->setSpacing(spacing);

    m_prevRadius = m_radius;
  }
  else
  {
    qWarning() << "MorphologicalEditionFilter: Empty Output;";
  }
}
