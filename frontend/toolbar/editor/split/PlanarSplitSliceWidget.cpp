/*
 * PlanarSplitSliceWidget.cpp
 *
 *  Created on: Nov 5, 2012
 *      Author: Félix de las Pozas Álvarez
 */

// EspINA
#include "toolbar/editor/split/PlanarSplitSliceWidget.h"
#include "toolbar/editor/split/vtkPlanarSplitWidget.h"

// vtk
#include <vtkPoints.h>

//-----------------------------------------------------------------------------
PlanarSplitSliceWidget::PlanarSplitSliceWidget(vtkAbstractWidget *widget)
: SliceWidget(widget)
, m_plane(AXIAL)
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
  vtkPlanarSplitWidget *widget = reinterpret_cast<vtkPlanarSplitWidget*>(m_widget);
  widget->setPoints(points);
}

//-----------------------------------------------------------------------------
void PlanarSplitSliceWidget::setEnabled(bool enabled)
{
  m_widget->SetEnabled(enabled);
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPoints> PlanarSplitSliceWidget::getPoints()
{
  vtkPlanarSplitWidget *widget = reinterpret_cast<vtkPlanarSplitWidget*>(m_widget);
  return widget->getPoints();
}

//-----------------------------------------------------------------------------
void PlanarSplitSliceWidget::setOrientation(PlaneType plane)
{
  m_plane = plane;
  vtkPlanarSplitWidget *widget = reinterpret_cast<vtkPlanarSplitWidget*>(m_widget);
  widget->setOrientation(plane);

}
