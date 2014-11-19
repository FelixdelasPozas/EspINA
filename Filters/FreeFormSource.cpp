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

#include "FreeFormSource.h"

// ESPINA
#include <GUI/ModelFactory.h>
#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.hxx>
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>

// ITK
#include <itkImageRegionIteratorWithIndex.h>
#include <vtkMath.h>

// Qt
#include <QDebug>

using namespace ESPINA;

//-----------------------------------------------------------------------------
FreeFormSource::FreeFormSource(InputSList   inputs,
                               Filter::Type  type,
                               SchedulerSPtr scheduler)
: Filter(inputs, type, scheduler)
, m_mask(nullptr)
{
  //m_outputs[0] = OutputSPtr{new Output(this, 0, NmVector3{1,1,1})};
  qWarning() << "Free Form Source is going to be deprecated. Use Source Filter instead";
}

//-----------------------------------------------------------------------------
FreeFormSource::~FreeFormSource()
{
}

//------------------------------------------------------------------------
void FreeFormSource::restoreState(const State& state)
{
}

//-----------------------------------------------------------------------------
State FreeFormSource::state() const
{
  return State();
}

//-----------------------------------------------------------------------------
Snapshot FreeFormSource::saveFilterSnapshot() const
{
  return Snapshot();
}

//----------------------------------------------------------------------------
bool FreeFormSource::needUpdate(Output::Id id) const
{
  if (id != 0) throw Undefined_Output_Exception();

  return m_outputs.isEmpty() || !validOutput(id) || ignoreStorageContent();
}

//----------------------------------------------------------------------------
void FreeFormSource::execute(Output::Id id)
{
  if (id != 0) throw Undefined_Output_Exception();
  if (m_inputs.size() != 1) throw Invalid_Number_Of_Inputs_Exception();
  if (m_mask == nullptr)    throw Invalid_Input_Data_Exception();

  emit progress(50);
  if (!canExecute()) return;

  DefaultVolumetricDataSPtr volume{new SparseVolume<itkVolumeType>(m_mask->bounds().bounds(), m_mask->spacing(), m_mask->origin())};
  volume->draw(m_mask);

  emit progress(75);
  if (!canExecute()) return;

  auto mesh = MeshDataSPtr{new MarchingCubesMesh<itkVolumeType>(volume)};

  auto spacing = m_mask->spacing();

  if (!m_outputs.contains(0))
  {
    m_outputs[0] = OutputSPtr(new Output(this, 0, spacing));
  }
  m_outputs[0]->setData(volume);
  m_outputs[0]->setData(mesh);
  m_outputs[0]->setSpacing(spacing);

  emit progress(100);
}

//----------------------------------------------------------------------------
bool FreeFormSource::areEditedRegionsInvalidated()
{
  return false;
}
