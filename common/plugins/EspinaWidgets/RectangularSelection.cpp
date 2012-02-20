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

//----------------------------------------------------------------------------
RectangularRegion::RectangularRegion()
: m_box(NULL)
{

}

//----------------------------------------------------------------------------
pq3DWidget* RectangularRegion::createWidget(vtkPVSliceView::VIEW_PLANE plane)
{
  qDebug() << "Creating Widget";
  pqObjectBuilder *builder =  pqApplicationCore::instance()->getObjectBuilder();
  vtkSMProxy *refBox =  builder->createProxy("implicit_functions","NonRotatingBox",pqApplicationCore::instance()->getActiveServer(),"widgets");
  Q_ASSERT(refBox);
  refBox->UpdateSelfAndAllInputs();
  qDebug() << "Reference created";
  QList<pq3DWidget *> widgets =  pq3DWidget::createWidgets(refBox, getProxy());

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

  vtkNonRotatingBoxWidget *boxwidget = dynamic_cast<vtkNonRotatingBoxWidget*>(widgets[0]->getWidgetProxy()->GetWidget());
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

  return widgets[0];
}

vtkSMProxy *RectangularRegion::getProxy()
{
  if (!m_box)
  {
    pqObjectBuilder *builder =  pqApplicationCore::instance()->getObjectBuilder();
    m_box =  builder->createProxy("implicit_functions","NonRotatingBox",pqApplicationCore::instance()->getActiveServer(),"widgets");
  }
  return m_box;
}
