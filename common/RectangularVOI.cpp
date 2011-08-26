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


#include "RectangularVOI.h"

#include "sample.h"
#include "filter.h"

#include <pqApplicationCore.h>

#include <pqObjectBuilder.h>
#include <pqPipelineSource.h>
#include <pq3DWidget.h>
#include <vtkSMProxy.h>
#include <vtkSMPropertyHelper.h>
#include <espina.h>
#include <products.h>
#include <vtkSMNewWidgetRepresentationProxy.h>

#include <QDebug>
#include <assert.h>
#include <cache/cachedObjectBuilder.h>
#include <vtkBoxWidget2.h>
#include <vtkBoxRepresentation.h>
#include <vtkProperty.h>

const QString RectangularVOI::ApplyFilter::FilterType = "RectangularVOI::ApplyFilter";

RectangularVOI::ApplyFilter::ApplyFilter(vtkProduct* input, double* bounds)
{
   CachedObjectBuilder *cob = CachedObjectBuilder::instance();

   vtkFilter::Arguments args;
   args.push_back(vtkFilter::Argument(QString("Input"),vtkFilter::INPUT, input->id()));
   QString VolumeArg = QString("%1,%2,%3,%4,%5,%6").arg(bounds[0]).arg(bounds[1]).arg(bounds[2]).arg(bounds[3]).arg(bounds[4]).arg(bounds[5]);
   args.push_back(vtkFilter::Argument(QString("VOI"),vtkFilter::INTVECT, VolumeArg));
   m_rvoi = cob->createFilter("filters","RectangularVOI",args);
   
   m_args.append(ESPINA_ARG("Type",FilterType));
   m_args.append(ESPINA_ARG("Input",input->id()));
   m_args.append(ESPINA_ARG("Bound", VolumeArg));
}

//-----------------------------------------------------------------------------
RectangularVOI::ApplyFilter::ApplyFilter(ITraceNode::Arguments &args)
{
   CachedObjectBuilder *cob = CachedObjectBuilder::instance();

   vtkFilter::Arguments vtkArgs;
   vtkArgs.push_back(vtkFilter::Argument(QString("Input"),vtkFilter::INPUT, args["Input"]));
   vtkArgs.push_back(vtkFilter::Argument(QString("VOI"),vtkFilter::INTVECT, args["Bound"]));
   m_rvoi = cob->createFilter("filters","RectangularVOI", vtkArgs);

   m_args.append(ESPINA_ARG("Type",FilterType));
   m_args.append(ESPINA_ARG("Input",args["Input"]));
   m_args.append(ESPINA_ARG("Bound", args["Bound"]));
}

//-----------------------------------------------------------------------------
RectangularVOI::ApplyFilter::~ApplyFilter()
{
   CachedObjectBuilder *cob = CachedObjectBuilder::instance();
   
   cob->removeFilter(m_rvoi);
}


//-----------------------------------------------------------------------------
void RectangularVOI::ApplyFilter::removeProduct(vtkProduct* product)
{
  assert(false);
}

//-----------------------------------------------------------------------------
RectangularVOI::RectangularVOI(bool registerPlugin)
: m_box(NULL)
{
  //bzero(m_widget,4*sizeof(pq3DWidget *));
  QString registerName = ApplyFilter::FilterType;
  if (registerPlugin)
    ProcessingTrace::instance()->registerPlugin(registerName, this);
}

//-----------------------------------------------------------------------------
EspinaFilter* RectangularVOI::createFilter(QString filter, ITraceNode::Arguments& args)
{
  if (filter == ApplyFilter::FilterType)
    return new ApplyFilter(args);
  else
    return NULL;
}

//-----------------------------------------------------------------------------
EspinaFilter *RectangularVOI::applyVOI(vtkProduct* product)
{
  // To apply widget bounds to vtkBox source
  if (m_widgets.size() > 0)
    m_widgets.first()->accept();
  
  double voiExtent[6];
  rvoiExtent(voiExtent);
  //WARNING: How to deal with bounding boxes out of resources...
  
  //m_rvoi[0] = std::max(productExtent[0], round(pos[0] + m_rvoi[0] * scale[0]/productSpacing[0]));
  //m_rvoi[1] = round(pos[0] + m_rvoi[1] * scale[0]);
  //m_rvoi[2] = round(pos[1] + m_rvoi[2] * scale[1]);
  //m_rvoi[3] = round(pos[1] + m_rvoi[3] * scale[1]);
  //m_rvoi[4] = round(pos[2] + m_rvoi[4] * scale[2]);
  //m_rvoi[5] = round((pos[2] + m_rvoi[5] * scale[2]/2));
  
  //qDebug() << "RectangularVOI Plugin::ApplyVOI on: "<< voiExtent[0]<< voiExtent[1]<< voiExtent[2]<< voiExtent[3]<< voiExtent[4]<< voiExtent[5];
  EspinaFilter *rvoi = new ApplyFilter(product,voiExtent);
  
  return rvoi;
}

//-----------------------------------------------------------------------------
EspinaFilter *RectangularVOI::restoreVOITransormation(vtkProduct* product)
{
  return NULL;
}


//-----------------------------------------------------------------------------
vtkSMProxy* RectangularVOI::getProxy()
{
  if (!m_box)
  {
    pqObjectBuilder *builder =  pqApplicationCore::instance()->getObjectBuilder();
    m_box =  builder->createProxy("implicit_functions","Box",pqApplicationCore::instance()->getActiveServer(),"widgets");
  }
  return m_box;
}

//-----------------------------------------------------------------------------
pq3DWidget* RectangularVOI::newWidget()
{
  QList<pq3DWidget *> widgets =  pq3DWidget::createWidgets(m_product->creator()->pipelineSource()->getProxy(), getProxy());
  
  assert(widgets.size() == 1);
  connect(widgets[0],SIGNAL(widgetEndInteraction()),this,SLOT(endInteraction()));
  
  m_widgets.push_back(widgets[0]);
  
  vtkBoxWidget2 *boxwidget = dynamic_cast<vtkBoxWidget2*>(widgets[0]->getWidgetProxy()->GetWidget());
  assert(boxwidget);
//   vtkWidgetEventTranslator *et = boxwidget->GetEventTranslator();
  
  
  vtkBoxRepresentation *repbox =  dynamic_cast<vtkBoxRepresentation*>(boxwidget->GetRepresentation());
  repbox->HandlesOff();
  repbox->OutlineCursorWiresOff();
  vtkProperty *outline = repbox->GetOutlineProperty();
  outline->SetColor(1.0,1.0,0);
  
  return widgets[0];
}

//-----------------------------------------------------------------------------
void RectangularVOI::deleteWidget(pq3DWidget*& widget)
{
  m_widgets.removeOne(widget);
  qDebug() << "Active Widgets" << m_widgets.size();
  delete widget;
  widget = NULL;
}

//-----------------------------------------------------------------------------
bool RectangularVOI::contains(ISelectionHandler::VtkRegion region)
{
  foreach(Point p, region)
  {
    double voiExtent[6];
    rvoiExtent(voiExtent);
    if (p.x < voiExtent[0] || voiExtent[1] < p.x)
      return false;
    if (p.y < voiExtent[2] || voiExtent[3] < p.y)
      return false;
    if (p.z < voiExtent[4] || voiExtent[5] < p.z)
      return false;
  }
  return true;
}


//-----------------------------------------------------------------------------
void RectangularVOI::endInteraction()
{
  //Update all widgets with box proxy bounds
  assert(m_box);
  
  pq3DWidget *widget = qobject_cast<pq3DWidget *>(QObject::sender());
  widget->accept();
  double scale[3];
  vtkSMPropertyHelper(m_box,"Scale").Get(scale,3);
  double pos[3];
  vtkSMPropertyHelper(m_box,"Position").Get(pos,3);
  
  //qDebug() << "Moving RectangularVOI Plugin::Scale on: "<< scale[0]<< scale[1]<< scale[2];
  //qDebug() << "Moving RectangularVOI Plugin::Pos on: "<< pos[0]<< pos[1]<< pos[2];
  
//     qDebug() << "Bounds" << bounds[0] << bounds[1] << bounds[2] << bounds[3] << bounds[4] << bounds[5];
}

//-----------------------------------------------------------------------------
void RectangularVOI::cancelVOI()
{
  emit voiCancelled();
}

void RectangularVOI::rvoiExtent(double* rvoi)
{
  double bounds[6];
  vtkSMPropertyHelper(m_box,"Bounds").Get(bounds,6);
  double scale[3];
  vtkSMPropertyHelper(m_box,"Scale").Get(scale,3);
  double pos[3];
  vtkSMPropertyHelper(m_box,"Position").Get(pos,3);
  
  //qDebug() << "RectangularVOI Plugin: Scale: "<< scale[0]<< scale[1]<< scale[2];
  //qDebug() << "RectangularVOI Plugin: Pos: "<< pos[0]<< pos[1]<< pos[2];
  //qDebug() << "RectangularVOI Plugin: Extent: "<< m_rvoi[0]<< m_rvoi[1]<< m_rvoi[2]<< m_rvoi[3]<< m_rvoi[4]<< m_rvoi[5];
  
  //double productExtent[6] = {bounds[0],bounds[1],bounds[2], bounds[3], bounds[4], bounds[5]/2};
  double productSpacing[3];
  m_product->spacing(productSpacing);
  
  for (int i=0; i<6; i++)
    rvoi[i] = round((pos[i/2] + bounds[i]*scale[i/2])/productSpacing[i/2]);
}
