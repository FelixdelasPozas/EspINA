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

#include <pqApplicationCore.h>

#include <pqObjectBuilder.h>
#include <pqPipelineSource.h>
#include <pq3DWidget.h>
#include <vtkSMProxy.h>
#include <vtkSMPropertyHelper.h>
#include <espina.h>
#include <products.h>
#include <vtkSMNewWidgetRepresentationProxy.h>


RectangularVOI::RectangularVOI()
: m_box(NULL)
{
  bzero(m_widget,4*sizeof(pq3DWidget *));
}


void RectangularVOI::a2w()
{

}

void RectangularVOI::w2a()
{

}

vtkSMProxy* RectangularVOI::getProxy()
{
  if (!m_box)
  {
    pqObjectBuilder *builder =  pqApplicationCore::instance()->getObjectBuilder();
    m_box =  builder->createProxy("implicit_functions","Box",pqApplicationCore::instance()->getActiveServer(),"widgets");
  }
  return m_box;
}

pq3DWidget *RectangularVOI::widget()
{
  if (!m_widget[3])
  {
  QList<pq3DWidget *> widgtes =  pq3DWidget::createWidgets(EspINA::instance()->activeSample()->sourceData()->getProxy(), getProxy());
  assert(widgtes.size() == 1);
  //m_widget[3] = widgtes[0];
  }
  
  return m_widget[3];
  //m_VOIWidget->setView(m_view);
  //m_VOIWidget->setWidgetVisible(true);
  //m_VOIWidget->select();
}

pq3DWidget *RectangularVOI::widget(int plane)
{
  assert (plane < 3);
  if (!m_widget[plane])
  {
    QList<pq3DWidget *> widgtes =  pq3DWidget::createWidgets(EspINA::instance()->activeSample()->sourceData()->getProxy(), getProxy());
    assert(widgtes.size() == 1);
    m_widget[plane] = widgtes[0];
    connect(m_widget[plane],SIGNAL(widgetEndInteraction()),this,SLOT(endInteraction()));
  }
  
  return m_widget[plane];
}

void RectangularVOI::endInteraction()
{
  //Update all widgets with box proxy bounds
  assert(m_box);
  double bounds[6];
  
  pq3DWidget *widget = qobject_cast<pq3DWidget *>(QObject::sender());
  
  widget->accept();
  
  //for (int w=0;w < 3; w++)
  //{
    /*if (w == 1)
    {
      double rot[3] = {0,0,-90};
      vtkSMProxy *rep = m_widget[w]->getWidgetProxy()->GetRepresentationProxy();
      vtkSMPropertyHelper(rep,"Rotation").Set(rot,3);
      rep->UpdateVTKObjects();
    }
    */
    //m_widget[w]->accept();
    /*
    if (w == 1)
    {
      double rot[3] = {0.0,90,0};
      vtkSMProxy *rep = m_widget[w]->getWidgetProxy()->GetRepresentationProxy();
      vtkSMPropertyHelper(rep,"Rotation").Set(rot,3);
      rep->UpdateVTKObjects();
    }
    */
  //}
  
  
//   vtkSMPropertyHelper(m_box,"Bounds").Get(bounds,6);
//   
//   
//   for (int w=0;w < 4; w++)
//   {
//     if (!m_widget[w])
//       continue;
//     
//     m_widget[w]->resetBounds(bounds);
//   }
}

void RectangularVOI::cancelVOI()
{
  
}