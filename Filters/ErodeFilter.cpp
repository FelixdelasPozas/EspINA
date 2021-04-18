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
#include "ErodeFilter.h"
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.h>
#include <Core/Utils/ITKProgressReporter.h>

// ITK
#include <itkImageRegionConstIterator.h>
#include <itkBinaryBallStructuringElement.h>
#include <itkErodeObjectMorphologyImageFilter.h>

// Qt
#include <QDebug>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;

using StructuringElementType = itk::BinaryBallStructuringElement<itkVolumeType::PixelType, 3>;
using BinaryErodeFilter      = itk::ErodeObjectMorphologyImageFilter<itkVolumeType, itkVolumeType, StructuringElementType>;

//-----------------------------------------------------------------------------
ErodeFilter::ErodeFilter(InputSList          inputs,
                         const Filter::Type &type,
                         SchedulerSPtr       scheduler)
: MorphologicalEditionFilter{inputs, type, scheduler}
{
}

//-----------------------------------------------------------------------------
void ErodeFilter::execute(Output::Id id)
{
  Q_ASSERT(0 == id);
  Q_ASSERT(m_inputs.size() == 1);

  if (m_inputs.size() != 1)
  {
    auto what    = QObject::tr("Invalid number of inputs, number: %1").arg(m_inputs.size());
    auto details = QObject::tr("ErodeFilter::execute(id) -> Invalid number of inputs, number: %1").arg(m_inputs.size());

    throw EspinaException(what, details);
  }

  auto input       = m_inputs[0];
  auto inputVolume = readLockVolume(input->output());
  if (!inputVolume->isValid())
  {
    auto what    = QObject::tr("Invalid input volume");
    auto details = QObject::tr("ErodeFilter::execute(id) ->Invalid input volume.");

    throw EspinaException(what, details);
  }

  reportProgress(0);
  if (!canExecute()) return;

  //qDebug() << "Compute Image Erode";
  StructuringElementType ball;
  ball.SetRadius(m_radius);
  ball.CreateStructuringElement();

  BinaryErodeFilter::Pointer filter = BinaryErodeFilter::New();
  filter->SetInput(inputVolume->itkImage());
  filter->SetKernel(ball);
  filter->SetObjectValue(SEG_VOXEL_VALUE);

  ITKProgressReporter<BinaryErodeFilter> reporter(this, filter, 0, 100);

  filter->Update();

  reportProgress(100);
  if (!canExecute()) return;

  finishExecution(filter->GetOutput());
}
