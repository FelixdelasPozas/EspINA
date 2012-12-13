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

// Qt
#include <QEvent>
#include <QMouseEvent>
#include <QKeyEvent>

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
  vtkDistanceWidget *widget = reinterpret_cast<vtkDistanceWidget*>(caller);
  vtkDistanceRepresentation2D *rep = reinterpret_cast<vtkDistanceRepresentation2D*>(widget->GetRepresentation());

  if (vtkCommand::StartInteractionEvent == eventId)
  {

    rep->SetLabelFormat("%.1f nm");
    rep->RulerModeOn();
    if (rep->GetDistance() < 100)
      rep->SetRulerDistance(1.0);

    vtkProperty2D *property = reinterpret_cast<vtkPointHandleRepresentation2D*>(rep->GetPoint1Representation())->GetProperty();
    property->SetColor(0.0, 1.0, 0.0);
    property->SetLineWidth(2);

    property = reinterpret_cast<vtkPointHandleRepresentation2D*>(rep->GetPoint2Representation())->GetProperty();
    property->SetColor(0.0, 1.0, 0.0);
    property->SetLineWidth(2);
  }
  else if (vtkCommand::InteractionEvent == eventId)
  {
    if (rep->GetDistance() > 100)
    {
      double units = 1;
      double distance = rep->GetDistance();
      while (distance >= 100)
      {
        units *= 10.0;
        distance /= 10.0;
      }
      rep->SetRulerDistance(units);
      rep->BuildRepresentation();
    }
    else
      rep->SetRulerDistance(1.0);
  }
}

// fixes a bug in vtkDistanceRepresentation that misplaces the handles when zooming the view
//----------------------------------------------------------------------------
bool MeasureWidget::filterEvent(QEvent *e, EspinaRenderView *view)
{
  static bool zooming = false;
  static double p1[3];
  static double p2[3];
  static vtkDistanceWidget *actual = NULL;

  if (e->type() == QEvent::MouseButtonPress)
  {
    QMouseEvent *me = reinterpret_cast<QMouseEvent*>(e);
    if (me->button() == Qt::RightButton)
    {
      Q_ASSERT(actual == NULL);
      SliceView *slice = reinterpret_cast<SliceView *>(view);
      switch(slice->getViewType())
      {
        case AXIAL:
          actual = m_axial;
          break;
        case CORONAL:
          actual = m_coronal;
          break;
        case SAGITTAL:
          actual = m_sagittal;
          break;
        default:
          Q_ASSERT(false);
          break;
      }

      zooming = true;
      actual->GetDistanceRepresentation()->GetPoint1WorldPosition(p1);
      actual->GetDistanceRepresentation()->GetPoint2WorldPosition(p2);
    }
  }
  else if (e->type() == QEvent::MouseButtonRelease)
  {
    QMouseEvent *me = reinterpret_cast<QMouseEvent*>(e);
    if (me->button() == Qt::RightButton)
    {
      zooming = false;
      actual->GetDistanceRepresentation()->SetPoint1WorldPosition(p1);
      actual->GetDistanceRepresentation()->SetPoint2WorldPosition(p2);
      actual->GetDistanceRepresentation()->BuildRepresentation();
      actual = NULL;
    }
  }
  else if (e->type() == QEvent::MouseMove && zooming == true)
  {
    actual->GetDistanceRepresentation()->SetPoint1WorldPosition(p1);
    actual->GetDistanceRepresentation()->SetPoint2WorldPosition(p2);
    actual->GetDistanceRepresentation()->BuildRepresentation();
  }
  else if (e->type() == QEvent::KeyPress)
  {
    QKeyEvent *ke = reinterpret_cast<QKeyEvent*>(e);
    if (ke->key() == Qt::Key_Backspace)
    {
      zooming = false;
      actual = NULL;
      foreach(vtkDistanceWidget *widget, m_distanceWidgets)
      {
        widget->SetWidgetStateToStart();
        widget->GetDistanceRepresentation()->VisibilityOff();
        widget->GetDistanceRepresentation()->GetPoint1Representation()->VisibilityOff();
        widget->GetDistanceRepresentation()->GetPoint2Representation()->VisibilityOff();
        widget->Render();
      }
    }
  }

  return EspinaWidget::filterEvent(e, view);
}
