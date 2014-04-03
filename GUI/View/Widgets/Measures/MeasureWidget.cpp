/*
 * MeasureWidget.cpp
 *
 *  Created on: Dec 11, 2012
 *      Author: Felix de las Pozas Alvarez
 */

// EspINA
#include "MeasureWidget.h"
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

//----------------------------------------------------------------------------
MeasureWidget::MeasureWidget()
: m_command{vtkSmartPointer<vtkDistanceCommand>::New()}
{
  m_command->setWidget(this);
}

//----------------------------------------------------------------------------
MeasureWidget::~MeasureWidget()
{
  for(auto view: m_widgets.keys())
    unregisterView(view);

  m_cameras.clear();
  m_widgets.clear();
  m_command = nullptr;
}

//----------------------------------------------------------------------------
void MeasureWidget::registerView(RenderView *view)
{
  auto view2d = dynamic_cast<View2D *>(view);

  if (!view2d || m_widgets.keys().contains(view))
    return;

  auto widget = vtkDistanceWidget::New();
  widget->AddObserver(vtkCommand::StartInteractionEvent, m_command);
  widget->AddObserver(vtkCommand::InteractionEvent, m_command);
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
    camera->RemoveObserver(m_command);

  m_cameras.remove(widget);

  widget->Off();
  view->mainRenderer()->RemoveActor(widget->GetRepresentation());
  widget->RemoveObserver(m_command);
  widget->SetCurrentRenderer(nullptr);
  widget->SetInteractor(nullptr);
  widget->Delete();

  m_widgets.remove(view);
}

//----------------------------------------------------------------------------
void MeasureWidget::setEnabled(bool enable)
{
  for(vtkDistanceWidget *widget: m_cameras.keys())
    widget->SetEnabled(enable);
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
  }

  return false;
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

//----------------------------------------------------------------------------
void vtkDistanceCommand::Execute(vtkObject *caller, unsigned long int eventId, void* callData)
{
  MeasureWidget *eWidget = dynamic_cast<MeasureWidget *>(m_widget);

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

      for(auto dWidget: eWidget->m_widgets.values())
      {
        if(dWidget == widget)
        {
          if(eWidget->m_cameras[dWidget].empty())
          {
            vtkCamera *camera = renderer->GetActiveCamera();
            camera->AddObserver(vtkCommand::AnyEvent, this);
            eWidget->m_cameras[dWidget] << camera;
            renderer = renderers->GetNextItem();
          }
        }
      }

      rep->SetLabelFormat("%.1f nm");
      rep->RulerModeOn();
      rep->SetRulerDistance(eWidget->ComputeRulerTickDistance(rep->GetDistance()));

      vtkProperty2D *property = reinterpret_cast<vtkPointHandleRepresentation2D*>(rep->GetPoint1Representation())->GetProperty();
      property->SetColor(0.0, 1.0, 0.0);
      property->SetLineWidth(2);

      property = reinterpret_cast<vtkPointHandleRepresentation2D*>(rep->GetPoint2Representation())->GetProperty();
      property->SetColor(0.0, 1.0, 0.0);
      property->SetLineWidth(2);
    }
    else if (vtkCommand::InteractionEvent == eventId)
    {
      double newTickDistance = eWidget->ComputeRulerTickDistance(rep->GetDistance());
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
    for(vtkDistanceWidget *dWidget: eWidget->m_cameras.keys())
      if(eWidget->m_cameras[dWidget].contains(camera))
      {
        dWidget->GetDistanceRepresentation()->GetPoint1WorldPosition(p1);
        dWidget->GetDistanceRepresentation()->SetPoint1WorldPosition(p1);
        dWidget->GetDistanceRepresentation()->GetPoint2WorldPosition(p2);
        dWidget->GetDistanceRepresentation()->SetPoint2WorldPosition(p2);
        dWidget->GetDistanceRepresentation()->BuildRepresentation();
      }
  }
}

