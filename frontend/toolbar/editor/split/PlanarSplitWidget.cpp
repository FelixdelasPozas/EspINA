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
#include "common/EspinaRegions.h"

// vtk
#include <vtkAbstractWidget.h>
#include <vtkPoints.h>
#include <vtkPlane.h>
#include <vtkMath.h>
#include <vtkImplicitPlaneWidget2.h>
#include <vtkAbstractWidget.h>
#include <vtkImplicitPlaneRepresentation.h>
#include <vtkImplicitFunctionToImageStencil.h>
#include <vtkImageStencilData.h>

typedef EspinaInteractorAdapter<vtkPlanarSplitWidget> PlanarSplitWidgetAdapter;
typedef EspinaInteractorAdapter<vtkImplicitPlaneWidget2> PlanarSplitWidgetVolumeAdapter;

//-----------------------------------------------------------------------------
PlanarSplitWidget::PlanarSplitWidget()
: m_axial(NULL)
, m_coronal(NULL)
, m_sagittal(NULL)
, m_volume(NULL)
, m_mainWidget(NONE)
, m_vtkVolumeInformation(NULL)
{
}

//-----------------------------------------------------------------------------
PlanarSplitWidget::~PlanarSplitWidget()
{
  if (m_mainWidget != NONE)
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

  if (m_volume)
    m_volume->Delete();
}

//-----------------------------------------------------------------------------
vtkAbstractWidget *PlanarSplitWidget::createWidget()
{
  m_volume = vtkImplicitPlaneWidget2::New();
  m_volume->AddObserver(vtkCommand::EndInteractionEvent, this);
  return m_volume;
}

//-----------------------------------------------------------------------------
void PlanarSplitWidget::deleteWidget(vtkAbstractWidget *widget)
{
  if (m_volume)
    m_volume->Delete();
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

  if (m_volume->GetInteractor() == iren)
  {
    PlanarSplitWidgetVolumeAdapter *sw = dynamic_cast<PlanarSplitWidgetVolumeAdapter *>(m_volume);
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

  if (m_volume)
    m_volume->SetEnabled(enable);
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

  if (m_volume)
  {
    Q_ASSERT(m_mainWidget != VOLUME_WIDGET);

    double point1[3], point2[3], normal[3];
    points->GetPoint(0, point1);
    points->GetPoint(1, point2);
    double vector[3] = { point2[0]-point1[0], point2[1]-point1[1], point2[2]-point1[2] };
    double upVector[3] = { 0,0,0 };
    upVector[m_mainWidget] = 1;

    vtkMath::Cross(vector, upVector, normal);
    vtkImplicitPlaneRepresentation *rep = static_cast<vtkImplicitPlaneRepresentation*>(m_volume->GetRepresentation());
    rep->SetNormal(normal);
    rep->SetOrigin(point1);
    rep->Modified();
  }
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
      // return NULL
      break;
  }

  return points;
}

//-----------------------------------------------------------------------------
void PlanarSplitWidget::Execute(vtkObject *caller, unsigned long eventId, void *callData)
{
  PlanarSplitWidgetAdapter *sWidget = static_cast<PlanarSplitWidgetAdapter*>(caller);
  PlanarSplitWidgetVolumeAdapter *pWidget = static_cast<PlanarSplitWidgetVolumeAdapter*>(caller);
  sWidget->RemoveObserver(this);

  if (m_axial->getWidget() == sWidget)
  {
    m_mainWidget = AXIAL_WIDGET;
    m_coronal->disableWidget();
    m_sagittal->disableWidget();
    m_volume->EnabledOff();
  }

  if (m_coronal->getWidget() == sWidget)
  {
    m_mainWidget = CORONAL_WIDGET;
    m_axial->disableWidget();
    m_sagittal->disableWidget();
    m_volume->EnabledOff();
  }

  if (m_sagittal->getWidget() == sWidget)
  {
    m_mainWidget = SAGITTAL_WIDGET;
    m_coronal->disableWidget();
    m_axial->disableWidget();
    m_volume->EnabledOff();
  }

  if (m_volume == pWidget)
  {
    m_mainWidget = VOLUME_WIDGET;
    m_coronal->disableWidget();
    m_axial->disableWidget();
    m_sagittal->disableWidget();
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

  if (m_volume)
  {
    vtkImplicitPlaneRepresentation *rep = static_cast<vtkImplicitPlaneRepresentation*>(m_volume->GetRepresentation());
    rep->SetPlaceFactor(1);
    rep->PlaceWidget(bounds);
    rep->SetOrigin((bounds[0]+bounds[1])/2,(bounds[2]+bounds[3])/2,(bounds[4]+bounds[5])/2);
    rep->UpdatePlacement();
    rep->OutlineTranslationOff();
    rep->ScaleEnabledOff();
    rep->OutsideBoundsOff();
    rep->Modified();
  }
}

//-----------------------------------------------------------------------------
bool PlanarSplitWidget::planeIsValid()
{
  if(this->m_mainWidget == NONE)
    return false;

  if(this->m_mainWidget == VOLUME_WIDGET)
    return true;

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

  if (m_mainWidget == NONE || !this->planeIsValid())
    return plane;

  if(m_mainWidget != VOLUME_WIDGET)
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
  else
  {
    plane = vtkSmartPointer<vtkPlane>::New();
    static_cast<vtkImplicitPlaneRepresentation*>(m_volume->GetRepresentation())->GetPlane(plane);
  }

  return plane;
}

vtkSmartPointer<vtkImageStencilData> PlanarSplitWidget::getStencilForVolume(EspinaVolume *volume)
{
  if (!this->planeIsValid())
    return NULL;

  EspinaVolume::PointType origin = volume->GetOrigin();
  EspinaVolume::SpacingType spacing = volume->GetSpacing();
  int segExtent[6];
  VolumeExtent(volume, segExtent);

  vtkSmartPointer<vtkImplicitFunctionToImageStencil> plane2stencil = vtkSmartPointer<vtkImplicitFunctionToImageStencil>::New();
  plane2stencil->SetInput(this->getImplicitPlane());
  plane2stencil->SetOutputOrigin(0, 0, 0);
  plane2stencil->SetOutputSpacing(spacing[0], spacing[1], spacing[2]);
  plane2stencil->SetOutputWholeExtent(segExtent);
  plane2stencil->Update();

  vtkSmartPointer<vtkImageStencilData> stencil = vtkSmartPointer<vtkImageStencilData>::New();
  stencil = plane2stencil->GetOutput();

  return stencil;
}
