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

#include <vtkBoxRepresentation.h>
#include <vtkProperty.h>
#include "vtkRectangularWidget.h"
#include "vtkRectangularRepresentation.h"

#include <QDebug>

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
  foreach(vtkAbstractWidget *widget, m_widgets)
  {
    widget->Off();
  }
  m_widgets.clear();
}


//----------------------------------------------------------------------------
vtkAbstractWidget* RectangularRegion::createWidget()
{
  return NULL;
}
//----------------------------------------------------------------------------
void RectangularRegion::deleteWidget(vtkAbstractWidget* widget)
{
  Q_ASSERT(false);
}


//----------------------------------------------------------------------------
SliceWidget* RectangularRegion::createSliceWidget(PlaneType plane)
{
  vtkRectangularWidget *widget = vtkRectangularWidget::New();
  Q_ASSERT(widget);
  widget->SetPlane(plane);

  m_widgets << widget;

  return new SliceWidget(widget);
}

//----------------------------------------------------------------------------
void RectangularRegion::setEnabled(bool enable)
{
  vtkAbstractWidget *widget;
  foreach(widget, m_widgets)
  {
    widget->SetProcessEvents(enable);
    widget->GetRepresentation()->SetPickable(enable);
  }
}

//----------------------------------------------------------------------------
void RectangularRegion::setBounds(Nm bounds[6])
{
  foreach(vtkAbstractWidget *widget, m_widgets)
  {
    widget->GetRepresentation()->PlaceWidget(bounds);
  }
}

//----------------------------------------------------------------------------
void RectangularRegion::bounds(Nm bounds[6])
{
  Q_ASSERT(!m_widgets.isEmpty());

  vtkAbstractWidget *widget = m_widgets[0];

  memcpy(bounds, widget->GetRepresentation()->GetBounds(), 6*sizeof(double));
}

// //----------------------------------------------------------------------------
// pq3DWidget* RectangularRegion::createWidget(QString name)
// {
//   vtkPVXMLElement* hints = getProxy()->GetHints();
//   Q_ASSERT(hints->GetNumberOfNestedElements() == 1);
//   vtkPVXMLElement* element = hints->GetNestedElement(0);
//   QList<pq3DWidgetInterface *> interfaces =
//   pqApplicationCore::instance()->interfaceTracker()->interfaces<pq3DWidgetInterface*>();
//   pq3DWidget *widget = 0;
// 
//   // Create the widget from plugins.
//   foreach (pq3DWidgetInterface* iface, interfaces)
//   {
//     widget = iface->newWidget(name, getProxy(), getProxy());
//     if (widget)
//     {
//       widget->setHints(element);
//       return widget;
//     }
//   }
//   return NULL;
// }
// 