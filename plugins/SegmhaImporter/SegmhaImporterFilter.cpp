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

#include <common/cache/CachedObjectBuilder.h>
#include <common/EspinaCore.h>
#include <pqServer.h>
#include <pqActiveObjects.h>
#include <pqFileDialog.h>
#include <pqCoreUtilities.h>
#include <QApplication>

#include <QDebug>
#include <pqPipelineSource.h>
#include <vtkSMProxy.h>
#include <vtkSMStringVectorProperty.h>
#include <vtkSMPropertyHelper.h>
#include <common/File.h>
#include <common/model/EspinaFactory.h>

// #include <plugins/CountingRegion/extensions/CountingRegionSampleExtension.h>

const QString SegmhaImporterFilter::FILE = "File";
const QString SegmhaImporterFilter::BLOCKS = "Blocks";

// SegmhaImporterFilter::SetupWidget::SetupWidget(EspinaFilter *parent)
// : QWidget()
// {
//   setupUi(this);
//   SegmhaImporterFilter *filter = dynamic_cast<SegmhaImporterFilter *>(parent);
//   m_xSeed->setText(QString("%1").arg(filter->m_seed[0]));
//   m_ySeed->setText(QString("%1").arg(filter->m_seed[1]));
//   m_zSeed->setText(QString("%1").arg(filter->m_seed[2]));
//   m_threshold->setValue(filter->m_threshold);
// }


//-----------------------------------------------------------------------------
SegmhaImporterFilter::SegmhaImporterFilter(const QString file)
: m_segReader(NULL)
, m_channel  (NULL)
{
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();

  bool isRefChannelLoaded = false;
  if (false == isRefChannelLoaded)
  {
    pqServer* server = pqActiveObjects::instance().activeServer();
//     vtkSMReaderFactory* readerFactory =
//       vtkSMProxyManager::GetProxyManager()->GetReaderFactory();

      QString filters = "MetaImage File (*.mha, *.mhd)";
      if (!filters.isEmpty())
      {
	filters += ";;";
      }
      filters += "All files (*)";

      pqFileDialog fileDialog(server, pqCoreUtilities::mainWidget(),
			      QObject::tr("Open Channel File:"), QString(), filters);
      fileDialog.setObjectName("ChannelOpenDialog");
      fileDialog.setFileMode(pqFileDialog::ExistingFiles);


      QApplication::setOverrideCursor(Qt::ArrowCursor);
      if (fileDialog.exec() == QDialog::Rejected)
      {
	QApplication::restoreOverrideCursor();
	return;
      }
      QApplication::restoreOverrideCursor();

      File channelFile(fileDialog.getSelectedFiles()[0]);
      EspinaCore::instance()->loadFile(channelFile);
      m_channel = EspinaCore::instance()->model()->channel(channelFile.name());
  }

  m_args[FILE] = file;
  m_segReader = cob->loadFile(file);
  Q_ASSERT(m_segReader->getNumberOfData() > 0);
  m_segReader->pipelineSource()->updatePipeline();

  vtkSMProxy *readerProxy = m_segReader->pipelineSource()->getProxy();
  readerProxy->UpdatePropertyInformation();

  ModelItem::Vector samples = m_channel->relatedItems(IN, "mark");
  Q_ASSERT(!samples.isEmpty());
  Sample *sample = dynamic_cast<Sample *>(samples.first());

  if (sample->extension("CountingRegionExtension"))
  {
    int margins[6];
    vtkSMPropertyHelper(readerProxy, "CountingBrick").Get(margins,6);

    // NOTE: Counting Region margin's order
    QString rcb = QString("RectangularRegion=%1,%2,%3,%4,%5,%6;")
      .arg(margins[0]).arg(margins[3])
      .arg(margins[1]).arg(margins[4])
      .arg(margins[2]).arg(margins[5]);
    qDebug() << "Using Counting Region" << rcb;
//     stack->extension("CountingRegionExtension")->setArguments(rcb);
  }

  vtkSMProperty *p;
  // Load Taxonomy
  p = readerProxy->GetProperty("Taxonomy");
  vtkSMStringVectorProperty* TaxProp = vtkSMStringVectorProperty::SafeDownCast(p);
  QString TaxonomySerialization(TaxProp->GetElement(0));

  QStringList taxonomies = TaxonomySerialization.split(";");

  Taxonomy *tax = new Taxonomy();
  QStringList availableTaxonomies;
  foreach(QString taxonomy, taxonomies)
  {
    if (taxonomy == "")
      continue;

    QStringList values = taxonomy.split(",");
    QChar zero = '0';
    QString color = QString("#%1%2%3")
    .arg(values[2].toInt(),2,16,zero)
    .arg(values[3].toInt(),2,16,zero)
    .arg(values[4].toInt(),2,16,zero);

    TaxonomyNode *node = tax->addElement(values[1]);
    node->setColor(QColor(color));
    availableTaxonomies.append(values[1]);
  }

  EspinaCore::instance()->model()->setTaxonomy(tax);
  tax->print();

  int numSegs;
  vtkSMPropertyHelper(readerProxy, "NumSegmentations").Get(&numSegs,1);

  // Create segmentation's taxonomy list
  p = readerProxy->GetProperty("SegTaxonomies");
  vtkSMStringVectorProperty* SegTaxProp = vtkSMStringVectorProperty::SafeDownCast(p);
  QString segTaxonomiesProp(SegTaxProp->GetElement(0));

  QStringList segTaxonomies = segTaxonomiesProp.split(";");
  QString readerId = File::extendedName(file) + ":0";

  for (int p=0; p < numSegs; p++)
  {
    // Extract Seg Filter
    pqFilter::Arguments extractArgs;
    extractArgs << pqFilter::Argument(QString("Input"),pqFilter::Argument::INPUT, readerId);
    extractArgs << pqFilter::Argument(QString("Block"),pqFilter::Argument::INTVECT, QString::number(p));
    pqFilter *segImage = cob->createFilter("filters","ExtractBlockAsImage",extractArgs);
    segImage->pipelineSource()->updatePipeline();
    vtkSMProxy *proxy = segImage->pipelineSource()->getProxy();
    proxy->UpdatePropertyInformation();

    Segmentation *seg = EspinaFactory::instance()->createSegmentation(this, p, segImage->data(0));
    QString qualifiedName = availableTaxonomies[segTaxonomies[p].toInt()-1];
    TaxonomyNode *node = tax->element(qualifiedName);
    Q_ASSERT(node);
    std::cout << "Getting taxonomy "<< segTaxonomies[p].toStdString() << ": " << node->qualifiedName().toStdString() << std::endl;
    seg->setTaxonomy(node);
    seg->setNumber(vtkSMPropertyHelper(proxy,"Label").GetAsInt());
    qDebug() << "Loading Segmentation" << seg->id() << "Taxonomy: " << seg->taxonomy()->name();

    m_blocks[QString::number(p)] = seg;
  }

  m_args.setBlocks(m_blocks.keys());
}

SegmhaImporterFilter::SegmhaImporterFilter(ModelItem::Arguments args)
: m_segReader(NULL)
, m_channel  (NULL)
, m_args     (args)
{
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
}


//-----------------------------------------------------------------------------
SegmhaImporterFilter::~SegmhaImporterFilter()
{
//   CachedObjectBuilder *cob = CachedObjectBuilder::instance();
//   if (m_restoreFilter)
//     delete m_restoreFilter;
//   if (m_segReader)
//     cob->removeFilter(m_segReader);
//   if (m_applyFilter)
//     delete m_applyFilter;
}

//-----------------------------------------------------------------------------
QString SegmhaImporterFilter::serialize() const
{
  return m_args.serialize();
}

//-----------------------------------------------------------------------------
QVariant SegmhaImporterFilter::data(int role) const
{
  if (role == Qt::DisplayRole)
    return SIF;
  else
    return QVariant();
}

//-----------------------------------------------------------------------------
Segmentation *SegmhaImporterFilter::product(int index) const
{
  return m_blocks[QString::number(index)];
}


// //-----------------------------------------------------------------------------
// void SegmhaImporterFilter::removeProduct(vtkProduct* product
// )
// {
//   assert(m_numSeg > 0);
//   m_blocks.remove(product);
//   m_numSeg--;
//   assert(m_blocks.size() == m_numSeg);
// 
//   QStringList prevArgs = m_args.split(';',QString::SkipEmptyParts);
//   QStringList newBlocks;
//   foreach(QString blockNo, m_blocks)
//     newBlocks.append(blockNo);
// 
//   QString blockList = newBlocks.join(",");
//   prevArgs[0] = "Blocks="+blockList;
// 
//   m_args = prevArgs.join(";");
//   m_args.append(';');
// }
// 
// QWidget* SegmhaImporterFilter::createWidget()
// {
//   return new QLabel("There is no information\n available for imported\n segmentations");
// }
