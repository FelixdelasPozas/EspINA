/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.es>

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


#include "RectangularSelection.h"

#include "vtkNonRotatingBoxWidget.h"

#include <pq3DWidget.h>
#include <pqApplicationCore.h>
#include <pqObjectBuilder.h>
#include <vtkBoxRepresentation.h>
#include <vtkProperty.h>
#include <vtkSMNewWidgetRepresentationProxy.h>
#include <vtkSMProxy.h>

#include <QDebug>
#include <pqPipelineSource.h>
#include <vtkSMPropertyHelper.h>
#include <pq3DWidget.h>

//----------------------------------------------------------------------------
RectangularRegion::RectangularRegion()
: m_box(NULL)
{
}

//----------------------------------------------------------------------------
RectangularRegion::~RectangularRegion()
{
//   qDebug() << "Destroying RectangularRegion";
//   qDebug() << "  containing" << m_widgets.size() << "widgets";
  foreach(pq3DWidget *widget, m_widgets)
  {
    widget->deselect();
    widget->deleteLater();
  }
  m_widgets.clear();
}

//----------------------------------------------------------------------------
vtkSMProxy *RectangularRegion::getProxy()
{
  if (!m_box)
  {
    pqObjectBuilder *builder =  pqApplicationCore::instance()->getObjectBuilder();
    m_box =  builder->createProxy("implicit_functions","NonRotatingBox",pqApplicationCore::instance()->getActiveServer(),"widgets");
  }
  return m_box;
}

//----------------------------------------------------------------------------
pq3DWidget* RectangularRegion::createWidget()
{
  QList<pq3DWidget *> widgets =  pq3DWidget::createWidgets(getProxy(), getProxy());

  Q_ASSERT(widgets.size() == 1);
  // By default ParaView doesn't "Apply" the changes to the widget. So we set
  // up a slot to "Apply" when the interaction ends.
  QObject::connect(widgets[0], SIGNAL(widgetEndInteraction()),
		   widgets[0], SLOT(accept()));
//   QObject::connect(widgets[0], SIGNAL(widgetEndInteraction()),
// 		   this, SLOT(modifyVOI()));

  vtkAbstractWidget *widget = widgets[0]->getWidgetProxy()->GetWidget();
  vtkNonRotatingBoxWidget *boxwidget = dynamic_cast<vtkNonRotatingBoxWidget*>(widget);
  Q_ASSERT(boxwidget);

  vtkBoxRepresentation *repbox =  dynamic_cast<vtkBoxRepresentation*>(boxwidget->GetRepresentation());
  repbox->HandlesOff();
  repbox->OutlineCursorWiresOff();
  vtkProperty *outline = repbox->GetOutlineProperty();
  outline->SetColor(1.0,1.0,0);

  m_widgets << widgets;

  return widgets[0];

}

//----------------------------------------------------------------------------
pq3DWidget* RectangularRegion::createSliceWidget(vtkPVSliceView::VIEW_PLANE plane)
{
  QList<pq3DWidget *> widgets =  pq3DWidget::createWidgets(getProxy(), getProxy());

  Q_ASSERT(widgets.size() == 1);
  // By default ParaView doesn't "Apply" the changes to the widget. So we set
  // up a slot to "Apply" when the interaction ends.
  QObject::connect(widgets[0], SIGNAL(widgetEndInteraction()),
		   widgets[0], SLOT(accept()));
//   QObject::connect(widgets[0], SIGNAL(widgetEndInteraction()),
// 		   this, SLOT(modifyVOI()));

//   m_widgets
//   Widget newWidget;
//   newWidget.widget = widgets[0];
//   newWidget.viewType = viewType;
//   m_widgets.push_back(newWidget);

  vtkAbstractWidget *widget = widgets[0]->getWidgetProxy()->GetWidget();
  vtkNonRotatingBoxWidget *boxwidget = dynamic_cast<vtkNonRotatingBoxWidget*>(widget);
  Q_ASSERT(boxwidget);

  if (plane == vtkPVSliceView::SAGITTAL)
    boxwidget->SetInvertZCursor(true);
//   if (plane == vtkPVSliceView::3D)
//     boxwidget->SetProcessEvents(false);

  vtkBoxRepresentation *repbox =  dynamic_cast<vtkBoxRepresentation*>(boxwidget->GetRepresentation());
  repbox->HandlesOff();
  repbox->OutlineCursorWiresOff();
  vtkProperty *outline = repbox->GetOutlineProperty();
  outline->SetColor(1.0,1.0,0);

  m_widgets << widgets;

  return widgets[0];
}

//----------------------------------------------------------------------------
void RectangularRegion::setEnabled(bool enable)
{
  foreach(pq3DWidget *widget, m_widgets)
  {
    vtkAbstractWidget *basewidget = widget->getWidgetProxy()->GetWidget();
    vtkNonRotatingBoxWidget *boxwidget = dynamic_cast<vtkNonRotatingBoxWidget*>(basewidget);
    Q_ASSERT(boxwidget);
    boxwidget->SetProcessEvents(enable);
    vtkBoxRepresentation *repbox =  dynamic_cast<vtkBoxRepresentation*>(boxwidget->GetRepresentation());
    repbox->SetPickable(enable);
  }
}

//----------------------------------------------------------------------------
void RectangularRegion::setBounds(double bounds[6])
{
  vtkSMProxy *proxy = getProxy();
  vtkSMPropertyHelper(proxy,"Bounds").Set(bounds,6);
  double pos[3] = {0, 0, 0};
  vtkSMPropertyHelper(proxy, "Position").Set(pos,3);
  double scale[3] = {1, 1, 1};
  vtkSMPropertyHelper(proxy, "Scale").Set(scale,3);
  proxy->UpdateVTKObjects();
}

//----------------------------------------------------------------------------
void RectangularRegion::bounds(double bounds[6])
{
  vtkSMProxy *proxy = getProxy();
  double pos[3];
  vtkSMPropertyHelper(proxy, "Position").Get(pos,3);
  double scale[3];
  vtkSMPropertyHelper(proxy, "Scale").Get(scale,3);
  vtkSMPropertyHelper(proxy, "Bounds").Get(bounds,6);
  for (int i = 0; i < 6; i++)
    bounds[i] = pos[i/2] + bounds[i]*scale[i/2];
}

