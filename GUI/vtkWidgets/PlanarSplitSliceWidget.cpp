/*
 * PlanarSplitSliceWidget.cpp
 *
 *  Created on: Nov 5, 2012
 *      Author: Félix de las Pozas Álvarez
 */

// EspINA
#include "GUI/vtkWidgets/PlanarSplitSliceWidget.h"
#include "GUI/vtkWidgets/vtkPlanarSplitWidget.h"

// vtk
#include <vtkPoints.h>

//-----------------------------------------------------------------------------
PlanarSplitSliceWidget::PlanarSplitSliceWidget(vtkAbstractWidget *widget)
: SliceWidget(widget)
, m_plane(AXIAL)
, m_mainWidget(true)
{
}

//-----------------------------------------------------------------------------
PlanarSplitSliceWidget::~PlanarSplitSliceWidget()
{
  if (m_widget)
    m_widget->Delete();
}

//-----------------------------------------------------------------------------
void PlanarSplitSliceWidget::setSlice(Nm pos, PlaneType plane)
{
  // nothing to do in this case
}

//-----------------------------------------------------------------------------
void PlanarSplitSliceWidget::setPoints(vtkSmartPointer<vtkPoints> points)
{
  if (m_mainWidget)
  {
    vtkPlanarSplitWidget *widget = reinterpret_cast<vtkPlanarSplitWidget*>(m_widget);
    widget->setPoints(points);
  }
}

//-----------------------------------------------------------------------------
void PlanarSplitSliceWidget::setEnabled(bool enabled)
{
  if(m_mainWidget)
    m_widget->SetEnabled(enabled);
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPoints> PlanarSplitSliceWidget::getPoints()
{
  if (m_mainWidget)
  {
    vtkPlanarSplitWidget *widget = reinterpret_cast<vtkPlanarSplitWidget*>(m_widget);
    return widget->getPoints();
  }

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  return points;
}

//-----------------------------------------------------------------------------
void PlanarSplitSliceWidget::setOrientation(PlaneType plane)
{
  m_plane = plane;
  vtkPlanarSplitWidget *widget = reinterpret_cast<vtkPlanarSplitWidget*>(m_widget);
  widget->setOrientation(plane);

}

//-----------------------------------------------------------------------------
void PlanarSplitSliceWidget::disableWidget()
{
  m_mainWidget = false;
  vtkPlanarSplitWidget* widget = static_cast<vtkPlanarSplitWidget*>(m_widget);
  widget->disableWidget();
}
