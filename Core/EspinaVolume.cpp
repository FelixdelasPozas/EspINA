/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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


#include "EspinaVolume.h"

#include <itkExtractImageFilter.h>
#include <itkImageRegionExclusionIteratorWithIndex.h>
#include <itkLabelImageToShapeLabelMapFilter.h>
#include <itkStatisticsLabelObject.h>
#include <boost/graph/graph_concepts.hpp>
#include <QDebug>

typedef itk::StatisticsLabelObject<unsigned int, 3> LabelObjectType;
typedef itk::LabelMap<LabelObjectType> LabelMapType;
typedef itk::LabelImageToShapeLabelMapFilter<itkVolumeType, LabelMapType> Image2LabelFilterType;
typedef itk::ExtractImageFilter<itkVolumeType, itkVolumeType> ExtractType;

//----------------------------------------------------------------------------
EspinaVolume::EspinaVolume(itkVolumeType::Pointer volume)
: m_volume(volume)
, itk2vtk(NULL)
{
}


//----------------------------------------------------------------------------
EspinaVolume::EspinaVolume(const EspinaRegion& region, itkVolumeType::SpacingType spacing)
: m_volume(itkVolumeType::New())
{
  m_volume->SetRegions(volumeRegion(region, spacing));
  m_volume->SetSpacing(spacing);
  m_volume->Allocate();
  m_volume->FillBuffer(0);
}

//----------------------------------------------------------------------------
EspinaVolume::EspinaVolume(const VolumeRegion& region, itkVolumeType::SpacingType spacing)
: m_volume(itkVolumeType::New())
{
  m_volume->SetRegions(region);
  m_volume->SetSpacing(spacing);
  m_volume->Allocate();
  m_volume->FillBuffer(0);
}

//----------------------------------------------------------------------------
EspinaVolume EspinaVolume::operator=(itkVolumeType::Pointer volume)
{
  m_volume = volume;

  if (itk2vtk.IsNotNull())
  {
    itk2vtk->SetInput(m_volume);
    itk2vtk->Update();
  }

  return *this;
}

//----------------------------------------------------------------------------
void EspinaVolume::setVolume(itkVolumeType::Pointer volume, bool disconnect)
{
  m_volume = volume;
  if (disconnect)
    m_volume->DisconnectPipeline();

  if (itk2vtk.IsNotNull())
  {
    itk2vtk->SetInput(m_volume);
    itk2vtk->Update();
  }
}

//----------------------------------------------------------------------------
itkVolumeType::IndexType EspinaVolume::index(Nm x, Nm y, Nm z)
{
  itkVolumeType::PointType   origin  = m_volume->GetOrigin();
  itkVolumeType::SpacingType spacing = m_volume->GetSpacing();
  itkVolumeType::IndexType   res;

  // add 0.5 before int conversion rounds the index
  res[0] = (x - origin[0]) / spacing[0] + 0.5;
  res[1] = (y - origin[1]) / spacing[1] + 0.5;
  res[2] = (z - origin[2]) / spacing[2] + 0.5;

  return res;
}

//----------------------------------------------------------------------------
void EspinaVolume::extent(int out[6]) const
{
  itkVolumeType::SpacingType spacing = m_volume->GetSpacing();
  itkVolumeType::RegionType  region  = m_volume->GetLargestPossibleRegion();
  itkVolumeType::PointType origin    = m_volume->GetOrigin();

  for(int i=0; i<3; i++)
  {
    int min = 2*i, max = 2*i+1;
    out[min] = int(origin[i]/spacing[i] + 0.5) + region.GetIndex(i);
    out[max] = out[min] + region.GetSize(i) - 1;
  }
}

//----------------------------------------------------------------------------
void EspinaVolume::bounds(double out[6]) const
{
  itkVolumeType::SpacingType spacing = m_volume->GetSpacing();
  itkVolumeType::RegionType  region  = m_volume->GetLargestPossibleRegion();
  itkVolumeType::PointType origin    = m_volume->GetOrigin();

  for(int i=0; i<3; i++)
  {
    int min = 2*i, max = 2*i+1;
    out[min] = origin[i] + region.GetIndex()[i]*spacing[i];
    out[max] = out[min] + (region.GetSize()[i] - 1)*spacing[i];
  }
}

//----------------------------------------------------------------------------
void EspinaVolume::spacing(double out[3]) const
{
  for(int i=0; i<3; i++)
    out[i] = m_volume->GetSpacing()[i];
}

//----------------------------------------------------------------------------
EspinaRegion EspinaVolume::espinaRegion()
{
  double region[6];
  bounds(region);

  return EspinaRegion(region);
}

//----------------------------------------------------------------------------
EspinaVolume::VolumeRegion EspinaVolume::volumeRegion()
{
  return volumeRegion(espinaRegion());
}

//----------------------------------------------------------------------------
EspinaVolume::VolumeRegion EspinaVolume::volumeRegion(const EspinaRegion& region)
{
  itkVolumeType::SpacingType spacing = m_volume->GetSpacing();

  VolumeRegion res = volumeRegion(region, spacing);

  itkVolumeType::PointType origin = m_volume->GetOrigin();

  if (origin[0] != 0 || origin[1] != 0 || origin[2] != 0)
  {
    for (int i = 0; i < 3; i++)
      res.SetIndex(i, res.GetIndex(i)-(origin[i]/spacing[i]+0.5));
  }

  return res;
}

//----------------------------------------------------------------------------
itkVolumeIterator EspinaVolume::iterator()
{
  return iterator(espinaRegion());
}

//----------------------------------------------------------------------------
itkVolumeIterator EspinaVolume::iterator(const EspinaRegion& region)
{
  return itkVolumeIterator(m_volume, volumeRegion(region));
}

//----------------------------------------------------------------------------
itkVolumeConstIterator EspinaVolume::constIterator()
{
  return constIterator(espinaRegion());
}

//----------------------------------------------------------------------------
itkVolumeConstIterator EspinaVolume::constIterator(const EspinaRegion& region)
{
  return itkVolumeConstIterator(m_volume, volumeRegion(region));
}

//----------------------------------------------------------------------------
itkVolumeType::Pointer EspinaVolume::toITK()
{
  return m_volume;
}

//----------------------------------------------------------------------------
const itkVolumeType::Pointer EspinaVolume::toITK() const
{
  return m_volume;
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* EspinaVolume::toVTK()
{
  if (itk2vtk.IsNull())
  {
    //qDebug() << "Converting from ITK to VTK";
    itk2vtk = itk2vtkFilterType::New();
    itk2vtk->ReleaseDataFlagOn();
    itk2vtk->SetInput(m_volume);
    itk2vtk->Update();
  }

  return itk2vtk->GetOutput()->GetProducerPort();
}

//----------------------------------------------------------------------------
const vtkAlgorithmOutput* EspinaVolume::toVTK() const
{
  if (itk2vtk.IsNull())
  {
    //qDebug() << "Converting from ITK to VTK";
    itk2vtk = itk2vtkFilterType::New();
    itk2vtk->ReleaseDataFlagOn();
    itk2vtk->SetInput(m_volume);
    itk2vtk->Update();
  }

  return itk2vtk->GetOutput()->GetProducerPort();
}

//----------------------------------------------------------------------------
void EspinaVolume::update()
{
  m_volume->Update();
  if (itk2vtk.IsNotNull())
  {
    itk2vtk->Update();
  }
}

typedef itk::ImageRegionExclusionIteratorWithIndex<itkVolumeType> ExclusionIterator;


// Expand To Fit Region's Auxiliar Function
//-----------------------------------------------------------------------------
EspinaVolume::VolumeRegion BoundingBox(EspinaVolume::VolumeRegion v1,
                                       EspinaVolume::VolumeRegion v2)
{
  EspinaVolume::VolumeRegion res;
  EspinaVolume::VolumeRegion::IndexType minIndex, maxIndex;

  for(unsigned int i = 0; i < 3; i++)
  {
    minIndex.SetElement(i, std::min(v1.GetIndex(i),v2.GetIndex(i)));
    maxIndex.SetElement(i, std::max(v1.GetIndex(i)+v1.GetSize(i) - 1,
                                    v2.GetIndex(i)+v2.GetSize(i) - 1));
    res.SetIndex(i, minIndex[i]);
    res.SetSize (i, maxIndex[i] - minIndex[i] + 1);
  }
  return res;
}

//----------------------------------------------------------------------------
void EspinaVolume::expandToFitRegion(EspinaRegion region)
{
  if (m_volume->GetLargestPossibleRegion() != m_volume->GetBufferedRegion())
  {
    m_volume->SetBufferedRegion(m_volume->GetLargestPossibleRegion());
    m_volume->Update();
  }

  EspinaRegion currentRegion = espinaRegion();
  if (!region.isInside(currentRegion))
  {
    EspinaRegion expandedRegion = BoundingBox(currentRegion, region);
    EspinaVolume expandedVolume(expandedRegion, m_volume->GetSpacing());

    VolumeRegion commonRegion = expandedVolume.volumeRegion(currentRegion);
    // Do a block copy for the overlapping region.
    itk::ImageAlgorithm::Copy(this->m_volume.GetPointer(),
                              expandedVolume.m_volume.GetPointer(),
                              volumeRegion(), commonRegion);

    ExclusionIterator outIter(expandedVolume.m_volume.GetPointer(), expandedVolume.volumeRegion());
    outIter.SetExclusionRegion(commonRegion);
    outIter.GoToBegin();
    while ( !outIter.IsAtEnd() )
    {
      outIter.Set(0);
      ++outIter;
    }

    setVolume(expandedVolume.m_volume);
  }
}



//----------------------------------------------------------------------------
EspinaVolume::VolumeRegion EspinaVolume::volumeRegion(EspinaRegion region, itkVolumeType::SpacingType spacing)
{
  VolumeRegion res;

  itkVolumeType::IndexType min, max;
  for (int i = 0; i < 3; i++)
  {
    min[i] = region[2*i  ]/spacing[i]+0.5;
    max[i] = region[2*i+1]/spacing[i]+0.5;

    res.SetIndex(i, min[i]);
    res.SetSize (i, max[i] - min[i] + 1);
  }
  return res;
}

//----------------------------------------------------------------------------
ChannelVolume::ChannelVolume(itkVolumeType::Pointer volume)
: EspinaVolume(volume)
{
}

//----------------------------------------------------------------------------
ChannelVolume::ChannelVolume(const EspinaRegion& region, itkVolumeType::SpacingType spacing)
: EspinaVolume(region, spacing)
{
}

//----------------------------------------------------------------------------
SegmentationVolume::SegmentationVolume(itkVolumeType::Pointer volume)
: EspinaVolume(volume)
{
}

//----------------------------------------------------------------------------
SegmentationVolume::SegmentationVolume(const EspinaRegion& region, itkVolumeType::SpacingType spacing)
: EspinaVolume(region, spacing)
{
}

//----------------------------------------------------------------------------
bool SegmentationVolume::collision(SegmentationVolume v)
{
  Q_ASSERT(false);
  return false;
}


//----------------------------------------------------------------------------
void SegmentationVolume::strechToFitContent()
{
  Image2LabelFilterType::Pointer image2label = Image2LabelFilterType::New();
  image2label->ReleaseDataFlagOn();
  image2label->SetInput(m_volume);
  image2label->Update();

  // Get segmentation's Bounding Box
  LabelMapType            *labelMap = image2label->GetOutput();
  LabelObjectType     *segmentation = labelMap->GetLabelObject(SEG_VOXEL_VALUE);
  LabelObjectType::RegionType segBB = segmentation->GetBoundingBox();

  // Extract the volume corresponding to the Bounding Box
  ExtractType::Pointer extractor = ExtractType::New();
  extractor->SetInput(m_volume);
  extractor->SetExtractionRegion(segBB);
  extractor->Update();

  m_volume = extractor->GetOutput();
  m_volume->DisconnectPipeline();
}
