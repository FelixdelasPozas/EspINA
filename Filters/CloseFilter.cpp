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
#include "CloseFilter.h"
#include "Utils/ItkProgressReporter.h"
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>

// ITK
#include <itkImageRegionConstIterator.h>
#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryMorphologicalClosingImageFilter.h>

// Qt
#include <QDebug>

using namespace ESPINA;

using StructuringElementType = itk::BinaryBallStructuringElement<itkVolumeType::PixelType, 3>;
using BinaryClosingFilter    = itk::BinaryMorphologicalClosingImageFilter<itkVolumeType, itkVolumeType, StructuringElementType>;

//-----------------------------------------------------------------------------
CloseFilter::CloseFilter(InputSList          inputs,
                         const Filter::Type &type,
                         SchedulerSPtr       scheduler)
: MorphologicalEditionFilter{inputs, type, scheduler}
{
}

//-----------------------------------------------------------------------------
CloseFilter::~CloseFilter()
{
//   qDebug() << "Destroying" << TYPE;
}

//-----------------------------------------------------------------------------
void CloseFilter::execute(Output::Id id)
{
  Q_ASSERT(0 == id);
  Q_ASSERT(m_inputs.size() == 1);

  if (m_inputs.size() != 1) throw Invalid_Number_Of_Inputs_Exception();

  auto input       = m_inputs[0];
  auto inputVolume = readLockVolume(input->output());
  if (!inputVolume->isValid()) throw Invalid_Input_Data_Exception();

  reportProgress(0);
  if (!canExecute()) return;

  StructuringElementType ball;
  ball.SetRadius(m_radius);
  ball.CreateStructuringElement();

  BinaryClosingFilter::Pointer filter = BinaryClosingFilter::New();
  filter->SetInput(inputVolume->itkImage());
  filter->SetKernel(ball);
  filter->SetForegroundValue(SEG_VOXEL_VALUE);

  ITKProgressReporter<BinaryClosingFilter> reporter(this, filter, 0, 100);

  filter->Update();

  reportProgress(100);
  if (!canExecute()) return;

  finishExecution(filter->GetOutput());
}
