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
#include <GUI/View/Widgets/Zoom/ZoomWidget3D.h>
#include <GUI/View/Widgets/Zoom/vtkZoomSelectionWidget.h>

// VTK
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

using namespace ESPINA::GUI::View::Widgets;
using namespace ESPINA::GUI::Representations::Managers;

//----------------------------------------------------------------------------
ZoomWidget3D::ZoomWidget3D(ZoomEventHandler* eventHandler)
: m_eventHandler{eventHandler}
, m_widget      {vtkSmartPointer<vtkZoomSelectionWidget>::New()}
, m_view        {nullptr}
{
  connect(m_eventHandler, SIGNAL(movement(QPoint, RenderView *)),
          this,           SLOT(onMouseMovement(QPoint, RenderView *)));
  connect(m_eventHandler, SIGNAL(leftPress(QPoint, RenderView *)),
          this,           SLOT(onLeftMousePress(QPoint, RenderView *)));
  connect(m_eventHandler, SIGNAL(leftRelease(QPoint, RenderView *)),
          this,           SLOT(onLeftMouseRelease(QPoint, RenderView *)));

  m_widget->SetWidgetType(vtkZoomSelectionWidget::VOLUME_WIDGET);
}

//----------------------------------------------------------------------------
ZoomWidget3D::~ZoomWidget3D()
{
  disconnect(m_eventHandler, SIGNAL(movement(QPoint, RenderView *)),
             this,           SLOT(onMouseMovement(QPoint, RenderView *)));
  disconnect(m_eventHandler, SIGNAL(leftPress(QPoint, RenderView *)),
             this,           SLOT(onLeftMousePress(QPoint, RenderView *)));
  disconnect(m_eventHandler, SIGNAL(leftRelease(QPoint, RenderView *)),
             this,           SLOT(onLeftMouseRelease(QPoint, RenderView *)));
}

//----------------------------------------------------------------------------
TemporalRepresentation3DSPtr ZoomWidget3D::cloneImplementation()
{
  return std::make_shared<ZoomWidget3D>(m_eventHandler);
}

//----------------------------------------------------------------------------
void ZoomWidget3D::onMouseMovement(QPoint point, RenderView* view)
{
  if(m_view == view)
  {
    m_view->renderWindow()->GetInteractor()->SetEventInformationFlipY(point.x(), point.y());
    m_view->renderWindow()->GetInteractor()->MouseMoveEvent();
  }
}

//----------------------------------------------------------------------------
void ZoomWidget3D::onLeftMousePress(QPoint point, RenderView* view)
{
  if(m_view == view)
  {
    m_view->renderWindow()->GetInteractor()->SetEventInformationFlipY(point.x(), point.y());
    m_view->renderWindow()->GetInteractor()->LeftButtonPressEvent();
  }
}

//----------------------------------------------------------------------------
void ZoomWidget3D::onLeftMouseRelease(QPoint point, RenderView* view)
{
  if(m_view == view)
  {
    m_view->renderWindow()->GetInteractor()->SetEventInformationFlipY(point.x(), point.y());
    m_view->renderWindow()->GetInteractor()->LeftButtonReleaseEvent();

    m_view->refresh();
  }
}

//----------------------------------------------------------------------------
bool ZoomWidget3D::acceptCrosshairChange(const NmVector3& crosshair) const
{
  return false;
}

//----------------------------------------------------------------------------
bool ZoomWidget3D::acceptSceneResolutionChange(const NmVector3& resolution) const
{
  return false;
}

//----------------------------------------------------------------------------
bool ZoomWidget3D::acceptSceneBoundsChange(const Bounds& bounds) const
{
  return false;
}

//----------------------------------------------------------------------------
bool ZoomWidget3D::acceptInvalidationFrame(const GUI::Representations::FrameCSPtr frame) const
{
  return false;
}

//----------------------------------------------------------------------------
void ZoomWidget3D::initializeImplementation(RenderView* view)
{
  m_view = view;
}

//----------------------------------------------------------------------------
void ZoomWidget3D::uninitializeImplementation()
{
}

//----------------------------------------------------------------------------
vtkAbstractWidget* ZoomWidget3D::vtkWidget()
{
  return m_widget;
}
