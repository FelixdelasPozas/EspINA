/*
 * PlanarSplitSliceWidget.cpp
 *
 *  Created on: Nov 5, 2012
 *      Author: Felix de las Pozas Alvarez
 */

// EspINA
#include "GUI/View/Widgets/PlanarSplit/PlanarSplitSliceWidget.h"
#include "GUI/View/Widgets/PlanarSplit/vtkPlanarSplitWidget.h"

// vtk
#include <vtkPoints.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
PlanarSplitSliceWidget::PlanarSplitSliceWidget(vtkAbstractWidget *widget)
: SliceWidget(widget)
, m_plane(Plane::XY)
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
void PlanarSplitSliceWidget::setSlice(Nm pos, Plane plane)
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
void PlanarSplitSliceWidget::setOrientation(Plane plane)
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
