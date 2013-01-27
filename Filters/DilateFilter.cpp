/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge PeÃ±a Pastor <email>

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


#include "DilateFilter.h"

#include <Core/Model/EspinaFactory.h>

#include <QDebug>

#include <itkConstantPadImageFilter.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
DilateFilter::DilateFilter(NamedInputs inputs,
                           Arguments   args,
                           FilterType  type)
: MorphologicalEditionFilter(inputs, args, type)
{

}


//-----------------------------------------------------------------------------
DilateFilter::~DilateFilter()
{
//   qDebug() << "Destroying" << TYPE;
}

//-----------------------------------------------------------------------------
void DilateFilter::run()
{
  Q_ASSERT(m_inputs.size() == 1);
  m_input = m_inputs.first()->toITK();

  //   qDebug() << "Compute Image Dilate";
  itkVolumeType::SizeType lowerExtendRegion;
  lowerExtendRegion[0] = m_params.radius();
  lowerExtendRegion[1] = m_params.radius();
  lowerExtendRegion[2] = m_params.radius();

  itkVolumeType::SizeType upperExtendRegion;
  upperExtendRegion[0] = m_params.radius();
  upperExtendRegion[1] = m_params.radius();
  upperExtendRegion[2] = m_params.radius();

  itk::SmartPointer<itk::ConstantPadImageFilter<itkVolumeType,itkVolumeType> > padFilter = itk::ConstantPadImageFilter<itkVolumeType,itkVolumeType>::New();
  padFilter->SetConstant(0);
  padFilter->SetInput(m_input);
  padFilter->SetPadLowerBound(lowerExtendRegion);
  padFilter->SetPadUpperBound(upperExtendRegion);

  StructuringElementType ball;
  ball.SetRadius(m_params.radius());
  ball.CreateStructuringElement();

  m_filter = BinaryDilateFilter::New();
  m_filter->SetInput(padFilter->GetOutput());
  m_filter->SetKernel(ball);
  m_filter->SetObjectValue(SEG_VOXEL_VALUE);
  m_filter->SetReleaseDataFlag(false);
  m_filter->Update();

  createOutput(0, m_filter->GetOutput());

  m_outputs[0].volume->markAsModified();
  emit modified(this);
}
