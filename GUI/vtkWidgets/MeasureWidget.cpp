/*
 * MeasureWidget.cpp
 *
 *  Created on: Dec 11, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#include "MeasureWidget.h"
#include "MeasureSliceWidget.h"
#include "EspinaInteractorAdapter.h"
#include <GUI/ViewManager.h>
#include <Core/EspinaTypes.h>
#include <GUI/QtWidget/EspinaRenderView.h>
#include <GUI/QtWidget/SliceView.h>
#include <GUI/vtkWidgets/EspinaWidget.h>

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
: m_axial(NULL)
, m_coronal(NULL)
, m_sagittal(NULL)
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

  if (m_axial)
  {
    m_axial->RemoveObserver(this);
    m_axial->SetEnabled(false);
    m_axial->Delete();
  }

  if (m_coronal)
  {
    m_coronal->RemoveObserver(this);
    m_coronal->SetEnabled(false);
    m_coronal->Delete();
  }

  if (m_sagittal)
  {
    m_sagittal->RemoveObserver(this);
    m_sagittal->SetEnabled(false);
    m_sagittal->Delete();
  }

  foreach(vtkCamera *camera, m_axialCameras)
    camera->RemoveObserver(this);

  foreach(vtkCamera *camera, m_coronalCameras)
    camera->RemoveObserver(this);

  foreach(vtkCamera *camera, m_sagittalCameras)
    camera->RemoveObserver(this);
}

//----------------------------------------------------------------------------
vtkAbstractWidget *MeasureWidget::createWidget()
{
  return NULL;
}

//----------------------------------------------------------------------------
void MeasureWidget::deleteWidget(vtkAbstractWidget *widget)
{
  Q_ASSERT(false);
}

//----------------------------------------------------------------------------
SliceWidget *MeasureWidget::createSliceWidget(PlaneType plane)
{
  MeasureSliceWidget *widget = NULL;

  switch(plane)
  {
    case AXIAL:
      m_axial = MeasureWidgetAdapter::New();
      m_axial->AddObserver(vtkCommand::StartInteractionEvent, this);
      m_axial->AddObserver(vtkCommand::InteractionEvent, this);
      m_distanceWidgets << m_axial;
      widget = new MeasureSliceWidget(m_axial);
      break;
    case CORONAL:
      m_coronal = MeasureWidgetAdapter::New();
      m_coronal->AddObserver(vtkCommand::StartInteractionEvent, this);
      m_coronal->AddObserver(vtkCommand::InteractionEvent, this);
      m_distanceWidgets << m_coronal;
      widget = new MeasureSliceWidget(m_coronal);
      break;
    case SAGITTAL:
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
  foreach(vtkDistanceWidget *widget, m_distanceWidgets)
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
  foreach(vtkDistanceWidget *widget, m_distanceWidgets)
    widget->SetEnabled(enable);

  foreach(MeasureSliceWidget *widget, m_sliceWidgets)
    widget->SetEnabled(enable);
}

//----------------------------------------------------------------------------
void MeasureWidget::Execute(vtkObject *caller, unsigned long int eventId, void* callData)
{
  vtkDistanceWidget *widget = NULL;
  vtkDistanceRepresentation2D *rep = NULL;
  vtkCamera *camera = NULL;

  if (strcmp("vtkOpenGLCamera", caller->GetClassName()) == 0)
    camera = reinterpret_cast<vtkCamera*>(caller);

  if (strcmp("vtkDistanceWidget", caller->GetClassName()) == 0)
  {
    widget = reinterpret_cast<vtkDistanceWidget*>(caller);
    rep = reinterpret_cast<vtkDistanceRepresentation2D*>(widget->GetRepresentation());
  }

  if (widget != NULL)
  {
    if (vtkCommand::StartInteractionEvent == eventId)
    {
      vtkRendererCollection *renderers = widget->GetInteractor()->GetRenderWindow()->GetRenderers();
      vtkRenderer *renderer = renderers->GetFirstRenderer();

      if ((widget == m_axial) && (m_axialCameras.empty()))
        while (renderer != NULL)
        {
          vtkCamera *camera = renderer->GetActiveCamera();
          camera->AddObserver(vtkCommand::AnyEvent, this);
          m_axialCameras << camera;
          renderer = renderers->GetNextItem();
        }

      if ((widget == m_coronal) && (m_coronalCameras.empty()))
        while (renderer != NULL)
        {
          vtkCamera *camera = renderer->GetActiveCamera();
          camera->AddObserver(vtkCommand::AnyEvent, this);
          m_coronalCameras << camera;
          renderer = renderers->GetNextItem();
        }

      if ((widget == m_sagittal) && (m_sagittalCameras.empty()))
        while (renderer != NULL)
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

  if (camera != NULL)
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
bool MeasureWidget::filterEvent(QEvent *e, EspinaRenderView *view)
{
  if (e->type() == QEvent::KeyPress)
  {
    QKeyEvent *ke = reinterpret_cast<QKeyEvent*>(e);
    if (ke->key() == Qt::Key_Backspace)
    {
      foreach(vtkDistanceWidget *widget, m_distanceWidgets)
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
