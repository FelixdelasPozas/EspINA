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

#include "ErodeFilter.h"

#include <Core/Model/EspinaFactory.h>

#include <itkImageRegionConstIterator.h>
#include <itkBinaryBallStructuringElement.h>
#include <itkErodeObjectMorphologyImageFilter.h>

#include <QDebug>

using namespace EspINA;

typedef itk::BinaryBallStructuringElement<itkVolumeType::PixelType, 3> StructuringElementType;
typedef itk::ErodeObjectMorphologyImageFilter<itkVolumeType, itkVolumeType, StructuringElementType> BinaryErodeFilter;

//-----------------------------------------------------------------------------
ErodeFilter::ErodeFilter(NamedInputs inputs,
                         Arguments   args,
                         FilterType  type)
: MorphologicalEditionFilter(inputs, args, type)
{
}

//-----------------------------------------------------------------------------
ErodeFilter::~ErodeFilter()
{
//   qDebug() << "Destroying" << TYPE;
}

//-----------------------------------------------------------------------------
void ErodeFilter::run(FilterOutputId oId)
{
  Q_ASSERT(0 == oId);
  Q_ASSERT(m_inputs.size() == 1);

  SegmentationVolumeSPtr input = segmentationVolume(m_inputs[0]);
  Q_ASSERT(input);

  //qDebug() << "Compute Image Erode";
  StructuringElementType ball;
  ball.SetRadius(m_params.radius());
  ball.CreateStructuringElement();

  BinaryErodeFilter::Pointer filter = BinaryErodeFilter::New();
  filter->SetInput(input->toITK());
  filter->SetKernel(ball);
  filter->SetObjectValue(SEG_VOXEL_VALUE);
  filter->Update();

  m_isOutputEmpty = true;
  typedef itk::ImageRegionConstIterator<itkVolumeType> ImageIterator;
  ImageIterator it(filter->GetOutput(), filter->GetOutput()->GetLargestPossibleRegion());
  it.GoToBegin();
  for(it.GoToBegin(); !it.IsAtEnd(); ++it)
    if (it.Get() == SEG_VOXEL_VALUE)
    {
      m_isOutputEmpty = false;
      break;
    }

  if (!m_isOutputEmpty)
  {
    SegmentationRepresentationSList repList;
    repList << RawSegmentationVolumeSPtr(new RawSegmentationVolume(filter->GetOutput()));

    createOutput(0, repList);
  } else
    qWarning() << "Erode Filter: Empty Output;";

  emit modified(this);
}
