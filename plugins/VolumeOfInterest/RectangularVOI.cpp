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

#include "products.h"
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

RectangularVOI::ApplyFilter::ApplyFilter(vtkProduct* input, double* bounds)
{
   CachedObjectBuilder *cob = CachedObjectBuilder::instance();

   vtkFilter::Arguments args;
   args.push_back(vtkFilter::Argument(QString("Input"),vtkFilter::INPUT, input->id()));
   QString VolumeArg = QString("%1,%2,%3,%4,%5,%6").arg(bounds[0]).arg(bounds[1]).arg(bounds[2]).arg(bounds[3]).arg(bounds[4]).arg(bounds[5]);
   args.push_back(vtkFilter::Argument(QString("VOI"),vtkFilter::INTVECT, VolumeArg));
   m_rvoi = cob->createFilter("filters","RectangularVOI",args);
   
   m_args.append("RectangularVOI::Apply=").append(VolumeArg);
}



//-----------------------------------------------------------------------------
RectangularVOI::RectangularVOI()
: m_box(NULL)
{
  bzero(m_widget,4*sizeof(pq3DWidget *));
}

//-----------------------------------------------------------------------------
IFilter *RectangularVOI::applyVOI(vtkProduct* product)
{
  // To apply widget bounds to vtkBox source
  if (m_widget[0])
    m_widget[0]->accept();
  vtkSMPropertyHelper(m_box,"Bounds").Get(m_rvoi,6);
  double scale[3];
  vtkSMPropertyHelper(m_box,"Scale").Get(scale,3);
  double pos[3];
  vtkSMPropertyHelper(m_box,"Position").Get(pos,3);
  
  qDebug() << "RectangularVOI Plugin::Scale on: "<< scale[0]<< scale[1]<< scale[2];
  qDebug() << "RectangularVOI Plugin::Pos on: "<< pos[0]<< pos[1]<< pos[2];
  
  double productExtent[6] = {m_rvoi[0],m_rvoi[1],m_rvoi[2], m_rvoi[3], m_rvoi[4], m_rvoi[5]/2};
  double productSpacing[3] = {1,1,2};
  
 // CALCULAR LA NUEVA REGION
  
  m_rvoi[0] = std::max(productExtent[0], round(pos[0] + m_rvoi[0] * scale[0]/productSpacing[0]));
  m_rvoi[1] = round(pos[0] + m_rvoi[1] * scale[0]);
  m_rvoi[2] = round(pos[1] + m_rvoi[2] * scale[1]);
  m_rvoi[3] = round(pos[1] + m_rvoi[3] * scale[1]);
  m_rvoi[4] = round(pos[2] + m_rvoi[4] * scale[2]);
  m_rvoi[5] = round((pos[2] + m_rvoi[5] * scale[2]/2));
  
  qDebug() << "RectangularVOI Plugin::ApplyVOI on: "<< m_rvoi[0]<< m_rvoi[1]<< m_rvoi[2]<< m_rvoi[3]<< m_rvoi[4]<< m_rvoi[5];
  EspinaFilter *rvoi = new ApplyFilter(product,m_rvoi);
  
  return rvoi;
}

//-----------------------------------------------------------------------------
IFilter *RectangularVOI::restoreVOITransormation(vtkProduct* product)
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
pq3DWidget *RectangularVOI::widget()
{
  if (!m_widget[3])
  {
  QList<pq3DWidget *> widgtes =  pq3DWidget::createWidgets(EspINA::instance()->activeSample()->creator()->pipelineSource()->getProxy(), getProxy());
  assert(widgtes.size() == 1);
  m_widget[3] = widgtes[0];
    connect(m_widget[3],SIGNAL(widgetEndInteraction()),this,SLOT(endInteraction()));
  }
  
  return m_widget[3];
}

//-----------------------------------------------------------------------------
pq3DWidget *RectangularVOI::widget(int plane)
{
  assert (plane < 3);
  if (!m_widget[plane])
  {
    QList<pq3DWidget *> widgtes =  pq3DWidget::createWidgets(EspINA::instance()->activeSample()->creator()->pipelineSource()->getProxy(), getProxy());
    assert(widgtes.size() == 1);
    m_widget[plane] = widgtes[0];
    connect(m_widget[plane],SIGNAL(widgetEndInteraction()),this,SLOT(endInteraction()));
  }
  
  return m_widget[plane];
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
  
  qDebug() << "Moving RectangularVOI Plugin::Scale on: "<< scale[0]<< scale[1]<< scale[2];
  qDebug() << "Moving RectangularVOI Plugin::Pos on: "<< pos[0]<< pos[1]<< pos[2];
  /*
  int idx;
  for (idx=0; idx<4; idx++)
  {
    if (m_widget[idx] == widget)
      break;
  }
  */
  
//   widget->getControlledProxy()->UpdateProperty("Bounds");
//   widget->getControlledProxy()->UpdateVTKObjects();
//   widget->getControlledProxy()->UpdateProperty("Bounds");
//   vtkSMPropertyHelper(widget->getControlledProxy(),"Bounds").Get(bounds,6);
//   qDebug() << "Controlled Bounds" << bounds[0] << bounds[1] << bounds[2] << bounds[3] << bounds[4] << bounds[5];
//   widget->getReferenceProxy()->UpdateProperty("Bounds");
//   widget->getReferenceProxy()->UpdateVTKObjects();
//   widget->getReferenceProxy()->UpdateProperty("Bounds");
//   vtkSMPropertyHelper(widget->getReferenceProxy(),"Bounds").Get(bounds,6);
//   qDebug() << "Reference Bounds" << bounds[0] << bounds[1] << bounds[2] << bounds[3] << bounds[4] << bounds[5];
//   widget->getWidgetProxy()->GetRepresentationProxy()->UpdateProperty("Bounds");
//   widget->getWidgetProxy()->GetRepresentationProxy()->UpdateVTKObjects();
//   widget->getWidgetProxy()->GetRepresentationProxy()->UpdateProperty("Bounds");
//   vtkSMPropertyHelper(widget->getWidgetProxy()->GetRepresentationProxy(),"Bounds").Get(bounds,6);
//   qDebug() << "Representation Bounds" << bounds[0] << bounds[1] << bounds[2] << bounds[3] << bounds[4] << bounds[5];
//    double rot[3] = {0,0,0};
//    //double pos[3] = {0,0,0};
//    vtkSMProxy *rep = widget->getControlledProxy();
//    rep->PrintSelf(std::cout,vtkIndent(5));
//    vtkSMPropertyHelper(rep,"Rotation").Set(rot,3);
//    //vtkSMPropertyHelper(rep,"Position").Set(pos,3);
//    rep->UpdateVTKObjects();
//   
//   double rot[3][3] = 
//   {
//     {0,0,0} ,
//     {0,90,0} ,
//     {0,90,0}
//   };
//   double pos[3][3] = 
//   {
//     {0,0,0} ,
//     {0,0,0} ,
//     {0,0,0}
//   };
//   for (idx=0;idx<3;idx++)
//   {
//     vtkSMProxy *rep = m_widget[idx]->getWidgetProxy()->GetRepresentationProxy();
//     vtkSMPropertyHelper(rep,"Rotation").Set(rot[idx],3);
//     vtkSMPropertyHelper(rep,"Position").Set(pos[idx],3);
//     rep->UpdateVTKObjects();
//   }
 //     vtkSMPropertyHelper(m_widget[idx]->getControlledProxy(),"Bounds").Get(bounds,6);
 //     qDebug() << "Bounds" << bounds[0] << bounds[1] << bounds[2] << bounds[3] << bounds[4] << bounds[5];
}

//-----------------------------------------------------------------------------
void RectangularVOI::cancelVOI()
{
  
}