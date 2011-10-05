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


#include "SeedGrowSegmentationFilter.h"

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
#include <QSpinBox>
#include <QLayout>
#include <EspinaPluginManager.h>
#include <pqOutputPort.h>
#include <vtkPVDataInformation.h>
#include <vtkSMProxy.h>
#include <QMessageBox>



SeedGrowSegmentationFilter::SetupWidget::SetupWidget(EspinaFilter *parent)
: QWidget()
{
  setupUi(this);
  SeedGrowSegmentationFilter *filter = dynamic_cast<SeedGrowSegmentationFilter *>(parent);
  m_xSeed->setText(QString("%1").arg(filter->m_seed[0]));
  m_ySeed->setText(QString("%1").arg(filter->m_seed[1]));
  m_zSeed->setText(QString("%1").arg(filter->m_seed[2]));
  m_threshold->setValue(filter->m_threshold);
}


QString stripName(QString args){return args.split(";")[0];}//FAKE
QString stripArgs(QString args){return args.split(";")[1];}//FAKE


SeedGrowSegmentationFilter::SeedGrowSegmentationFilter(EspinaProduct* input, IVOI* voi, ITraceNode::Arguments& args)
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
  m_grow = cob->createFilter("filters","SeedGrowSegmentationFilter",growArgs);
  
  //! Create segmenations. SeedGrowSegmentationFilter has only 1 output
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
  
  if (voi)
  {
    int extent[6];
    seg->creator()->pipelineSource()->updatePipeline();
    seg->creator()->pipelineSource()->getProxy()->UpdatePropertyInformation();
    seg->outputPort()->getDataInformation()->GetExtent(extent);
    QStringList voiArgs = m_applyFilter->getFilterArguments().split(';');
    QStringList bounds = voiArgs[2].section('=',-1).split(',');
    for (int i=0; i < 6; i++)
    {
//       std::cout << extent[i] << " - " << bounds[i].toInt() << std::endl;
      if (extent[i] == bounds[i].toInt())
      {
	QString title("Seed Grow Segmentation Filter Information");
	QString text("New segmentation may be incomplete due to VOI restriction.");
	
	QApplication::setOverrideCursor(Qt::ArrowCursor);
	QApplication::setOverrideCursor(Qt::ArrowCursor);
	QMessageBox msgBox(QMessageBox::Information,title,text);
	msgBox.exec();
	QApplication::restoreOverrideCursor();
	QApplication::restoreOverrideCursor();
	break;
	//"Seed Grow Segmentation Filter Information",
      }
    }
  }
  
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


SeedGrowSegmentationFilter::SeedGrowSegmentationFilter(ITraceNode::Arguments& args)
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
    m_applyFilter = EspinaPluginManager::instance()->createFilter(voiArgs["Type"],voiArgs);
//     m_applyFilter = trace->getRegistredPlugin(voiArgs["Type"])->createFilter(voiArgs["Type"],voiArgs); // 
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
//   Cache::Index id = cob->generateId("filter", "SeedGrowSegmentationFilter", growArgs);
//   m_grow = cob->getFilter(id);
//   if( !m_grow )
  m_grow = cob->createFilter("filters","SeedGrowSegmentationFilter",growArgs);
  
  //! Create segmenations. SeedGrowSegmentationFilter has only 1 output
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

SeedGrowSegmentationFilter::~SeedGrowSegmentationFilter()
{
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  if (m_restoreFilter)
    delete m_restoreFilter;
  if (m_grow)
    cob->removeFilter(m_grow);
  if (m_applyFilter)
    delete m_applyFilter;
}

void SeedGrowSegmentationFilter::removeProduct(EspinaProduct* product)
{
  m_numSeg = 0;
}

QWidget* SeedGrowSegmentationFilter::createSetupWidget()
{
  return new SetupWidget(this);
}
