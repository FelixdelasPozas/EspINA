/*

 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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
#include "SkeletonWidget2D.h"
#include "vtkSkeletonWidget.h"
#include <GUI/View/View2D.h>

// VTK
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>

// C++
#include <limits>

// Qt
#include <QKeyEvent>
#include <QEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>

using namespace ESPINA;
using namespace ESPINA::GUI::Representations::Managers;
using namespace ESPINA::GUI::View::Widgets::Skeleton;

//-----------------------------------------------------------------------------
SkeletonWidget2D::SkeletonWidget2D(SkeletonEventHandlerSPtr handler)
: m_widget    {vtkSmartPointer<vtkSkeletonWidget>::New()}
, m_position  {-std::numeric_limits<Nm>::max()}
, m_handler   {handler}
, m_view      {nullptr}
, m_hasHandler{false}
{
}
  
//-----------------------------------------------------------------------------
SkeletonWidget2D::~SkeletonWidget2D()
{
  if(m_widget)
  {
    if(m_view)
    {
      uninitializeImplementation();
    }

    m_widget = nullptr;
  }
}

//-----------------------------------------------------------------------------
void SkeletonWidget2D::initialize(vtkSmartPointer<vtkPolyData> pd)
{
  m_widget->Initialize(pd);
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> SkeletonWidget2D::getSkeleton()
{
  return m_widget->getSkeleton();
}

//-----------------------------------------------------------------------------
void SkeletonWidget2D::setRepresentationColor(const QColor &color)
{
  if(m_widget->representationColor() != color)
  {
    m_widget->setRepresentationColor(color);
  }
}

//-----------------------------------------------------------------------------
void SkeletonWidget2D::setSpacing(const NmVector3 &spacing)
{
  if(m_widget->spacing() != spacing)
  {
    m_widget->SetSpacing(spacing);
  }
}

//-----------------------------------------------------------------------------
void SkeletonWidget2D::setPlane(Plane plane)
{
  if(m_widget->orientation() != plane && plane != Plane::UNDEFINED)
  {
    m_widget->SetOrientation(plane);
  }
}

//-----------------------------------------------------------------------------
void SkeletonWidget2D::setRepresentationDepth(Nm depth)
{
  if(m_widget->shift() != depth)
  {
    m_widget->SetShift(depth);
  }
}

//-----------------------------------------------------------------------------
TemporalRepresentation2DSPtr SkeletonWidget2D::cloneImplementation()
{
  return std::make_shared<SkeletonWidget2D>(m_handler);
}

//-----------------------------------------------------------------------------
void SkeletonWidget2D::display(const GUI::Representations::FrameCSPtr& frame)
{
  auto plane = m_widget->orientation();

  if(m_view && (plane != Plane::UNDEFINED))
  {
    auto framePos = frame->crosshair[normalCoordinateIndex(plane)];

    if(m_position != framePos)
    {
      m_position = framePos;

      m_widget->changeSlice(plane, m_position);
    }
  }
}

//-----------------------------------------------------------------------------
bool SkeletonWidget2D::acceptCrosshairChange(const NmVector3& crosshair) const
{
  auto plane = m_widget->orientation();

  if(m_view && plane != Plane::UNDEFINED)
  {
    auto planePos = crosshair[normalCoordinateIndex(plane)];

    return m_position != planePos;
  }

  return false;
}

//--------------------------------------------------------------------
void SkeletonWidget2D::initializeImplementation(RenderView* view)
{
  if(m_view) return; // already configured

  auto view2d = view2D_cast(view);

  if(view2d)
  {
    m_view = view;
    auto plane = view2d->plane();
    auto spacing = view2d->sceneResolution();
    m_position = view2d->crosshair()[normalCoordinateIndex(plane)];

    m_widget->SetOrientation(plane);
    m_widget->SetSpacing(spacing);
    m_widget->changeSlice(plane, m_position);
    m_widget->SetShift(view2d->widgetDepth());
    m_widget->SetCurrentRenderer(view->mainRenderer());
    m_widget->SetInteractor(view->renderWindow()->GetInteractor());
    m_widget->EnabledOn();

    connectSignals();
  }
}

//--------------------------------------------------------------------
void SkeletonWidget2D::connectSignals()
{
  connect(m_handler.get(), SIGNAL(trackStarted(Track, RenderView*)),
         this,             SLOT(onTrackStarted(Track, RenderView*)));

  connect(m_handler.get(), SIGNAL(trackUpdated(Track)),
          this,            SLOT(onTrackUpdated(Track)));

  connect(m_handler.get(), SIGNAL(trackStopped(Track, RenderView*)),
          this,            SLOT(onTrackStopped(Track, RenderView*)));

  connect(m_handler.get(), SIGNAL(cursorPosition(const QPoint &)),
           this,           SLOT(onCursorPositionChanged(const QPoint &)));

  connect(m_handler.get(), SIGNAL(endStroke()),
           this,           SLOT(onStrokeEnded()));

  connect(m_handler.get(), SIGNAL(cancelled()),
          this,            SLOT(onCancellation()));
}

//--------------------------------------------------------------------
void SkeletonWidget2D::disconnectSignals()
{
  disconnect(m_handler.get(), SIGNAL(trackStarted(Track, RenderView*)),
             this,             SLOT(onTrackStarted(Track, RenderView*)));

  disconnect(m_handler.get(), SIGNAL(trackUpdated(Track)),
             this,            SLOT(onTrackUpdated(Track)));

  disconnect(m_handler.get(), SIGNAL(trackStopped(Track, RenderView*)),
             this,            SLOT(onTrackStopped(Track, RenderView*)));

  disconnect(m_handler.get(), SIGNAL(cursorPosition(const QPoint &)),
             this,           SLOT(onCursorPositionChanged(const QPoint &)));

  disconnect(m_handler.get(), SIGNAL(endStroke()),
             this,           SLOT(onStrokeEnded()));

  disconnect(m_handler.get(), SIGNAL(cancelled()),
             this,            SLOT(onCancellation()));
}

//--------------------------------------------------------------------
void SkeletonWidget2D::onTrackStarted(Track track, RenderView* view)
{
  if(m_view == view)
  {
    m_hasHandler = true;

    auto point = track.first();

    qDebug() << "enter view" << normalCoordinateIndex(m_widget->orientation());

    // add two points, one where clicked and the other to act as cursor.
    for(int i = 0; i < 2; ++i)
    {
      m_widget->GetInteractor()->SetEventInformationFlipY(point.x(), point.y(), Qt::ControlModifier == QApplication::keyboardModifiers(), Qt::ShiftModifier == QApplication::keyboardModifiers());
      m_widget->addPoint();
    }
  }
}

//--------------------------------------------------------------------
void SkeletonWidget2D::onTrackUpdated(Track track)
{
  if(!m_hasHandler) return;

  for(auto point: track)
  {
    qDebug() << "track updated on view" << normalCoordinateIndex(m_widget->orientation());

    m_widget->GetInteractor()->SetEventInformationFlipY(point.x(), point.y(), Qt::ControlModifier == QApplication::keyboardModifiers(), Qt::ShiftModifier == QApplication::keyboardModifiers());
    m_widget->addPoint();
  }
}

//--------------------------------------------------------------------
void SkeletonWidget2D::onTrackStopped(Track track, RenderView* view)
{
  if(!m_hasHandler) return;

//  int x,y;
//  view->eventPosition(x, y);
//
//
//  qDebug() << "track stopped on view" << normalCoordinateIndex(m_widget->orientation());
//
//  m_widget->GetInteractor()->SetEventInformationFlipY(x, y, Qt::ControlModifier == QApplication::keyboardModifiers(), Qt::ShiftModifier == QApplication::keyboardModifiers());
//  m_widget->movePoint();
}


//--------------------------------------------------------------------
void SkeletonWidget2D::onCursorPositionChanged(const QPoint& p)
{
  if(!m_hasHandler) return;

  qDebug() << "cursor position" << p << "plane" << normalCoordinateIndex(m_widget->orientation());

  m_widget->GetInteractor()->SetEventInformationFlipY(p.x(), p.y(), Qt::ControlModifier == QApplication::keyboardModifiers(), Qt::ShiftModifier == QApplication::keyboardModifiers());
  m_widget->movePoint();
}

//--------------------------------------------------------------------
void SkeletonWidget2D::onStrokeEnded()
{
  if(!m_hasHandler) return;

  qDebug() << "stroke ended - plane" << normalCoordinateIndex(m_widget->orientation());
  m_widget->stop();
}

//--------------------------------------------------------------------
void SkeletonWidget2D::onCancellation()
{
  if(!m_hasHandler) return;

  m_hasHandler = false;

  qDebug() << "received cancel";
  m_widget->stop();
}

//--------------------------------------------------------------------
void SkeletonWidget2D::uninitializeImplementation()
{
  if(m_view)
  {
    disconnectSignals();

    m_widget->EnabledOff();
    m_widget->SetCurrentRenderer(nullptr);
    m_widget->SetInteractor(nullptr);

    m_view = nullptr;
  }
}
