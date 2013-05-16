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
#include <Core/Model/MarchingCubesMesh.h>

#include <itkBinaryBallStructuringElement.h>
#include <itkDilateObjectMorphologyImageFilter.h>
#include <itkConstantPadImageFilter.h>

#include <QDebug>

using namespace EspINA;

typedef itk::ConstantPadImageFilter<itkVolumeType,itkVolumeType> PadFilterType;
typedef itk::BinaryBallStructuringElement<itkVolumeType::PixelType, 3> StructuringElementType;
typedef itk::DilateObjectMorphologyImageFilter<itkVolumeType, itkVolumeType, StructuringElementType> BinaryDilateFilter;

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
void DilateFilter::run(FilterOutputId oId)
{
  Q_ASSERT(0 == oId);
  Q_ASSERT(m_inputs.size() == 1);

  SegmentationVolumeSPtr input = segmentationVolume(m_inputs[0]);
  Q_ASSERT(input);

  //   qDebug() << "Compute Image Dilate";
  itkVolumeType::SizeType lowerExtendRegion;
  lowerExtendRegion[0] = m_params.radius();
  lowerExtendRegion[1] = m_params.radius();
  lowerExtendRegion[2] = m_params.radius();

  itkVolumeType::SizeType upperExtendRegion;
  upperExtendRegion[0] = m_params.radius();
  upperExtendRegion[1] = m_params.radius();
  upperExtendRegion[2] = m_params.radius();

  PadFilterType::Pointer padFilter = PadFilterType::New();
  padFilter->SetConstant(SEG_BG_VALUE);
  padFilter->SetInput(input->toITK());
  padFilter->SetPadLowerBound(lowerExtendRegion);
  padFilter->SetPadUpperBound(upperExtendRegion);

  StructuringElementType ball;
  ball.SetRadius(m_params.radius());
  ball.CreateStructuringElement();

  BinaryDilateFilter::Pointer filter = BinaryDilateFilter::New();
  filter->SetInput(padFilter->GetOutput());
  filter->SetKernel(ball);
  filter->SetObjectValue(SEG_VOXEL_VALUE);
  filter->ReleaseDataFlagOff();
  filter->Update();

  m_isOutputEmpty = false;

  RawSegmentationVolumeSPtr volumeRepresentation(new RawSegmentationVolume(filter->GetOutput()));

  SegmentationRepresentationSList repList;
  repList << volumeRepresentation;
  repList << MeshTypeSPtr(new MarchingCubesMesh(volumeRepresentation));//TODO: Pass the volume or the proxy?

  addOutputRepresentations(0, repList);

  emit modified(this);
}
