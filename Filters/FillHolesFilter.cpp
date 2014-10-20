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
#include "FillHolesFilter.h"
#include "Utils/ItkProgressReporter.h"
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.hxx>

// ITK
#include <itkBinaryFillholeImageFilter.h>

using namespace ESPINA;

using BinaryFillholeFilter = itk::BinaryFillholeImageFilter<itkVolumeType>;

//-----------------------------------------------------------------------------
FillHolesFilter::FillHolesFilter(InputSList    inputs,
                                 Filter::Type  type,
                                 SchedulerSPtr scheduler)
: Filter(inputs, type, scheduler)
{
}

//-----------------------------------------------------------------------------
FillHolesFilter::~FillHolesFilter()
{
}

//-----------------------------------------------------------------------------
void FillHolesFilter::restoreState(const State& state)
{
}

//-----------------------------------------------------------------------------
State FillHolesFilter::state() const
{
  return State();
}

//-----------------------------------------------------------------------------
Snapshot FillHolesFilter::saveFilterSnapshot() const
{
  return Snapshot();
}

//-----------------------------------------------------------------------------
bool FillHolesFilter::needUpdate() const
{
  return m_outputs.isEmpty();
}

//-----------------------------------------------------------------------------
bool FillHolesFilter::needUpdate(Output::Id id) const
{
  if (id != 0) throw Undefined_Output_Exception();

  // TODO: When input exists, check its timeStamp
  return m_outputs.isEmpty() || !validOutput(id);
}

//-----------------------------------------------------------------------------
void FillHolesFilter::execute(Output::Id id)
{
  Q_ASSERT(0 == id);
  Q_ASSERT(m_inputs.size() == 1);

  if (m_inputs.size() != 1) throw Invalid_Number_Of_Inputs_Exception();

  auto input       = m_inputs[0];
  auto inputVolume = volumetricData(input->output());
  if (!inputVolume) throw Invalid_Input_Data_Exception();

  emit progress(0);
  if (!canExecute()) return;

  BinaryFillholeFilter::Pointer filter = BinaryFillholeFilter::New();

  ITKProgressReporter<BinaryFillholeFilter> reporter(this, filter);

  filter->SetInput(inputVolume->itkImage());
  filter->Update();

  emit progress(100);
  if (!canExecute()) return;

  auto output = filter->GetOutput();

  auto volume = DefaultVolumetricDataSPtr{sparseCopy<itkVolumeType>(output)};
  auto mesh   = MeshDataSPtr{new MarchingCubesMesh<itkVolumeType>(volume)};

  if (!m_outputs.contains(0))
  {
    m_outputs[0] = OutputSPtr(new Output(this, 0));
  }

  m_outputs[0]->setData(volume);
  m_outputs[0]->setData(mesh);

  m_outputs[0]->setSpacing(input->output()->spacing());
}

//-----------------------------------------------------------------------------
bool FillHolesFilter::areEditedRegionsInvalidated()
{
  // TODO
  return false;
}
