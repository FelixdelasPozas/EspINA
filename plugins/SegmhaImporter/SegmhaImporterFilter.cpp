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
#include <common/EspinaCore.h>
#include <common/model/EspinaFactory.h>

// ITK

// Qt
#include <QApplication>
#include <QDebug>
#include <QFileDialog>

// VTK
#include <vtkImageReslice.h>
#include <vtkImageChangeInformation.h>


const ModelItem::ArgumentId SegmhaImporterFilter::FILE = ModelItem::ArgumentId("File", true);
const ModelItem::ArgumentId SegmhaImporterFilter::BLOCKS = ModelItem::ArgumentId("Blocks", true);

SegmhaImporterFilter::SegmentationObject::SegmentationObject(const QString& line)
{
  QStringList elements = line.split(" ");

  label      = elements[1].split("=")[1].toUInt();
  taxonomyId = elements[2].split("=")[1].toUInt();
  selected   = elements[3].split("=")[1].toUInt();
}

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

//   ModelItemExtension *crExtension = sample->extension("CountingRegionExtension");
//   if (crExtension)
//   {
//     int margins[6];
//     vtkSMPropertyHelper(readerProxy, "CountingBrick").Get(margins,6);
// 
//     // NOTE: Counting Region margin's order
//     QString rcb = QString("RectangularRegion=%1,%2,%3,%4,%5,%6;")
//       .arg(margins[0]).arg(margins[3])
//       .arg(margins[1]).arg(margins[4])
//       .arg(margins[2]).arg(margins[5]);
//     qDebug() << "Using Counting Region" << rcb;
//     ModelItem::Arguments args;
//     args[ArgumentId("REGION", ArgumentId::KEY)] = rcb;
//     crExtension->initialize(args);
//   }
// 
//   vtkSMProperty *p;
//   // Load Taxonomy
//   p = readerProxy->GetProperty("Taxonomy");
//   vtkSMStringVectorProperty* TaxProp = vtkSMStringVectorProperty::SafeDownCast(p);
//   QString TaxonomySerialization(TaxProp->GetElement(0));
// 
//   QStringList taxonomies = TaxonomySerialization.split(";");
// 
//   Taxonomy *tax = new Taxonomy();
//   QStringList availableTaxonomies;
//   foreach(QString taxonomy, taxonomies)
//   {
//     if (taxonomy == "")
//       continue;
// 
//     QStringList values = taxonomy.split(",");
//     QChar zero = '0';
//     QString color = QString("#%1%2%3")
//     .arg(values[2].toInt(),2,16,zero)
//     .arg(values[3].toInt(),2,16,zero)
//     .arg(values[4].toInt(),2,16,zero);
// 
//     TaxonomyNode *node = tax->addElement(values[1]);
//     node->setColor(QColor(color));
//     availableTaxonomies.append(values[1]);
//   }
// 
//   EspinaCore::instance()->model()->setTaxonomy(tax);
//   tax->print();
// 
//   int numSegs;
//   vtkSMPropertyHelper(readerProxy, "NumSegmentations").Get(&numSegs,1);
// 
//   // Create segmentation's taxonomy list
//   p = readerProxy->GetProperty("SegTaxonomies");
//   vtkSMStringVectorProperty* SegTaxProp = vtkSMStringVectorProperty::SafeDownCast(p);
//   QString segTaxonomiesProp(SegTaxProp->GetElement(0));
// 
//   QStringList segTaxonomies = segTaxonomiesProp.split(";");
//   QString readerId = File::extendedName(file) + ":0";
// 
//   for (int p=0; p < numSegs; p++)
//   {
//     // Extract Seg Filter
//     pqFilter::Arguments extractArgs;
//     extractArgs << pqFilter::Argument(QString("Input"),pqFilter::Argument::INPUT, readerId);
//     extractArgs << pqFilter::Argument(QString("Block"),pqFilter::Argument::INTVECT, QString::number(p));
//     pqFilter *segImage = cob->createFilter("filters","ExtractBlockAsImage",extractArgs);
//     segImage->pipelineSource()->updatePipeline();
//     vtkSMProxy *proxy = segImage->pipelineSource()->getProxy();
//     proxy->UpdatePropertyInformation();
// 
//     Segmentation *seg = EspinaFactory::instance()->createSegmentation(this, p, segImage->data(0));
//     QString qualifiedName = availableTaxonomies[segTaxonomies[p].toInt()-1];
//     TaxonomyNode *node = tax->element(qualifiedName);
//     Q_ASSERT(node);
//     std::cout << "Getting taxonomy "<< segTaxonomies[p].toStdString() << ": " << node->qualifiedName().toStdString() << std::endl;
//     seg->setTaxonomy(node);
//     seg->setNumber(vtkSMPropertyHelper(proxy,"Label").GetAsInt());
//     qDebug() << "Loading Segmentation" << seg->id() << "Taxonomy: " << seg->taxonomy()->name();
// 
//     m_blocks[QString::number(p)] = seg;
//   }
// 
//   m_args.setBlocks(m_blocks.keys());
// }
/*
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();

  pqFilter *segFilter;

  QStringList blocks = m_args.blocks();
  foreach(QString block, m_args.blocks())
  {
    QString segId = id() + "_" + block;
    segFilter = cob->loadFile(segId);
    if (segFilter == NULL)
      continue;

    Segmentation *seg = EspinaFactory::instance()->createSegmentation(this, block.toInt(), segFilter->data(0));
    m_blocks[block] = seg;
  }

//   ModelItem::Vector channels = 
}*/


//-----------------------------------------------------------------------------
SegmhaImporterFilter::~SegmhaImporterFilter()
{
}

//-----------------------------------------------------------------------------
void SegmhaImporterFilter::markAsModified()
{
  Filter::markAsModified();
}

//-----------------------------------------------------------------------------
bool SegmhaImporterFilter::needUpdate() const
{
  return m_lmapReader.IsNull() || m_needUpdate;
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
  QList<TaxonomyNode *> taxonomies;

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
      TaxonomyNode *taxonomy = m_taxonomy->addElement(tax.name);
      //QColor color(tax.color[0], tax.color[1], tax.color[1]);
      taxonomy->setColor(tax.color);
      taxonomies << taxonomy;
    }
    else if (infoType == "Counting Brick")
    {
//       TODO: Create counting region
//       int cb[6];
//       parseCountingBrick(line,cb);
//       this->SetCountingBrick(cb);
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
  m_lmapReader->SetFileName(m_args[FILE].toStdString());
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
  image2label->Update();//TODO: Check if needed

  LabelMapType *labelMap = image2label->GetOutput();
  qDebug() << "Number of Label Objects" << labelMap->GetNumberOfLabelObjects();

  LabelObjectType * object;
  foreach(SegmentationObject seg, metaData)
  {
    try
    {
      //qDebug() << "Loading Segmentation " << seg.label;
      Output output;

      object = labelMap->GetLabelObject(seg.label);
      LabelObjectType::RegionType region = object->GetBoundingBox();

      output.labelMap = LabelMapType::New();
      output.labelMap->SetSpacing(labelMap->GetSpacing());
      output.labelMap->SetRegions(region);
      output.labelMap->Allocate();
      object->SetLabel(SEG_VOXEL_VALUE);
      output.labelMap->AddLabelObject(object);
      output.labelMap->Update();

      output.image = Label2ImageFilterType::New();
      output.image->SetInput(output.labelMap);
      output.image->Update();

      m_volumes << output;
      m_taxonomies << taxonomies[seg.taxonomyId-1];
    } catch (...)
    {
      std::cerr << "Couldn't import segmentation " << seg.label << std::endl;
    }
  }
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
int SegmhaImporterFilter::numberOutputs() const
{
  return m_volumes.size();
}

//-----------------------------------------------------------------------------
EspinaVolume* SegmhaImporterFilter::output(OutputNumber i) const
{
  if (i < m_volumes.size())
    return m_volumes.value(i).image->GetOutput();
  else
    return NULL;
}

//-----------------------------------------------------------------------------
bool SegmhaImporterFilter::prefetchFilter()
{
  return Filter::prefetchFilter();
}

//-----------------------------------------------------------------------------
TaxonomyNode* SegmhaImporterFilter::taxonomy(OutputNumber i)
{
  return m_taxonomies.value(i, NULL);
}
