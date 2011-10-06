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

// Debug
#include "espina_debug.h"

// EspINA
#include "espINAFactory.h"
#include "espina.h"
#include "sample.h"
#include "segmentation.h"
#include "cache/cachedObjectBuilder.h"

#include <pqActiveObjects.h>
#include <pqCoreUtilities.h>
#include <pqServer.h>
#include <pqFileDialog.h>
#include <vtkSMProxy.h>
#include <vtkSMProxyManager.h>
#include <vtkSMReaderFactory.h>
#include <vtkSMPropertyHelper.h>
#include <QApplication>
#include <QLabel>
#include <vtkSMStringVectorProperty.h>

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

const QString SegmhaImporterFilter::ID = "SegmhaImporterFilter";

QString stripName(QString args){return args.split(";")[0];}//FAKE
QString stripArgs(QString args){return args.split(";")[1];}//FAKE


//-----------------------------------------------------------------------------
SegmhaImporterFilter::SegmhaImporterFilter(pqPipelineSource* reader, const QString &id)
: m_applyFilter(NULL)
, m_segReader(NULL)
, m_restoreFilter(NULL)
, m_finalFilter(NULL)
, m_numSeg(0)
{
  type = FILTER;
  
  ProcessingTrace* trace = ProcessingTrace::instance();
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  
  if (!EspINA::instance()->activeSample())
  {
    pqServer* server = pqActiveObjects::instance().activeServer();
    vtkSMReaderFactory* readerFactory = 
      vtkSMProxyManager::GetProxyManager()->GetReaderFactory();
      
//     QString filters = readerFactory->GetSupportedFileTypes(server->GetConnectionID());
      QString filters = "MetaImage File (*.mha, *.mhd)";
    
      if (!filters.isEmpty())
      {
	filters += ";;";
      }
      filters += "All files (*)";
      
      pqFileDialog fileDialog(server, pqCoreUtilities::mainWidget(), 
			      QObject::tr("Open Sample File:"), QString(), filters);
      fileDialog.setObjectName("FileOpenDialog");
      fileDialog.setFileMode(pqFileDialog::ExistingFiles);
      
      
      QApplication::setOverrideCursor(Qt::ArrowCursor);
      if (fileDialog.exec() == QDialog::Rejected)
      {
	EspINA::instance()->clear();
	QApplication::restoreOverrideCursor();
	return;
      }
      QApplication::restoreOverrideCursor();
      EspINA::instance()->loadFile(fileDialog.getSelectedFiles()[0], "add");
  }
  
  reader->updatePipeline();
  
  Sample *stack = EspINA::instance()->activeSample();
  
  if (stack->extension("CountingRegionExtension"))
  {
    int margins[6];
    
    reader->getProxy()->UpdatePropertyInformation();
    vtkSMPropertyHelper(reader->getProxy(),"CountingBrick").Get(margins,6);
  
    // NOTE: Counting Region margin's order 
    QString rcb = QString("RectangularRegion=%1,%2,%3,%4,%5,%6;")
      .arg(margins[0]).arg(margins[3])
      .arg(margins[1]).arg(margins[4])
      .arg(margins[2]).arg(margins[5]);
    stack->extension("CountingRegionExtension")->setArguments(rcb);
  }
  
    
  m_args = ESPINA_ARG("Sample",EspINA::instance()->activeSample()->id());
  QString readerId = id + ":0";
  m_args.append(ESPINA_ARG("File",readerId));
  
  m_segReader = CachedObjectBuilder::instance()->registerProductCreator(id, reader);
  
  // Load Taxonomy
  reader->getProxy()->UpdatePropertyInformation();
  
  vtkSMStringVectorProperty* TaxProp =
    vtkSMStringVectorProperty::SafeDownCast(reader->getProxy()->GetProperty("Taxonomy"));
  QString taxonomyFile(TaxProp->GetElement(0));
  
  QStringList taxonomies = taxonomyFile.split(";");
  
  TaxonomyNode *root = new TaxonomyNode("Segmha");
  QStringList availableTaxonomies;
  foreach(QString taxonomy, taxonomies)
  {
    if (taxonomy == "")
      continue;
    
    QStringList values = taxonomy.split(" ");
    QChar zero = '0';
    QString color = QString("#%1%2%3")
    .arg(values[2].toInt(),2,16,zero)
    .arg(values[3].toInt(),2,16,zero)
    .arg(values[4].toInt(),2,16,zero);
    
    root->addElement(values[1],"Segmha",color); 
    availableTaxonomies.append(values[1]);
  }
  EspINA::instance()->loadTaxonomy(root);
//   std::cout << "Taxonomy read: " << taxonomyFile.toStdString() << std::endl;
  
  
  
  reader->getProxy()->UpdatePropertyInformation();
  vtkSMPropertyHelper(reader->getProxy(),"NumSegmentations").Get(&m_numSeg,1);
  
  QStringList blockNos;
  
  // Trace EspinaFilter
  trace->addNode(this);
    // Connect input
  QString inputId = EspINA::instance()->activeSample()->id();
  trace->connect(inputId,this,"Sample");
  
  // Create segmentation's taxonomy list
  vtkSMStringVectorProperty* SegTaxProp =
    vtkSMStringVectorProperty::SafeDownCast(reader->getProxy()->GetProperty("SegTaxonomies"));
  QString segTaxonomiesProp(SegTaxProp->GetElement(0));

  QStringList segTaxonomies = segTaxonomiesProp.split(";");
  
  for (int p=0; p<m_numSeg; p++)
  {
    //! Extract Seg Filter
    vtkFilter::Arguments extractArgs;
    extractArgs.push_back(vtkFilter::Argument(QString("Input"),vtkFilter::INPUT, readerId));
    extractArgs.push_back(vtkFilter::Argument(QString("Block"),vtkFilter::INTVECT,QString("%1").arg(p)));
    vtkFilter *segImage = cob->createFilter("filters","ExtractBlockAsImage",extractArgs);
    blockNos.append(QString("%1").arg(p));
    
    Segmentation *seg = EspINAFactory::instance()->CreateSegmentation(this, &segImage->product(0));
    m_blocks[seg] = blockNos[p];

    // Trace Segmentation
    trace->addNode(seg);
    // Trace connection
    trace->connect(this, seg,"Segmentation");
    
    std::cout << "Getting taxonomy "<< segTaxonomies[p].toStdString() << ": " << availableTaxonomies[segTaxonomies[p].toInt()-1].toStdString() << std::endl;
    seg->setTaxonomy(root->getComponent(availableTaxonomies[segTaxonomies[p].toInt()-1]));
    
    std::cout << "Adding " << seg->taxonomy()->getName().toStdString() << " segmentation" << std::endl;
    
    EspINA::instance()->addSegmentation(seg);
  }
  
  QString blockList = blockNos.join(",");
  m_args.append(ESPINA_ARG("Blocks",blockList));
}

//-----------------------------------------------------------------------------
SegmhaImporterFilter::SegmhaImporterFilter(ITraceNode::Arguments& args)
: m_applyFilter(NULL)
, m_segReader(NULL)
, m_restoreFilter(NULL)
, m_finalFilter(NULL)
, m_numSeg(0)
{
  type = FILTER;
  
  ProcessingTrace* trace = ProcessingTrace::instance();
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  
  foreach(QString key, args.keys())
  {
    m_args.append(ESPINA_ARG(key, args[key]));
  }
  
  QStringList blockNos = args["Blocks"].split(",");
  m_numSeg = blockNos.size();

  // Trace EspinaFilter
  trace->addNode(this);
    // Connect input
  QString inputId = args["Sample"];
  trace->connect(inputId,this,"Sample");
  
  for (int p=0; p<m_numSeg; p++)
  {
    //! Extract Seg Filter
    vtkFilter::Arguments extractArgs;
    extractArgs.push_back(vtkFilter::Argument(QString("Input"),vtkFilter::INPUT, args["File"]));
    extractArgs.push_back(vtkFilter::Argument(QString("Block"),vtkFilter::INTVECT,blockNos[p]));
    vtkFilter *segImage = cob->createFilter("filters","ExtractBlockAsImage",extractArgs);
    
    Segmentation *seg = EspINAFactory::instance()->CreateSegmentation(this, &segImage->product(0));
    m_blocks[seg] = blockNos[p];
    
    // Trace Segmentation
    trace->addNode(seg);
    // Trace connection
    trace->connect(this, seg,"Segmentation");
    
    EspINA::instance()->addSegmentation(seg);
  }
}


//-----------------------------------------------------------------------------
SegmhaImporterFilter::~SegmhaImporterFilter()
{
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  if (m_restoreFilter)
    delete m_restoreFilter;
  if (m_segReader)
    cob->removeFilter(m_segReader);
  if (m_applyFilter)
    delete m_applyFilter;
}

//-----------------------------------------------------------------------------
void SegmhaImporterFilter::removeProduct(EspinaProduct* product)
{
  assert(m_numSeg > 0);
  m_blocks.remove(product);
  m_numSeg--;
  assert(m_blocks.size() == m_numSeg);

  QStringList prevArgs = m_args.split(';',QString::SkipEmptyParts);
  QStringList newBlocks;
  foreach(QString blockNo, m_blocks)
    newBlocks.append(blockNo);

  QString blockList = newBlocks.join(",");
  prevArgs[0] = "Blocks="+blockList;

  m_args = prevArgs.join(";");
  m_args.append(';');
}

QWidget* SegmhaImporterFilter::createSetupWidget()
{
  return new QLabel("There is no information\n available for imported\n segmentations");
}
