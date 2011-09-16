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
#include "selectionManager.h"
#include "cache/cachedObjectBuilder.h"

#include <pqPipelineSource.h>
#include <vtkSMProxy.h>
#include <vtkSMPropertyHelper.h>


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

const QString SegmhaImporterFilter::ID = "SegmhaImporterFiler";

QString stripName(QString args){return args.split(";")[0];}//FAKE
QString stripArgs(QString args){return args.split(";")[1];}//FAKE


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
  
  QString readerId = id + ":0";
  m_segReader = CachedObjectBuilder::instance()->registerProductCreator(id, reader);
  reader->updatePipeline();
  
  reader->getProxy()->UpdatePropertyInformation();
  vtkSMPropertyHelper(reader->getProxy(),"NumSegmentations").Get(&m_numSeg,1);
  
  // Trace EspinaFilter
  trace->addNode(this);
    // Connect input
  QString inputId = "peque.mhd:0";
   trace->connect(inputId,this,"Sample");
  
  for (int p=0; p<m_numSeg; p++)
  {
    //! Extract Seg Filter
    vtkFilter::Arguments growArgs;
    growArgs.push_back(vtkFilter::Argument(QString("Input"),vtkFilter::INPUT, readerId));
    growArgs.push_back(vtkFilter::Argument(QString("Block"),vtkFilter::INTVECT,QString("%1").arg(p)));
    vtkFilter *segImage = cob->createFilter("filters","ExtractBlockAsImage",growArgs);
    
    Segmentation *seg = EspINAFactory::instance()->CreateSegmentation(this, &segImage->product(0));
    
    // Trace Segmentation
    trace->addNode(seg);
    // Trace connection
    trace->connect(this, seg,"Segmentation");
    
    EspINA::instance()->addSegmentation(seg);
  }
}


/*
SegmhaImporterFilter::SegmhaImporterFilter(EspinaProduct* input, IVOI* voi, ITraceNode::Arguments& args)
: m_applyFilter(NULL)
, m_grow(NULL)
, m_restoreFilter(NULL)
, m_finalFilter(NULL)
{
  type = FILTER;
  ProcessingTrace* trace = ProcessingTrace::instance();
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();

  //m_args = QString("%1=%2;").arg("Sample").arg(input->label());
  m_args = ESPINA_ARG("Sample", input->getArgument("Id"));
  foreach(QString argName, args.keys())
  {
    m_args.append(ESPINA_ARG(argName, args[argName]));
  }
  
  vtkProduct voiOutput(input->creator(),input->portNumber());
  //! Executes VOI
  if (voi)
  {
    m_applyFilter = voi->applyVOI(input);
    if (m_applyFilter)
    {
      voiOutput = m_applyFilter->product(0);
      m_args.append(ESPINA_ARG("ApplyVOI", "["+m_applyFilter->getFilterArguments() + "]"));
    }
  }

  //! Execute Grow Filter
  vtkFilter::Arguments growArgs;
  growArgs.push_back(vtkFilter::Argument(QString("Input"),vtkFilter::INPUT, voiOutput.id()));
  growArgs.push_back(vtkFilter::Argument(QString("Seed"),vtkFilter::INTVECT,args["Seed"]));
  QStringList seed = args["Seed"].split(",");
  m_seed[0] = seed[0].toInt();
  m_seed[1] = seed[1].toInt();
  m_seed[2] = seed[2].toInt();
  
  growArgs.push_back(vtkFilter::Argument(QString("Threshold"),vtkFilter::DOUBLEVECT,args["Threshold"]));
  m_threshold = args["Threshold"].toInt();
  //growArgs.push_back(vtkFilter::Argument(QString("ProductPorts"),vtkFilter::INTVECT, "0"));
  m_grow = cob->createFilter("filters","SegmhaImporterFilter",growArgs);
  
  //! Create segmenations. SegmhaImporterFilter has only 1 output
  assert(m_grow->numProducts() == 1);
  
  m_finalFilter = m_grow;
  
  vtkProduct growOutput = m_grow->product(0);
  //! Restore possible VOI transformation
  if (voi)
  {
    m_restoreFilter = voi->restoreVOITransormation(&growOutput);
    if (m_restoreFilter)
    {
      growOutput = m_restoreFilter->product(0);
      m_finalFilter = m_restoreFilter;
      //TODO Anadir args
      
    }
  }

  assert(m_finalFilter->numProducts() == 1);
  m_numSeg = m_finalFilter->numProducts();
  
  //WARNING: taking address of temporary => &m_finalFilter->product(0) ==> Need review
  Segmentation *seg = EspINAFactory::instance()->CreateSegmentation(this, &m_finalFilter->product(0));

  // Trace EspinaFilter
  trace->addNode(this);
  // Connect input
  trace->connect(input,this,"Sample");
  // Trace Segmentation
  trace->addNode(seg);
  // Trace connection
  trace->connect(this, seg,"Segmentation");
  
  EspINA::instance()->addSegmentation(seg);
}


SegmhaImporterFilter::SegmhaImporterFilter(ITraceNode::Arguments& args)
: m_applyFilter(NULL)
, m_grow(NULL)
, m_restoreFilter(NULL)
, m_finalFilter(NULL)
{
  foreach(QString key, args.keys())
  {
    if( key == "ApplyVOI" )
      m_args.append(ESPINA_ARG(key, "["+ args[key] + "]"));
    else
      m_args.append(ESPINA_ARG(key, args[key]));
  }
  type = FILTER;
  ProcessingTrace* trace = ProcessingTrace::instance();
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();

  vtkProduct input(args["Sample"]);

  vtkProduct voiOutput(input.creator(),input.portNumber());
  //! Executes VOI
  if (args.contains("ApplyVOI") )
  {
    ITraceNode::Arguments voiArgs = ITraceNode::parseArgs(args["ApplyVOI"]);
    m_applyFilter = trace->getRegistredPlugin(voiArgs["Type"])->createFilter(voiArgs["Type"],voiArgs);
    if (m_applyFilter)
    {
      voiOutput = m_applyFilter->product(0);
      //m_args.append("ApplyVOI=" + applyFilter->getFileArguments());
      //m_args.append(ESPINA_ARG("ApplyVOI", "["+m_applyFilter->getFilterArguments() + "]"));
    }
  }

  //! Execute Grow Filter
  vtkFilter::Arguments growArgs;
  growArgs.push_back(vtkFilter::Argument(QString("Input"),vtkFilter::INPUT, voiOutput.id()));
  growArgs.push_back(vtkFilter::Argument(QString("Seed"),vtkFilter::INTVECT,args["Seed"]));
  QStringList seed = args["Seed"].split(",");
  m_seed[0] = seed[0].toInt();
  m_seed[1] = seed[1].toInt();
  m_seed[2] = seed[2].toInt();
  
  growArgs.push_back(vtkFilter::Argument(QString("Threshold"),vtkFilter::DOUBLEVECT,args["Threshold"]));
  m_threshold = args["Threshold"].toInt();
  //growArgs.push_back(vtkFilter::Argument(QString("ProductPorts"),vtkFilter::INTVECT, "0"));
  // Disk cache. If the .seg contains .mhd files now it try to load them
//   Cache::Index id = cob->generateId("filter", "SegmhaImporterFilter", growArgs);
//   m_grow = cob->getFilter(id);
//   if( !m_grow )
  m_grow = cob->createFilter("filters","SegmhaImporterFilter",growArgs);
  
  //! Create segmenations. SegmhaImporterFilter has only 1 output
  assert(m_grow->numProducts() == 1);

  m_finalFilter = m_grow;

  vtkProduct growOutput = m_grow->product(0);
  //! Restore possible VOI transformation
  if (args.contains("RestoreVOI"))
  {
    
    //TODO: Restore
//     m_restoreFilter = voi->restoreVOITransormation(&growOutput);
//     if (m_restoreFilter)
//     {
//       growOutput = m_restoreFilter->product(0);
//       m_finalFilter = m_restoreFilter;
//       //Anadir args
//     }
  }

  assert(m_finalFilter->numProducts() == 1);
  m_numSeg = m_finalFilter->numProducts();

  Segmentation *seg = EspINAFactory::instance()->CreateSegmentation(this, &m_finalFilter->product(0));

  // Trace EspinaFilter
  trace->addNode(this);
  // Connect input
  trace->connect(args["Sample"],this,"Sample");
  // Trace Segmentation
  trace->addNode(seg);
  // Trace connection
  trace->connect(this, seg,"Segmentation");

  EspINA::instance()->addSegmentation(seg);
}
*/

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

void SegmhaImporterFilter::removeProduct(EspinaProduct* product)
{
  assert(false);//TODO
  m_numSeg = 0;
}

QWidget* SegmhaImporterFilter::createSetupWidget()
{
  assert(false);//TODO
  return new QPushButton("Fake");//SetupWidget(this);
}
