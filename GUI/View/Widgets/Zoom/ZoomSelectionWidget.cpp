/*
 * ZoomSelectionWidget.cpp
 *
 *  Created on: Nov 14, 2012
 *      Author: Felix de las Pozas Alvarez
 */

// EspINA
#include "ZoomSelectionWidget.h"
#include "vtkZoomSelectionWidget.h"
#include <GUI/View/Widgets/EspinaInteractorAdapter.h>
#include <GUI/View/RenderView.h>
#include "vtkZoomSelectionWidget.h"
#include <Support/ViewManager.h>
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

using namespace EspINA;

using ZoomSelectionWidgetAdapter = EspinaInteractorAdapter<vtkZoomSelectionWidget>;

//----------------------------------------------------------------------------
ZoomSelectionWidget::ZoomSelectionWidget()
{
}

//----------------------------------------------------------------------------
ZoomSelectionWidget::~ZoomSelectionWidget()
{
  for(vtkZoomSelectionWidget *widget: m_views.values())
  {
    widget->SetEnabled(false);
    widget->RemoveObserver(this);
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
  if (view2d)
  {
    auto widget = ZoomSelectionWidgetAdapter::New();
    widget->AddObserver(vtkCommand::EndInteractionEvent, this);

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

    widget->SetCurrentRenderer(view2d->renderWindow()->GetRenderers()->GetFirstRenderer());
    widget->SetInteractor(view2d->renderWindow()->GetInteractor());
    widget->SetEnabled(true);

    m_views.insert(view, widget);
  }
}

//----------------------------------------------------------------------------
void ZoomSelectionWidget::unregisterView(RenderView *view)
{
  if (!m_views.keys().contains(view))
    return;

  m_views[view]->SetEnabled(false);
  m_views[view]->SetCurrentRenderer(nullptr);
  m_views[view]->SetInteractor(nullptr);
  m_views[view]->Delete();

  m_views.remove(view);
}

//----------------------------------------------------------------------------
bool ZoomSelectionWidget::processEvent(vtkRenderWindowInteractor *iren,
                          long unsigned int event)
{
  for(auto widget: m_views.values())
    if (widget->GetInteractor() == iren)
    {
      ZoomSelectionWidgetAdapter *sw = static_cast<ZoomSelectionWidgetAdapter *>(widget);
      return sw->ProcessEventsHandler(event);
    }

  return false;
}

//----------------------------------------------------------------------------
void ZoomSelectionWidget::setEnabled(bool enable)
{
  for(auto widget: m_views.values())
    widget->SetEnabled(enable);
}

//----------------------------------------------------------------------------
void ZoomSelectionWidget::Execute(vtkObject *caller, unsigned long int eventId, void* callData)
{
  // this is needed to update the thumbnail when zooming the view.
  ZoomSelectionWidgetAdapter *widget = dynamic_cast<ZoomSelectionWidgetAdapter *>(caller);
  if(!widget)
    return;

  m_views.key(widget)->updateView();
}

//----------------------------------------------------------------------------
bool ZoomSelectionWidget::filterEvent(QEvent* e, RenderView* view)
{
  if ( QEvent::MouseButtonPress != e->type()
    && QEvent::MouseButtonRelease != e->type()
    && QEvent::MouseMove != e->type() )
    return false;

  QMouseEvent *me = static_cast<QMouseEvent *>(e);

  // give interactor the event information
  vtkRenderWindowInteractor *iren = view->renderWindow()->GetInteractor();

  int oldPos[2];
  iren->GetEventPosition(oldPos);
  iren->SetEventInformationFlipY(me->x(), me->y(),
                                 (me->modifiers() & Qt::ControlModifier) > 0 ? 1 : 0,
                                 (me->modifiers() & Qt::ShiftModifier ) > 0 ? 1 : 0,
                                 0,
                                 me->type() == QEvent::MouseButtonDblClick ? 1 : 0);
  long unsigned int eventId = 0;

  const QEvent::Type t = e->type();
  if(t == QEvent::MouseMove)
  {
    eventId = vtkCommand::MouseMoveEvent;
  }
  else if(t == QEvent::MouseButtonPress || t == QEvent::MouseButtonDblClick)
  {
    switch(me->button())
    {
      case Qt::LeftButton:
        eventId =vtkCommand::LeftButtonPressEvent;
        break;

      case Qt::MidButton:
        eventId = vtkCommand::MiddleButtonPressEvent;
        break;

      case Qt::RightButton:
        eventId = vtkCommand::RightButtonPressEvent;
        break;

      default:
        break;
    }
  }
  else if(t == QEvent::MouseButtonRelease)
  {
    switch(me->button())
    {
      case Qt::LeftButton:
        eventId = vtkCommand::LeftButtonReleaseEvent;
        break;

      case Qt::MidButton:
        eventId = vtkCommand::MiddleButtonReleaseEvent;
        break;

      case Qt::RightButton:
        eventId = vtkCommand::RightButtonReleaseEvent;
        break;

      default:
        break;
    }
  }

  bool handled = processEvent(iren, eventId);

  if (!handled)
    iren->SetEventPosition(oldPos);

  return handled;
}
