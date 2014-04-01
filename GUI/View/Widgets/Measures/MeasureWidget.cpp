/*
 * MeasureWidget.cpp
 *
 *  Created on: Dec 11, 2012
 *      Author: Felix de las Pozas Alvarez
 */

// EspINA
#include "MeasureWidget.h"
#include <GUI/View/Widgets/EspinaInteractorAdapter.h>
#include <Core/EspinaTypes.h>
#include <GUI/View/RenderView.h>
#include <GUI/View/View2D.h>
#include <GUI/View/Widgets/EspinaWidget.h>

//vtk
#include <vtkDistanceWidget.h>
#include <vtkDistanceRepresentation2D.h>
#include <vtkProperty2D.h>
#include <vtkPointHandleRepresentation2D.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>

// Qt
#include <QEvent>
#include <QMouseEvent>
#include <QKeyEvent>

using namespace EspINA;

using  MeasureWidgetAdapter = EspinaInteractorAdapter<vtkDistanceWidget>;

//----------------------------------------------------------------------------
MeasureWidget::MeasureWidget()
{
}

//----------------------------------------------------------------------------
MeasureWidget::~MeasureWidget()
{
  for(auto view: m_widgets.keys())
    unregisterView(view);

  m_cameras.clear();
  m_widgets.clear();
}

//----------------------------------------------------------------------------
void MeasureWidget::registerView(RenderView *view)
{
  if (m_widgets.keys().contains(view))
    return;

  View2D* view2d = dynamic_cast<View2D *>(view);

  if (!view2d)
    return;

  auto widget = MeasureWidgetAdapter::New();
  widget->AddObserver(vtkCommand::StartInteractionEvent, this);
  widget->AddObserver(vtkCommand::InteractionEvent, this);
  m_widgets.insert(view2d, widget);

  widget->SetCurrentRenderer(view2d->mainRenderer());
  widget->SetInteractor(view2d->renderWindow()->GetInteractor());
  widget->CreateDefaultRepresentation();
  widget->On();
}

//----------------------------------------------------------------------------
void MeasureWidget::unregisterView(RenderView *view)
{
  if (!m_widgets.keys().contains(view))
    return;

  auto widget = m_widgets[view];

  for(auto camera: m_cameras[widget])
    camera->RemoveObserver(this);

  m_cameras.remove(widget);

  widget->SetEnabled(false);
  widget->RemoveObserver(this);
  widget->SetCurrentRenderer(nullptr);
  widget->SetInteractor(nullptr);
  widget->Delete();

  m_widgets.remove(view);
}

//----------------------------------------------------------------------------
bool MeasureWidget::processEvent(vtkRenderWindowInteractor *iren,
                                 long unsigned int event)
{
  for(auto widget: m_widgets.values())
    if (widget->GetInteractor() == iren)
    {
      MeasureWidgetAdapter *sw = static_cast<MeasureWidgetAdapter *>(widget);
      return sw->ProcessEventsHandler(event);
    }

  return false;
}

//----------------------------------------------------------------------------
void MeasureWidget::setEnabled(bool enable)
{
  for(vtkDistanceWidget *widget: m_cameras.keys())
    widget->SetEnabled(enable);
}

//----------------------------------------------------------------------------
void MeasureWidget::Execute(vtkObject *caller, unsigned long int eventId, void* callData)
{
  vtkDistanceWidget *widget = nullptr;
  vtkDistanceRepresentation2D *rep = nullptr;
  vtkCamera *camera = nullptr;

  if (strcmp("vtkOpenGLCamera", caller->GetClassName()) == 0)
    camera = reinterpret_cast<vtkCamera*>(caller);

  if (strcmp("vtkDistanceWidget", caller->GetClassName()) == 0)
  {
    widget = reinterpret_cast<vtkDistanceWidget*>(caller);
    rep = reinterpret_cast<vtkDistanceRepresentation2D*>(widget->GetRepresentation());
  }

  if (widget != nullptr)
  {
    if (vtkCommand::StartInteractionEvent == eventId)
    {
      vtkRendererCollection *renderers = widget->GetInteractor()->GetRenderWindow()->GetRenderers();
      vtkRenderer *renderer = renderers->GetFirstRenderer();

      for(auto eWidget: m_widgets.values())
      {
        if(eWidget == widget)
        {
          if(m_cameras[eWidget].empty())
          {
            vtkCamera *camera = renderer->GetActiveCamera();
            camera->AddObserver(vtkCommand::AnyEvent, this);
            m_cameras[eWidget] << camera;
            renderer = renderers->GetNextItem();
          }
        }
      }

      rep->SetLabelFormat("%.1f nm");
      rep->RulerModeOn();
      rep->SetRulerDistance(ComputeRulerTickDistance(rep->GetDistance()));

      vtkProperty2D *property = reinterpret_cast<vtkPointHandleRepresentation2D*>(rep->GetPoint1Representation())->GetProperty();
      property->SetColor(0.0, 1.0, 0.0);
      property->SetLineWidth(2);

      property = reinterpret_cast<vtkPointHandleRepresentation2D*>(rep->GetPoint2Representation())->GetProperty();
      property->SetColor(0.0, 1.0, 0.0);
      property->SetLineWidth(2);
    }
    else if (vtkCommand::InteractionEvent == eventId)
    {
      double newTickDistance = ComputeRulerTickDistance(rep->GetDistance());
      if (rep->GetRulerDistance() != newTickDistance)
      {
        rep->SetRulerDistance(newTickDistance);
        rep->BuildRepresentation();
      }
    }
    return;
  }

  if (camera != nullptr)
  {
    double p1[3], p2[3];
    for(vtkDistanceWidget *eWidget: m_cameras.keys())
      if(m_cameras[eWidget].contains(camera))
      {
        eWidget->GetDistanceRepresentation()->GetPoint1WorldPosition(p1);
        eWidget->GetDistanceRepresentation()->SetPoint1WorldPosition(p1);
        eWidget->GetDistanceRepresentation()->GetPoint2WorldPosition(p2);
        eWidget->GetDistanceRepresentation()->SetPoint2WorldPosition(p2);
        eWidget->GetDistanceRepresentation()->BuildRepresentation();
      }
  }
}

//----------------------------------------------------------------------------
bool MeasureWidget::filterEvent(QEvent *e, RenderView *view)
{
  if (e->type() == QEvent::KeyPress)
  {
    QKeyEvent *ke = reinterpret_cast<QKeyEvent*>(e);
    if (ke->key() == Qt::Key_Backspace)
    {
      for(auto widget: m_cameras.keys())
      {
        widget->SetWidgetStateToStart();
        widget->GetDistanceRepresentation()->VisibilityOff();
        widget->GetDistanceRepresentation()->GetPoint1Representation()->VisibilityOff();
        widget->GetDistanceRepresentation()->GetPoint2Representation()->VisibilityOff();
        widget->Render();
      }
    }
    return false;
  }

  if (!m_widgets.contains(view))
    return false;

  if (QEvent::MouseButtonPress != e->type() &&
      QEvent::MouseButtonRelease != e->type() &&
      QEvent::MouseMove != e->type())
  {
    return false;
  }

  QMouseEvent *me = static_cast<QMouseEvent *>(e);

  // give interactor the event information
  vtkRenderWindowInteractor *iren = view->renderWindow()->GetInteractor();

  int oldPos[2];
  iren->GetEventPosition(oldPos);
  iren->SetEventInformationFlipY(me->x(), me->y(), (me->modifiers() & Qt::ControlModifier) > 0 ? 1 : 0,
      (me->modifiers() & Qt::ShiftModifier) > 0 ? 1 : 0, 0, me->type() == QEvent::MouseButtonDblClick ? 1 : 0);
  long unsigned int eventId = 0;

  const QEvent::Type t = e->type();
  if (t == QEvent::MouseMove)
  {
    eventId = vtkCommand::MouseMoveEvent;
  }
  else
    if (t == QEvent::MouseButtonPress || t == QEvent::MouseButtonDblClick)
    {
      switch (me->button())
      {
        case Qt::LeftButton:
          eventId = vtkCommand::LeftButtonPressEvent;
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
    else
      if (t == QEvent::MouseButtonRelease)
      {
        switch (me->button())
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

//----------------------------------------------------------------------------
double MeasureWidget::ComputeRulerTickDistance(double distance)
{
  double result = 1.0;

  while (distance >= 10)
  {
    result *= 10.0;
    distance /= 10.0;
  }

  return result;
}

//----------------------------------------------------------------------------
void MeasureWidget::setInUse(bool value)
{
  if(m_inUse == value)
    return;

  m_inUse = value;

  emit eventHandlerInUse(value);
}

