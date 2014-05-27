/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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


#include "SegmhaReader.h"

#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.hxx>
#include <Core/Analysis/Data/Volumetric/SparseVolume.h>
#include <Core/Analysis/Data/VolumetricDataUtils.h>
#include <Core/Analysis/Sample.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Analysis/Channel.h>
#include <Core/Factory/CoreFactory.h>
#include <Filters/SourceFilter.h>
#include <Support/Readers/ChannelReader.h>

// EspINA

// Qt
#include <QApplication>
#include <QDebug>
#include <QFileDialog>

// VTK
#include <vtkImageReslice.h>
#include <vtkImageChangeInformation.h>

// ITK
#include <itkImageFileReader.h>
#include <itkImageFileReader.h>
#include <itkImageToVTKImageFilter.h>
#include <itkLabelImageToShapeLabelMapFilter.h>
#include <itkMetaImageIO.h>
#include <itkShapeLabelObject.h>
#include <itkVTKImageToImageFilter.h>

using namespace EspINA;

using SegmentationLabelMap  = itk::Image<unsigned short , 3>;
using LabelMapReader        = itk::ImageFileReader<SegmentationLabelMap>;
using ImageToVTKImageFilter = itk::ImageToVTKImageFilter<SegmentationLabelMap>;
using VTKImageToImageFilter = itk::VTKImageToImageFilter<SegmentationLabelMap>;
using LabelMapObject        = itk::ShapeLabelObject<unsigned int, 3>;
using LabelMap              = itk::LabelMap<LabelMapObject>;
using Image2LabelFilter     = itk::LabelImageToShapeLabelMapFilter<SegmentationLabelMap, LabelMap>;
using Label2VolumeFilter    = itk::LabelMapToLabelImageFilter<LabelMap, itkVolumeType>;

//---------------------------------------------------------------------------
IO::AnalysisReader::ExtensionList SegmhaReader::supportedFileExtensions() const
{
  ExtensionList supportedExtensions;

  Extensions extensions;
  extensions << "segmha";

  supportedExtensions["Segmentation Labelmaps"] = extensions;

  return supportedExtensions;
}

//---------------------------------------------------------------------------
AnalysisSPtr SegmhaReader::read(const QFileInfo& file, CoreFactorySPtr factory, ErrorHandlerSPtr handler)
{
  ClassificationSPtr classification{new Classification()};

  QFileInfo localFile = file;

  if (!localFile.exists())
  {
    if (handler)
    {
      localFile = handler->fileNotFound(file);
    }

    if (!localFile.exists()) return AnalysisSPtr();
  }

  QFileInfo channelFile = localFile.absoluteFilePath().replace(".segmha", ".mhd");
  ChannelReader channelReader;

  auto analysis = channelReader.read(channelFile, factory, handler);

  LabelMapReader::Pointer labelMapReader = LabelMapReader::New();

  qDebug() << "Reading segmentation's meta data from file:";
  QList<SegmentationObject> segmentationObjects;
  CategorySList categories;

  QFile metaDataReader(localFile.absoluteFilePath());
  metaDataReader.open(QIODevice::ReadOnly);

  QTextStream stream(&metaDataReader);

  QString line;
  while (!(line = stream.readLine()).isNull())
  {
    QString infoType = line.split(":")[0];

    if (infoType == "Object")
    {
      SegmentationObject seg(line);
      segmentationObjects << seg;
    }
    else if (infoType == "Segment")
    {
      CategoryObject info(line);
      CategorySPtr category = classification->createNode(info.name);
      category->setColor(info.color);
      categories << category;
    }
    else if (infoType == "Counting Brick")
    {
      QStringList margins   = line.split('=');
      QStringList inclusive = margins[1].split(',');
      QStringList exclusive = margins[2].split(',');

      m_inclusive[0] = inclusive[0].section('[',-1).toInt();
      m_inclusive[1] = inclusive[1].toInt();
      m_inclusive[2] = inclusive[2].section(']',0,0).toInt();

      m_exclusive[0] = exclusive[0].section('[',-1).toInt();
      m_exclusive[1] = exclusive[1].toInt();
      m_exclusive[2] = exclusive[2].section(']',0,0).toInt();
    }
  }

  metaDataReader.close();

  analysis->setClassification(classification);

  int numSegmentations = segmentationObjects.size();
  //this->SetSegCategories(segTaxonomies.toUtf8());
  std::cout << "  Total Number of Segmentations: " << numSegmentations << std::endl;
////   std::cout << "Total Number of Categories: " << taxonomies.split(";").size() << std::endl;

  //qDebug() << "Reading ITK image from file";
  // Read the original image, whose pixels are indeed labelmap object ids
  labelMapReader->SetFileName(localFile.absoluteFilePath().toUtf8().data());
  labelMapReader->SetImageIO(itk::MetaImageIO::New());
  labelMapReader->Update();

  //qDebug() << "Invert ITK image's slices";
  // EspINA python used an inversed representation of the samples
  auto originalImage = ImageToVTKImageFilter::New();
  originalImage->SetInput(labelMapReader->GetOutput());
  originalImage->Update();

  auto reslicer = vtkSmartPointer<vtkImageReslice>::New();
  reslicer->SetInputData(originalImage->GetOutput());
  reslicer->SetResliceAxesDirectionCosines(1,0,0,0,-1,0,0,0,-1);
  reslicer->Update();

  auto infoChanger = vtkSmartPointer<vtkImageChangeInformation>::New();
  infoChanger->SetInputData(reslicer->GetOutput());
  infoChanger->SetInformationInputData(originalImage->GetOutput());
  infoChanger->Update();

  auto vtk2itk_filter = VTKImageToImageFilter::New();
  vtk2itk_filter->SetInput(infoChanger->GetOutput());
  vtk2itk_filter->Update();

  qDebug() << "Converting from ITK to LabelMap";
  // Convert labeled image to label map
  auto image2label = Image2LabelFilter::New();
  image2label->SetInput(vtk2itk_filter->GetOutput());
  image2label->Update();

  auto labelMap = image2label->GetOutput();
  qDebug() << "Number of Label Objects" << labelMap->GetNumberOfLabelObjects();

  auto sourceFilter = factory->createFilter<SourceFilter>(InputSList(), "SegmhaReader");

  auto sample  = analysis->samples().first();
  auto channel = analysis->channels().first();

  auto spacing = ItkSpacing<itkVolumeType>(channel->output()->spacing());

  Output::Id id = 0;

  LabelMapObject*   object;
  SegmentationSList segmentations;

  for(auto segmentationObject : segmentationObjects)
  {
    try
    {
      //qDebug() << "Loading Segmentation " << seg.label;
      object      = labelMap->GetLabelObject(segmentationObject.label);
      auto region = object->GetBoundingBox();

      auto segLabelMap = LabelMap::New();
      segLabelMap->SetSpacing(spacing);
      segLabelMap->SetRegions(region);
      segLabelMap->Allocate();

      object->SetLabel(SEG_VOXEL_VALUE);

      segLabelMap->AddLabelObject(object);
      segLabelMap->Update();

      auto label2volume = Label2VolumeFilter::New();
      label2volume->SetInput(segLabelMap);
      label2volume->Update();

      auto volume = label2volume->GetOutput();

      OutputSPtr output{new Output(sourceFilter.get(), id)};

      Bounds    bounds  = equivalentBounds<itkVolumeType>(volume, volume->GetLargestPossibleRegion());
      NmVector3 spacing = ToNmVector3<itkVolumeType>(volume->GetSpacing());

      DefaultVolumetricDataSPtr volumetricData{new SparseVolume<itkVolumeType>(bounds, spacing)};
      volumetricData->draw(volume);

      MeshDataSPtr meshData{new MarchingCubesMesh<itkVolumeType>(volumetricData)};

      output->setData(volumetricData);
      output->setData(meshData);
      output->setSpacing(spacing);

      sourceFilter->addOutput(id, output);

      auto category     = categories[segmentationObject.categoryId - 1];
      auto segmentation = factory->createSegmentation(sourceFilter, id);

      segmentation->setCategory(category);
      segmentation->setNumber(segmentationObject.label);

      analysis->add(segmentation);
      analysis->addRelation(sample, segmentation, Sample::CONTAINS);

      segmentations << segmentation;

      id++;
    } catch (...)
    {
      std::cerr << "Couldn't import segmentation " << segmentationObject.label << std::endl;
    }
  }

  // TODO 2014-05-27 Restore CF if available



  return analysis;
}

//---------------------------------------------------------------------------
SegmhaReader::SegmentationObject::SegmentationObject(const QString& line)
{
  QStringList elements = line.split(" ");

  label      = elements[1].split("=")[1].toUInt();
  categoryId = elements[2].split("=")[1].toUInt();
  selected   = elements[3].split("=")[1].toUInt();
}

//---------------------------------------------------------------------------
SegmhaReader::CategoryObject::CategoryObject(const QString& line)
{
  QStringList elements = line.split(" ");
  QStringList rgb;

  int nameIdx  = line.indexOf("name=")+6;
  int valueIdx = line.indexOf("value=")+6;
  int colorIdx = line.indexOf("color=")+6;

  name  = line.mid(nameIdx,valueIdx-2-nameIdx-6);
  label = line.mid(valueIdx, colorIdx-valueIdx-6).toInt();
  rgb   = line.mid(colorIdx).split(",");

  int r = rgb[0].replace(',',"").toInt();
  int g = rgb[1].replace(',',"").toInt();
  int b = rgb[2].toInt();

  color = QColor(r, g, b);
}

// //-----------------------------------------------------------------------------
// QString SegmhaImporterReader::serialize() const
// {
//   QStringList blockList;
//   foreach(FilterOutputId outputId, m_outputs.keys())
//     blockList << QString::number(outputId);
//
//   m_args[BLOCKS] = blockList.join(",");
//   return Filter::serialize();
// }
//
//
// //-----------------------------------------------------------------------------
// TaxonomyElementSPtr SegmhaImporterReader::taxonomy(FilterOutputId i)
// {
//   return m_taxonomies.value(i, TaxonomyElementSPtr());
// }
//
// //-----------------------------------------------------------------------------
// void SegmhaImporterReader::initSegmentation(SegmentationSPtr seg, FilterOutputId i)
// {
//   seg->setTaxonomy(taxonomy(i));
//
//   seg->setNumber(m_labels.value(i,-1));
// }