/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include "ZoomSelectionWidget.h"
#include "vtkZoomSelectionWidget.h"
#include <GUI/View/RenderView.h>
#include "vtkZoomSelectionWidget.h"
#include <GUI/View/View2D.h>

// vtk
#include <vtkAbstractWidget.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkWidgetRepresentation.h>
#include <vtkCommand.h>
#include <vtkRenderWindowInteractor.h>

// Qt
#include <QMouseEvent>
#include <QEvent>

using namespace ESPINA;

//----------------------------------------------------------------------------
ZoomSelectionWidget::ZoomSelectionWidget()
: m_command{vtkSmartPointer<vtkZoomCommand>::New()}
{
  m_command->setWidget(this);

  QPixmap cursorBitmap;
  cursorBitmap.load(":/espina/zoom_cursor.png", "PNG", Qt::ColorOnly);
  setCursor(QCursor(cursorBitmap, 0, 0));
}

//----------------------------------------------------------------------------
ZoomSelectionWidget::~ZoomSelectionWidget()
{
  for(ZoomSelectionWidgetAdapter *widget: m_views.values())
  {
    widget->SetEnabled(false);
    widget->RemoveObserver(m_command);
    widget->SetCurrentRenderer(nullptr);
    widget->SetInteractor(nullptr);
    widget->Delete();
  }
}

//----------------------------------------------------------------------------
void ZoomSelectionWidget::registerView(RenderView *view)
{
  if (m_views.keys().contains(view))
    return;

  View2D *view2d = dynamic_cast<View2D *>(view);
  if (view2d == nullptr)
    return;

  ZoomSelectionWidgetAdapter *widget = ZoomSelectionWidgetAdapter::New();
  Q_ASSERT(widget);

  widget->AddObserver(vtkCommand::EndInteractionEvent, m_command);

  switch (view2d->plane())
  {
    case Plane::XY:
      widget->SetWidgetType(vtkZoomSelectionWidget::AXIAL_WIDGET);
      break;
    case Plane::XZ:
      widget->SetWidgetType(vtkZoomSelectionWidget::CORONAL_WIDGET);
      break;
    case Plane::YZ:
      widget->SetWidgetType(vtkZoomSelectionWidget::SAGITTAL_WIDGET);
      break;
    default:
      Q_ASSERT(false);
      break;
  }

  widget->SetCurrentRenderer(view->mainRenderer());
  widget->SetInteractor(view2d->renderWindow()->GetInteractor());
  widget->On();

  m_views.insert(view, widget);
}

//----------------------------------------------------------------------------
void ZoomSelectionWidget::unregisterView(RenderView *view)
{
  if (!m_views.keys().contains(view)) return;

  m_views[view]->SetEnabled(false);
  m_views[view]->SetCurrentRenderer(nullptr);
  m_views[view]->SetInteractor(nullptr);
  m_views[view]->Delete();

  m_views.remove(view);
}

//----------------------------------------------------------------------------
void ZoomSelectionWidget::setEnabled(bool enable)
{
  for(auto widget: m_views.values())
    widget->SetEnabled(enable);
}

//----------------------------------------------------------------------------
bool ZoomSelectionWidget::filterEvent(QEvent *e, RenderView *view)
{
  if (e->type() != QEvent::MouseMove && e->type() != QEvent::MouseButtonPress && e->type() != QEvent::MouseButtonRelease)
    return false;

  if (isInUse() && m_views.contains(view))
    return m_views[view]->ProcessBasicQtEvent(e);

  return false;
}

//----------------------------------------------------------------------------
void vtkZoomCommand::Execute(vtkObject *caller, unsigned long int eventId, void* callData)
{
  // this is needed to update the thumbnail when zooming the view.
  ZoomSelectionWidgetAdapter *widget = dynamic_cast<ZoomSelectionWidgetAdapter *>(caller);
  if(!widget)
    return;

  if(!m_widget->m_views.values().contains(widget))
    return;

  m_widget->m_views.key(widget)->refresh();
}
