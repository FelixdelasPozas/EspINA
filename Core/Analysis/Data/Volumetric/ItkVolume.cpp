/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#include "ItkVolume.h"
#include "Core/Analysis/Filter.h"

// ITK
#include <itkExtractImageFilter.h>
//#include <itkImageRegionExclusionIteratorWithIndex.h>
//#include <itkLabelImageToShapeLabelMapFilter.h>
#include <itkStatisticsLabelObject.h>
#include <itkRegionOfInterestImageFilter.h>
#include <itkMetaImageIO.h>
#include <itkImageFileWriter.h>
#include <itkImageFileReader.h>

// VTK
#include <vtkAlgorithmOutput.h>
#include <vtkCellArray.h>
#include <vtkDiscreteMarchingCubes.h>
#include <vtkImageExport.h>
#include <vtkImageStencilToImage.h>
#include <vtkImplicitFunction.h>
#include <vtkMath.h>
#include <vtkPolyData.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkSmartPointer.h>

// Qt
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QDir>

#include <iostream>

using namespace EspINA;

typedef itk::StatisticsLabelObject<unsigned int, 3>                       LabelObjectType;
typedef itk::LabelMap<LabelObjectType>                                    LabelMapType;
typedef itk::LabelImageToShapeLabelMapFilter<itkVolumeType, LabelMapType> Image2LabelFilterType;
typedef itk::ExtractImageFilter<itkVolumeType, itkVolumeType>             ExtractType;
typedef itk::ImageFileReader<itkVolumeType>                               RawSegmentationVolumeReader;
typedef itk::ImageFileWriter<itkVolumeType>                               RawSegmentationVolumeWriter;
typedef itk::RegionOfInterestImageFilter<itkVolumeType, itkVolumeType>    ROIFilter;


//----------------------------------------------------------------------------
template<class T>
itkVolumeType::IndexType volumeIndex(typename T::Pointer volume, Nm x, Nm y, Nm z)
{
  itkVolumeType::PointType   origin  = volume->GetOrigin();
  itkVolumeType::SpacingType spacing = volume->GetSpacing();
  itkVolumeType::IndexType   res;

  res[0] = int((x - origin[0]) / spacing[0] + 0.5);
  res[1] = int((y - origin[1]) / spacing[1] + 0.5);
  res[2] = int((z - origin[2]) / spacing[2] + 0.5);

  return res;
}

//----------------------------------------------------------------------------
template<class T>
void volumeExtent(typename T::Pointer volume, int out[6])
{
  if (volume)
  {
    itkVolumeType::SpacingType spacing = volume->GetSpacing();
    itkVolumeType::RegionType  region  = volume->GetLargestPossibleRegion();
    itkVolumeType::PointType   origin  = volume->GetOrigin();

    for(int i=0; i<3; i++)
    {
      int min = 2*i, max = 2*i+1;
      out[min] = vtkMath::Round(origin[i]/spacing[i]) + region.GetIndex(i);
      out[max] = out[min] + region.GetSize(i) - 1;
    }
  } else
  {
    out[0] = out[2] = out[4] =  0;
    out[1] = out[3] = out[5] = -1;
  }
}

//----------------------------------------------------------------------------
template<class T>
void volumeBounds(typename T::PointType   origin,
                  typename T::RegionType  region,
                  typename T::SpacingType spacing,
                  double                  out[6])
{
  for(int i=0; i<3; i++)
  {
    int min = 2*i, max = 2*i+1;
    out[min] = origin[i] + (region.GetIndex()[i]-0.5)*spacing[i];
    out[max] = out[min]  + region.GetSize ()[i]*spacing[i];
  }
}
//----------------------------------------------------------------------------
template<class T>
void volumeBounds(typename T::Pointer volume, double out[6])
{
  if (volume)
  {
    typename T::SpacingType spacing = volume->GetSpacing();
    typename T::RegionType  region  = volume->GetLargestPossibleRegion();
    typename T::PointType   origin  = volume->GetOrigin();

    volumeBounds(origin, region, spacing, out);
  }
  else
    vtkMath::UninitializeBounds(out);
}

//----------------------------------------------------------------------------
template<class T>
int voxelIndex(Nm point, Nm spacing)
{
  int voxel = 0;

  if (point >= 0)
    //voxel = ceil(point/spacing + 0.5);
    voxel = int(point/spacing + 0.5);
  else
    voxel = floor(point/spacing + 0.5);

  return voxel;
}

/// Espina regions are defined using bounds. Bounds are given as a range of
/// nm composing a volume. This range is inclusive in its lower limit and exclusive
/// in its upper limit, i.e. [lower bound, upper bound). Thus, is important not to
/// take upper bound as part of it when the volume is multiple of the spacing
//----------------------------------------------------------------------------
template<class T>
itkVolumeType::RegionType volumeRegionAux(Bounds                  region,
                                          typename T::SpacingType spacing)
{
  typename T::RegionType res;

  for (int i = 0; i < 3; i++)
  {
    int voxel = voxelIndex(region[2*i], spacing[i]);

    Nm limit       = region[2*i+1] - 0.25*spacing[i];
    Nm voxelBottom = (voxel - 0.5) * spacing[i];

    int size = 0;
    while (voxelBottom < limit)
    {
      voxelBottom += spacing[i];
      ++size;
    }

    res.SetIndex(i, voxel);
    res.SetSize (i, size);
  }

  return res;
}

//----------------------------------------------------------------------------
template<class T>
itkVolumeType::RegionType volumeRegionAux(typename T::Pointer volume,
                                          const Bounds&       region)
{
  typename T::SpacingType spacing = volume->GetSpacing();
  typename T::RegionType volumeRegion;

//   Nm bounds[6];
//   volumeBounds(volume, bounds);
//   VolumeRepresentation::VolumeRegion vr = volumeRegionAux(EspinaRegion(bounds), spacing);

  typename T::RegionType res = volumeRegionAux(region, spacing);
//   itkVolumeType::IndexType min, max;
//   for (int i = 0; i < 3; i++)
//   {
//     min[i] = int(region[2*i  ]/spacing[i]);
//     max[i] = int(region[2*i+1]/spacing[i]);
//
//     res.SetIndex(i,          min[i]);
//     res.SetSize (i, max[i] - min[i] + 1);
//   }

  typename T::PointType origin = volume->GetOrigin();
  if (origin[0] != 0 || origin[1] != 0 || origin[2] != 0)
  {
    qWarning() << "Non zero origin";
    for (int i = 0; i < 3; i++)
      res.SetIndex(i, res.GetIndex(i) - vtkMath::Round(origin[i]/spacing[i]));
  }

  return res;
}

//----------------------------------------------------------------------------
template<class T>
Bounds espinaRegionAux(typename T::Pointer volume, const Nm bounds[6])
{
  Bounds EspinaBounds{bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5] };
  typename T::RegionType volumeRegion = volumeRegionAux(volume, EspinaBounds);

  Nm regionBounds[6];
  volumeBounds(volume->GetOrigin(), volumeRegion, volume->GetSpacing(), regionBounds);

  return Bounds{regionBounds[0], regionBounds[1], regionBounds[2], regionBounds[3], regionBounds[4], regionBounds[5]};
}


//----------------------------------------------------------------------------
template<class T>
ItkVolume<T>::ItkVolume(typename T::Pointer volume,
                        OutputSPtr          output)
: m_volume(volume)
, m_VTKGenerationTime(0)
, m_ITKGenerationTime(0)
{

}

//----------------------------------------------------------------------------
template<class T>
void ItkVolume<T>::setVolume(typename T::Pointer volume, bool disconnect)
{
  m_volume = volume;
  m_volume->ReleaseDataFlagOff();

  if (disconnect)
    m_volume->DisconnectPipeline();

  if (itk2vtk.IsNotNull())
  {
    itk2vtk->SetInput(m_volume);
    itk2vtk->Update();
    m_VTKGenerationTime = m_volume->GetMTime();
  }

  markAsModified(true);
}

//----------------------------------------------------------------------------
template<class T>
bool ItkVolume<T>::setInternalData(DataSPtr rhs)
{
  //  RawChannelVolumeSPtr volume = boost::dynamic_pointer_cast<RawChannelVolume>(rhs);
  //  setVolume(volume->toITK(), true);
    // TODO
  return true;
}

//----------------------------------------------------------------------------
template<class T>
typename T::IndexType ItkVolume<T>::index(Nm x, Nm y, Nm z)
{
  return volumeIndex(m_volume, x, y, z);
}

//----------------------------------------------------------------------------
template<typename T>
Bounds ItkVolume<T>::bounds() const
{
  return equivalentBounds(m_volume, m_volume->GetLargestPossibleRegion());
}

////----------------------------------------------------------------------------
//template<class T>
//EspinaRegion RawVolume<T>::espinaRegion() const
//{
//  double region[6];
//  bounds(region);
//
//  return EspinaRegion(region);
//}

////----------------------------------------------------------------------------
//RawChannelVolume::VolumeRegion RawChannelVolume::volumeRegion() const
//{
//  return volumeRegion(espinaRegion());
//}

////----------------------------------------------------------------------------
//RawChannelVolume::VolumeRegion RawChannelVolume::volumeRegion(const EspinaRegion& region) const
//{
//  return volumeRegionAux(m_volume, region);
//}

////----------------------------------------------------------------------------
//itkVolumeIterator RawChannelVolume::iterator()
//{
//  return iterator(espinaRegion());
//}
//
////----------------------------------------------------------------------------
//itkVolumeIterator RawChannelVolume::iterator(const EspinaRegion& region)
//{
//  return itkVolumeIterator(m_volume, volumeRegion(region));
//}
//
////----------------------------------------------------------------------------
//itkVolumeConstIterator RawChannelVolume::constIterator()
//{
//  return constIterator(espinaRegion());
//}
//
////----------------------------------------------------------------------------
//itkVolumeConstIterator RawChannelVolume::constIterator(const EspinaRegion& region)
//{
//  return itkVolumeConstIterator(m_volume, volumeRegion(region));
//}
//
////----------------------------------------------------------------------------
//itkVolumeType::Pointer RawChannelVolume::toITK()
//{
//  return m_volume;
//}
//
////----------------------------------------------------------------------------
//const itkVolumeType::Pointer RawChannelVolume::toITK() const
//{
//  return m_volume;
//}
//
////----------------------------------------------------------------------------
//vtkAlgorithmOutput* RawChannelVolume::toVTK()
//{
//  return const_cast<vtkAlgorithmOutput *>(static_cast<const RawChannelVolume *>(this)->toVTK());
//}
//
////----------------------------------------------------------------------------
//const vtkAlgorithmOutput* RawChannelVolume::toVTK() const
//{
//  if (itk2vtk.IsNull() || itk2vtk->GetInput() != m_volume)
//  {
//    itk2vtk = itk2vtkFilterType::New();
//    itk2vtk->ReleaseDataFlagOn();
//    itk2vtk->SetInput(m_volume);
//    itk2vtk->Update();
//    m_VTKGenerationTime = m_volume->GetMTime();
//  }
//  else if (m_volume->GetMTime() != m_VTKGenerationTime)
//  {
//    itk2vtk->Update();
//    m_VTKGenerationTime = m_volume->GetMTime();
//  }
//
//  Q_ASSERT(false);//TODO 2013-10-08 return itk2vtk->GetOutput()->GetProducerPort();
//}

////----------------------------------------------------------------------------
//void RawChannelVolume::markAsModified(bool emitSignal)
//{
//  OutputRepresentation::updateModificationTime();
//  toITK()->Modified();
//
//  if (emitSignal)
//    emit representationChanged();
//}

// //----------------------------------------------------------------------------
// RawChannelVolume::RawChannelVolume(const EspinaRegion &region,
//                                    itkVolumeType::SpacingType spacing,
//                                    FilterOutput *output)
// {
//
// }

//----------------------------------------------------------------------------
template<class T>
RawVolumeSPtr<T> EspINA::rawVolume(OutputSPtr output)
{
  RawVolumeSPtr<T> volume = std::dynamic_pointer_cast<RawVolumeSPtr<T>>(output->data(RawVolume<T>::TYPE));
  Q_ASSERT(volume.get());
  return volume;
}

////----------------------------------------------------------------------------
//RawSegmentationVolume::RawSegmentationVolume(FilterOutput *output)
//: SegmentationVolume(output)
//, m_volume(NULL)
//, m_VTKGenerationTime(0)
//, m_ITKGenerationTime(0)
//{
//
//}
//
////----------------------------------------------------------------------------
//RawSegmentationVolume::RawSegmentationVolume(itkVolumeType::Pointer volume,
//                                             FilterOutput *output)
//: SegmentationVolume(output)
//, m_volume(volume)
//, m_VTKGenerationTime(0)
//, m_ITKGenerationTime(0)
//{
//  m_volume->ReleaseDataFlagOff();
//}
//
////----------------------------------------------------------------------------
//RawSegmentationVolume::RawSegmentationVolume(const EspinaRegion        &region,
//                                             itkVolumeType::SpacingType spacing,
//                                             FilterOutput              *output)
//: SegmentationVolume(output)
//, m_volume(itkVolumeType::New())
//, m_VTKGenerationTime(0)
//, m_ITKGenerationTime(0)
//{
//  m_volume->SetRegions(volumeRegionAux(region, spacing));
//  m_volume->SetSpacing(spacing);
//  m_volume->Allocate();
//  m_volume->FillBuffer(0);
//  m_volume->ReleaseDataFlagOff();
//  m_volume->Update();
//}
//
////----------------------------------------------------------------------------
//RawSegmentationVolume::RawSegmentationVolume(const VolumeRegion        &region,
//                                             itkVolumeType::SpacingType spacing,
//                                             FilterOutput *output)
//: SegmentationVolume(output)
//, m_volume(itkVolumeType::New())
//, m_VTKGenerationTime(0)
//, m_ITKGenerationTime(0)
//{
//  m_volume->SetRegions(region);
//  m_volume->SetSpacing(spacing);
//  m_volume->Allocate();
//  m_volume->FillBuffer(0);
//  m_volume->ReleaseDataFlagOff();
//  m_volume->Update();
//}
//
////----------------------------------------------------------------------------
//bool RawSegmentationVolume::setInternalData(SegmentationRepresentationSPtr rhs)
//{
//  RawSegmentationVolumeSPtr volume = boost::dynamic_pointer_cast<RawSegmentationVolume>(rhs);
//  setVolume(volume->toITK(), true);
//
//  markAsModified(true);
//
//  return true;
//}

//----------------------------------------------------------------------------
template<class T>
void ItkVolume<T>::draw(const vtkImplicitFunction *brush,
                        const Bounds &bounds,
                        const typename T::ValueType value)
{
//  EspinaRegion region = espinaRegionAux(m_volume, bounds);
//
//  addEditedRegion(region);
//
//  expandToFitRegion(region);
//
//  itkVolumeType::SpacingType spacing = m_volume->GetSpacing();
//  itkVolumeType::PointType   origin  = m_volume->GetOrigin();
//
//  // NOTE: We use raw bounds to avoid round problems on the limit
//  // of the voxel interval
//  itkVolumeIterator it = iterator(EspinaRegion(bounds));
//  it.GoToBegin();
//  for (; !it.IsAtEnd(); ++it )
//  {
//    double tx = it.GetIndex()[0]*spacing[0] + origin[0];
//    double ty = it.GetIndex()[1]*spacing[1] + origin[1];
//    double tz = it.GetIndex()[2]*spacing[2] + origin[2];
//
//    if (brush->FunctionValue(tx, ty, tz) <= 0)
//      it.Set(value);
//  }
//  markAsModified(emitSignal);
}

//----------------------------------------------------------------------------
template<class T>
void ItkVolume<T>::draw(typename T::IndexType index,
                        typename T::PixelType value)
{
//  itkVolumeType::SpacingType spacing = m_volume->GetSpacing();
//  double voxelBounds[6] = { index[0]*spacing[0],
//  index[0]*spacing[0],
//  index[1]*spacing[1],
//  index[1]*spacing[1],
//  index[2]*spacing[2],
//  index[2]*spacing[2] };
//
//  EspinaRegion voxelRegion = espinaRegionAux(m_volume, voxelBounds);
//
//  addEditedRegion(voxelRegion); // FIXME: Mover al undo command?
//
//  expandToFitRegion(voxelRegion);
//
//  m_volume->SetPixel(index, value);
//
//  markAsModified(emitSignal);
}

//----------------------------------------------------------------------------
//void RawVolume::draw(Nm x, Nm y, Nm z,
//                            itkVolumeType::PixelType value,
//                            bool emitSignal)
//{
//  double voxelBounds[6] = {x, x, y, y, z, z};
//
//  EspinaRegion voxelRegion = espinaRegionAux(m_volume, voxelBounds);
//
//  addEditedRegion(voxelRegion);
//
//  expandToFitRegion(voxelRegion);
//
//  m_volume->SetPixel(index(x, y, z), value);
//  markAsModified(emitSignal);
//}

////----------------------------------------------------------------------------
//void RawSegmentationVolume::draw(vtkPolyData *contour,
//                            Nm slice,
//                            PlaneType plane,
//                            itkVolumeType::PixelType value,
//                            bool emitSignal)
//{
//  double contourBounds[6];
//  contour->ComputeBounds();
//  contour->GetBounds(contourBounds);
//
//  // vtkPolyDataToImageStencil filter only works in XY plane so we must rotate the contour to that plane
//  double spacingNm[3];
//  this->spacing(spacingNm);
//
//  // FIXME: contour bounds need to be corrected to obtain the correct EspinaRegion for the volume
//  // this bounds are wrong on purpose to get the right volume.
//  contourBounds[(2*plane)+1] = contourBounds[(2*plane)] + spacingNm[plane]/2;
//  for (int i = 0; i < 3; i++)
//  {
//    if (i == plane)
//      continue;
//
//    int voxelIndex = vtkMath::Floor((contourBounds[2*i]+spacingNm[i]/2)/spacingNm[i]);
//    contourBounds[2*i] = voxelIndex * spacingNm[i];
//
//    voxelIndex = vtkMath::Floor((contourBounds[(2*i)+1]+spacingNm[i]/2)/spacingNm[i]);
//    contourBounds[(2*i)+1] = ((voxelIndex+1) * spacingNm[i]) - spacingNm[i]/2;
//  }
//
//  EspinaRegion polyDataRegion = espinaRegionAux(m_volume, contourBounds);
//  if(!polyDataRegion.isInside(espinaRegion()))
//    expandToFitRegion(polyDataRegion);
//
//  addEditedRegion(polyDataRegion);
//
//  int count = contour->GetPoints()->GetNumberOfPoints();
//  vtkSmartPointer<vtkPolyData> rotatedContour = vtkSmartPointer<vtkPolyData>::New();
//  vtkPoints *points = vtkPoints::New();
//  vtkCellArray *lines = vtkCellArray::New();
//  vtkIdType index = 0;
//
//  points->SetNumberOfPoints(count);
//  vtkIdType numLines = count + 1;
//
//  if (numLines > 0)
//  {
//    double pos[3];
//    vtkIdType *lineIndices = new vtkIdType[numLines];
//
//    for (int i = 0; i < count; i++)
//    {
//      contour->GetPoint(i, pos);
//      switch (plane)
//      {
//        case AXIAL:
//          break;
//        case CORONAL:
//          pos[1] = pos[2];
//          break;
//        case SAGITTAL:
//          pos[0] = pos[1];
//          pos[1] = pos[2];
//          break;
//        default:
//          Q_ASSERT(false);
//          break;
//      }
//      pos[2] = slice;
//
//      points->InsertPoint(index, pos);
//      lineIndices[index] = index;
//      index++;
//    }
//
//    lineIndices[index] = 0;
//
//    lines->InsertNextCell(numLines, lineIndices);
//    delete[] lineIndices;
//  }
//
//  rotatedContour->SetPoints(points);
//  rotatedContour->SetLines(lines);
//
//  points->Delete();
//  lines->Delete();
//
//  VolumeRepresentation::VolumeRegion contourRegion = volumeRegionAux(polyDataRegion, m_volume->GetSpacing());
//  VolumeRepresentation::VolumeRegion::IndexType contourRegionIndex = contourRegion.GetIndex();
//  VolumeRepresentation::VolumeRegion::SizeType contourRegionSize = contourRegion.GetSize();
//
//  int extent[6];
//  extent[0] = contourRegionIndex[0];
//  extent[1] = contourRegionIndex[0] + contourRegionSize[0] -1;
//  extent[2] = contourRegionIndex[1];
//  extent[3] = contourRegionIndex[1] + contourRegionSize[1] -1;
//  extent[4] = contourRegionIndex[2];
//  extent[5] = contourRegionIndex[2] + contourRegionSize[2] -1;
//
//  // extent and spacing should be changed because vtkPolyDataToImageStencil filter only works in XY plane
//  // and we've rotated the contour to that plane
//  double temporal;
//  switch(plane)
//  {
//    case AXIAL:
//      break;
//    case CORONAL:
//      temporal = spacingNm[1];
//      spacingNm[1] = spacingNm[2];
//      spacingNm[2] = temporal;
//
//      extent[2] = extent[4];
//      extent[3] = extent[5];
//      break;
//    case SAGITTAL:
//      temporal = spacingNm[0];
//      spacingNm[0] = spacingNm[1];
//      spacingNm[1] = spacingNm[2];
//      spacingNm[2] = temporal;
//
//      extent[0] = extent[2];
//      extent[1] = extent[3];
//      extent[2] = extent[4];
//      extent[3] = extent[5];
//      break;
//    default:
//      Q_ASSERT(false);
//      break;
//  }
//  extent[4] = contourRegionIndex[plane];
//  extent[5] = contourRegionIndex[plane] + contourRegionSize[plane] -1;
//
//  vtkSmartPointer<vtkPolyDataToImageStencil> polyDataToStencil = vtkSmartPointer<vtkPolyDataToImageStencil>::New();
//  polyDataToStencil->SetInputData(rotatedContour);
//  polyDataToStencil->SetOutputOrigin(0,0,0);
//  polyDataToStencil->SetOutputSpacing(spacingNm[0], spacingNm[1], spacingNm[2]);
//  polyDataToStencil->SetOutputWholeExtent(extent[0], extent[1], extent[2], extent[3], extent[4], extent[5]);
//  polyDataToStencil->SetTolerance(0);
//
//  vtkSmartPointer<vtkImageStencilToImage> stencilToImage = vtkSmartPointer<vtkImageStencilToImage>::New();
//  stencilToImage->SetInputConnection(polyDataToStencil->GetOutputPort());
//  stencilToImage->SetOutputScalarTypeToUnsignedChar();
//  stencilToImage->SetInsideValue(1);
//  stencilToImage->SetOutsideValue(0);
//  stencilToImage->Update();
//
//  vtkImageData *outputImage = stencilToImage->GetOutput();
//
//  itk::Index<3> imageIndex;
//  imageIndex[0] = imageIndex[1] = imageIndex[2] = 0;
//  unsigned char *pixel;
//  for (int x = extent[0]; x <= extent[1]; ++x)
//  {
//    for (int y = extent[2]; y <= extent[3]; ++y)
//    {
//      for (int z = extent[4]; z <= extent[5]; ++z)
//      {
//        switch(plane)
//        {
//          case AXIAL:
//            imageIndex[0] = x;
//            imageIndex[1] = y;
//            imageIndex[2] = z;
//            break;
//          case CORONAL:
//            imageIndex[0] = x;
//            imageIndex[1] = z;
//            imageIndex[2] = y;
//            break;
//          case SAGITTAL:
//            imageIndex[0] = z;
//            imageIndex[1] = x;
//            imageIndex[2] = y;
//            break;
//          default:
//            Q_ASSERT(false);
//            break;
//        }
//
//        pixel = reinterpret_cast<unsigned char*>(outputImage->GetScalarPointer(x, y, z));
//        // FIXME: rounding errors can make the m_volume->LargestPossibleRegion() be different (off by one voxel)
//        if (!m_volume->GetLargestPossibleRegion().IsInside(imageIndex))
//          continue;
//
//        if (*pixel == 1)
//          m_volume->SetPixel(imageIndex, value);
//      }
//    }
//  }
//  std::cout << std::flush;
//
//  markAsModified(emitSignal);
//}
//
////----------------------------------------------------------------------------
//void RawChannelVolume::extent(int out[6]) const
//{
//  volumeExtent(m_volume, out);
//}
//
////----------------------------------------------------------------------------
//void RawChannelVolume::bounds(double out[6]) const
//{
//  volumeBounds(m_volume, out);
//}
//
////----------------------------------------------------------------------------
//void RawChannelVolume::spacing(double out[3]) const
//{
//  for(int i=0; i<3; i++)
//    out[i] = m_volume->GetSpacing()[i];
//}
//
////----------------------------------------------------------------------------
//itkVolumeType::SpacingType RawChannelVolume::spacing() const
//{
//  return m_volume->GetSpacing();
//}
//
//
////----------------------------------------------------------------------------
//void RawSegmentationVolume::draw(itkVolumeType::Pointer volume,
//                            bool emitSignal)
//{
//  RawSegmentationVolume drawnVolume(volume);
//  EspinaRegion drawnRegion = drawnVolume.espinaRegion();
//
//  if (m_volume.IsNull())
//  {
//    setVolume(volume, true);
//  }
//  else
//  {
//    expandToFitRegion(drawnRegion);
//
//    itkVolumeType::RegionType commonRegion = m_volume->GetLargestPossibleRegion();
//    commonRegion.Crop(volume->GetLargestPossibleRegion());
//
//    itkVolumeIterator it = itkVolumeIterator(volume,   commonRegion);
//    itkVolumeIterator ot = itkVolumeIterator(m_volume, commonRegion);
//
//    it.GoToBegin();
//    ot.GoToBegin();
//    for (; !it.IsAtEnd(); ++it, ++ot )
//    {
//      ot.Set(it.Get());
//    }
//  }
//
//  addEditedRegion(drawnRegion);
//
//  markAsModified(emitSignal);
//}
//
////----------------------------------------------------------------------------
//void RawSegmentationVolume::fill(itkVolumeType::PixelType value, bool emitSignal)
//{
//  fill(espinaRegion(), value, emitSignal);
//}
//
////----------------------------------------------------------------------------
//void RawSegmentationVolume::fill(const EspinaRegion &region,
//                            itkVolumeType::PixelType value,
//                            bool emitSignal)
//{
//  addEditedRegion(region);
//
//  expandToFitRegion(region);
//
//  itkVolumeIterator ot = iterator(region);
//
//  ot.GoToBegin();
//  for (; !ot.IsAtEnd(); ++ot )
//  {
//    ot.Set(value);
//  }
//
//  markAsModified(emitSignal);
//}
//
////----------------------------------------------------------------------------
//bool RawSegmentationVolume::dumpSnapshot(const QString &prefix, Snapshot &snapshot) const
//{
//  bool dumped = true;
//
//  QDir temporalDir = QDir::tempPath();
//
//  QString mhd = temporalDir.absoluteFilePath(prefix + ".mhd");
//  QString raw = temporalDir.absoluteFilePath(prefix + ".raw");
//
//  itk::MetaImageIO::Pointer io = itk::MetaImageIO::New();
//  io->SetFileName(mhd.toUtf8());
//  RawSegmentationVolumeWriter::Pointer writer = RawSegmentationVolumeWriter::New();
//  writer->SetFileName(mhd.toUtf8().data());
//
//  bool releaseFlag = m_volume->GetReleaseDataFlag();
//  m_volume->ReleaseDataFlagOff();
//  writer->SetInput(m_volume);
//  writer->SetImageIO(io);
//  writer->Write();
//  m_volume->SetReleaseDataFlag(releaseFlag);
//
//  QFile mhdFile(mhd);
//  dumped &= mhdFile.open(QIODevice::ReadOnly);
//  QFile rawFile(raw);
//  dumped &= rawFile.open(QIODevice::ReadOnly);
//
//  QByteArray mhdArray(mhdFile.readAll());
//  QByteArray rawArray(rawFile.readAll());
//
//  mhdFile.close();
//  rawFile.close();
//
//  temporalDir.remove(mhd);
//  temporalDir.remove(raw);
//
//  SnapshotEntry mhdEntry(cachePath(prefix + ".mhd"), mhdArray);
//  SnapshotEntry rawEntry(cachePath(prefix + ".raw"), rawArray);
//
//  snapshot << mhdEntry << rawEntry;
//
//  return dumped;
//}
//
////----------------------------------------------------------------------------
//bool RawSegmentationVolume::fetchSnapshot(Filter *filter, const QString &prefix)
//{
//  // Version 3 seg files compatibility
//  QString cachedFileV3 = QString("%1.mhd").arg(prefix);
//
//  itkVolumeType::Pointer cachedVolumeV3 = filter->readVolumeFromCache(cachedFileV3);
//
//  bool fetchedV3 = cachedVolumeV3.IsNotNull();
//
//  if (fetchedV3)
//  {
//    setVolume(cachedVolumeV3, true);
//    return true;
//  }
//
//  QString cachedFile = QString("%1/%2.mhd").arg(TYPE).arg(prefix);
//
//  itkVolumeType::Pointer cachedVolume = filter->readVolumeFromCache(cachedFile);
//
//  bool fetched = cachedVolume.IsNotNull();
//
//  if (fetched)
//    setVolume(cachedVolume, true);
//
//
//  return fetched;
//}
//
////----------------------------------------------------------------------------
//void RawSegmentationVolume::clearEditedRegions()
//{
//  m_editedRegions.clear();
//}
//
////----------------------------------------------------------------------------
//void RawSegmentationVolume::commitEditedRegions(bool withData) const
//{
//  FilterOutput::EditedRegionSList regions;
//
//  foreach(EditedVolumeRegionSPtr editedRegion, m_editedRegions)
//  {
//    if (withData)
//    {
//      Q_ASSERT(m_volume);
//
//      bool releaseFlag = m_volume->GetReleaseDataFlag();
//      m_volume->ReleaseDataFlagOff();
//
//      ROIFilter::Pointer roiFilter = ROIFilter::New();
//      roiFilter->SetRegionOfInterest(volumeRegion(editedRegion->Region));
//      roiFilter->SetInput(m_volume);
//      roiFilter->Update();
//
//      editedRegion->Volume = roiFilter->GetOutput(0);
//      editedRegion->Volume->DisconnectPipeline();
//
//      m_volume->SetReleaseDataFlag(releaseFlag);
//    }
//
//    regions << editedRegion;
//  }
//
//  SegmentationOutputPtr output = dynamic_cast<SegmentationOutputPtr>(m_output);
//  output->push(regions);
//}
//
////----------------------------------------------------------------------------
//void RawSegmentationVolume::restoreEditedRegions(const QDir &cacheDir, const QString &outputId)
//{
//  foreach(EditedVolumeRegionSPtr region, m_editedRegions)
//  {
//    QString file = cacheDir.absoluteFilePath(cachePath(QString("%1_%2.mhd").arg(outputId).arg(region->Id)));
//    if (!cacheDir.exists(file))
//    {//v3 support
//      file = cacheDir.absoluteFilePath(QString("%1_%2.mhd").arg(outputId).arg(m_editedRegions.indexOf(region)));
//    }
//    itkVolumeType::Pointer editedVolume = m_output->filter()->readVolumeFromCache(file);
//    if (editedVolume.IsNotNull())
//    {
//      draw(editedVolume, true);
//    } else
//    {
//      qWarning() << "VolumeType: Couldn't restore edited region";
//    }
//  }
//}
//
////----------------------------------------------------------------------------
//void RawSegmentationVolume::setVolume(itkVolumeType::Pointer volume, bool disconnect)
//{
//  m_volume = volume;
//  m_volume->ReleaseDataFlagOff();
//
//  if (disconnect)
//    m_volume->DisconnectPipeline();
//
//  if (itk2vtk.IsNotNull())
//  {
//    itk2vtk->SetInput(m_volume);
//    itk2vtk->Update();
//
//    m_VTKGenerationTime = m_volume->GetMTime();
//  }
//}
//
////----------------------------------------------------------------------------
//void RawSegmentationVolume::addEditedRegion(const EspinaRegion &region, int cacheId)
//{
//  int  i        = 0;
//  bool included = false;
//  while (i < m_editedRegions.size() && !included)
//  {
//    included = region.isInside(m_editedRegions[i]->Region);
//    ++i;
//  }
//  if (!included)
//    m_editedRegions << EditedVolumeRegionSPtr(new EditedVolumeRegion(cacheId, region));
//}
//
////----------------------------------------------------------------------------
//itkVolumeType::IndexType RawSegmentationVolume::index(Nm x, Nm y, Nm z)
//{
//  return volumeIndex(m_volume, x, y, z);
//}
//
////----------------------------------------------------------------------------
//void RawSegmentationVolume::extent(int out[6]) const
//{
//  volumeExtent(m_volume, out);
//}
//
////----------------------------------------------------------------------------
//void RawSegmentationVolume::bounds(double out[6]) const
//{
//  volumeBounds(m_volume, out);
//}
//
////----------------------------------------------------------------------------
//void RawSegmentationVolume::spacing(double out[3]) const
//{
//  for(int i=0; i<3; i++)
//    out[i] = m_volume->GetSpacing()[i];
//}
//
////----------------------------------------------------------------------------
//itkVolumeType::SpacingType RawSegmentationVolume::spacing() const
//{
//  return m_volume->GetSpacing();
//}
//
////----------------------------------------------------------------------------
//EspinaRegion RawSegmentationVolume::espinaRegion(Nm bounds[6]) const
//{
//  return espinaRegionAux(m_volume, bounds);
//}
//
////----------------------------------------------------------------------------
//EspinaRegion RawSegmentationVolume::espinaRegion() const
//{
//  double region[6];
//  bounds(region);
//
//  return EspinaRegion(region);
//}
//
////----------------------------------------------------------------------------
//RawSegmentationVolume::VolumeRegion RawSegmentationVolume::volumeRegion() const
//{
//  return volumeRegion(espinaRegion());
//}
//
////----------------------------------------------------------------------------
//RawSegmentationVolume::VolumeRegion RawSegmentationVolume::volumeRegion(const EspinaRegion& region) const
//{
//  return volumeRegionAux(m_volume, region);
//}
//
////----------------------------------------------------------------------------
//itkVolumeIterator RawSegmentationVolume::iterator()
//{
//  return iterator(espinaRegion());
//}
//
////----------------------------------------------------------------------------
//itkVolumeIterator RawSegmentationVolume::iterator(const EspinaRegion& region)
//{
//  return itkVolumeIterator(m_volume, volumeRegion(region));
//}
//
////----------------------------------------------------------------------------
//itkVolumeConstIterator RawSegmentationVolume::constIterator()
//{
//  return constIterator(espinaRegion());
//}
//
////----------------------------------------------------------------------------
//itkVolumeConstIterator RawSegmentationVolume::constIterator(const EspinaRegion& region)
//{
//  return itkVolumeConstIterator(m_volume, volumeRegion(region));
//}
//
////----------------------------------------------------------------------------
//itkVolumeType::Pointer RawSegmentationVolume::toITK()
//{
//  return m_volume;
//}
//
////----------------------------------------------------------------------------
//const itkVolumeType::Pointer RawSegmentationVolume::toITK() const
//{
//  return m_volume;
//}
//
////----------------------------------------------------------------------------
//vtkAlgorithmOutput* RawSegmentationVolume::toVTK()
//{
//  return const_cast<vtkAlgorithmOutput *>(
//    static_cast<const RawSegmentationVolume *>(this)->toVTK());
//}
//
////----------------------------------------------------------------------------
//const vtkAlgorithmOutput* RawSegmentationVolume::toVTK() const
//{
//  if (itk2vtk.IsNull() || itk2vtk->GetInput() != m_volume)
//  {
//    itk2vtk = itk2vtkFilterType::New();
//    itk2vtk->ReleaseDataFlagOn();
//    itk2vtk->SetInput(m_volume);
//    itk2vtk->UpdateLargestPossibleRegion();
//    m_VTKGenerationTime = m_volume->GetMTime();
//  }
//  else if (m_volume->GetMTime() != m_VTKGenerationTime)
//  {
//    itk2vtk->UpdateLargestPossibleRegion();
//    m_VTKGenerationTime = m_volume->GetMTime();
//  }
//
//  Q_ASSERT(false);//TODO 2013-10-08 return itk2vtk->GetOutput()->GetProducerPort();
//}
//
////----------------------------------------------------------------------------
//itkVolumeType::Pointer RawSegmentationVolume::cloneVolume() const
//{
//  return cloneVolume(m_volume->GetLargestPossibleRegion());
//}
//
////----------------------------------------------------------------------------
//itkVolumeType::Pointer RawSegmentationVolume::cloneVolume(const EspinaRegion &region) const
//{
//  return cloneVolume(volumeRegion(region));
//}
//
////----------------------------------------------------------------------------
//itkVolumeType::Pointer RawSegmentationVolume::cloneVolume(const RawSegmentationVolume::VolumeRegion &region) const
//{
//  itkVolumeType::RegionType cloneRegion = m_volume->GetLargestPossibleRegion();
//  cloneRegion.Crop(region);
//
//  ExtractType::Pointer extractor = ExtractType::New();
//  extractor->SetNumberOfThreads(1);
//  extractor->SetInput(m_volume);
//  extractor->SetExtractionRegion(cloneRegion);
//  extractor->Update();
//
//  itkVolumeType::Pointer res = extractor->GetOutput();
//  res->DisconnectPipeline();
//  return res;
//}
//
////----------------------------------------------------------------------------
//void RawSegmentationVolume::markAsModified(bool emitSignal)
//{
//  OutputRepresentation::updateModificationTime();
//  toITK()->Modified();
//
//  if (emitSignal)
//    emit representationChanged();
//}
//
////----------------------------------------------------------------------------
//void RawSegmentationVolume::update()
//{
//  m_volume->Update();
//
//  if (itk2vtk.IsNotNull() && m_VTKGenerationTime != m_volume->GetMTime())
//  {
//    itk2vtk->Update();
//    m_VTKGenerationTime = m_volume->GetMTime();
//  }
//}
//
//typedef itk::ImageRegionExclusionIteratorWithIndex<itkVolumeType> ExclusionIterator;
//
//
//// Expand To Fit Region's Auxiliar Function
////-----------------------------------------------------------------------------
//RawSegmentationVolume::VolumeRegion BoundingBox(RawSegmentationVolume::VolumeRegion v1,
//                                                RawSegmentationVolume::VolumeRegion v2)
//{
//  RawSegmentationVolume::VolumeRegion res;
//  RawSegmentationVolume::VolumeRegion::IndexType minIndex, maxIndex;
//
//  for(unsigned int i = 0; i < 3; i++)
//  {
//    minIndex.SetElement(i, std::min(v1.GetIndex(i),v2.GetIndex(i)));
//    maxIndex.SetElement(i, std::max(v1.GetIndex(i)+v1.GetSize(i) - 1,
//                                    v2.GetIndex(i)+v2.GetSize(i) - 1));
//    res.SetIndex(i, minIndex[i]);
//    res.SetSize (i, maxIndex[i] - minIndex[i] + 1);
//  }
//  return res;
//}
//
////----------------------------------------------------------------------------
//void RawSegmentationVolume::expandToFitRegion(EspinaRegion region)
//{
//  if (m_volume->GetLargestPossibleRegion() != m_volume->GetBufferedRegion())
//  {
//    m_volume->SetBufferedRegion(m_volume->GetLargestPossibleRegion());
//    m_volume->Update();
//  }
//
//  EspinaRegion currentRegion = espinaRegion();
//  if (!region.isInside(currentRegion))
//  {
//    EspinaRegion expandedRegion = BoundingBox(currentRegion, region);
//    RawSegmentationVolume expandedVolume(expandedRegion, m_volume->GetSpacing());
//
//    VolumeRegion commonRegion = expandedVolume.volumeRegion(currentRegion);
//    // Do a block copy for the overlapping region.
//    itk::ImageAlgorithm::Copy(this->m_volume.GetPointer(),
//                              expandedVolume.m_volume.GetPointer(),
//                              volumeRegion(), commonRegion);
//
//    ExclusionIterator outIter(expandedVolume.m_volume.GetPointer(), expandedVolume.volumeRegion());
//    outIter.SetExclusionRegion(commonRegion);
//    outIter.GoToBegin();
//    while ( !outIter.IsAtEnd() )
//    {
//      outIter.Set(0);
//      ++outIter;
//    }
//
//    setVolume(expandedVolume.m_volume);
//  }
//}
//
//
////----------------------------------------------------------------------------
//bool RawSegmentationVolume::collision(SegmentationVolumeSPtr segmentation)
//{
//  Q_ASSERT(false);
//  return false;
//}
//
////----------------------------------------------------------------------------
//bool RawSegmentationVolume::fitToContent() throw(itk::ExceptionObject)
//{
//  Image2LabelFilterType::Pointer image2label = Image2LabelFilterType::New();
//  image2label->ReleaseDataFlagOn();
//  image2label->SetInput(m_volume);
//  image2label->Update();
//
//  // Get segmentation's Bounding Box
//  LabelMapType *labelMap = image2label->GetOutput();
//  if (labelMap->GetNumberOfLabelObjects() == 0)
//  {
//    throw itk::ExceptionObject( __FILE__, __LINE__, "segmentation volume is empty" );
//  }
//
//  LabelObjectType     *segmentation = labelMap->GetLabelObject(SEG_VOXEL_VALUE);
//  LabelObjectType::RegionType segBB = segmentation->GetBoundingBox();
//
//  // Extract the volume corresponding to the Bounding Box
//  ExtractType::Pointer extractor = ExtractType::New();
//  extractor->SetInput(m_volume);
//  extractor->SetExtractionRegion(segBB);
//  extractor->Update();
//
//  bool reduced = m_volume->GetLargestPossibleRegion() != extractor->GetOutput()->GetLargestPossibleRegion();
//  if (reduced)
//  {
//    setVolume(extractor->GetOutput(), true);
//    markAsModified();
//  }
//
//  return reduced;
//}
//
////----------------------------------------------------------------------------
//RawSegmentationVolumeSPtr EspINA::rawSegmentationVolume(OutputSPtr output)
//{
//  SegmentationOutputSPtr segmentationOutput = boost::dynamic_pointer_cast<SegmentationOutput>(output);
//  Q_ASSERT(segmentationOutput.get());
//  return boost::dynamic_pointer_cast<RawSegmentationVolume>(segmentationOutput->representation(SegmentationVolume::TYPE));
//}
