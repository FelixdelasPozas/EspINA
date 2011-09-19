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
	assert(false);
      }
      QApplication::restoreOverrideCursor();
      EspINA::instance()->loadFile(fileDialog.getSelectedFiles()[0], "add");
  }
  
    
  m_args = ESPINA_ARG("Sample",EspINA::instance()->activeSample()->id());
  QString readerId = id + ":0";
  m_args.append(ESPINA_ARG("File",readerId));
  
  m_segReader = CachedObjectBuilder::instance()->registerProductCreator(id, reader);
  reader->updatePipeline();
  
  reader->getProxy()->UpdatePropertyInformation();
  vtkSMPropertyHelper(reader->getProxy(),"NumSegmentations").Get(&m_numSeg,1);

  QStringList blockNos;
  
  // Trace EspinaFilter
  trace->addNode(this);
    // Connect input
  QString inputId = EspINA::instance()->activeSample()->id();
  trace->connect(inputId,this,"Sample");
  
  for (int p=0; p<m_numSeg; p++)
  {
    //! Extract Seg Filter
    vtkFilter::Arguments extractArgs;
    extractArgs.push_back(vtkFilter::Argument(QString("Input"),vtkFilter::INPUT, readerId));
    extractArgs.push_back(vtkFilter::Argument(QString("Block"),vtkFilter::INTVECT,QString("%1").arg(p)));
    vtkFilter *segImage = cob->createFilter("filters","ExtractBlockAsImage",extractArgs);
    blockNos.append(QString("%1").arg(p));
    
    Segmentation *seg = EspINAFactory::instance()->CreateSegmentation(this, &segImage->product(0));
    
    // Trace Segmentation
    trace->addNode(seg);
    // Trace connection
    trace->connect(this, seg,"Segmentation");
    
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
  assert(false);//TODO
  m_numSeg = 0;
}

QWidget* SegmhaImporterFilter::createSetupWidget()
{
  return new QLabel("There is no information\n available for imported\n segmentations");
}
