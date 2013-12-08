/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
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

// EspINA
#include <GUI/ModelFactory.h>
#include <Core/Analysis/Data/Volumetric/SparseVolume.h>
#include <Core/Analysis/Data/VolumetricDataUtils.h>

// ITK
#include <itkImageRegionIteratorWithIndex.h>
#include <vtkMath.h>

// Qt
#include <QDebug>

using namespace EspINA;

//-----------------------------------------------------------------------------
FreeFormSource::FreeFormSource(OutputSList   inputs,
                               Filter::Type  type,
                               SchedulerSPtr scheduler)
: Filter(inputs, type, scheduler)
, m_mask(nullptr)
{
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
  if (m_inputs.size() != 0) throw Invalid_Number_Of_Inputs_Exception();
  if (m_mask == nullptr) throw Invalid_Input_Data_Exception();

  emit progress(25);

  if (m_outputs.isEmpty())
    m_outputs << OutputSPtr(new Output(this, 0));

  emit progress(50);
  if (!canExecute()) return;

  SparseVolume<itkVolumeType> *volume{new SparseVolume<itkVolumeType>(m_mask->bounds(), m_mask->spacing())};
  volume->draw(m_mask);

  emit progress(75);
  if (!canExecute()) return;

  m_outputs[0]->setData(DefaultVolumetricDataSPtr(volume));
  m_outputs[0]->setSpacing(m_mask->spacing());

  emit progress(100);
}

//----------------------------------------------------------------------------
bool FreeFormSource::invalidateEditedRegions()
{
  return false;
}
