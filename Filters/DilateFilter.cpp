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
#include "DilateFilter.h"
#include "Utils/ItkProgressReporter.h"
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.hxx>

// ITK
#include <itkBinaryBallStructuringElement.h>
#include <itkDilateObjectMorphologyImageFilter.h>
#include <itkConstantPadImageFilter.h>

// Qt
#include <QDebug>

using namespace ESPINA;

using PadFilterType          = itk::ConstantPadImageFilter<itkVolumeType,itkVolumeType>;
using StructuringElementType = itk::BinaryBallStructuringElement<itkVolumeType::PixelType, 3>;
using BinaryDilateFilter     = itk::DilateObjectMorphologyImageFilter<itkVolumeType, itkVolumeType, StructuringElementType>;

//-----------------------------------------------------------------------------
DilateFilter::DilateFilter(InputSList inputs, const Filter::Type &type, SchedulerSPtr scheduler)
: MorphologicalEditionFilter{inputs, type, scheduler}
{
}

//-----------------------------------------------------------------------------
DilateFilter::~DilateFilter()
{
//   qDebug() << "Destroying" << TYPE;
}

//-----------------------------------------------------------------------------
void DilateFilter::execute()
{
  Q_ASSERT(m_inputs.size() == 1);

  if (m_inputs.size() != 1) throw Invalid_Number_Of_Inputs_Exception();

  auto input       = m_inputs[0];


  auto inputVolume = readLockVolume(input->output());
  if (!inputVolume->isValid()) throw Invalid_Input_Data_Exception();

  reportProgress(0);
  if (!canExecute()) return;

  //   qDebug() << "Compute Image Dilate";
  itkVolumeType::SizeType lowerExtendRegion;
  lowerExtendRegion[0] = m_radius;
  lowerExtendRegion[1] = m_radius;
  lowerExtendRegion[2] = m_radius;

  itkVolumeType::SizeType upperExtendRegion;
  upperExtendRegion[0] = m_radius;
  upperExtendRegion[1] = m_radius;
  upperExtendRegion[2] = m_radius;

  PadFilterType::Pointer padFilter = PadFilterType::New();
  padFilter->SetConstant(SEG_BG_VALUE);
  padFilter->SetInput(inputVolume->itkImage());
  padFilter->SetPadLowerBound(lowerExtendRegion);
  padFilter->SetPadUpperBound(upperExtendRegion);
  padFilter->Update();
  ITKProgressReporter<PadFilterType> padReporter(this, padFilter, 0, 25);

  reportProgress(25);
  if (!canExecute()) return;

  StructuringElementType ball;
  ball.SetRadius(m_radius);
  ball.CreateStructuringElement();

  BinaryDilateFilter::Pointer filter = BinaryDilateFilter::New();
  filter->SetInput(padFilter->GetOutput());
  filter->SetKernel(ball);
  filter->SetObjectValue(SEG_VOXEL_VALUE);
  filter->SetNumberOfThreads(1);
  filter->ReleaseDataFlagOff();

  ITKProgressReporter<BinaryDilateFilter> dilateReporter(this, filter, 25, 100);

  filter->Update();

  reportProgress(100);

  if (!canExecute()) return;

  finishExecution(filter->GetOutput());
}
