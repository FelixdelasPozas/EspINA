/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#include "SourceFilter.h"
#include <Core/Analysis/Data/Volumetric/SparseVolume.h>


using namespace EspINA;

//-----------------------------------------------------------------------------
SourceFilter::SourceFilter(InputSList    inputs,
                           Filter::Type  type,
                           SchedulerSPtr scheduler)
: Filter(inputs, type, scheduler)
{
}

//-----------------------------------------------------------------------------
SourceFilter::~SourceFilter()
{
}

//------------------------------------------------------------------------
void SourceFilter::restoreState(const State& state)
{
}

//-----------------------------------------------------------------------------
State SourceFilter::state() const
{
  return State();
}

//-----------------------------------------------------------------------------
void SourceFilter::addOutput(itkVolumeType::Pointer volume)
{
  Output::Id lastOutput = m_outputs.size();

  OutputSPtr output{new Output(this, lastOutput)};

  Bounds    bounds  = equivalentBounds<itkVolumeType>(volume, volume->GetLargestPossibleRegion());
  NmVector3 spacing = ToNmVector3<itkVolumeType>(volume->GetSpacing());

  DefaultVolumetricDataSPtr data{new SparseVolume<itkVolumeType>(bounds, spacing)};
  data->draw(volume);

  output->setData(data);


  m_outputs[lastOutput] = output;
}

//-----------------------------------------------------------------------------
Snapshot SourceFilter::saveFilterSnapshot() const
{
  return Snapshot();
}

//----------------------------------------------------------------------------
bool SourceFilter::needUpdate() const
{
  return false;
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

//   emit progress(25);
//
//   if (!m_outputs.contains(0))
//   {
//     m_outputs[0] = OutputSPtr(new Output(this, 0));
//   }
//
//   emit progress(50);
//   if (!canExecute()) return;
//
//   DefaultVolumetricDataSPtr volume{new SparseVolume<itkVolumeType>(m_mask->bounds().bounds(), m_mask->spacing(), m_mask->origin())};
//   auto sparseVolume = std::dynamic_pointer_cast<SparseVolume<itkVolumeType>>(volume);
//   sparseVolume->draw(m_mask);
//
//   emit progress(75);
//   if (!canExecute()) return;
//
//   m_outputs[0]->setData(volume);
//   m_outputs[0]->setData(MeshDataSPtr{new MarchingCubesMesh<itkVolumeType>(volume)});
//   m_outputs[0]->setSpacing(m_mask->spacing());
//
//   emit progress(100);
}

//----------------------------------------------------------------------------
bool SourceFilter::invalidateEditedRegions()
{
  return false;
}
