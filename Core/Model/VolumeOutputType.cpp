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

#include "VolumeOutputType.h"
#include "Filter.h"

// ITK
#include <itkExtractImageFilter.h>
#include <itkImageRegionExclusionIteratorWithIndex.h>
#include <itkLabelImageToShapeLabelMapFilter.h>
#include <itkStatisticsLabelObject.h>
#include <itkMetaImageIO.h>
#include <itkImageFileWriter.h>
#include <itkImageFileReader.h>

// VTK
#include <vtkAlgorithmOutput.h>
#include <vtkCellArray.h>
#include <vtkDiscreteMarchingCubes.h>
#include <vtkImageConstantPad.h>
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

using namespace EspINA;

const FilterOutput::OutputTypeName VolumeOutputType::TYPE = "VolumeOutputType";

typedef itk::StatisticsLabelObject<unsigned int, 3>                       LabelObjectType;
typedef itk::LabelMap<LabelObjectType>                                    LabelMapType;
typedef itk::LabelImageToShapeLabelMapFilter<itkVolumeType, LabelMapType> Image2LabelFilterType;
typedef itk::ExtractImageFilter<itkVolumeType, itkVolumeType>             ExtractType;
typedef itk::ImageFileReader<itkVolumeType>                               EspinaVolumeReader;
typedef itk::ImageFileWriter<itkVolumeType>                               EspinaVolumeWriter;

//----------------------------------------------------------------------------
VolumeOutputType::VolumeOutputType(FilterOutput *output)
: OutputType(output)
, m_volume(NULL)
, m_VTKGenerationTime(0)
, m_ITKGenerationTime(0)
{

}

//----------------------------------------------------------------------------
VolumeOutputType::VolumeOutputType(itkVolumeType::Pointer volume, 
                                   FilterOutput *output)
: OutputType(output)
, m_volume(volume)
, m_VTKGenerationTime(0)
, m_ITKGenerationTime(0)
{
}

//----------------------------------------------------------------------------
VolumeOutputType::VolumeOutputType(const EspinaRegion& region,
                                   itkVolumeType::SpacingType spacing,
                                   FilterOutput *output)
: OutputType(output)
, m_volume(itkVolumeType::New())
, m_VTKGenerationTime(0)
, m_ITKGenerationTime(0)
{
  m_volume->SetRegions(volumeRegion(region, spacing));
  m_volume->SetSpacing(spacing);
  m_volume->Allocate();
  m_volume->FillBuffer(0);
  m_volume->Update();
}

//----------------------------------------------------------------------------
VolumeOutputType::VolumeOutputType(const VolumeRegion& region,
                                   itkVolumeType::SpacingType spacing,
                                   FilterOutput *output)
: OutputType(output)
, m_volume(itkVolumeType::New())
, m_VTKGenerationTime(0)
, m_ITKGenerationTime(0)
{
  m_volume->SetRegions(region);
  m_volume->SetSpacing(spacing);
  m_volume->Allocate();
  m_volume->FillBuffer(0);
  m_volume->Update();
}

//----------------------------------------------------------------------------
bool VolumeOutputType::setInternalData(FilterOutput::OutputTypeSPtr rhs)
{
  VolumeOutputTypeSPtr volume = boost::dynamic_pointer_cast<VolumeOutputType>(rhs);
  setVolume(volume->toITK(), true);

  return true;
}

//----------------------------------------------------------------------------
long unsigned int VolumeOutputType::timeStamp()
{
  return m_volume->GetMTime();
}

//----------------------------------------------------------------------------
void VolumeOutputType::draw(vtkImplicitFunction *brush,
                            const Nm bounds[6],
                            itkVolumeType::PixelType value,
                            bool emitSignal)
{
  EspinaRegion region(bounds);

  addEditedRegion(region);

  expandToFitRegion(region);

  itkVolumeType::SpacingType spacing = m_volume->GetSpacing();
  itkVolumeType::PointType   origin  = m_volume->GetOrigin();

  itkVolumeIterator it = iterator(region);
  it.GoToBegin();
  for (; !it.IsAtEnd(); ++it )
  {
    double tx = it.GetIndex()[0]*spacing[0] + origin[0];
    double ty = it.GetIndex()[1]*spacing[1] + origin[1];
    double tz = it.GetIndex()[2]*spacing[2] + origin[2];

    if (brush->FunctionValue(tx, ty, tz) <= 0)
      it.Set(value);
  }

  markAsModified(emitSignal);
}

//----------------------------------------------------------------------------
void VolumeOutputType::draw(itkVolumeType::IndexType index,
                            itkVolumeType::PixelType value,
                            bool emitSignal)
{
  itkVolumeType::SpacingType spacing = m_volume->GetSpacing();
  double voxelBounds[6] = { index[0]*spacing[0],
  index[0]*spacing[0],
  index[1]*spacing[1],
  index[1]*spacing[1],
  index[2]*spacing[2],
  index[2]*spacing[2] };
  EspinaRegion voxelRegion(voxelBounds);

  addEditedRegion(voxelRegion); // FIXME: Mover al undo command?

  expandToFitRegion(voxelRegion);

  m_volume->SetPixel(index, value);

  markAsModified(emitSignal);
}

//----------------------------------------------------------------------------
void VolumeOutputType::draw(Nm x, Nm y, Nm z,
                            itkVolumeType::PixelType value,
                            bool emitSignal)
{
  double voxelBounds[6] = {x, x, y, y, z, z};
  EspinaRegion voxelRegion(voxelBounds);

  addEditedRegion(voxelRegion);

  expandToFitRegion(voxelRegion);

  m_volume->SetPixel(index(x, y, z), value);
  markAsModified(emitSignal);
}

//----------------------------------------------------------------------------
void VolumeOutputType::draw(vtkPolyData *contour,
                            Nm slice,
                            PlaneType plane,
                            itkVolumeType::PixelType value,
                            bool emitSignal)
{
  double contourBounds[6];
  contour->ComputeBounds();
  contour->GetBounds(contourBounds);

  EspinaRegion polyDataRegion(contourBounds);

  if(!polyDataRegion.isInside(espinaRegion()))
    expandToFitRegion(polyDataRegion);

  // vtkPolyDataToImageStencil filter only works in XY plane so we must rotate the contour to that plane
  itkVolumeType::SpacingType spacing = m_volume->GetSpacing();

  for (int i = 0; i < 6; ++i) // TODO: Test
    polyDataRegion[i] = vtkMath::Round(polyDataRegion[i] /spacing[i/2]) * spacing[i/2];

  addEditedRegion(polyDataRegion);

  int count = contour->GetPoints()->GetNumberOfPoints();
  vtkSmartPointer<vtkPolyData> rotatedContour = vtkSmartPointer<vtkPolyData>::New();
  vtkPoints *points = vtkPoints::New();
  vtkCellArray *lines = vtkCellArray::New();
  vtkIdType index = 0;

  points->SetNumberOfPoints(count);
  vtkIdType numLines = count + 1;

  // NOTE 1: sliceposition should always be less or equal to slice parameter as it represents the
  // extent*spacing position of the slice, this is so because the user could be drawing using
  // "fit to slices" and round() will go to the upper integer if slice is greater than spacing/2.
  // Floor() gives wrong values if extent*spacing = slice as it will give an smaller number, and
  // I don't want to rely on static_cast<int>(number) to achieve the wanted effect.
  // NOTE 2: image reslicing starts in spacing[reslicing plane]/2 instead of 0, so we correct this
  // to match the drawing with what the user sees on screen.
  double slicePosition = vtkMath::Round((slice + spacing[plane]/2)/spacing[plane]) * spacing[plane];
  if (slicePosition > (slice + (spacing[plane]/2)))
    slicePosition = vtkMath::Floor((slice + spacing[plane]/2)/spacing[plane]) * spacing[plane];

  if (numLines > 0)
  {
    double pos[3];
    vtkIdType *lineIndices = new vtkIdType[numLines];

    for (int i = 0; i < count; i++)
    {
      contour->GetPoint(i, pos);
      switch (plane)
      {
        case AXIAL:
          break;
        case CORONAL:
          pos[1] = pos[2];
          break;
        case SAGITTAL:
          pos[0] = pos[1];
          pos[1] = pos[2];
          break;
        default:
          Q_ASSERT(false);
          break;
      }
      pos[2] = slicePosition;

      points->InsertPoint(index, pos);
      lineIndices[index] = index;
      index++;
    }

    lineIndices[index] = 0;

    lines->InsertNextCell(numLines, lineIndices);
    delete[] lineIndices;
  }

  rotatedContour->SetPoints(points);
  rotatedContour->SetLines(lines);

  points->Delete();
  lines->Delete();

  rotatedContour->Update();

  int extent[6] = {
    vtkMath::Round(contourBounds[0]/spacing[0]),
    vtkMath::Round(contourBounds[1]/spacing[0]),
    vtkMath::Round(contourBounds[2]/spacing[1]),
    vtkMath::Round(contourBounds[3]/spacing[1]),
    vtkMath::Round(contourBounds[4]/spacing[2]),
    vtkMath::Round(contourBounds[5]/spacing[2])
  };

  // extent and spacing should be changed because vtkPolyDataToImageStencil filter only works in XY plane
  // and we've rotated the contour to that plane
  double temporal;
  switch(plane)
  {
    case AXIAL:
      break;
    case CORONAL:
      temporal = spacing[1];
      spacing[1] = spacing[2];
      spacing[2] = temporal;

      extent[2] = extent[4];
      extent[3] = extent[5];
      break;
    case SAGITTAL:
      temporal = spacing[0];
      spacing[0] = spacing[1];
      spacing[1] = spacing[2];
      spacing[2] = temporal;

      extent[0] = extent[2];
      extent[1] = extent[3];
      extent[2] = extent[4];
      extent[3] = extent[5];
      break;
    default:
      Q_ASSERT(false);
      break;
  }
  extent[4] = extent[5] = vtkMath::Round(slicePosition/spacing[2]);

  vtkSmartPointer<vtkPolyDataToImageStencil> polyDataToStencil = vtkSmartPointer<vtkPolyDataToImageStencil>::New();
  polyDataToStencil->SetInputConnection(rotatedContour->GetProducerPort());
  polyDataToStencil->SetOutputOrigin(0,0,0);
  polyDataToStencil->SetOutputSpacing(spacing[0], spacing[1], spacing[2]);
  polyDataToStencil->SetOutputWholeExtent(extent[0], extent[1], extent[2], extent[3], extent[4], extent[5]);
  polyDataToStencil->SetTolerance(0);

  vtkSmartPointer<vtkImageStencilToImage> stencilToImage = vtkSmartPointer<vtkImageStencilToImage>::New();
  stencilToImage->SetInputConnection(polyDataToStencil->GetOutputPort());
  stencilToImage->SetOutputScalarTypeToUnsignedChar();
  stencilToImage->SetInsideValue(1);
  stencilToImage->SetOutsideValue(0);
  stencilToImage->Update();

  vtkImageData *outputImage = stencilToImage->GetOutput();

  // we have to check the image index to know if there is a discrepancy between bounds and index
  spacing = m_volume->GetSpacing();
  itkVolumeType::IndexType temporalIndex = m_volume->GetLargestPossibleRegion().GetIndex();
  bounds(contourBounds);
  bool transformIndex = false;
  if ( vtkMath::Round(contourBounds[0]/spacing[0]) != temporalIndex[0]
    || vtkMath::Round(contourBounds[2]/spacing[1]) != temporalIndex[1]
    || vtkMath::Round(contourBounds[4]/spacing[2]) != temporalIndex[2])
  {
    transformIndex = true;
    temporalIndex[0] = vtkMath::Round(contourBounds[0]/spacing[0]);
    temporalIndex[1] = vtkMath::Round(contourBounds[2]/spacing[1]);
    temporalIndex[2] = vtkMath::Round(contourBounds[4]/spacing[2]);
  }

  itk::Index<3> imageIndex;
  unsigned char *pixel;
  for (int x = extent[0]; x <= extent[1]; ++x)
  {
    for (int y = extent[2]; y <= extent[3]; ++y)
    {
      for (int z = extent[4]; z <= extent[5]; ++z)
      {
        switch(plane)
        {
          case AXIAL:
            imageIndex[0] = x;
            imageIndex[1] = y;
            imageIndex[2] = z;
            break;
          case CORONAL:
            imageIndex[0] = x;
            imageIndex[1] = z;
            imageIndex[2] = y;
            break;
          case SAGITTAL:
            imageIndex[0] = z;
            imageIndex[1] = x;
            imageIndex[2] = y;
            break;
          default:
            Q_ASSERT(false);
            break;
        }

        if (transformIndex)
        {
          imageIndex[0] -= temporalIndex[0];
          imageIndex[1] -= temporalIndex[1];
          imageIndex[2] -= temporalIndex[2];
        }

        pixel = reinterpret_cast<unsigned char*>(outputImage->GetScalarPointer(x, y, z));
        Q_ASSERT(m_volume->GetLargestPossibleRegion().IsInside(imageIndex));
        if (*pixel == 1)
          m_volume->SetPixel(imageIndex, value);
      }
    }
  }

  markAsModified(emitSignal);
}

//----------------------------------------------------------------------------
void VolumeOutputType::draw(itkVolumeType::Pointer volume,
                            bool emitSignal)
{
  VolumeOutputType drawnVolume(volume);
  EspinaRegion drawnRegion = drawnVolume.espinaRegion();

  if (m_volume.IsNull())
  {
    setVolume(volume, true);
  }
  else
  {
    expandToFitRegion(drawnRegion);

    itkVolumeIterator it = drawnVolume.iterator(drawnRegion);
    itkVolumeIterator ot = iterator(drawnRegion);

    it.GoToBegin();
    ot.GoToBegin();
    for (; !it.IsAtEnd(); ++it, ++ot )
    {
      ot.Set(it.Get());
    }
  }

  addEditedRegion(drawnRegion);

  markAsModified(emitSignal);
}

//----------------------------------------------------------------------------
void VolumeOutputType::fill(itkVolumeType::PixelType value, bool emitSignal)
{
  fill(espinaRegion(), value, emitSignal);
}

//----------------------------------------------------------------------------
void VolumeOutputType::fill(const EspinaRegion &region,
                            itkVolumeType::PixelType value,
                            bool emitSignal)
{
  addEditedRegion(region);

  expandToFitRegion(region);

  itkVolumeIterator ot = iterator(region);

  ot.GoToBegin();
  for (; !ot.IsAtEnd(); ++ot )
  {
    ot.Set(value);
  }

  markAsModified(emitSignal);
}

//----------------------------------------------------------------------------
bool VolumeOutputType::dumpSnapshot(const QString &prefix, Snapshot &snapshot)
{
  bool dumped = true;

  QString mhd = prefix + ".mhd";
  QString raw = prefix + ".raw";

  itk::MetaImageIO::Pointer io = itk::MetaImageIO::New();
  io->SetFileName(mhd.toUtf8());
  EspinaVolumeWriter::Pointer writer = EspinaVolumeWriter::New();
  writer->SetFileName(mhd.toUtf8().data());

  bool releaseFlag = m_volume->GetReleaseDataFlag();
  m_volume->ReleaseDataFlagOff();
  writer->SetInput(m_volume);
  writer->SetImageIO(io);
  writer->Write();
  m_volume->SetReleaseDataFlag(releaseFlag);

  QFile mhdFile(mhd);
  dumped &= mhdFile.open(QIODevice::ReadOnly);
  QFile rawFile(raw);
  dumped &= rawFile.open(QIODevice::ReadOnly);

  QByteArray mhdArray(mhdFile.readAll());
  QByteArray rawArray(rawFile.readAll());

  mhdFile.close();
  rawFile.close();

  QFileInfo mhdFileInfo(mhd);
  QFileInfo rawFileInfo(raw);

  QDir temporalDir = mhdFileInfo.dir();
  temporalDir.remove(mhd);
  temporalDir.remove(raw);

  SnapshotEntry mhdEntry(TYPE + "/" + mhdFileInfo.fileName(), mhdArray);
  SnapshotEntry rawEntry(TYPE + "/" + rawFileInfo.fileName(), rawArray);

  snapshot << mhdEntry << rawEntry;

  return dumped;
}

//----------------------------------------------------------------------------
bool VolumeOutputType::fetchSnapshot(const QString &prefix)
{
  // Version 3 seg files compatibility
  QString cachedFileV3 = QString("%1.mhd").arg(prefix);

  itkVolumeType::Pointer cachedVolumeV3 = m_output->filter()->readVolumeFromCache(cachedFileV3);

  bool fetchedV3 = cachedVolumeV3.IsNotNull();

  if (fetchedV3)
  {
    setVolume(cachedVolumeV3, true);
    return true;
  }

  QString cachedFile = QString("%1/%2.mhd").arg(TYPE).arg(prefix);

  itkVolumeType::Pointer cachedVolume = m_output->filter()->readVolumeFromCache(cachedFile);

  bool fetched = cachedVolume.IsNotNull();

  if (fetched)
    setVolume(cachedVolume, true);


  return fetched;
}

//----------------------------------------------------------------------------
void VolumeOutputType::clearEditedRegions()
{

}

//----------------------------------------------------------------------------
void VolumeOutputType::dumpEditedRegions(const QString &prefix) const
{
  //FIXME: Simplify regions
  for (int i = 0; i < m_editedRegions.size(); ++i)
  {
    QString file = QString("%1_%2.mhd").arg(prefix).arg(i);

    // FIXME
//     // Dump modified volume
//     if (!output->isCached())
//     {
//       QString regionName = QString("%1_%2").arg(filterPrefix).arg(r);
//       QString mhd = temporalDir.absoluteFilePath(regionName + ".mhd");
//       QString raw = temporalDir.absoluteFilePath(regionName + ".raw");
//       
//       itkVolumeType::Pointer regionVolume = output.volume->toITK();
//       if (regionVolume)
//       {
//         bool releaseFlag = regionVolume->GetReleaseDataFlag();
//         regionVolume->ReleaseDataFlagOff();
//         
//         io->SetFileName(mhd.toUtf8());
//         writer->SetFileName(mhd.toUtf8().data());
//         
//         ROIFilter::Pointer roiFilter = ROIFilter::New();
//         roiFilter->SetRegionOfInterest(output.volume->volumeRegion(editedRegion));
//         roiFilter->SetInput(regionVolume);
//         roiFilter->Update();
//         
//         writer->SetInput(roiFilter->GetOutput());
//         writer->SetImageIO(io);
//         writer->Write();
//         regionVolume->SetReleaseDataFlag(releaseFlag);
//       } else
//       {
//         // dump cache volumes
//         mhd = m_cacheDir.absoluteFilePath(regionName + ".mhd");
//         raw = m_cacheDir.absoluteFilePath(regionName + ".raw");
//       }
//       
//       QFile mhdFile(mhd);
//       mhdFile.open(QIODevice::ReadOnly);
//       QFile rawFile(raw);
//       rawFile.open(QIODevice::ReadOnly);
//       
//       QByteArray mhdArray(mhdFile.readAll());
//       QByteArray rawArray(rawFile.readAll());
//       
//       mhdFile.close();
//       rawFile.close();
//       
//       if (regionVolume)
//       {
//         temporalDir.remove(mhd);
//         temporalDir.remove(raw);
//       }
//       
//       SnapshotEntry mhdEntry(regionName + ".mhd", mhdArray);
//       SnapshotEntry rawEntry(regionName + ".raw", rawArray);
//       
//       snapshot << mhdEntry << rawEntry;
//     }
  }
  
}

//----------------------------------------------------------------------------
void VolumeOutputType::restoreEditedRegion(const EspinaRegion &region, const QString &prefix)
{
  // Restore previous edited regions if restoring a modified output
  int regionId = m_editedRegions.size() - 1;

  // Version 3 seg files compatibility
  QString fileV3 = QString("%1_%2.mhd").arg(prefix).arg(regionId);
  itkVolumeType::Pointer editedVolumeV3 = m_output->filter()->readVolumeFromCache(fileV3);
  if (editedVolumeV3.IsNotNull())
  {
    draw(editedVolumeV3, true);
  } else
  {
    qWarning() << "VolumeOutputType: Couldn't restore edited region";
  }

  QString file = QString("%1_%2.mhd").arg(prefix).arg(regionId);
  itkVolumeType::Pointer editedVolume = m_output->filter()->readVolumeFromCache(file);
  if (editedVolume.IsNotNull())
  {
    draw(editedVolume, true);
  } else
  {
    qWarning() << "VolumeOutputType: Couldn't restore edited region";
  }
}

//----------------------------------------------------------------------------
void VolumeOutputType::setVolume(itkVolumeType::Pointer volume, bool disconnect)
{
  m_volume = volume;

  if (disconnect)
    m_volume->DisconnectPipeline();

  if (itk2vtk.IsNotNull())
  {
    itk2vtk->SetInput(m_volume);
    itk2vtk->Update();
    m_VTKGenerationTime = m_volume->GetMTime();
  }
}

//----------------------------------------------------------------------------
void VolumeOutputType::addEditedRegion(const EspinaRegion &region)
{
  int  i        = 0;
  bool included = false;
  while (i < m_editedRegions.size() && !included)
  {
    included = region.isInside(m_editedRegions[i]);
    ++i;
  }
  if (!included)
    m_editedRegions << region;
}

//----------------------------------------------------------------------------
FilterOutput::NamedRegionList VolumeOutputType::editedRegions() const
{
  FilterOutput::NamedRegionList regions; // TODO: Move to Output base class??

  foreach (EspinaRegion region, m_editedRegions)
  {
    regions << FilterOutput::NamedRegion(TYPE, region);
  }

  return regions;
}


//----------------------------------------------------------------------------
itkVolumeType::IndexType VolumeOutputType::index(Nm x, Nm y, Nm z)
{
  itkVolumeType::PointType   origin  = m_volume->GetOrigin();
  itkVolumeType::SpacingType spacing = m_volume->GetSpacing();
  itkVolumeType::IndexType   res;

  res[0] = vtkMath::Round((x - origin[0]) / spacing[0]);
  res[1] = vtkMath::Round((y - origin[1]) / spacing[1]);
  res[2] = vtkMath::Round((z - origin[2]) / spacing[2]);

  return res;
}

//----------------------------------------------------------------------------
void VolumeOutputType::extent(int out[6]) const
{
  itkVolumeType::SpacingType spacing = m_volume->GetSpacing();
  itkVolumeType::RegionType  region  = m_volume->GetLargestPossibleRegion();
  itkVolumeType::PointType origin    = m_volume->GetOrigin();

  for(int i=0; i<3; i++)
  {
    int min = 2*i, max = 2*i+1;
    out[min] = vtkMath::Round(origin[i]/spacing[i]) + region.GetIndex(i);
    out[max] = out[min] + region.GetSize(i) - 1;
  }
}

//----------------------------------------------------------------------------
void VolumeOutputType::bounds(double out[6]) const
{
  if (m_volume)
  {
    itkVolumeType::SpacingType spacing = m_volume->GetSpacing();
    itkVolumeType::RegionType  region  = m_volume->GetLargestPossibleRegion();
    itkVolumeType::PointType   origin  = m_volume->GetOrigin();

    for(int i=0; i<3; i++)
    {
      int min = 2*i, max = 2*i+1;
      out[min] = origin[i] + region.GetIndex()[i]*spacing[i];
      out[max] = out[min] + (region.GetSize()[i] - 1)*spacing[i];
    }
  }
  else
    vtkMath::UninitializeBounds(out);
}

//----------------------------------------------------------------------------
void VolumeOutputType::spacing(double out[3]) const
{
  for(int i=0; i<3; i++)
    out[i] = m_volume->GetSpacing()[i];
}

//----------------------------------------------------------------------------
EspinaRegion VolumeOutputType::espinaRegion() const
{
  double region[6];
  bounds(region);

  return EspinaRegion(region);
}

//----------------------------------------------------------------------------
VolumeOutputType::VolumeRegion VolumeOutputType::volumeRegion() const
{
  return volumeRegion(espinaRegion());
}

//----------------------------------------------------------------------------
VolumeOutputType::VolumeRegion VolumeOutputType::volumeRegion(const EspinaRegion& region) const
{
  itkVolumeType::SpacingType spacing = m_volume->GetSpacing();

  VolumeRegion res = volumeRegion(region, spacing);

  itkVolumeType::PointType origin = m_volume->GetOrigin();

  if (origin[0] != 0 || origin[1] != 0 || origin[2] != 0)
  {
    for (int i = 0; i < 3; i++)
      res.SetIndex(i, res.GetIndex(i) - vtkMath::Round(origin[i]/spacing[i]));
  }

  return res;
}

//----------------------------------------------------------------------------
itkVolumeIterator VolumeOutputType::iterator()
{
  return iterator(espinaRegion());
}

//----------------------------------------------------------------------------
itkVolumeIterator VolumeOutputType::iterator(const EspinaRegion& region)
{
  return itkVolumeIterator(m_volume, volumeRegion(region));
}

//----------------------------------------------------------------------------
itkVolumeConstIterator VolumeOutputType::constIterator()
{
  return constIterator(espinaRegion());
}

//----------------------------------------------------------------------------
itkVolumeConstIterator VolumeOutputType::constIterator(const EspinaRegion& region)
{
  return itkVolumeConstIterator(m_volume, volumeRegion(region));
}

//----------------------------------------------------------------------------
itkVolumeType::Pointer VolumeOutputType::toITK()
{
  return m_volume;
}

//----------------------------------------------------------------------------
const itkVolumeType::Pointer VolumeOutputType::toITK() const
{
  return m_volume;
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* VolumeOutputType::toVTK()
{
  return const_cast<vtkAlgorithmOutput *>(
    static_cast<const VolumeOutputType *>(this)->toVTK());
}

//----------------------------------------------------------------------------
const vtkAlgorithmOutput* VolumeOutputType::toVTK() const
{
  if (itk2vtk.IsNull() || itk2vtk->GetInput() != m_volume)
  {
    itk2vtk = itk2vtkFilterType::New();
    itk2vtk->ReleaseDataFlagOn();
    itk2vtk->SetInput(m_volume);
    itk2vtk->Update();
    m_VTKGenerationTime = m_volume->GetMTime();
  }
  else if (m_volume->GetMTime() != m_VTKGenerationTime)
  {
    itk2vtk->Update();
    m_VTKGenerationTime = m_volume->GetMTime();
  }

  return itk2vtk->GetOutput()->GetProducerPort();
}

//----------------------------------------------------------------------------
itkVolumeType::Pointer VolumeOutputType::cloneVolume() const
{
  return cloneVolume(volumeRegion());
}

//----------------------------------------------------------------------------
itkVolumeType::Pointer VolumeOutputType::cloneVolume(const EspinaRegion &region) const
{
  return cloneVolume(volumeRegion(region));
}

//----------------------------------------------------------------------------
itkVolumeType::Pointer VolumeOutputType::cloneVolume(const VolumeOutputType::VolumeRegion &region) const
{
  ExtractType::Pointer extractor = ExtractType::New();
  extractor->SetNumberOfThreads(1);
  extractor->SetInput(m_volume);
  extractor->SetExtractionRegion(region);
  extractor->Update();

  itkVolumeType::Pointer res = extractor->GetOutput();
  res->DisconnectPipeline();
  return res;
}

//----------------------------------------------------------------------------
void VolumeOutputType::markAsModified(bool emitSignal)
{
  toITK()->Modified();

  if (emitSignal)
    emit modified();
}

//----------------------------------------------------------------------------
void VolumeOutputType::update()
{
  m_volume->Update();

  if (itk2vtk.IsNotNull() && m_VTKGenerationTime != m_volume->GetMTime())
  {
    itk2vtk->Update();
    m_VTKGenerationTime = m_volume->GetMTime();
  }
}

typedef itk::ImageRegionExclusionIteratorWithIndex<itkVolumeType> ExclusionIterator;


// Expand To Fit Region's Auxiliar Function
//-----------------------------------------------------------------------------
VolumeOutputType::VolumeRegion BoundingBox(VolumeOutputType::VolumeRegion v1,
                                       VolumeOutputType::VolumeRegion v2)
{
  VolumeOutputType::VolumeRegion res;
  VolumeOutputType::VolumeRegion::IndexType minIndex, maxIndex;

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
void VolumeOutputType::expandToFitRegion(EspinaRegion region)
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
    VolumeOutputType expandedVolume(expandedRegion, m_volume->GetSpacing());

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
/*    
    m_volume->Print(std::cout);
    expandedVolume.m_volume->Print(std::cout);

    expandedVolume.m_volume->SetOrigin(m_volume->GetOrigin()); */

    setVolume(expandedVolume.m_volume);
  }
}



//----------------------------------------------------------------------------
VolumeOutputType::VolumeRegion VolumeOutputType::volumeRegion(EspinaRegion region, itkVolumeType::SpacingType spacing) const
{
  VolumeRegion res;

  itkVolumeType::IndexType min, max;
  for (int i = 0; i < 3; i++)
  {
    min[i] = vtkMath::Round(region[2*i  ]/spacing[i]);
    max[i] = vtkMath::Round(region[2*i+1]/spacing[i]);

    res.SetIndex(i, min[i]);
    res.SetSize (i, max[i] - min[i] + 1);
  }
  return res;
}

//----------------------------------------------------------------------------
VolumeOutputTypeSPtr EspINA::volumeOutput(OutputSPtr output)
{
  return boost::dynamic_pointer_cast<VolumeOutputType>(output->data(VolumeOutputType::TYPE));
}

//----------------------------------------------------------------------------
ChannelVolumeType::ChannelVolumeType(itkVolumeType::Pointer volume, FilterOutput *output)
: VolumeOutputType(volume, output)
{
}

//----------------------------------------------------------------------------
ChannelVolumeType::ChannelVolumeType(const EspinaRegion& region,
                                     itkVolumeType::SpacingType spacing,
                                     FilterOutput *output)
: VolumeOutputType(region, spacing, output)
{
}

//----------------------------------------------------------------------------
ChannelVolumeTypeSPtr EspINA::channelVolumeOutput(OutputSPtr output)
{
  return boost::dynamic_pointer_cast<ChannelVolumeType>(output->data(VolumeOutputType::TYPE));
}

//----------------------------------------------------------------------------
SegmentationVolumeType::SegmentationVolumeType(FilterOutput *output)
: VolumeOutputType(output)
{

}

//----------------------------------------------------------------------------
SegmentationVolumeType::SegmentationVolumeType(itkVolumeType::Pointer volume,
                                               FilterOutput *output)
: VolumeOutputType(volume, output)
{
}

//----------------------------------------------------------------------------
SegmentationVolumeType::SegmentationVolumeType(const EspinaRegion& region,
                                               itkVolumeType::SpacingType spacing,
                                               FilterOutput *output)
: VolumeOutputType(region, spacing, output)
{
}

//----------------------------------------------------------------------------
bool SegmentationVolumeType::collision(SegmentationVolumeType v)
{
  Q_ASSERT(false);
  return false;
}

//----------------------------------------------------------------------------
bool SegmentationVolumeType::fitToContent() throw(itk::ExceptionObject)
{
  Image2LabelFilterType::Pointer image2label = Image2LabelFilterType::New();
  image2label->ReleaseDataFlagOn();
  image2label->SetInput(m_volume);
  image2label->Update();

  // Get segmentation's Bounding Box
  LabelMapType *labelMap = image2label->GetOutput();
  if (labelMap->GetNumberOfLabelObjects() == 0)
  {
    throw itk::ExceptionObject( __FILE__, __LINE__, "segmentation volume is empty" );
  }

  LabelObjectType     *segmentation = labelMap->GetLabelObject(SEG_VOXEL_VALUE);
  LabelObjectType::RegionType segBB = segmentation->GetBoundingBox();

  // Extract the volume corresponding to the Bounding Box
  ExtractType::Pointer extractor = ExtractType::New();
  extractor->SetInput(m_volume);
  extractor->SetExtractionRegion(segBB);
  extractor->Update();

  bool reduced = m_volume->GetLargestPossibleRegion() != extractor->GetOutput()->GetLargestPossibleRegion();
  if (reduced)
  {
    setVolume(extractor->GetOutput(), true);
    markAsModified();
  }

  return reduced;
}

//----------------------------------------------------------------------------
SegmentationVolumeTypeSPtr EspINA::segmentationVolumeOutput(OutputSPtr output)
{
  return boost::dynamic_pointer_cast<SegmentationVolumeType>(output->data(VolumeOutputType::TYPE));
}


// //------------------------------------------------------------------------
// vtkAlgorithmOutput* SegmentationVolume::toMesh()
// {
//   vtkAlgorithmOutput *vtkVolume = toVTK();
//   int extent[6];
//   vtkImageData *image = vtkImageData::SafeDownCast(vtkVolume->GetProducer()->GetOutputDataObject(0));
//   image->GetExtent(extent);
//   extent[0]--;
//   extent[1]++;
//   extent[2]--;
//   extent[3]++;
//   extent[4]--;
//   extent[5]++;
// 
//   if (NULL == m_padfilter)
//   {
//     // segmentation image need to be padded to avoid segmentation voxels from touching the edges of the
//     // image (and create morphologically correct actors)
// 
//     m_padfilter = vtkSmartPointer<vtkImageConstantPad>::New();
//     m_padfilter->SetInputConnection(vtkVolume);
//     m_padfilter->SetOutputWholeExtent(extent[0], extent[1], extent[2], extent[3], extent[4], extent[5]);
//     m_padfilter->SetConstant(0);
// 
//     m_march = vtkSmartPointer<vtkDiscreteMarchingCubes>::New();
//     m_march->ReleaseDataFlagOn();
//     m_march->SetNumberOfContours(1);
//     m_march->GenerateValues(1, 255, 255);
//     m_march->ComputeScalarsOff();
//     m_march->ComputeNormalsOff();
//     m_march->ComputeGradientsOff();
//     m_march->SetInputConnection(m_padfilter->GetOutputPort());
//   }
//   else
//   {
//     if (m_padfilter->GetInputConnection(0,0) != toVTK())
//       m_padfilter->SetInputConnection(toVTK());
// 
//     int outputExtent[6];
//     m_padfilter->GetOutputWholeExtent(outputExtent);
//     if (memcmp(extent, outputExtent, 6*sizeof(int)) != 0)
//     {
//       m_padfilter->SetOutputWholeExtent(extent[0], extent[1], extent[2], extent[3], extent[4], extent[5]);
//       m_padfilter->Update();
//     }
//   }
// 
//   m_march->Update();
//   return m_march->GetOutput()->GetProducerPort();
// }
