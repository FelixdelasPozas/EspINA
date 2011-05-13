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

#include <selectionManager.h>
#include <cache/cachedObjectBuilder.h>

#include <pqPipelineSource.h>
#include <espINAFactory.h>
#include <espina.h>

QString stripName(QString args){return args.split(";")[0];}//FAKE
QString stripArgs(QString args){return args.split(";")[1];}//FAKE


SeedGrowSegmentationFilter::SeedGrowSegmentationFilter(EspinaProduct* input, IVOI* voi, ITraceNode::Arguments& args, EspinaPlugin* parent

)
: m_plugin(parent)
{
  type = FILTER;
  ProcessingTrace* trace = ProcessingTrace::instance();
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  
  foreach(QString argName, args.keys())
  {
    m_args.append(QString("%1:%2;").arg(argName).arg(args[argName]));
  }
  
  vtkProduct voiOutput(input->creator(),input->portNumber());
  //! Executes VOI
  if (voi)
  {
    m_applyFilter = voi->applyVOI(input);
    if (m_applyFilter)
    {
      voiOutput = m_applyFilter->product(0);
      //m_args.append("ApplyVOI=" + applyFilter->getFileArguments());
      m_args += "Param=value;";
    }
  }

  //! Execute Grow Filter
  vtkFilter::Arguments growArgs;
  growArgs.push_back(vtkFilter::Argument(QString("Input"),vtkFilter::INPUT, voiOutput.id()));
  growArgs.push_back(vtkFilter::Argument(QString("Seed"),vtkFilter::INTVECT,args["Seed"]));
  growArgs.push_back(vtkFilter::Argument(QString("Threshold"),vtkFilter::DOUBLEVECT,args["Threshold"]));
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
      //Anadir args
    }
  }

  assert(m_finalFilter->numProducts() == 1);
  
  Segmentation *seg = EspINAFactory::instance()->CreateSegmentation(&m_finalFilter->product(0));

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
{
/*
  //Invariante: input had already been created
  vtkProduct input(args["Sample"]);//ALERT: input.filter.filter is NULL. It should be managed by the constructor
  
  ProcessingTrace* trace = ProcessingTrace::instance();
  
  //! Executes VOI
  //QString voiFilterName = stripName(args["VOI"]);
  //QString voiFilterArgs = stripArgs(args["VOI"]);
  //EspinaPlugin *voiPlugin = trace->getRegistredPlugin(voiFilterName);
  //m_voi = dynamic_cast<IVOI *>(voiPlugin->createFilter(voiFilterName,voiFilterArgs));
  //vtkProduct tmpProduct = m_voi->applyVOI(input);
  
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  //! TODO: Execute Blur Filter
  /**
  EspinaParamList blurArgs;
  blurArgs.push_back(EspinaParam("input",input->id()));
  QString kernel = QString("2,2,2");
  blurArgs.push_back(EspinaParam("Kernel",kernel.toStdString()));
  **/
  /*
  //! Execute Grow Filter
  vtkFilter::Arguments growArgs;
  growArgs.push_back(vtkFilter::Argument(QString("Input"),vtkFilter::INPUT, input.id()));
  growArgs.push_back(vtkFilter::Argument(QString("Seed"),vtkFilter::INTVECT,args["Seed"]));
  growArgs.push_back(vtkFilter::Argument(QString("Threshold"),vtkFilter::INTVECT,args["Threshold"]));
  
  m_grow = cob->createFilter("filters","SeedGrowSegmentationFilter",growArgs);
  
  //! Create segmenations. SeedGrowSegmentationFilter has only 1 output
  assert(m_grow->numProducts() == 1);
  //! Restore possible VOI transformation
  //vtkProduct newSeg = m_voi->restoreVOITransormation(m_grow.products(0));
  ////!IFilter *restoreFilter = m_voi->restoreVOITransormation(m_grow.products(0));
  ////! In this case, there should be only 1 product
  ////!(restoreFilter->numProducts() == 0);
  
  Segmentation *seg = EspINAFactory::instance()->CreateSegmentation(m_grow->product(0));

  // Trace EspinaFilter
  trace->addNode(this);
  // Connect input
  //TODO:trace->connect(input.id,this,"Sample");
  // Trace Segmentation
  trace->addNode(seg);
  // Trace connection
  trace->connect(this, seg,"Segmentation");
  
  EspINA::instance()->addSegmentation(seg);
  */
}
