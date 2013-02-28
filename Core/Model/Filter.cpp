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
    QStringList editList;
    if (m_args.contains(EDIT))
      editList = m_args[EDIT].split(",");

    foreach(QString cachedFile, m_cacheDir.entryList())
    {
      QString filterIdTag = QString("%1_").arg(m_cacheId);
      if (cachedFile.startsWith(filterIdTag))
      {
        QString id = cachedFile.split("_").last();
        id = id.split(".").first();
        if (cachedFile.endsWith("mhd"))
        {
          OutputId oId = id.toInt();

          createOutput(oId);

          if (editList.contains(id))
            m_outputs[oId].isEdited = true;
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
  QStringList editList;
  foreach(Output filterOutput, m_outputs)
  {
    if (filterOutput.isCached && filterOutput.isEdited)
      editList << QString::number(filterOutput.id);
  }

  if (!editList.isEmpty())
    m_args[EDIT] = editList.join(",");

  return m_args.serialize();
}

//----------------------------------------------------------------------------
void Filter::draw(OutputId oId,
                  vtkImplicitFunction* brush,
                  const Nm bounds[6],
                  itkVolumeType::PixelType value,
                  bool emitSignal)
{
  EspinaRegion region(bounds);

  EspinaVolume::Pointer volume = output(oId).volume;
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

  markAsEdited(oId);

  emit modified(this);
}

//----------------------------------------------------------------------------
void Filter::draw(OutputId oId,
                  itkVolumeType::IndexType index,
                  itkVolumeType::PixelType value,
                  bool emitSignal)
{
  EspinaVolume::Pointer volume  = output(oId).volume;

  itkVolumeType::SpacingType spacing = volume->toITK()->GetSpacing();
  double voxelBounds[6] = { index[0]*spacing[0],
                            index[0]*spacing[0],
                            index[1]*spacing[1],
                            index[1]*spacing[1],
                            index[2]*spacing[2],
                            index[2]*spacing[2] };
  EspinaRegion voxelRegion(voxelBounds);

  volume->expandToFitRegion(voxelRegion);

  volume->toITK()->SetPixel(index, value);
  volume->markAsModified(emitSignal);

  markAsEdited(oId);

  emit modified(this);
}

//----------------------------------------------------------------------------
void Filter::draw(OutputId oId,
                  Nm x, Nm y, Nm z,
                  itkVolumeType::PixelType value,
                  bool emitSignal)
{
  EspinaVolume::Pointer volume  = output(oId).volume;

  double voxelBounds[6] = {x, x, y, y, z, z};
  EspinaRegion voxelRegion(voxelBounds);

  volume->expandToFitRegion(voxelRegion);

  volume->toITK()->SetPixel(volume->index(x, y, z), value);
  volume->markAsModified(emitSignal);

  markAsEdited(oId);

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

  EspinaVolume::Pointer volume = output(oId).volume;
  double bounds[6];
  contour->ComputeBounds();
  contour->GetBounds(bounds);
  EspinaRegion polyDataRegion(bounds);

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

  markAsEdited(oId);
  emit modified(this);
}

//----------------------------------------------------------------------------
void Filter::draw(OutputId oId,
                  itkVolumeType::Pointer volume,
                  bool emitSignal)
{
  EspinaVolume::Pointer filterVolume = output(oId).volume;
  EspinaVolume drawnVolume(volume);

  EspinaRegion region = drawnVolume.espinaRegion();

  filterVolume->expandToFitRegion(region);

  itkVolumeIterator it = drawnVolume  .iterator(region);
  itkVolumeIterator ot = filterVolume->iterator(region);

  it.GoToBegin();
  ot.GoToBegin();
  for (; !it.IsAtEnd(); ++it, ++ot )
  {
    ot.Set(it.Get());
  }
  filterVolume->markAsModified(emitSignal);

  markAsEdited(oId);

  emit modified(this);
}

//----------------------------------------------------------------------------
void Filter::restoreOutput(OutputId oId, itkVolumeType::Pointer volume)
{
  Output &filterOutput = output(oId);

  filterOutput.volume->setVolume(volume);
  filterOutput.volume->markAsModified();

  markAsEdited(oId);

  emit modified(this);
}

//----------------------------------------------------------------------------
Filter::OutputList Filter::editedOutputs() const
{
  OutputList res;

  foreach(Output filterOutput, m_outputs)
  {
    if (filterOutput.isEdited)
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
      m_outputs[oId].isEdited = false;

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
  if (m_outputs.contains(id))
    m_outputs[id].volume->setVolume(volume->toITK());
  else
    m_outputs[id] = Output(this, id, volume);
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

    std::string tmpFile = m_cacheDir.absoluteFilePath(file).toStdString();
    io->SetFileName(tmpFile.c_str());
    reader->SetImageIO(io);
    reader->SetFileName(tmpFile);
    reader->Update();
  }

  return reader;
}

//----------------------------------------------------------------------------
void Filter::markAsEdited(OutputId oId)
{
  Q_ASSERT(m_outputs.contains(oId));

  m_outputs[oId].isEdited = true;

  QStringList editedOutputList;
  foreach(Output filterOutput, m_outputs)
  {
    if (filterOutput.isEdited)
      editedOutputList << QString::number(filterOutput.id);
  }

  m_args[EDIT] = editedOutputList.join(",");
}

//----------------------------------------------------------------------------
void Filter::updateCacheFlags()
{
  QStringList namedInputList = m_args[INPUTS].split(",", QString::SkipEmptyParts);
  foreach(QString namedInput, namedInputList)
  {
    QStringList input = namedInput.split("_");
    FilterSPtr filter = m_namedInputs[input[0]];
    OutputId oId = input[1].toInt();

    if (filter->validOutput(oId))
    {
      Output &output = filter->output(oId);
      output.isCached = output.isCached || output.isEdited;
    }
  }
}

//----------------------------------------------------------------------------
void ChannelFilter::createOutput(Filter::OutputId id, itkVolumeType::Pointer volume)
{
  if (id != 0)
    qDebug() << id;
  if (m_outputs.contains(id))
    m_outputs[id].volume->setVolume(volume);
  else
    m_outputs[id] = Output(this, id, ChannelVolume::Pointer(new ChannelVolume(volume)));
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
  if (m_outputs.contains(id))
    m_outputs[id].volume->setVolume(volume->toITK());
  else
    m_outputs[id] = Output(this, id, volume);
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

//----------------------------------------------------------------------------
bool Filter::dumpSnapshot(Snapshot &snapshot)
{
  itk::MetaImageIO::Pointer io = itk::MetaImageIO::New();
  EspinaVolumeWriter::Pointer writer = EspinaVolumeWriter::New();

  QDir temporalDir = QDir::tempPath();
  bool result = false;

  foreach(Output output, this->outputs())
  {
    if (!output.isCached)
      continue;

    result = true;

    QString volumeName = QString("%1_%2").arg(this->id()).arg(output.id);
    QString mhd = temporalDir.absoluteFilePath(volumeName + ".mhd");
    QString raw = temporalDir.absoluteFilePath(volumeName + ".raw");

    io->SetFileName(mhd.toStdString());
    writer->SetFileName(mhd.toStdString());

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

    SnapshotEntry mhdEntry(volumeName + QString(".mhd"), mhdArray);
    SnapshotEntry rawEntry(volumeName + QString(".raw"), rawArray);

    snapshot << mhdEntry << rawEntry;
  }

  return result;
}
