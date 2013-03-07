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


#include "Filter.h"

#include <itkImageAlgorithm.h>
#include <itkImageRegionExclusionIteratorWithIndex.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkMetaImageIO.h>
#include <itkVTKImageToImageFilter.h>
#include <itkImageFileWriter.h>
#include <itkRegionOfInterestImageFilter.h>

#include <vtkImplicitFunction.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkImageStencilToImage.h>
#include <vtkImageExport.h>
#include <vtkMath.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>

#include <QDir>
#include <QMessageBox>
#include <QWidget>
#include <QDebug>

using namespace EspINA;

const QString EspINA::Filter::CREATELINK = "CreateSegmentation";

typedef ModelItem::ArgumentId ArgumentId;

const ArgumentId Filter::ID     = "ID";
const ArgumentId Filter::INPUTS = "Inputs";
const ArgumentId Filter::EDIT   = "Edit"; // Backwards compatibility

const int EspINA::Filter::Output::INVALID_OUTPUT_ID = -1;

typedef itk::ImageFileWriter<itkVolumeType> EspinaVolumeWriter;

//----------------------------------------------------------------------------
void Filter::Output::addEditedRegion(const EspinaRegion &region)
{
  int  i        = 0;
  bool included = false;
  while (i < editedRegions.size() && !included)
  {
    included = region.isInside(editedRegions[i]);
    ++i;
  }
  if (!included)
    editedRegions << region;
}



//----------------------------------------------------------------------------
Filter::~Filter()
{
  qDebug() << "Destruyendo Filter";
}

//----------------------------------------------------------------------------
void Filter::setCacheDir(QDir dir)
{
  m_cacheDir = dir;

  // Load cached outputs
  if (m_outputs.isEmpty())
  {
    QStringList namedFilters;
    namedFilters <<  QString("%1_*.mhd").arg(m_cacheId);
    foreach(QString cachedFile, m_cacheDir.entryList(namedFilters))
    {
      QStringList ids = cachedFile.section(".",0,0).split("_");
      OutputId oId = ids[1].toInt();
      QString outputName = QString("%1_%2").arg(m_cacheId).arg(oId);

      if (!validOutput(oId))
        createOutput(oId);

      Output &editedOutput = m_outputs[oId];

      if (editedOutput.editedRegions.isEmpty() && m_cacheDir.exists(outputName + ".trc"))
      {
        QFile file(m_cacheDir.absoluteFilePath(outputName + ".trc"));
        if (file.open(QIODevice::ReadOnly))
        {
          while (!file.atEnd())
          {
            QByteArray line = file.readLine();
            QStringList values = QString(line).split(" ");
            Nm bounds[6];
            for (int i = 0; i < 6; ++i)
              bounds[i] = values[i].toDouble();

            editedOutput.editedRegions << EspinaRegion(bounds);
          }
        }
      }
    }
  }
}

//----------------------------------------------------------------------------
Filter::Filter(Filter::NamedInputs  namedInputs,
               ModelItem::Arguments args,
               FilterType           type)
: m_namedInputs(namedInputs)
, m_args(args)
, m_type(type)
, m_cacheId(-1)
, m_traceable(false)
{
  if (m_args.contains(ID)) {
    m_cacheId = m_args[ID].toInt();
  } else {
    m_args[ID] = "-1";
  }
}

//----------------------------------------------------------------------------
QVariant Filter::data(int role) const
{
  if (Qt::DisplayRole == role)
    return m_type;
  else
    return QVariant();
}

//----------------------------------------------------------------------------
QString Filter::serialize() const
{
  // NOTE: EDIT arg is being deprecated
  Arguments copy = m_args;
  copy.remove(EDIT);

  return copy.serialize();
}

//----------------------------------------------------------------------------
void Filter::draw(OutputId oId,
                  vtkImplicitFunction* brush,
                  const Nm bounds[6],
                  itkVolumeType::PixelType value,
                  bool emitSignal)
{
  EspinaRegion region(bounds);

  Filter::Output &currentOutput = output(oId);

  currentOutput.addEditedRegion(region);

  EspinaVolume::Pointer volume = currentOutput.volume;
  volume->expandToFitRegion(region);

  itkVolumeType::SpacingType spacing = volume->toITK()->GetSpacing();
  itkVolumeType::PointType   origin  = volume->toITK()->GetOrigin();

  itkVolumeIterator it = volume->iterator(region);
  it.GoToBegin();
  for (; !it.IsAtEnd(); ++it )
  {
    double tx = it.GetIndex()[0]*spacing[0] + origin[0];
    double ty = it.GetIndex()[1]*spacing[1] + origin[1];
    double tz = it.GetIndex()[2]*spacing[2] + origin[2];

    if (brush->FunctionValue(tx, ty, tz) <= 0)
      it.Set(value);
  }
  volume->markAsModified(emitSignal);

  emit modified(this);
}

//----------------------------------------------------------------------------
void Filter::draw(OutputId oId,
                  itkVolumeType::IndexType index,
                  itkVolumeType::PixelType value,
                  bool emitSignal)
{
  Filter::Output &currentOutput = output(oId);

  EspinaVolume::Pointer volume = currentOutput.volume;

  itkVolumeType::SpacingType spacing = volume->toITK()->GetSpacing();
  double voxelBounds[6] = { index[0]*spacing[0],
                            index[0]*spacing[0],
                            index[1]*spacing[1],
                            index[1]*spacing[1],
                            index[2]*spacing[2],
                            index[2]*spacing[2] };
  EspinaRegion voxelRegion(voxelBounds);

  currentOutput.addEditedRegion(voxelRegion);

  volume->expandToFitRegion(voxelRegion);

  volume->toITK()->SetPixel(index, value);
  volume->markAsModified(emitSignal);

  emit modified(this);
}

//----------------------------------------------------------------------------
void Filter::draw(OutputId oId,
                  Nm x, Nm y, Nm z,
                  itkVolumeType::PixelType value,
                  bool emitSignal)
{
  Filter::Output &currentOutput = output(oId);

  EspinaVolume::Pointer volume = currentOutput.volume;

  double voxelBounds[6] = {x, x, y, y, z, z};
  EspinaRegion voxelRegion(voxelBounds);

  currentOutput.addEditedRegion(voxelRegion);

  volume->expandToFitRegion(voxelRegion);

  volume->toITK()->SetPixel(volume->index(x, y, z), value);
  volume->markAsModified(emitSignal);

  emit modified(this);
}

//----------------------------------------------------------------------------
void Filter::draw(OutputId oId,
                  vtkPolyData *contour,
                  Nm slice, PlaneType plane,
                  itkVolumeType::PixelType value,
                  bool emitSignal)
{
  if (contour->GetPoints()->GetNumberOfPoints() == 0)
    return;

  Filter::Output &currentOutput = output(oId);

  EspinaVolume::Pointer volume = currentOutput.volume;

  double bounds[6];
  contour->ComputeBounds();
  contour->GetBounds(bounds);

  EspinaRegion polyDataRegion(bounds);

  currentOutput.addEditedRegion(polyDataRegion);

  if(!polyDataRegion.isInside(volume->espinaRegion()))
    volume->expandToFitRegion(polyDataRegion);

  // vtkPolyDataToImageStencil filter only works in XY plane so we must rotate the contour to that plane
  itkVolumeType::SpacingType spacing = volume->toITK()->GetSpacing();
  int count = contour->GetPoints()->GetNumberOfPoints();
  vtkSmartPointer<vtkPolyData> rotatedContour = vtkSmartPointer<vtkPolyData>::New();
  vtkPoints *points = vtkPoints::New();
  vtkCellArray *lines = vtkCellArray::New();
  vtkIdType index = 0;

  points->SetNumberOfPoints(count);
  vtkIdType numLines = count + 1;

  if (numLines > 0)
  {
    double pos[3];
    vtkIdType *lineIndices = new vtkIdType[numLines];

    for (int i = 0; i < count; i++)
    {
      double temporal;
      contour->GetPoint(i, pos);
      switch (plane)
      {
        case AXIAL:
          pos[2] -= 0.5 * spacing[2];
          break;
        case CORONAL:
          temporal = pos[1];
          pos[1] = pos[2];
          pos[2] = temporal - 0.5 * spacing[1];
          break;
        case SAGITTAL:
          temporal = pos[0];
          pos[0] = pos[1];
          pos[1] = pos[2];
          pos[2] = temporal - 0.5 * spacing[0];
          break;
        default:
          Q_ASSERT(false);
          break;
      }
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

  int extent[6];
  volume->extent(extent);
  double temporal;
  int temporalvalues[2];

  // extent and spacing should be changed because vtkPolyDataToImageStencil filter only works in XY plane
  // and we've rotated the contour to that plane
  switch(plane)
  {
    case AXIAL:
      break;
    case CORONAL:
      temporal = spacing[1];
      spacing[1] = spacing[2];
      spacing[2] = temporal;
      temporalvalues[0] = extent[2];
      temporalvalues[1] = extent[3];
      extent[2] = extent[4];
      extent[3] = extent[5];
      extent[4] = temporalvalues[0];
      extent[5] = temporalvalues[1];
      break;
    case SAGITTAL:
      temporal = spacing[0];
      spacing[0] = spacing[1];
      spacing[1] = spacing[2];
      spacing[2] = temporal;
      temporalvalues[0] = extent[0];
      temporalvalues[1] = extent[1];
      extent[0] = extent[2];
      extent[1] = extent[3];
      extent[2] = extent[4];
      extent[3] = extent[5];
      extent[4] = temporalvalues[0];
      extent[5] = temporalvalues[1];
      break;
    default:
      Q_ASSERT(false);
      break;
  }

  vtkSmartPointer<vtkPolyDataToImageStencil> polyDataToStencil = vtkSmartPointer<vtkPolyDataToImageStencil>::New();
  polyDataToStencil->SetInputConnection(rotatedContour->GetProducerPort());
  polyDataToStencil->SetOutputOrigin(0,0,0);
  polyDataToStencil->SetOutputSpacing(spacing[0], spacing[1], spacing[2]);
  polyDataToStencil->SetOutputWholeExtent(extent[0], extent[1], extent[2], extent[3], vtkMath::Round(slice/spacing[2]), vtkMath::Round(slice/spacing[2]));
  polyDataToStencil->SetTolerance(0);

  vtkSmartPointer<vtkImageStencilToImage> stencilToImage = vtkSmartPointer<vtkImageStencilToImage>::New();
  stencilToImage->SetInputConnection(polyDataToStencil->GetOutputPort());
  stencilToImage->SetOutputScalarTypeToUnsignedChar();
  stencilToImage->SetInsideValue(1);
  stencilToImage->SetOutsideValue(0);

  vtkSmartPointer<vtkImageExport> exporter = vtkSmartPointer<vtkImageExport>::New();
  exporter->SetInputConnection(stencilToImage->GetOutputPort());

  typedef itk::VTKImageToImageFilter<itkVolumeType> VTKImporterType;
  VTKImporterType::Pointer importer = VTKImporterType::New();
  importer->SetUpdateInformationCallback(exporter->GetUpdateInformationCallback());
  importer->SetPipelineModifiedCallback(exporter->GetPipelineModifiedCallback());
  importer->SetWholeExtentCallback(exporter->GetWholeExtentCallback());
  importer->SetSpacingCallback(exporter->GetSpacingCallback());
  importer->SetOriginCallback(exporter->GetOriginCallback());
  importer->SetScalarTypeCallback(exporter->GetScalarTypeCallback());
  importer->SetNumberOfComponentsCallback(exporter->GetNumberOfComponentsCallback());
  importer->SetPropagateUpdateExtentCallback(exporter->GetPropagateUpdateExtentCallback());
  importer->SetUpdateDataCallback(exporter->GetUpdateDataCallback());
  importer->SetDataExtentCallback(exporter->GetDataExtentCallback());
  importer->SetBufferPointerCallback(exporter->GetBufferPointerCallback());
  importer->SetCallbackUserData(exporter->GetCallbackUserData());
  importer->Update();

  // we have to check the image index to know if there is a discrepancy between bounds and index
  spacing = volume->toITK()->GetSpacing();
  itkVolumeType::IndexType temporalIndex = volume->toITK()->GetLargestPossibleRegion().GetIndex();
  volume->bounds(bounds);
  bool transformIndex = false;
  if (bounds[0]/spacing[0] != temporalIndex[0] || bounds[2]/spacing[1] != temporalIndex[1] || bounds[4]/spacing[2] != temporalIndex[2])
  {
    transformIndex = true;
    temporalIndex[0] = bounds[0]/spacing[0];
    temporalIndex[1] = bounds[2]/spacing[1];
    temporalIndex[2] = bounds[4]/spacing[2];
  }

  itk::Index<3> imageIndex;
  itk::ImageRegionIteratorWithIndex<itkVolumeType> init(importer->GetOutput(), importer->GetOutput()->GetLargestPossibleRegion());
  init.GoToBegin();
  while(!init.IsAtEnd())
  {
    if (1 == init.Value())
    {
      int temporal;
      imageIndex[0] = init.GetIndex()[0];
      imageIndex[1] = init.GetIndex()[1];
      imageIndex[2] = init.GetIndex()[2];

      switch(plane)
      {
        case AXIAL:
          break;
        case CORONAL:
          temporal = imageIndex[2];
          imageIndex[2] = imageIndex[1];
          imageIndex[1] = temporal;
          break;
        case SAGITTAL:
          temporal = imageIndex[2];
          imageIndex[2] = imageIndex[1];
          imageIndex[1] = imageIndex[0];
          imageIndex[0] = temporal;
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

      Q_ASSERT(volume->toITK()->GetLargestPossibleRegion().IsInside(imageIndex));
      volume->toITK()->SetPixel(imageIndex, value);
    }
    ++init;
  }
  volume->markAsModified(emitSignal);

  emit modified(this);
}

//----------------------------------------------------------------------------
void Filter::draw(OutputId oId,
                  itkVolumeType::Pointer volume,
                  bool emitSignal)
{
  Filter::Output &currentOutput = output(oId);

  EspinaVolume::Pointer filterVolume = currentOutput.volume;

  EspinaVolume drawnVolume(volume);
  EspinaRegion drawnRegion = drawnVolume.espinaRegion();

  currentOutput.addEditedRegion(drawnRegion);

  filterVolume->expandToFitRegion(drawnRegion);

  itkVolumeIterator it = drawnVolume  .iterator(drawnRegion);
  itkVolumeIterator ot = filterVolume->iterator(drawnRegion);

  it.GoToBegin();
  ot.GoToBegin();
  for (; !it.IsAtEnd(); ++it, ++ot )
  {
    ot.Set(it.Get());
  }

  filterVolume->markAsModified(emitSignal);

  emit modified(this);
}

//----------------------------------------------------------------------------
void Filter::fill(OutputId oId,
                  itkVolumeType::PixelType value,
                  bool emitSignal)
{
  fill(oId, m_outputs[oId].volume->espinaRegion(), value, emitSignal);
}

//----------------------------------------------------------------------------
void Filter::fill(OutputId oId,
                  const EspinaRegion &region,
                  itkVolumeType::PixelType value,
                  bool emitSignal)
{
  Output &currentOutput = output(oId);

  EspinaVolume::Pointer volume = currentOutput.volume;

  volume->expandToFitRegion(region);

  itkVolumeIterator ot = volume->iterator(region);

  ot.GoToBegin();
  for (; !ot.IsAtEnd(); ++ot )
  {
    ot.Set(value);
  }

  volume->markAsModified(emitSignal);

  emit modified(this);
}

//----------------------------------------------------------------------------
void Filter::restoreOutput(OutputId oId, itkVolumeType::Pointer volume)
{
  Output &filterOutput = output(oId);

  filterOutput.volume->setVolume(volume);
  filterOutput.volume->markAsModified();

  emit modified(this);
}

//----------------------------------------------------------------------------
Filter::OutputList Filter::editedOutputs() const
{
  OutputList res;

  foreach(Output filterOutput, m_outputs)
  {
    if (filterOutput.isEdited())
      res << filterOutput;
  }

  return res;
}

//----------------------------------------------------------------------------
bool Filter::validOutput(Filter::OutputId oId)
{
  return m_outputs.contains(oId);
}

//----------------------------------------------------------------------------
const Filter::Output Filter::output(OutputId oId) const
{
  return m_outputs.value(oId, Output());
}

//----------------------------------------------------------------------------
Filter::Output &Filter::output(OutputId oId)
{
  Q_ASSERT(m_outputs.contains(oId));
  return m_outputs[oId];
}

//----------------------------------------------------------------------------
bool Filter::needUpdate() const
{
  bool update = true;

  if (!m_outputs.isEmpty())
  {
    update = false;
    foreach(Output filterOutput, m_outputs)
    {
      update = update || !filterOutput.isValid();
    }
  }

  return update;
}

//----------------------------------------------------------------------------
 void Filter::update()
{
  if (!needUpdate())
    return;

  if (!fetchSnapshot())
  {
    m_inputs.clear();

    foreach(OutputId oId, m_outputs.keys())
      m_outputs[oId].editedRegions.clear();

    QStringList namedInputList = m_args[INPUTS].split(",", QString::SkipEmptyParts);
    foreach(QString namedInput, namedInputList)
    {
      QStringList input = namedInput.split("_");
      FilterSPtr inputFilter = m_namedInputs[input[0]];
      inputFilter->update();
      OutputId oId = input[1].toInt();
      m_inputs << inputFilter->output(oId).volume;
    }

    run();
  }
}

//----------------------------------------------------------------------------
void Filter::createOutput(Filter::OutputId id, EspinaVolume::Pointer volume)
{
  if (!m_outputs.contains(id))
    m_outputs[id] = Output(this, id, volume);
  else if (volume)
    m_outputs[id].volume->setVolume(volume->toITK());
}

//----------------------------------------------------------------------------
void Filter::resetCacheFlags()
{
  foreach(OutputId oId, m_outputs.keys())
  {
    output(oId).isCached = false;
  }
}

//----------------------------------------------------------------------------
bool Filter::fetchSnapshot()
{
  if (m_outputs.isEmpty())
    return false;

  //WARNING: Hace falta que todos filtros carguen las outputs al crearse
  // aunque estas puedan ser invalidas NULL volume pointer
  EspinaVolumeReader::Pointer reader;

  foreach(OutputId oId, m_outputs.keys())
  {
    QString tmpFile = QString("%1_%2.mhd").arg(m_cacheId).arg(oId);
    reader = tmpFileReader(tmpFile);
    if (reader.IsNull())
      return false;

    m_outputs[oId].volume->setVolume(reader->GetOutput(), true);
  }

  emit modified(this);

  return true;
}

//----------------------------------------------------------------------------
Filter::EspinaVolumeReader::Pointer Filter::tmpFileReader(const QString file)
{
  EspinaVolumeReader::Pointer reader;

  if (m_cacheDir.exists(file))
  {
    itk::MetaImageIO::Pointer io = itk::MetaImageIO::New();
    reader = EspinaVolumeReader::New();

    QByteArray tmpFile = m_cacheDir.absoluteFilePath(file).toUtf8();
    io->SetFileName(tmpFile);
    reader->SetImageIO(io);
    reader->SetFileName(tmpFile.data());
    reader->Update();
  }

  return reader;
}

//----------------------------------------------------------------------------
void ChannelFilter::createOutput(Filter::OutputId id, itkVolumeType::Pointer volume)
{
  if (id != 0)
    qDebug() << id;
  if (!m_outputs.contains(id))
    m_outputs[id] = Output(this, id, ChannelVolume::Pointer(new ChannelVolume(volume)));
  else if (volume)
    m_outputs[id].volume->setVolume(volume);
}

//----------------------------------------------------------------------------
void ChannelFilter::createOutput(Filter::OutputId id, EspinaVolume::Pointer volume)
{
  Filter::createOutput(id, volume);
}

//----------------------------------------------------------------------------
void ChannelFilter::createOutput(Filter::OutputId id, const EspinaRegion& region, itkVolumeType::SpacingType spacing)
{
  ChannelVolume::Pointer volume(new ChannelVolume(region, spacing));
  if (!m_outputs.contains(id))
    m_outputs[id] = Output(this, id, volume);
  else if (volume)
    m_outputs[id].volume->setVolume(volume->toITK());
}

//----------------------------------------------------------------------------
void SegmentationFilter::createOutput(Filter::OutputId id, EspinaVolume::Pointer volume)
{
  Filter::createOutput(id, volume);
}

//----------------------------------------------------------------------------
void SegmentationFilter::createOutput(Filter::OutputId id, itkVolumeType::Pointer volume)
{
  if (m_outputs.contains(id))
    m_outputs[id].volume->setVolume(volume);
  else
    m_outputs[id] = Output(this, id, SegmentationVolume::Pointer(new SegmentationVolume(volume)));
}

//----------------------------------------------------------------------------
void SegmentationFilter::createOutput(Filter::OutputId id, const EspinaRegion& region, itkVolumeType::SpacingType spacing)
{
  SegmentationVolume::Pointer volume(new SegmentationVolume(region, spacing));
  if (m_outputs.contains(id))
    m_outputs[id].volume->setVolume(volume->toITK());
  else
    m_outputs[id] = Output(this, id, volume);
}

//----------------------------------------------------------------------------
FilterPtr EspINA::filterPtr(ModelItemPtr item)
{
  Q_ASSERT(EspINA::FILTER == item->type());
  FilterPtr ptr = dynamic_cast<FilterPtr>(item);
  Q_ASSERT(ptr);

  return ptr;
}

//----------------------------------------------------------------------------
FilterSPtr EspINA::filterPtr(ModelItemSPtr& item)
{
  Q_ASSERT(EspINA::FILTER == item->type());
  FilterSPtr ptr = qSharedPointerDynamicCast<Filter>(item);
  Q_ASSERT(!ptr.isNull());

  return ptr;

}

typedef itk::RegionOfInterestImageFilter<itkVolumeType, itkVolumeType> ROIFilter;


//----------------------------------------------------------------------------
bool Filter::dumpSnapshot(Snapshot &snapshot)
{
  itk::MetaImageIO::Pointer io = itk::MetaImageIO::New();
  EspinaVolumeWriter::Pointer writer = EspinaVolumeWriter::New();

  QDir temporalDir = QDir::tempPath();
  bool result = false;

  foreach(Output output, this->outputs())
  {
    QString outputName = QString("%1_%2").arg(id()).arg(output.id);
    if (output.isCached)
    {
      result = true;

      QString mhd = temporalDir.absoluteFilePath(outputName + ".mhd");
      QString raw = temporalDir.absoluteFilePath(outputName + ".raw");

      io->SetFileName(mhd.toUtf8());
      writer->SetFileName(mhd.toUtf8().data());

      update();

      itkVolumeType::Pointer volume = output.volume->toITK();
      bool releaseFlag = volume->GetReleaseDataFlag();
      volume->ReleaseDataFlagOff();
      writer->SetInput(volume);
      writer->SetImageIO(io);
      writer->Write();
      volume->SetReleaseDataFlag(releaseFlag);

      QFile mhdFile(mhd);
      mhdFile.open(QIODevice::ReadOnly);
      QFile rawFile(raw);
      rawFile.open(QIODevice::ReadOnly);

      QByteArray mhdArray(mhdFile.readAll());
      QByteArray rawArray(rawFile.readAll());

      mhdFile.close();
      rawFile.close();
      temporalDir.remove(mhd);
      temporalDir.remove(raw);

      SnapshotEntry mhdEntry(outputName + ".mhd", mhdArray);
      SnapshotEntry rawEntry(outputName + ".raw", rawArray);

      snapshot << mhdEntry << rawEntry;
    }

    // Backwards compatibility with seg files version 1.0
    if (m_traceable && m_args.contains(EDIT))
    {
      update();

      QStringList outputIds = m_args[EDIT].split(",");
      if (outputIds.contains(QString::number(output.id)))
      {
        EspinaRegion region = output.volume->espinaRegion();
        m_outputs[output.id].addEditedRegion(region);
        output = m_outputs[output.id];
      }
    }

    if (m_traceable && output.isEdited())
    {
      result = true;

      std::ostringstream regions;
      for (int r = 0; r < output.editedRegions.size(); ++r)
      {
        EspinaRegion editedRegion = output.editedRegions[r];
        for (int i = 0; i < 6; ++i)
        {
          regions << editedRegion[i] << " ";
        }
        regions << std::endl;

        // Dump modified volume
        if (!output.isCached)
        {
          QString regionName = QString("%1_%2").arg(outputName).arg(r);
          QString mhd = temporalDir.absoluteFilePath(regionName + ".mhd");
          QString raw = temporalDir.absoluteFilePath(regionName + ".raw");

          itkVolumeType::Pointer regionVolume = output.volume->toITK();
          if (regionVolume)
          {
            bool releaseFlag = regionVolume->GetReleaseDataFlag();
            regionVolume->ReleaseDataFlagOff();

            io->SetFileName(mhd.toUtf8());
            writer->SetFileName(mhd.toUtf8().data());

            ROIFilter::Pointer roiFilter = ROIFilter::New();
            roiFilter->SetRegionOfInterest(output.volume->volumeRegion(editedRegion));
            roiFilter->SetInput(regionVolume);
            roiFilter->Update();

            writer->SetInput(roiFilter->GetOutput());
            writer->SetImageIO(io);
            writer->Write();
            regionVolume->SetReleaseDataFlag(releaseFlag);
          } else
          {
            // dump cache volumes
            mhd = m_cacheDir.absoluteFilePath(regionName + ".mhd");
            raw = m_cacheDir.absoluteFilePath(regionName + ".raw");
          }

          QFile mhdFile(mhd);
          mhdFile.open(QIODevice::ReadOnly);
          QFile rawFile(raw);
          rawFile.open(QIODevice::ReadOnly);

          QByteArray mhdArray(mhdFile.readAll());
          QByteArray rawArray(rawFile.readAll());

          mhdFile.close();
          rawFile.close();

          if (regionVolume)
          {
            temporalDir.remove(mhd);
            temporalDir.remove(raw);
          }

          SnapshotEntry mhdEntry(regionName + ".mhd", mhdArray);
          SnapshotEntry rawEntry(regionName + ".raw", rawArray);

          snapshot << mhdEntry << rawEntry;
        }
      }

      snapshot << SnapshotEntry(outputName + ".trc", regions.str().c_str());
    }

    m_outputs[output.id].isCached = false;
  }

  return result;
}
