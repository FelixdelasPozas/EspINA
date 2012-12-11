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


#include "SegmhaImporterFilter.h"

// EspINA
#include <common/model/Segmentation.h>

// Qt
#include <QApplication>
#include <QDebug>
#include <QFileDialog>

// VTK
#include <vtkImageReslice.h>
#include <vtkImageChangeInformation.h>

//---------------------------------------------------------------------------
const ModelItem::ArgumentId SegmhaImporterFilter::FILE = "File";
const ModelItem::ArgumentId SegmhaImporterFilter::BLOCKS = "Blocks";

//---------------------------------------------------------------------------
SegmhaImporterFilter::SegmentationObject::SegmentationObject(const QString& line)
{
  QStringList elements = line.split(" ");

  label      = elements[1].split("=")[1].toUInt();
  taxonomyId = elements[2].split("=")[1].toUInt();
  selected   = elements[3].split("=")[1].toUInt();
}

//---------------------------------------------------------------------------
SegmhaImporterFilter::TaxonomyObject::TaxonomyObject(const QString& line)
{
  QStringList elements = line.split(" ");
  QStringList rgb;

  int nameIdx  = line.indexOf("name=")+6;
  int valueIdx = line.indexOf("value=")+6;
  int colorIdx = line.indexOf("color=")+6;

  name  = line.mid(nameIdx,valueIdx-2-nameIdx-6);
  label = line.mid(valueIdx, colorIdx-valueIdx-6).toInt();
  rgb = line.mid(colorIdx).split(",");
  int r = rgb[0].replace(',',"").toInt();
  int g = rgb[1].replace(',',"").toInt();
  int b = rgb[2].toInt();
  color = QColor(r, g, b);
}

//---------------------------------------------------------------------------
const QString SegmhaImporterFilter::TYPE = "Segmha Importer";
const QString SegmhaImporterFilter::SUPPORTED_FILES = tr("Segmentation LabelMaps (*.segmha)");

//-----------------------------------------------------------------------------
SegmhaImporterFilter::SegmhaImporterFilter(Filter::NamedInputs inputs,
					   ModelItem::Arguments args)
: Filter       (inputs, args)
, m_needUpdate (false)
, m_param      (m_args)
, m_taxonomy   (new Taxonomy())
{

}


//-----------------------------------------------------------------------------
SegmhaImporterFilter::~SegmhaImporterFilter()
{
}

//-----------------------------------------------------------------------------
QVariant SegmhaImporterFilter::data(int role) const
{
  if (role == Qt::DisplayRole)
    return TYPE;
  else
    return QVariant();
}

//-----------------------------------------------------------------------------
QString SegmhaImporterFilter::serialize() const
{
  QStringList blockList;
  foreach(OutputNumber i, m_outputs.keys())
    blockList << QString::number(i);

  m_args[BLOCKS] = blockList.join(",");
  return Filter::serialize();
}


//-----------------------------------------------------------------------------
void SegmhaImporterFilter::markAsModified()
{
  Filter::markAsModified();
}

//-----------------------------------------------------------------------------
bool SegmhaImporterFilter::needUpdate() const
{
  return m_sources.isEmpty() || m_needUpdate;
}

//-----------------------------------------------------------------------------
void SegmhaImporterFilter::run()
{
  QFileInfo file = m_args[FILE];
  if (!file.exists())
  {
    QFileDialog fileDialog;
    fileDialog.setObjectName("SelectSegmhaFile");
    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    fileDialog.setWindowTitle(QString("Select file for %1:").arg(file.fileName()));
    fileDialog.setFilter(SUPPORTED_FILES);

    if (fileDialog.exec() != QDialog::Accepted)
      return;

    m_args[FILE] = fileDialog.selectedFiles().first();
  }

  m_lmapReader = LabelMapReader::New();

  qDebug() << "Reading segmentation's meta data from file:";
  QList<SegmentationObject> metaData;
  QList<TaxonomyElement *> taxonomies;

  QFile metaDataReader(m_args[FILE]);
  metaDataReader.open(QIODevice::ReadOnly);
  QTextStream stream(&metaDataReader);

  QString line;
  while (!(line = stream.readLine()).isNull())
  {
    QString infoType = line.split(":")[0];

    if (infoType == "Object")
    {
      SegmentationObject seg(line);
      metaData << seg;
    }
    else if (infoType == "Segment")
    {
      TaxonomyObject tax(line);
      TaxonomyElement *taxonomy = m_taxonomy->addElement(tax.name);
      //QColor color(tax.color[0], tax.color[1], tax.color[1]);
      taxonomy->setColor(tax.color);
      taxonomies << taxonomy;
    }
    else if (infoType == "Counting Brick")
    {
      QStringList margins = line.split('=');
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
  int numSegmentations = metaData.size();
  //this->SetSegTaxonomies(segTaxonomies.toUtf8());
  std::cout << "  Total Number of Segmentations: " << numSegmentations << std::endl;
  //this->SetTaxonomy(taxonomies.toUtf8());
////   std::cout << "Total Number of Taxonomies: " << taxonomies.split(";").size() << std::endl;

  qDebug() << "Reading ITK image from file";
  // Read the original image, whose pixels are indeed labelmap object ids
  m_lmapReader->SetFileName(m_args[FILE].toUtf8().constData());
  m_lmapReader->SetImageIO(itk::MetaImageIO::New());
  m_lmapReader->Update();

  qDebug() << "Invert ITK image's slices";
  // EspINA python used an inversed representation of the samples
  ImageToVTKImageFilterType::Pointer originalImage =
    ImageToVTKImageFilterType::New();
  originalImage->SetInput(m_lmapReader->GetOutput());
  originalImage->Update();

  vtkSmartPointer<vtkImageReslice> reslicer =
    vtkSmartPointer<vtkImageReslice>::New();
  reslicer->SetInput(originalImage->GetOutput());
  reslicer->SetResliceAxesDirectionCosines(1,0,0,0,-1,0,0,0,-1);
  reslicer->Update();

  vtkSmartPointer<vtkImageChangeInformation> infoChanger =
    vtkSmartPointer<vtkImageChangeInformation>::New();
  infoChanger->SetInput(reslicer->GetOutput());
  infoChanger->SetInformationInput(originalImage->GetOutput());
  infoChanger->Update();

  VTKImageToImageFilterType::Pointer vtk2itk_filter =
    VTKImageToImageFilterType::New();
  vtk2itk_filter->SetInput(infoChanger->GetOutput());
  vtk2itk_filter->Update();

  qDebug() << "Converting from ITK to LabelMap";
  // Convert labeled image to label map
  Image2LabelFilterType::Pointer image2label =
    Image2LabelFilterType::New();
  image2label->SetInput(vtk2itk_filter->GetOutput());
  image2label->Update();

  LabelMapType *labelMap = image2label->GetOutput();
  qDebug() << "Number of Label Objects" << labelMap->GetNumberOfLabelObjects();

  LabelObjectType * object;
  OutputNumber id = 0;
  foreach(SegmentationObject seg, metaData)
  {
    try
    {
      //qDebug() << "Loading Segmentation " << seg.label;
      Source source;

      object = labelMap->GetLabelObject(seg.label);
      LabelObjectType::RegionType region = object->GetBoundingBox();

      source.labelMap = LabelMapType::New();
      source.labelMap->SetSpacing(labelMap->GetSpacing());
      source.labelMap->SetRegions(region);
      source.labelMap->Allocate();
      object->SetLabel(SEG_VOXEL_VALUE);
      source.labelMap->AddLabelObject(object);
      source.labelMap->Update();

      Label2ImageFilterType::Pointer image = Label2ImageFilterType::New();
      image->SetInput(source.labelMap);
      image->Update();

      source.image = image;

      m_outputs[id++] = image->GetOutput();
      m_sources << source;
      m_taxonomies << taxonomies[seg.taxonomyId-1];
      m_labels << seg.label;
    } catch (...)
    {
      std::cerr << "Couldn't import segmentation " << seg.label << std::endl;
    }
  }
}

//-----------------------------------------------------------------------------
bool SegmhaImporterFilter::prefetchFilter()
{
  QStringList blockList = m_args[BLOCKS].split(",");

  foreach(QString block, blockList)
  {
    QString tmpFile = id() + "_" + block + ".mhd";
    Source source;
    source.image = tmpFileReader(tmpFile);

    if (source.image.IsNull())
      return false;

    m_outputs[block.toInt()] = source.image->GetOutput();
    m_sources << source;
  }

  emit modified(this);
  return true;
}

//-----------------------------------------------------------------------------
TaxonomyElement* SegmhaImporterFilter::taxonomy(OutputNumber i)
{
  return m_taxonomies.value(i, NULL);
}

//-----------------------------------------------------------------------------
void SegmhaImporterFilter::initSegmentation(Segmentation* seg, int segId)
{
  seg->setTaxonomy(taxonomy(segId));
  seg->setNumber(m_labels.value(segId,-1));
}
