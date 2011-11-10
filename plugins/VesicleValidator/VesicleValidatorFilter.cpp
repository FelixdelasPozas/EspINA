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


#include "VesicleValidatorFilter.h"

#include <cache/cachedObjectBuilder.h>
#include <espINAFactory.h>
#include <espina.h>
#include <segmentation.h>
#include <sample.h>

// ParaQ
#include <pqApplicationCore.h>
#include <pqObjectBuilder.h>
#include <vtkSMPropertyHelper.h>
#include <vtkSMProxy.h>
#include <QLabel>

//-----------------------------------------------------------------------------
VesicleValidatorFilter::VesicleValidatorFilter (Sample *sample, const Point &pos, double SVA[6])
{
  memcpy(m_SVA,SVA,6*sizeof(double));
  type = FILTER;
  
  pqObjectBuilder *builder =  pqApplicationCore::instance()->getObjectBuilder();
//   m_SVA2 = builder->createProxy("implicit_functions","NonRotatingBox",pqApplicationCore::instance()->getActiveServer(),"widgets");
//   vtkSMPropertyHelper(m_SVA2,"Bounds").Set(SVA,6);
//   m_SVA2->UpdateVTKObjects();
  double spacing[3];
  sample->spacing(spacing);
  QString centerArg(QString("%1,%2,%3").arg(pos.x).arg(pos.y).arg(pos.z));
  QString spacingArg(QString("%1,%2,%3").arg(spacing[0]).arg(spacing[1]).arg(spacing[2]));
  
  m_args = ESPINA_ARG("Sample", sample->id());
  m_args.append(ESPINA_ARG("Type","VesicleValidator::VesicleValidatorFilter"));
//   m_args.append(ESPINA_ARG("SVA",region));
  m_args.append(ESPINA_ARG("Center",centerArg));
  m_args.append(ESPINA_ARG("Spacing",spacingArg));
  
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();

  vtkFilter::Arguments pointArgs;
  pointArgs.push_back(vtkFilter::Argument(QString("Input"),vtkFilter::INPUT, ""));
  pointArgs.push_back(vtkFilter::Argument(QString("Center"),vtkFilter::INTVECT, centerArg));
  pointArgs.push_back(vtkFilter::Argument(QString("Spacing"),vtkFilter::DOUBLEVECT, spacingArg));
  vtkFilter *point = cob->createFilter("sources","CrossSource", pointArgs);
  
  Segmentation *seg = EspINAFactory::instance()->CreateSegmentation(this, &(point->product(0)));
//   seg->addArgument("SVA",region);
  seg->addArgument("Center",centerArg);
//   seg->addArgument("Spacing",spacingArg);
  
  ProcessingTrace* trace = ProcessingTrace::instance();
  // Trace EspinaFilter
  trace->addNode(this);
  // Connect input
  trace->connect(sample, this, "Sample");
  // Trace Segmentation
  trace->addNode(seg);
  // Trace connection
  trace->connect(this, seg,"Validation");
  

  m_vesicles.append(seg);
  EspINA::instance()->addSegmentation(seg);
}

//-----------------------------------------------------------------------------
VesicleValidatorFilter::VesicleValidatorFilter (const ITraceNode::Arguments& args ) 
{
  type = FILTER;
  foreach(QString key, args.keys())
      m_args.append(ESPINA_ARG(key, args[key]));
  
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  
  vtkFilter::Arguments pointArgs;
  pointArgs.push_back(vtkFilter::Argument(QString("Input"),vtkFilter::INPUT, ""));
  pointArgs.push_back(vtkFilter::Argument(QString("Center"),vtkFilter::INTVECT, args["Center"]));
  pointArgs.push_back(vtkFilter::Argument(QString("Spacing"),vtkFilter::DOUBLEVECT, args["Spacing"]));
  vtkFilter *point = cob->createFilter("sources","CrossSource", pointArgs);
    
  Segmentation *seg = EspINAFactory::instance()->CreateSegmentation(this, &(point->product(0)));
  
  ProcessingTrace* trace = ProcessingTrace::instance();
  // Trace EspinaFilter
  trace->addNode(this);
  // Connect input
  trace->connect(args["Sample"], this, "Sample");
  // Trace Segmentation
  trace->addNode(seg);
  // Trace connection
  trace->connect(this, seg,"Validation");
  
  EspINA::instance()->addSegmentation(seg);
}


//-----------------------------------------------------------------------------
QWidget* VesicleValidatorFilter::createWidget()
{
  return new QLabel("No Informartion");
}

//-----------------------------------------------------------------------------
QList<vtkProduct *> VesicleValidatorFilter::products()
{
  return m_vesicles;
}

//-----------------------------------------------------------------------------
int VesicleValidatorFilter::numProducts()
{
  return m_vesicles.size();
}

//-----------------------------------------------------------------------------
vtkProduct VesicleValidatorFilter::product(int i)
{
  return *m_vesicles[i];
}


//-----------------------------------------------------------------------------
QString VesicleValidatorFilter::getArgument(QString name) const
{
  return (name=="Type")?"VesicleValidator::VesicleValidatorFilter":"";
}

//-----------------------------------------------------------------------------
void VesicleValidatorFilter::removeProduct ( vtkProduct* product )
{
  m_vesicles.clear();
}

//-----------------------------------------------------------------------------
void VesicleValidatorFilter::validateVesicle ( const Point& pos )
{
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();

  vtkFilter::Arguments pointArgs;
  pointArgs.push_back(vtkFilter::Argument(QString("Input"),vtkFilter::INPUT, ""));
  QString center(QString("%1,%2,%3").arg(pos.x).arg(pos.y).arg(pos.z));
  pointArgs.push_back(vtkFilter::Argument(QString("Center"),vtkFilter::INTVECT, center));
  vtkFilter *point = cob->createFilter("sources","CrossSource", pointArgs);
  
  Segmentation *seg = EspINAFactory::instance()->CreateSegmentation(this, &(point->product(0)));
  seg->addArgument("Center",center);
  
  ProcessingTrace* trace = ProcessingTrace::instance();
  // Trace Segmentation
  trace->addNode(seg);
  // Trace connection
  trace->connect(this, seg,"Validation");
  
  m_vesicles.append(seg);
  EspINA::instance()->addSegmentation(seg);
}
