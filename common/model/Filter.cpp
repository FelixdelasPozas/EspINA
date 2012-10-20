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

#include "EspinaRegions.h"

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

#include <QDir>
#include <QMessageBox>
#include <QWidget>

typedef ModelItem::ArgumentId ArgumentId;

const ArgumentId Filter::ID     = "ID";
const ArgumentId Filter::INPUTS = "Inputs";
const ArgumentId Filter::EDIT   = "Edit";

unsigned int Filter::m_lastId = 0;

//----------------------------------------------------------------------------
void Filter::resetId()
{
  m_lastId = 0;
}


//----------------------------------------------------------------------------
QString Filter::generateId()
{
  return QString::number(m_lastId++);
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
bool Filter::isEdited() const
{
  return m_args.contains(EDIT);
}

//----------------------------------------------------------------------------
QList<OutputNumber> Filter::editedOutputs() const
{
  QList<OutputNumber> res;
  QStringList values = m_args.value(EDIT, QString()).split(",");
  foreach (QString value, values)
    res << value.toInt();

  return res;
}


//----------------------------------------------------------------------------
int Filter::numberOutputs() const
{
  return m_outputs.size();
}

//----------------------------------------------------------------------------
EspinaVolume* Filter::output(OutputNumber i) const
{
  Q_ASSERT(m_outputs.contains(i));
  return m_outputs.value(i, NULL);
}

//----------------------------------------------------------------------------
void Filter::update()
{
  if (!needUpdate())
    return;

  if (!prefetchFilter())
  {
    if (m_args.contains(EDIT))
    {
      QMessageBox msg;
      msg.setText("Current Segmentation has been modified by the user."
                   "Changes will be lost if you decide to recompute it."
                   "Do you want to proceed?");
      msg.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
      if (msg.exec() != QMessageBox::Yes)
	return;
      m_args.remove(EDIT);
    }
    m_inputs.clear();
    QStringList namedInputList = m_args[INPUTS].split(",", QString::SkipEmptyParts);
    foreach(QString namedInput, namedInputList)
    {
      QStringList input = namedInput.split("_");
      Filter *inputFilter = m_namedInputs[input[0]];
      inputFilter->update();
      m_inputs << inputFilter->output(input[1].toUInt());
    }
    run();
  }
}

//----------------------------------------------------------------------------
bool Filter::prefetchFilter()
{
  QString tmpFile = id() + "_0.mhd";
  m_cachedFilter = tmpFileReader(tmpFile);

  if (m_cachedFilter.IsNotNull())
  {
    m_outputs[0] = m_cachedFilter->GetOutput();
    emit modified(this);
    return true;
  }

  return false;
}

//----------------------------------------------------------------------------
Filter::EspinaVolumeReader::Pointer Filter::tmpFileReader(const QString file)
{
  if (m_tmpDir.exists(file))
  {
    itk::MetaImageIO::Pointer io = itk::MetaImageIO::New();
    EspinaVolumeReader::Pointer reader = EspinaVolumeReader::New();

    std::string tmpFile = m_tmpDir.absoluteFilePath(file).toStdString();
    io->SetFileName(tmpFile.c_str());
    reader->SetImageIO(io);
    reader->SetFileName(tmpFile);
    reader->Update();

    return reader;
  }
  return NULL;
}

//TODO 2012-10-19 Try to use threaded method similar to tubular source
//----------------------------------------------------------------------------
void Filter::draw(OutputNumber i,
                  QList<vtkImplicitFunction *> brushes,
                  double bounds[6],
                  EspinaVolume::PixelType value)
{
  EspinaVolume::SpacingType spacing = m_outputs[i]->GetSpacing();
  EspinaVolume::RegionType region = BoundsToRegion(bounds, spacing);
  m_outputs[i] = addRegionToVolume(m_outputs[i], region);

  EspinaVolume::RegionType outputRegion = VolumeRegion(m_outputs[i], region);
  itk::ImageRegionIteratorWithIndex<EspinaVolume> it(m_outputs[i], outputRegion);
  it.GoToBegin();
  for (; !it.IsAtEnd(); ++it )
  {
    double tx = it.GetIndex()[0]*spacing[0] + m_outputs[i]->GetOrigin()[0];
    double ty = it.GetIndex()[1]*spacing[1] + m_outputs[i]->GetOrigin()[1];
    double tz = it.GetIndex()[2]*spacing[2] + m_outputs[i]->GetOrigin()[2];

    for (int i=0; i < brushes.size(); i++)
    {
      if (brushes.value(i)->FunctionValue(tx, ty, tz) <= 0)
      {
        it.Set(value);
        continue;
      }
    }
  }
  m_outputs[i]->Modified();
  if (!m_editedOutputs.contains(QString::number(i)))
    m_editedOutputs << QString::number(i);

  m_args[EDIT] = m_editedOutputs.join(",");
  emit modified(this);
}

//----------------------------------------------------------------------------
void Filter::draw(OutputNumber i,
                  EspinaVolume::IndexType index,
                  EspinaVolume::PixelType value)
{
  EspinaVolume::RegionType region;
  region.SetIndex(index);
  EspinaVolume::SizeType size;
  size.Fill(1);
  region.SetSize(size);
  m_outputs[i] = addRegionToVolume(m_outputs[i], region);

  EspinaVolume *image = m_outputs[i];
  if (image && image->GetLargestPossibleRegion().IsInside(index))
  {
    image->SetPixel(index, value);
    image->Modified();
    m_args[EDIT] = "Yes";
  }
}

//----------------------------------------------------------------------------
void Filter::draw(OutputNumber i,
		  Nm x, Nm y, Nm z,
		  EspinaVolume::PixelType value)
{
  EspinaVolume *image = output(i);
  if (image)
  {
    EspinaVolume::SpacingType spacing = image->GetSpacing();
    EspinaVolume::IndexType index;
    index[0] = x/spacing[0]+0.5;
    index[1] = y/spacing[1]+0.5;
    index[2] = z/spacing[2]+0.5;
    draw(i, index, value);
  }
}

//----------------------------------------------------------------------------
void Filter::draw(OutputNumber i, vtkPolyData *contour, Nm slice, PlaneType plane,
      EspinaVolume::PixelType value)
{
  int extent[6];
  VolumeExtent(m_outputs[i], extent);
  EspinaVolume::SpacingType spacing = m_outputs[i]->GetSpacing();

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
      Q_ASSERT(m_outputs[i]->GetLargestPossibleRegion().IsInside(index));
      m_outputs[i]->SetPixel(index, value);
    }
    ++init;
  }

  m_outputs[i]->Modified();

  if (!m_editedOutputs.contains(QString::number(i)))
    m_editedOutputs << QString::number(i);

  m_args[EDIT] = m_editedOutputs.join(",");
}

//----------------------------------------------------------------------------
EspinaVolume::Pointer Filter::addRegionToVolume(EspinaVolume::Pointer volume,
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
//     qDebug() << "Resize Image";
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
QWidget* Filter::createFilterInspector(QUndoStack* undoStack, ViewManager* vm)
{
  return new QWidget();
}
