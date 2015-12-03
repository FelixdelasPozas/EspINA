/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

 ESPINA is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 */

// ESPINA
#include <GUI/View/RenderView.h>
#include <GUI/View/Widgets/Zoom/ZoomWidget2D.h>
#include <GUI/View/Widgets/Zoom/vtkZoomSelectionWidget.h>

// VTK
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

using namespace ESPINA::GUI::View::Widgets;

//----------------------------------------------------------------------------
ZoomWidget2D::ZoomWidget2D(ZoomEventHandler *eventHandler)
: m_eventHandler{eventHandler}
, m_widget      {vtkSmartPointer<vtkZoomSelectionWidget>::New()}
, m_plane       {Plane::UNDEFINED}
, m_view        {nullptr}
{
  connect(m_eventHandler, SIGNAL(movement(QPoint, RenderView *)),
          this,           SLOT(onMouseMovement(QPoint, RenderView *)));
  connect(m_eventHandler, SIGNAL(leftPress(QPoint, RenderView *)),
          this,           SLOT(onLeftMousePress(QPoint, RenderView *)));
  connect(m_eventHandler, SIGNAL(leftRelease(QPoint, RenderView *)),
          this,           SLOT(onLeftMouseRelease(QPoint, RenderView *)));
}

//----------------------------------------------------------------------------
ZoomWidget2D::~ZoomWidget2D()
{
  disconnect(m_eventHandler, SIGNAL(movement(QPoint, RenderView *)),
             this,           SLOT(onMouseMovement(QPoint, RenderView *)));
  disconnect(m_eventHandler, SIGNAL(leftPress(QPoint, RenderView *)),
             this,           SLOT(onLeftMousePress(QPoint, RenderView *)));
  disconnect(m_eventHandler, SIGNAL(leftRelease(QPoint, RenderView *)),
             this,           SLOT(onLeftMouseRelease(QPoint, RenderView *)));
}

//----------------------------------------------------------------------------
void ZoomWidget2D::setPlane(Plane plane)
{
  if(m_plane == Plane::UNDEFINED && plane != Plane::UNDEFINED)
  {
    m_plane = plane;

    switch(m_plane)
    {
      case Plane::XY:
        m_widget->SetWidgetType(vtkZoomSelectionWidget::AXIAL_WIDGET);
        break;
      case Plane::XZ:
        m_widget->SetWidgetType(vtkZoomSelectionWidget::CORONAL_WIDGET);
        break;
      case Plane::YZ:
        m_widget->SetWidgetType(vtkZoomSelectionWidget::SAGITTAL_WIDGET);
        break;
      default:
        Q_ASSERT(false);
        break;
    }
  }
}

//----------------------------------------------------------------------------
void ZoomWidget2D::setRepresentationDepth(Nm depth)
{
  // don't need to check for the value, the widget will do that
  m_widget->setRepresentationDepth(depth);
}

//----------------------------------------------------------------------------
TemporalRepresentation2DSPtr ZoomWidget2D::clone()
{
  return std::make_shared<ZoomWidget2D>(m_eventHandler);
}

//----------------------------------------------------------------------------
bool ZoomWidget2D::acceptCrosshairChange(const NmVector3& crosshair) const
{
  return true;
}

//----------------------------------------------------------------------------
bool ZoomWidget2D::acceptSceneResolutionChange(const NmVector3& resolution) const
{
  return false;
}

//----------------------------------------------------------------------------
bool ZoomWidget2D::acceptSceneBoundsChange(const Bounds& bounds) const
{
  return false;
}

//----------------------------------------------------------------------------
bool ZoomWidget2D::acceptInvalidationFrame(const GUI::Representations::FrameCSPtr frame) const
{
  return false;
}

//----------------------------------------------------------------------------
void ZoomWidget2D::setCrosshair(const NmVector3& crosshair)
{
  if(m_view && m_plane != Plane::UNDEFINED)
  {
    m_widget->SetSlice(crosshair[normalCoordinateIndex(m_plane)]);
  }
}

//----------------------------------------------------------------------------
void ZoomWidget2D::initializeImplementation(RenderView* view)
{
  m_view = view;
}

//----------------------------------------------------------------------------
void ZoomWidget2D::uninitializeImplementation()
{
}

//----------------------------------------------------------------------------
void ZoomWidget2D::onMouseMovement(QPoint point, RenderView *view)
{
  if(m_view == view)
  {
    m_view->renderWindow()->GetInteractor()->SetEventInformationFlipY(point.x(), point.y());
    m_view->renderWindow()->GetInteractor()->MouseMoveEvent();
  }
}

//----------------------------------------------------------------------------
void ZoomWidget2D::onLeftMousePress(QPoint point, RenderView *view)
{
  if(m_view == view)
  {
    m_view->renderWindow()->GetInteractor()->SetEventInformationFlipY(point.x(), point.y());
    m_view->renderWindow()->GetInteractor()->LeftButtonPressEvent();
  }
}

//----------------------------------------------------------------------------
void ZoomWidget2D::onLeftMouseRelease(QPoint point, RenderView *view)
{
  if(m_view == view)
  {
    m_view->renderWindow()->GetInteractor()->SetEventInformationFlipY(point.x(), point.y());
    m_view->renderWindow()->GetInteractor()->LeftButtonReleaseEvent();

    view->refresh();
  }
}

//----------------------------------------------------------------------------
vtkAbstractWidget* ZoomWidget2D::vtkWidget()
{
  return m_widget;
}
