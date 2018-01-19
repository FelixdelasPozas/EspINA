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
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.h>
#include <Core/Utils/ITKProgressReporter.h>

// ITK
#include <itkBinaryFillholeImageFilter.h>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;

using BinaryFillholeFilter = itk::BinaryFillholeImageFilter<itkVolumeType>;

//-----------------------------------------------------------------------------
FillHolesFilter::FillHolesFilter(InputSList          inputs,
                                 const Filter::Type &type,
                                 SchedulerSPtr       scheduler)
: Filter(inputs, type, scheduler)
{
}

//-----------------------------------------------------------------------------
bool FillHolesFilter::needUpdate() const
{
  return m_outputs.isEmpty() || !validOutput(0);
}

//-----------------------------------------------------------------------------
void FillHolesFilter::execute()
{
  if (m_inputs.size() != 1)
  {
    auto what    = QObject::tr("Invalid number of inputs, number: %1").arg(m_inputs.size());
    auto details = QObject::tr("FillHolesFilter::execute(id) -> Invalid number of inputs, number: %1").arg(m_inputs.size());

    throw EspinaException(what, details);
  }

  auto input       = m_inputs[0];
  auto inputVolume = readLockVolume(input->output());
  if (!inputVolume->isValid())
  {
    auto what    = QObject::tr("Invalid input volume");
    auto details = QObject::tr("FillHolesFilter::execute(id) -> Invalid input volume");

    throw EspinaException(what, details);
  }

  reportProgress(0);
  if (!canExecute()) return;

  BinaryFillholeFilter::Pointer filter = BinaryFillholeFilter::New();

  ITKProgressReporter<BinaryFillholeFilter> reporter(this, filter);

  filter->SetInput(inputVolume->itkImage());
  filter->Update();

  reportProgress(100);
  if (!canExecute()) return;

  auto output  = filter->GetOutput();
  auto spacing = input->output()->spacing();

  auto volume = sparseCopy<itkVolumeType>(output);

  if (!m_outputs.contains(0))
  {
    m_outputs[0] = std::make_shared<Output>(this, 0, spacing);
  }

  m_outputs[0]->setData(volume);
  m_outputs[0]->setData(std::make_shared<MarchingCubesMesh>(m_outputs[0].get()));
  m_outputs[0]->setSpacing(spacing);
}
