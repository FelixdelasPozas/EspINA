/*
 * PlanarSplitWidget.cpp
 *
 *  Created on: Nov 5, 2012
 *      Author: Félix de las Pozas Álvarez
 */

// EspINA
#include "PlanarSplitWidget.h"
#include "PlanarSplitSliceWidget.h"
#include "common/widgets/EspinaInteractorAdapter.h"
#include "vtkPlanarSplitWidget.h"
#include "common/gui/ViewManager.h"

// vtk
#include <vtkAbstractWidget.h>
#include <vtkPoints.h>
#include <vtkPlane.h>
#include <vtkMath.h>

typedef EspinaInteractorAdapter<vtkPlanarSplitWidget> PlanarSplitWidgetAdapter;

//-----------------------------------------------------------------------------
PlanarSplitWidget::PlanarSplitWidget()
: m_axial(NULL)
, m_coronal(NULL)
, m_sagittal(NULL)
, m_mainWidget(NONE)
{
}

//-----------------------------------------------------------------------------
PlanarSplitWidget::~PlanarSplitWidget()
{
  if (m_mainWidget == NONE)
    foreach(vtkAbstractWidget *widget, m_widgets)
      widget->RemoveObserver(this);

  if (m_axial)
  {
    m_axial->setEnabled(false);
    delete m_axial;
  }

  if (m_coronal)
  {
    m_coronal->setEnabled(false);
    delete m_coronal;
  }

  if (m_sagittal)
  {
    m_sagittal->setEnabled(false);
    delete m_sagittal;
  }
}

//-----------------------------------------------------------------------------
vtkAbstractWidget *PlanarSplitWidget::createWidget()
{
  return NULL;
}

//-----------------------------------------------------------------------------
void PlanarSplitWidget::deleteWidget(vtkAbstractWidget *widget)
{
  Q_ASSERT(false);
}

//-----------------------------------------------------------------------------
SliceWidget *PlanarSplitWidget::createSliceWidget(PlaneType plane)
{
  switch(plane)
  {
    case AXIAL:
      if (!m_axial)
      {
        PlanarSplitWidgetAdapter *widget = new PlanarSplitWidgetAdapter();
        widget->AddObserver(vtkCommand::EndInteractionEvent, this);
        m_axial = new PlanarSplitSliceWidget(widget);
        m_axial->setOrientation(plane);
        m_widgets << widget;
      }
      return m_axial;
      break;
    case CORONAL:
      if (!m_coronal)
      {
        PlanarSplitWidgetAdapter *widget = new PlanarSplitWidgetAdapter();
        widget->AddObserver(vtkCommand::EndInteractionEvent, this);
        m_coronal = new PlanarSplitSliceWidget(widget);
        m_coronal->setOrientation(plane);
        m_widgets << widget;
      }
      return m_coronal;
      break;
    case SAGITTAL:
      if (!m_sagittal)
      {
        PlanarSplitWidgetAdapter *widget = new PlanarSplitWidgetAdapter();
        widget->AddObserver(vtkCommand::EndInteractionEvent, this);
        m_sagittal = new PlanarSplitSliceWidget(widget);
        m_sagittal->setOrientation(plane);
        m_widgets << widget;
      }
      return m_sagittal;
      break;
    default:
      Q_ASSERT(false);
      break;
  }

  return NULL; // dead code
}

//-----------------------------------------------------------------------------
bool PlanarSplitWidget::processEvent(vtkRenderWindowInteractor* iren,
                                 long unsigned int event)
{
  foreach(vtkAbstractWidget *widget, m_widgets)
    if (widget->GetInteractor() == iren)
    {
      PlanarSplitWidgetAdapter *sw = dynamic_cast<PlanarSplitWidgetAdapter *>(widget);
      return sw->ProcessEventsHandler(event);
    }

  return false;
}

//-----------------------------------------------------------------------------
void PlanarSplitWidget::setEnabled(bool enable)
{
  if (m_axial)
    m_axial->setEnabled(enable);

  if (m_coronal)
    m_coronal->setEnabled(enable);
  if (m_sagittal)
    m_sagittal->setEnabled(enable);
}

//-----------------------------------------------------------------------------
void PlanarSplitWidget::setPlanePoints(vtkSmartPointer<vtkPoints> points)
{
  if (m_axial)
    m_axial->setPoints(points);

  if (m_coronal)
    m_coronal->setPoints(points);

  if (m_sagittal)
    m_sagittal->setPoints(points);
}


//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPoints> PlanarSplitWidget::getPlanePoints()
{
  vtkSmartPointer<vtkPoints> points = NULL;

  switch(m_mainWidget)
  {
    case AXIAL_WIDGET:
      points = m_axial->getPoints();
      break;
    case CORONAL_WIDGET:
      points = m_coronal->getPoints();
      break;
    case SAGITTAL_WIDGET:
      points = m_sagittal->getPoints();
      break;
    default:
      break;
  }

  return points;
}

//-----------------------------------------------------------------------------
void PlanarSplitWidget::Execute(vtkObject *caller, unsigned long eventId, void *callData)
{
  PlanarSplitWidgetAdapter *widget = static_cast<PlanarSplitWidgetAdapter*>(caller);
  widget->RemoveObserver(this);

  if (m_axial->getWidget() == widget)
  {
    m_mainWidget = AXIAL_WIDGET;
    m_coronal->disableWidget();
    m_sagittal->disableWidget();
  }

  if (m_coronal->getWidget() == widget)
  {
    m_mainWidget = CORONAL_WIDGET;
    m_axial->disableWidget();
    m_sagittal->disableWidget();
  }

  if (m_sagittal->getWidget() == widget)
  {
    m_mainWidget = SAGITTAL_WIDGET;
    m_coronal->disableWidget();
    m_axial->disableWidget();
  }

  // disabling a widget modifies it's representation (bounds actor)
  m_viewManager->updateViews();
}

//-----------------------------------------------------------------------------
WidgetType PlanarSplitWidget::getMainWidget()
{
  return m_mainWidget;
}

//-----------------------------------------------------------------------------
void PlanarSplitWidget::setSegmentationBounds(double *bounds)
{
  foreach(vtkAbstractWidget *widget, m_widgets)
    {
      vtkPlanarSplitWidget *planarSplitWidget = static_cast<vtkPlanarSplitWidget*>(widget);
      planarSplitWidget->setSegmentationBounds(bounds);
    }
}

//-----------------------------------------------------------------------------
bool PlanarSplitWidget::planeIsValid()
{
  if(this->m_mainWidget == NONE)
    return false;

  vtkSmartPointer<vtkPoints> points = this->getPlanePoints();
  double point1[3], point2[3];
  points->GetPoint(0, point1);
  points->GetPoint(1, point2);

  return ((point1[0] != point2[0]) || (point1[1] != point2[1]) || (point1[2] != point2[2]));
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPlane> PlanarSplitWidget::getImplicitPlane()
{
  vtkSmartPointer<vtkPlane> plane = NULL;

  if((m_mainWidget != NONE) && this->planeIsValid())
  {
    vtkSmartPointer<vtkPoints> points = this->getPlanePoints();
    double point1[3], point2[3];
    points->GetPoint(0, point1);
    points->GetPoint(1, point2);

    double normal[3], upVector[3];
    double planeVector[3] = { point2[0]-point1[0], point2[1]-point1[1], point2[2]-point1[2] };

    switch(m_mainWidget)
    {
      case AXIAL_WIDGET:
        upVector[0] = upVector[1] = 0;
        upVector[2] = 1;
        break;
      case CORONAL_WIDGET:
        upVector[0] = upVector[2] = 0;
        upVector[1] = 1;
        break;
      case SAGITTAL_WIDGET:
        upVector[1] = upVector[2] = 0;
        upVector[0] = 1;
        break;
      default:
        Q_ASSERT(false);
        break;
    }

    plane = vtkSmartPointer<vtkPlane>::New();
    vtkMath::Cross(planeVector, upVector, normal);
    plane->SetOrigin(point1);
    plane->SetNormal(normal);
    plane->Modified();
  }

  return plane;
}
