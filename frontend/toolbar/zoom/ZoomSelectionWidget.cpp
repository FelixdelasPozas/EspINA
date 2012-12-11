/*
 * ZoomSelectionWidget.cpp
 *
 *  Created on: Nov 14, 2012
 *      Author: Félix de las Pozas Álvarez
 */

// EspINA
#include "toolbar/zoom/ZoomSelectionWidget.h"
#include "toolbar/zoom/ZoomSelectionSliceWidget.h"
#include "frontend/toolbar/zoom/vtkZoomSelectionWidget.h"
#include "common/widgets/EspinaInteractorAdapter.h"
#include "toolbar/zoom/vtkZoomSelectionWidget.h"
#include "common/gui/ViewManager.h"

// vtk
#include <vtkAbstractWidget.h>

typedef EspinaInteractorAdapter<vtkZoomSelectionWidget> ZoomSelectionWidgetAdapter;

//----------------------------------------------------------------------------
ZoomSelectionWidget::ZoomSelectionWidget()
: m_axial(NULL)
, m_coronal(NULL)
, m_sagittal(NULL)
, m_volume(NULL)
{
}

//----------------------------------------------------------------------------
ZoomSelectionWidget::~ZoomSelectionWidget()
{
  if (m_axial)
  {
    m_axial->SetEnabled(false);
    delete m_axial;
    m_axial = NULL;
  }

  if (m_coronal)
  {
    m_coronal->SetEnabled(false);
    delete m_coronal;
    m_coronal = NULL;
  }

  if (m_sagittal)
  {
    m_sagittal->SetEnabled(false);
    delete m_sagittal;
    m_sagittal = NULL;
  }

  // this deletes m_volume
  foreach(vtkAbstractWidget *widget, m_widgets)
  {
    widget->RemoveObserver(this);
    widget->SetEnabled(false);
    widget->Delete();
  }
  m_volume = NULL;
}

//----------------------------------------------------------------------------
vtkAbstractWidget *ZoomSelectionWidget::createWidget()
{
  return NULL;

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
void ZoomSelectionWidget::deleteWidget(vtkAbstractWidget *widget)
{
  Q_ASSERT(false);

  // dead code, for now
  if (!m_volume)
    return;

  m_volume->RemoveObserver(this);
  m_volume->SetEnabled(false);
  m_volume->Delete();
  m_volume = NULL;
}

//----------------------------------------------------------------------------
SliceWidget *ZoomSelectionWidget::createSliceWidget(PlaneType plane)
{
  vtkZoomSelectionWidget *widget = NULL;
  switch(plane)
  {
    case AXIAL:
      widget = ZoomSelectionWidgetAdapter::New();
      widget->AddObserver(vtkCommand::EndInteractionEvent, this);
      widget->SetWidgetType(vtkZoomSelectionWidget::AXIAL_WIDGET);
      m_widgets << widget;
      m_axial = new ZoomSelectionSliceWidget(widget);
      return m_axial;
      break;
    case CORONAL:
      widget = ZoomSelectionWidgetAdapter::New();
      widget->AddObserver(vtkCommand::EndInteractionEvent, this);
      widget->SetWidgetType(vtkZoomSelectionWidget::CORONAL_WIDGET);
      m_widgets << widget;
      m_coronal = new ZoomSelectionSliceWidget(widget);
      return m_coronal;
      break;
    case SAGITTAL:
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

  return NULL; // dead code
}

//----------------------------------------------------------------------------
bool ZoomSelectionWidget::processEvent(vtkRenderWindowInteractor *iren,
                          long unsigned int event)
{
  foreach(vtkAbstractWidget *widget, m_widgets)
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

  foreach(vtkAbstractWidget *widget, m_widgets)
      widget->SetEnabled(enable);
}

//----------------------------------------------------------------------------
void ZoomSelectionWidget::Execute(vtkObject *caller, unsigned long int eventId, void* callData)
{
  // this is needed to update the thumbnail when zooming the view.
  this->m_viewManager->updateViews();
}
