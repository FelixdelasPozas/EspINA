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

#include "Core/EspinaRegions.h"

#include <itkImageAlgorithm.h>
#include <itkImageRegionExclusionIteratorWithIndex.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkMetaImageIO.h>
#include <itkVTKImageToImageFilter.h>

#include <vtkImplicitFunction.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkImageStencilToImage.h>
#include <vtkImageExport.h>
#include <vtkMath.h>

#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QWidget>

typedef ModelItem::ArgumentId ArgumentId;

const ArgumentId Filter::ID      = "ID";
const ArgumentId Filter::INPUTS  = "Inputs";
const ArgumentId Filter::EDIT    = "Edit"; // Backwards compatibility

//----------------------------------------------------------------------------
void Filter::setTmpDir(QDir dir)
{
  m_tmpDir = dir;

  // Load cached outputs
  if (m_outputs.isEmpty())
  {
    QStringList editList;
    if (m_args.contains(EDIT))
      editList = m_args[EDIT].split(",");

    foreach(QString cachedFile, m_tmpDir.entryList())
    {
      QString filterIdTag = tmpId() + "_";
      if (cachedFile.contains(filterIdTag))
      {
        QString id = cachedFile.split("_").last();
        id = id.split(".").first();
        if (cachedFile.endsWith("mhd"))
        {
          Output cachedOutput(this, id.toInt());
          if (editList.contains(id))
            cachedOutput.isEdited = true;
          m_outputs << cachedOutput;
        }
      }
    }
  }
}

//----------------------------------------------------------------------------
Filter::Filter(Filter::NamedInputs  namedInputs,
               ModelItem::Arguments args)
: m_namedInputs(namedInputs)
, m_args(args)
{
  if (!m_args.contains(ID))
    m_args[ID] = "-1";
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
                  double bounds[6],
                  EspinaVolume::PixelType value)
{
  Output &filterOutput = output(oId);

  EspinaVolume::Pointer     volume  = filterOutput.volume;
  EspinaVolume::SpacingType spacing = volume->GetSpacing();
  EspinaVolume::RegionType  region  = BoundsToRegion(bounds, spacing);

  volume = expandVolume(volume, region);

  EspinaVolume::RegionType outputRegion = VolumeRegion(volume, region);
  itk::ImageRegionIteratorWithIndex<EspinaVolume>   it(volume, outputRegion);

  it.GoToBegin();
  for (; !it.IsAtEnd(); ++it )
  {
    double tx = it.GetIndex()[0]*spacing[0] + volume->GetOrigin()[0];
    double ty = it.GetIndex()[1]*spacing[1] + volume->GetOrigin()[1];
    double tz = it.GetIndex()[2]*spacing[2] + volume->GetOrigin()[2];

    if (brush->FunctionValue(tx, ty, tz) <= 0)
      it.Set(value);
  }

  volume->Modified();
  filterOutput.volume = volume;

  markAsEdited(oId);

  emit modified(this);
}

//----------------------------------------------------------------------------
void Filter::draw(OutputId oId,
                  EspinaVolume::IndexType index,
                  EspinaVolume::PixelType value)
{
  Output &filterOutput = output(oId);
  EspinaVolume::Pointer volume = filterOutput.volume;

  EspinaVolume::RegionType region;
  region.SetIndex(index);
  EspinaVolume::SizeType size;
  size.Fill(1);
  region.SetSize(size);
  volume = expandVolume(volume, region);

  if (volume && volume->GetLargestPossibleRegion().IsInside(index))
  {
    volume->SetPixel(index, value);
    volume->Modified();

    filterOutput.volume = volume;// TODO 2012-11-20 Se podria reemplazar por una referencia en la declaracion de volume

    markAsEdited(oId);

    emit modified(this);
  }
}

//----------------------------------------------------------------------------
void Filter::draw(OutputId oId,
                  Nm x, Nm y, Nm z,
                  EspinaVolume::PixelType value)
{
  Output &filterOutput = output(oId);
  EspinaVolume::Pointer volume  = filterOutput.volume;

  if (volume.IsNotNull())
  {
    EspinaVolume::SpacingType spacing = volume->GetSpacing();

    EspinaVolume::IndexType index;
    index[0] = x/spacing[0]+0.5;
    index[1] = y/spacing[1]+0.5;
    index[2] = z/spacing[2]+0.5;

    draw(oId, index, value);
  }
}

//----------------------------------------------------------------------------
void Filter::draw(OutputId oId,
                  vtkPolyData *contour,
                  Nm slice, PlaneType plane,
      EspinaVolume::PixelType value)
{
  Output &filterOutput = output(oId);
  EspinaVolume::Pointer volume  = filterOutput.volume;

  int extent[6];
  VolumeExtent(volume, extent);
  EspinaVolume::SpacingType spacing = volume->GetSpacing();

  double temporal;
  int temporalvalues[2];
  // extent and spacing should be changed because vtkPolyDataToImageStencil filter only works in XY plane.
  // the contour already has been rotated in the ContourSource part of this filter.
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
  polyDataToStencil = vtkSmartPointer<vtkPolyDataToImageStencil>::New();
  polyDataToStencil->SetInputConnection(contour->GetProducerPort());
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

  typedef itk::VTKImageToImageFilter<EspinaVolume> VTKImporterType;
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

  VolumeExtent(importer->GetOutput(), extent);

  itk::Index<3> index;
  itk::ImageRegionIteratorWithIndex<EspinaVolume> init(importer->GetOutput(), importer->GetOutput()->GetLargestPossibleRegion());
  init.GoToBegin();
  while(!init.IsAtEnd())
  {
    if (1 == init.Value())
    {
      int temporal;
      index[0] = init.GetIndex()[0];
      index[1] = init.GetIndex()[1];
      index[2] = init.GetIndex()[2];

      switch(plane)
      {
        case AXIAL:
          break;
        case CORONAL:
          temporal = index[2];
          index[2] = index[1];
          index[1] = temporal;
          break;
        case SAGITTAL:
          temporal = index[2];
          index[2] = index[1];
          index[1] = index[0];
          index[0] = temporal;
          break;
        default:
          Q_ASSERT(false);
          break;
      }
      Q_ASSERT(volume->GetLargestPossibleRegion().IsInside(index));
      volume->SetPixel(index, value);
    }
    ++init;
  }

  volume->Modified();
  filterOutput.volume = volume;

  markAsEdited(oId);

  emit modified(this);
}

//----------------------------------------------------------------------------
void Filter::draw(OutputId oId,
                  EspinaVolume::Pointer volume)
{
  Output &filterOutput = output(oId);
  EspinaVolume::Pointer filterVolume = filterOutput.volume;

  EspinaVolume::RegionType region = NormalizedRegion(volume);
  filterVolume = expandVolume(filterVolume, region);

  EspinaVolume::RegionType inputRegion  = VolumeRegion(volume,       region);
  EspinaVolume::RegionType outputRegion = VolumeRegion(filterVolume, region);

  itk::ImageRegionIteratorWithIndex<EspinaVolume> it(volume,       inputRegion);
  itk::ImageRegionIteratorWithIndex<EspinaVolume> ot(filterVolume, outputRegion);

  it.GoToBegin();
  ot.GoToBegin();
  for (; !it.IsAtEnd(); ++it, ++ot )
  {
    ot.Set(it.Get());
  }

  filterVolume->Modified();
  filterOutput.volume = filterVolume;

  markAsEdited(oId);

  emit modified(this);
}

//----------------------------------------------------------------------------
void Filter::restoreOutput(OutputId oId, EspinaVolume::Pointer volume)
{
  Output &filterOutput = output(oId);

  filterOutput.volume = volume;
  filterOutput.volume ->Modified();

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
  bool res = false;
  int i = 0;

  while (!res && i < m_outputs.size())
  {
    if (m_outputs[i].id == oId)
      res = true;
    i++;
  }

  return res;

}

//----------------------------------------------------------------------------
Filter::Output Filter::output(OutputId oId) const
{
  Output res;

  foreach(Output filterOutput, m_outputs)
  {
    if (filterOutput.id == oId)
    {
      res = filterOutput;
      break;
    }
  }

  return res;
}

//----------------------------------------------------------------------------
Filter::Output &Filter::output(OutputId oId)
{
  bool found = false;
  int i = 0;

  while (!found && i < m_outputs.size())
  {
    if (m_outputs[i].id == oId)
      found = true;
    else
      i++;
  }

  Q_ASSERT(found);

  return m_outputs[i];
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

  if (!prefetchFilter())
  {
    if (!editedOutputs().isEmpty())
    {
      QMessageBox msg;
      msg.setText(tr("Filter contains segmentations that have been modified by the user."
                     "Updating this filter will result in losing user modifications."
                     "Do you want to proceed?"));
      msg.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
      if (msg.exec() != QMessageBox::Yes)
        return;
    }

    m_inputs.clear();
    m_outputs.clear();//WARNING 2012-11-20 Otra opcion seria quitar el flag de edicion a todas las salidas

    QStringList namedInputList = m_args[INPUTS].split(",", QString::SkipEmptyParts);
    foreach(QString namedInput, namedInputList)
    {
      QStringList input = namedInput.split("_");
      Filter *inputFilter = m_namedInputs[input[0]];
      inputFilter->update();
      m_inputs << inputFilter->output(input[1].toInt()).volume.GetPointer();
    }

    run();
  }
}

//----------------------------------------------------------------------------
bool Filter::prefetchFilter()
{
  if (m_outputs.isEmpty())
    return false;

  //WARNING: Hace falta que todos filtros carguen las outputs al crearse
  // aunque estas puedan ser invalidas NULL volume pointer
  EspinaVolumeReader::Pointer reader;

  for(int i = 0; i < m_outputs.size(); i++)
  {
    Output &output = m_outputs[i];
    QString tmpFile = QString("%1_%2.mhd").arg(tmpId()).arg(output.id);
    reader = tmpFileReader(tmpFile);
    if (reader.IsNull())
      return false;

    output.volume = reader->GetOutput();
    output.volume->DisconnectPipeline();
  }

  emit modified(this);

  return true;
}

//----------------------------------------------------------------------------
Filter::EspinaVolumeReader::Pointer Filter::tmpFileReader(const QString file)
{
  EspinaVolumeReader::Pointer reader;

  if (m_tmpDir.exists(file))
  {
    itk::MetaImageIO::Pointer io = itk::MetaImageIO::New();
    reader = EspinaVolumeReader::New();

    std::string tmpFile = m_tmpDir.absoluteFilePath(file).toStdString();
    io->SetFileName(tmpFile.c_str());
    reader->SetImageIO(io);
    reader->SetFileName(tmpFile);
    reader->Update();
  }

  return reader;
}

//----------------------------------------------------------------------------
EspinaVolume::Pointer Filter::expandVolume(EspinaVolume::Pointer volume,
                                           EspinaVolume::RegionType region)
{
  EspinaVolume::Pointer res = volume;

  if (volume->GetLargestPossibleRegion() != volume->GetBufferedRegion())
  {
    volume->SetBufferedRegion(volume->GetLargestPossibleRegion());
    volume->Update();
  }

  EspinaVolume::RegionType imageRegion = volume->GetLargestPossibleRegion();
  EspinaVolume::RegionType normRegion = NormalizedRegion(volume);
  if (!normRegion.IsInside(region))
  {
    //qDebug() << "Resize Image";
    EspinaVolume::RegionType br = BoundingBoxRegion(normRegion, region);
    EspinaVolume::Pointer expandedImage = EspinaVolume::New();
    expandedImage->SetRegions(br);
    expandedImage->SetSpacing(volume->GetSpacing());
    expandedImage->Allocate();
    // Do a block copy for the overlapping region.
    itk::ImageAlgorithm::Copy(volume.GetPointer(), expandedImage.GetPointer(), imageRegion, normRegion);
    itk::ImageRegionExclusionIteratorWithIndex<EspinaVolume> outIter(expandedImage.GetPointer(), br);
    outIter.SetExclusionRegion(normRegion);
    outIter.GoToBegin();
    while ( !outIter.IsAtEnd() )
    {
      outIter.Set(0);
      ++outIter;
    }
    res = expandedImage;
  }
  return res;
}

//----------------------------------------------------------------------------
void Filter::markAsEdited(OutputId oId)
{
  output(oId).isEdited = true;

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
    Filter *filter = m_namedInputs[input[0]];
    OutputId oId = input[1].toInt();

    if (filter->validOutput(oId))
    {
      Output &output = filter->output(oId);
      output.isCached = output.isCached || output.isEdited;
    }
  }
}