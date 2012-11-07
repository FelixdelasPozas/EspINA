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

// vtk
#include <vtkAbstractWidget.h>
#include <vtkPoints.h>

typedef EspinaInteractorAdapter<vtkPlanarSplitWidget> PlanarSplitWidgetAdapter;

//-----------------------------------------------------------------------------
PlanarSplitWidget::PlanarSplitWidget()
: m_axialWidget(NULL)
, m_coronalWidget(NULL)
, m_sagittalWidget(NULL)
, m_mainWidget(AXIAL)
{
}

//-----------------------------------------------------------------------------
PlanarSplitWidget::~PlanarSplitWidget()
{
  if (m_axialWidget)
  {
    m_axialWidget->setEnabled(false);
    delete m_axialWidget;
  }

  if (m_coronalWidget)
  {
    m_coronalWidget->setEnabled(false);
    delete m_coronalWidget;
  }

  if (m_sagittalWidget)
  {
    m_sagittalWidget->setEnabled(false);
    delete m_sagittalWidget;
  }

  // m_widgets contents gets deleted when deleting the SliceWidgets
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
  PlanarSplitWidgetAdapter *widget = new PlanarSplitWidgetAdapter();

  switch (plane)
  {
    case AXIAL:
      if (!m_axialWidget)
      {
        m_axialWidget = new PlanarSplitSliceWidget(widget);
        m_axialWidget->setOrientation(AXIAL);
      }
      else
        delete widget;
      return m_axialWidget;
      break;
    case CORONAL:
      if (!m_coronalWidget)
      {
        m_coronalWidget = new PlanarSplitSliceWidget(widget);
        m_coronalWidget->setOrientation(CORONAL);
      }
      else
        delete widget;
      return m_coronalWidget;
      break;
    case SAGITTAL:
      if (!m_sagittalWidget)
      {
        m_sagittalWidget = new PlanarSplitSliceWidget(widget);
        m_sagittalWidget->setOrientation(SAGITTAL);
      }
      else
        delete widget;
      return m_sagittalWidget;
      break;
    default:
      Q_ASSERT(false);
      break;
  }

  Q_ASSERT(false);
  return NULL; // dead code
}

//-----------------------------------------------------------------------------
bool PlanarSplitWidget::processEvent(vtkRenderWindowInteractor* iren,
                                 long unsigned int event)
{
  foreach(vtkAbstractWidget *widget, m_widgets)
  {
    if (widget->GetInteractor() == iren)
    {
      PlanarSplitWidgetAdapter *sw = dynamic_cast<PlanarSplitWidgetAdapter *>(widget);
      return sw->ProcessEventsHandler(event);
    }
  }

  return false;
}

//-----------------------------------------------------------------------------
void PlanarSplitWidget::setEnabled(bool enable)
{
  if (m_axialWidget)
    m_axialWidget->setEnabled(enable);

  if (m_coronalWidget)
    m_coronalWidget->setEnabled(enable);

  if (m_sagittalWidget)
    m_sagittalWidget->setEnabled(enable);
}

//-----------------------------------------------------------------------------
void PlanarSplitWidget::setPlanePoints(vtkSmartPointer<vtkPoints> points)
{
  if (m_axialWidget)
    m_axialWidget->setPoints(points);

  if (m_coronalWidget)
    m_coronalWidget->setPoints(points);

  if (m_sagittalWidget)
    m_sagittalWidget->setPoints(points);
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPoints> PlanarSplitWidget::getPlanePoints()
{
  vtkSmartPointer<vtkPoints> points;
  if (!m_axialWidget && !m_coronalWidget && !m_sagittalWidget)
  {
    points = vtkSmartPointer<vtkPoints>::New();
    return points;
  }

  switch(m_mainWidget)
  {
    case AXIAL:
      points = m_axialWidget->getPoints();
      break;
    case CORONAL:
      points = m_coronalWidget->getPoints();
      break;
    case SAGITTAL:
      points = m_sagittalWidget->getPoints();
      break;
    default:
      Q_ASSERT(false);
      break;
  }

  return points;
}
