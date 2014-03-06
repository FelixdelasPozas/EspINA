/*
 * MeasureWidget.cpp
 *
 *  Created on: Dec 11, 2012
 *      Author: Félix de las Pozas Álvarez
 */

// EspINA
#include "MeasureWidget.h"
#include "MeasureSliceWidget.h"
#include <GUI/View/Widgets/EspinaInteractorAdapter.h>
#include <Support/ViewManager.h>
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

typedef EspinaInteractorAdapter<vtkDistanceWidget> MeasureWidgetAdapter;

//----------------------------------------------------------------------------
MeasureWidget::MeasureWidget()
: m_axial(nullptr)
, m_coronal(nullptr)
, m_sagittal(nullptr)
{
}

//----------------------------------------------------------------------------
MeasureWidget::~MeasureWidget()
{
  foreach(MeasureSliceWidget *widget, m_sliceWidgets)
  {
    widget->SetEnabled(false);
    delete widget;
  }
  m_sliceWidgets.clear();

  if (m_axial)
  {
    m_axial->RemoveObserver(this);
    m_axial->SetEnabled(false);
    m_axial->Delete();
    m_axial = nullptr;
  }

  if (m_coronal)
  {
    m_coronal->RemoveObserver(this);
    m_coronal->SetEnabled(false);
    m_coronal->Delete();
    m_coronal = nullptr;
  }

  if (m_sagittal)
  {
    m_sagittal->RemoveObserver(this);
    m_sagittal->SetEnabled(false);
    m_sagittal->Delete();
    m_sagittal = nullptr;
  }
}

//----------------------------------------------------------------------------
vtkAbstractWidget *MeasureWidget::create3DWidget(View3D *view)
{
  return nullptr;
}

//----------------------------------------------------------------------------
SliceWidget *MeasureWidget::createSliceWidget(View2D *view)
{
  MeasureSliceWidget *widget = nullptr;

  switch(view->plane())
  {
    case Plane::XY:
      m_axial = MeasureWidgetAdapter::New();
      m_axial->AddObserver(vtkCommand::StartInteractionEvent, this);
      m_axial->AddObserver(vtkCommand::InteractionEvent, this);
      m_distanceWidgets << m_axial;
      widget = new MeasureSliceWidget(m_axial);
      break;
    case Plane::XZ:
      m_coronal = MeasureWidgetAdapter::New();
      m_coronal->AddObserver(vtkCommand::StartInteractionEvent, this);
      m_coronal->AddObserver(vtkCommand::InteractionEvent, this);
      m_distanceWidgets << m_coronal;
      widget = new MeasureSliceWidget(m_coronal);
      break;
    case Plane::YZ:
      m_sagittal = MeasureWidgetAdapter::New();
      m_sagittal->AddObserver(vtkCommand::StartInteractionEvent, this);
      m_sagittal->AddObserver(vtkCommand::InteractionEvent, this);
      m_distanceWidgets << m_sagittal;
      widget = new MeasureSliceWidget(m_sagittal);
      break;
    default:
      Q_ASSERT(false);
      break;
  }

  m_sliceWidgets << widget;
  return widget;
}

//----------------------------------------------------------------------------
bool MeasureWidget::processEvent(vtkRenderWindowInteractor *iren,
                                 long unsigned int event)
{
  for(auto widget: m_distanceWidgets)
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
  for(auto widget: m_distanceWidgets)
    widget->SetEnabled(enable);

  for(auto widget: m_sliceWidgets)
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

      if ((widget == m_axial) && (m_axialCameras.empty()))
        while (renderer != nullptr)
        {
          vtkCamera *camera = renderer->GetActiveCamera();
          camera->AddObserver(vtkCommand::AnyEvent, this);
          m_axialCameras << camera;
          renderer = renderers->GetNextItem();
        }

      if ((widget == m_coronal) && (m_coronalCameras.empty()))
        while (renderer != nullptr)
        {
          vtkCamera *camera = renderer->GetActiveCamera();
          camera->AddObserver(vtkCommand::AnyEvent, this);
          m_coronalCameras << camera;
          renderer = renderers->GetNextItem();
        }

      if ((widget == m_sagittal) && (m_sagittalCameras.empty()))
        while (renderer != nullptr)
        {
          vtkCamera *camera = renderer->GetActiveCamera();
          camera->AddObserver(vtkCommand::AnyEvent, this);
          m_sagittalCameras << camera;
          renderer = renderers->GetNextItem();
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
    if (m_axialCameras.contains(camera))
    {
      m_axial->GetDistanceRepresentation()->GetPoint1WorldPosition(p1);
      m_axial->GetDistanceRepresentation()->SetPoint1WorldPosition(p1);
      m_axial->GetDistanceRepresentation()->GetPoint2WorldPosition(p2);
      m_axial->GetDistanceRepresentation()->SetPoint2WorldPosition(p2);
      m_axial->GetDistanceRepresentation()->BuildRepresentation();
    }
    else if (m_coronalCameras.contains(camera))
    {
      m_coronal->GetDistanceRepresentation()->GetPoint1WorldPosition(p1);
      m_coronal->GetDistanceRepresentation()->SetPoint1WorldPosition(p1);
      m_coronal->GetDistanceRepresentation()->GetPoint2WorldPosition(p2);
      m_coronal->GetDistanceRepresentation()->SetPoint2WorldPosition(p2);
      m_coronal->GetDistanceRepresentation()->BuildRepresentation();
    }
    else if (m_sagittalCameras.contains(camera))
    {
      m_sagittal->GetDistanceRepresentation()->GetPoint1WorldPosition(p1);
      m_sagittal->GetDistanceRepresentation()->SetPoint1WorldPosition(p1);
      m_sagittal->GetDistanceRepresentation()->GetPoint2WorldPosition(p2);
      m_sagittal->GetDistanceRepresentation()->SetPoint2WorldPosition(p2);
      m_sagittal->GetDistanceRepresentation()->BuildRepresentation();
    }
  }
}

// fixes a bug in vtkDistanceRepresentation that misplaces the handles when zooming the view
//----------------------------------------------------------------------------
bool MeasureWidget::filterEvent(QEvent *e, RenderView *view)
{
  if (e->type() == QEvent::KeyPress)
  {
    QKeyEvent *ke = reinterpret_cast<QKeyEvent*>(e);
    if (ke->key() == Qt::Key_Backspace)
    {
      for(auto widget: m_distanceWidgets)
      {
        widget->SetWidgetStateToStart();
        widget->GetDistanceRepresentation()->VisibilityOff();
        widget->GetDistanceRepresentation()->GetPoint1Representation()->VisibilityOff();
        widget->GetDistanceRepresentation()->GetPoint2Representation()->VisibilityOff();
        widget->Render();
      }

      m_axialCameras.clear();
      m_coronalCameras.clear();
      m_sagittalCameras.clear();
    }
  }

  return EspinaWidget::filterEvent(e, view);
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
