/*
 * ZoomSelectionWidget.cpp
 *
 *  Created on: Nov 14, 2012
 *      Author: Felix de las Pozas Alvarez
 */

// EspINA
#include "ZoomSelectionWidget.h"
#include "ZoomSelectionSliceWidget.h"
#include "vtkZoomSelectionWidget.h"
#include <GUI/View/Widgets/EspinaInteractorAdapter.h>
#include "vtkZoomSelectionWidget.h"

#include <Support/ViewManager.h>
#include <GUI/View/View2D.h>

// vtk
#include <vtkAbstractWidget.h>

using namespace EspINA;

typedef EspinaInteractorAdapter<vtkZoomSelectionWidget> ZoomSelectionWidgetAdapter;

//----------------------------------------------------------------------------
ZoomSelectionWidget::ZoomSelectionWidget()
: m_axial(nullptr)
, m_coronal(nullptr)
, m_sagittal(nullptr)
, m_volume(nullptr)
{
}

//----------------------------------------------------------------------------
ZoomSelectionWidget::~ZoomSelectionWidget()
{
  if (m_axial)
  {
    m_axial->SetEnabled(false);
    delete m_axial;
    m_axial = nullptr;
  }

  if (m_coronal)
  {
    m_coronal->SetEnabled(false);
    delete m_coronal;
    m_coronal = nullptr;
  }

  if (m_sagittal)
  {
    m_sagittal->SetEnabled(false);
    delete m_sagittal;
    m_sagittal = nullptr;
  }

  // this deletes m_volume
  for(auto widget: m_widgets)
  {
    widget->RemoveObserver(this);
    widget->SetEnabled(false);
    widget->Delete();
  }
  m_volume = nullptr;
}

//----------------------------------------------------------------------------
vtkAbstractWidget *ZoomSelectionWidget::create3DWidget(View3D *view)
{
  return nullptr;

  // dead code, for now
  if (!m_volume)
  {
    m_volume = ZoomSelectionWidgetAdapter::New();
    m_volume->AddObserver(vtkCommand::EndInteractionEvent, this);
    m_volume->SetWidgetType(vtkZoomSelectionWidget::VOLUME_WIDGET);
    m_widgets << m_volume;
  }

  return m_volume;
}

//----------------------------------------------------------------------------
SliceWidget *ZoomSelectionWidget::createSliceWidget(View2D *view)
{
  vtkZoomSelectionWidget *widget = nullptr;
  switch(view->plane())
  {
    case Plane::XY:
      widget = ZoomSelectionWidgetAdapter::New();
      widget->AddObserver(vtkCommand::EndInteractionEvent, this);
      widget->SetWidgetType(vtkZoomSelectionWidget::AXIAL_WIDGET);
      m_widgets << widget;
      m_axial = new ZoomSelectionSliceWidget(widget);
      return m_axial;
      break;
    case Plane::XZ:
      widget = ZoomSelectionWidgetAdapter::New();
      widget->AddObserver(vtkCommand::EndInteractionEvent, this);
      widget->SetWidgetType(vtkZoomSelectionWidget::CORONAL_WIDGET);
      m_widgets << widget;
      m_coronal = new ZoomSelectionSliceWidget(widget);
      return m_coronal;
      break;
    case Plane::YZ:
      widget = ZoomSelectionWidgetAdapter::New();
      widget->AddObserver(vtkCommand::EndInteractionEvent, this);
      widget->SetWidgetType(vtkZoomSelectionWidget::SAGITTAL_WIDGET);
      m_widgets << widget;
      m_sagittal = new ZoomSelectionSliceWidget(widget);
      return m_sagittal;
      break;
    default:
      Q_ASSERT(false);
      break;
  }

  return nullptr; // dead code
}

//----------------------------------------------------------------------------
bool ZoomSelectionWidget::processEvent(vtkRenderWindowInteractor *iren,
                          long unsigned int event)
{
  for(auto widget: m_widgets)
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
  m_axial->SetEnabled(enable);
  m_coronal->SetEnabled(enable);
  m_sagittal->SetEnabled(enable);

  for(auto widget: m_widgets)
      widget->SetEnabled(enable);
}

//----------------------------------------------------------------------------
void ZoomSelectionWidget::Execute(vtkObject *caller, unsigned long int eventId, void* callData)
{
  // this is needed to update the thumbnail when zooming the view.
  m_viewManager->updateViews();
}
