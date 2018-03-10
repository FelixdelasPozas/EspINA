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
#include <Core/Analysis/Data/SkeletonDataUtils.h>
#include <GUI/View/View2D.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Representations/Settings/SegmentationSkeletonPoolSettings.h>

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
using namespace ESPINA::Core;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Representations::Managers;
using namespace ESPINA::GUI::Representations::Settings;
using namespace ESPINA::GUI::View::Widgets::Skeleton;

//-----------------------------------------------------------------------------
SkeletonWidget2D::SkeletonWidget2D(SkeletonEventHandlerSPtr handler, SkeletonPoolSettingsSPtr settings)
: m_widget    {vtkSmartPointer<vtkSkeletonWidget>::New()}
, m_view      {nullptr}
, m_position  {-std::numeric_limits<Nm>::max()}
, m_handler   {handler}
, m_mode      {Mode::CREATE}
, m_moving    {false}
, m_settings  {settings}
{
  connect(m_settings.get(), SIGNAL(modified()), this, SLOT(updateRepresentation()));
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
  auto clone = std::make_shared<SkeletonWidget2D>(m_handler, m_settings);

  return clone;
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
    m_widget->setRepresentationWidth(m_settings->width());
    m_widget->setRepresentationShowText(m_settings->showAnnotations());
    m_widget->setRepresentationTextSize(m_settings->annotationsSize());
    m_widget->EnabledOn();

    m_handler->addWidget(this);

    connect(this, SIGNAL(updateWidgets()), m_handler.get(), SLOT(updateRepresentations()));

    connectSignals();
  }
}

//--------------------------------------------------------------------
void SkeletonWidget2D::connectSignals()
{
  connect(m_handler.get(), SIGNAL(mousePress(Qt::MouseButtons, const QPoint &, RenderView *)),
          this,            SLOT(onMousePress(Qt::MouseButtons, const QPoint &, RenderView *)));

  connect(m_handler.get(), SIGNAL(mouseRelease(Qt::MouseButtons, const QPoint &, RenderView *)),
          this,            SLOT(onMouseRelease(Qt::MouseButtons, const QPoint &, RenderView *)));

  connect(m_handler.get(), SIGNAL(started(Track, RenderView*)),
         this,             SLOT(onTrackStarted(Track, RenderView*)));

  connect(m_handler.get(), SIGNAL(updated(Track, RenderView *)),
          this,            SLOT(onTrackUpdated(Track, RenderView *)));

  connect(m_handler.get(), SIGNAL(stopped(RenderView *)),
           this,           SLOT(onStrokeEnded(RenderView *)));

  connect(m_handler.get(), SIGNAL(cursorPosition(const QPoint &, RenderView *)),
           this,           SLOT(onCursorPositionChanged(const QPoint &, RenderView *)));

  connect(m_handler.get(), SIGNAL(cancelled(RenderView *)),
          this,            SLOT(onStrokeEnded(RenderView *)));
}

//--------------------------------------------------------------------
void SkeletonWidget2D::disconnectSignals()
{
  disconnect(m_handler.get(), SIGNAL(mousePress(Qt::MouseButtons, const QPoint &, RenderView *)),
             this,            SLOT(onMousePress(Qt::MouseButtons, const QPoint &, RenderView *)));

  disconnect(m_handler.get(), SIGNAL(mouseRelease(Qt::MouseButtons, const QPoint &, RenderView *)),
             this,            SLOT(onMouseRelease(Qt::MouseButtons, const QPoint &, RenderView *)));

  disconnect(m_handler.get(), SIGNAL(started(Track, RenderView*)),
             this,            SLOT(onTrackStarted(Track, RenderView*)));

  disconnect(m_handler.get(), SIGNAL(updated(Track, RenderView *)),
             this,            SLOT(onTrackUpdated(Track, RenderView *)));

  disconnect(m_handler.get(), SIGNAL(stopped(RenderView *)),
             this,            SLOT(onStrokeEnded(RenderView *)));

  disconnect(m_handler.get(), SIGNAL(cursorPosition(const QPoint &, RenderView *)),
             this,            SLOT(onCursorPositionChanged(const QPoint &, RenderView *)));

  disconnect(m_handler.get(), SIGNAL(cancelled(RenderView *)),
             this,            SLOT(onStrokeEnded(RenderView *)));
}

//--------------------------------------------------------------------
void SkeletonWidget2D::onTrackStarted(Track track, RenderView* view)
{
  if(view != m_view) return;

  auto point = track.first();

  m_widget->GetInteractor()->SetEventInformationFlipY(point.x(), point.y(), 0, 0);

  switch (m_mode)
  {
    case Mode::CREATE:
      {
        m_widget->setIgnoreCursor(true);
        auto previousStroke = m_widget->stroke();
        m_widget->addPoint();
        auto stroke = m_widget->stroke();
        if(previousStroke != stroke)
        {
          emit strokeChanged(stroke);
        }
      }
      break;
    case Mode::MODIFY:
      if(m_moving)
      {
        m_widget->movePoint();
      }
      break;
    case Mode::MARK:
    case Mode::ERASE:
    default:
      m_widget->updateCursor();
      break;
      break;
  }
}

//--------------------------------------------------------------------
void SkeletonWidget2D::onTrackUpdated(Track track, RenderView *view)
{
  if(view != m_view) return;

  for(auto point: track)
  {
    m_widget->GetInteractor()->SetEventInformationFlipY(point.x(), point.y(), 0, 0);

    switch(m_mode)
    {
      case Mode::CREATE:
        {
          auto previousStroke = m_widget->stroke();
          m_widget->addPoint();
          auto stroke = m_widget->stroke();
          if(previousStroke != stroke)
          {
            emit strokeChanged(stroke);
          }
        }
        break;
      case Mode::MODIFY:
        if(m_moving)
        {
          m_widget->movePoint();
        }
        break;
      case Mode::ERASE:
        m_widget->updateCursor();
        break;
      case Mode::MARK:
      default:
        break;
    }
  }
}

//--------------------------------------------------------------------
void SkeletonWidget2D::onCursorPositionChanged(const QPoint& p, RenderView *view)
{
  if(view != m_view) return;

  m_widget->GetInteractor()->SetEventInformationFlipY(p.x(), p.y(), 0, 0);
  m_widget->GetInteractor()->MouseMoveEvent();

  switch(m_mode)
  {
    case Mode::CREATE:
      m_widget->movePoint();
      break;
    case Mode::MODIFY:
      if(m_moving)
      {
        m_widget->movePoint();
      }
      else
      {
        m_widget->selectNode();
      }
      m_widget->updateCursor();
      break;
    case Mode::MARK:
    case Mode::ERASE:
      m_widget->selectNode();
      m_widget->updateCursor();
      break;
    default:
      break;
  }

  m_view->refresh();
}

//--------------------------------------------------------------------
void SkeletonWidget2D::onStrokeEnded(RenderView *view)
{
  if(view != m_view) return;

  stop();
}

//--------------------------------------------------------------------
void SkeletonWidget2D::onMousePress(Qt::MouseButtons button, const QPoint &p, RenderView *view)
{
  if(view != m_view) return;

  switch(m_mode)
  {
    case Mode::CREATE:
      break;
    case Mode::ERASE:
      m_widget->GetInteractor()->SetEventInformationFlipY(p.x(), p.y(), 0, 0);
      if(m_widget->selectNode() && m_widget->deletePoint())
      {
        emit modified(m_widget->getSkeleton());
        emit updateWidgets();
      }
      break;
    case Mode::MARK:
      if (button == Qt::LeftButton)
      {
        m_widget->GetInteractor()->SetEventInformationFlipY(p.x(), p.y(), 0, 0);
        if(m_widget->selectNode())
        {
          auto paths = m_widget->selectedPaths();

          switch(paths.size())
          {
            case 0:
              {
                auto title   = tr("Skeleton edition tool");
                auto message = tr("Cannot mark stroke as truncated");
                auto details = tr("No selected strokes.");
                DefaultDialogs::InformationMessage(message, title, details);
              }
              break;
            case 1:
              {
                auto path = paths.first();
                auto title   = tr("Skeleton edition tool");
                auto message = tr("Cannot toggle selected stroke as truncated");

                if(!path.note.startsWith("Spine", Qt::CaseInsensitive) &&
                   !path.note.startsWith("Subspine", Qt::CaseInsensitive))
                {
                  auto details = tr("Stroke is not a spine or subspine.");
                  DefaultDialogs::InformationMessage(message, title, details);
                  break;
                }

                if(m_points.contains(NmVector3{path.seen.first()->position}) ||
                   m_points.contains(NmVector3{path.seen.last()->position}))
                {
                  auto details = tr("Stroke has a connection and can't be truncated.");
                  DefaultDialogs::InformationMessage(message, title, details);
                  break;
                }

                if((path.seen.first()->isTerminal()) && (path.seen.last()->isTerminal()))
                {
                  auto details = tr("Stroke is not connected.");
                  DefaultDialogs::InformationMessage(message, title, details);
                  break;
                }

                if((!path.seen.first()->isTerminal()) && (!path.seen.last()->isTerminal()))
                {
                  auto details = tr("Stroke has no terminal points.");
                  DefaultDialogs::InformationMessage(message, title, details);
                  break;
                }

                if((path.seen.first()->isBranching()) && (path.seen.last()->isBranching()))
                {
                  auto details = tr("Stroke has not a terminal node to toggle as truncated.");
                  DefaultDialogs::InformationMessage(message, title, details);
                  break;
                }

                m_widget->markAsTruncated();
                emit modified(m_widget->getSkeleton());
                emit updateWidgets();
              }
              break;
            default:
              {
                auto title   = tr("Skeleton edition tool");
                auto message = tr("Cannot mark stroke as truncated");
                auto details = tr("The selected node belongs to more than one stroke.");
                DefaultDialogs::InformationMessage(message, title, details);
              }
              break;
          }
        }
      }
      break;
    case Mode::MODIFY:
      if (button == Qt::LeftButton)
      {
        m_widget->GetInteractor()->SetEventInformationFlipY(p.x(), p.y(), 0, 0);
        if(m_widget->selectNode())
        {
          m_moving = true;
        }
      }
      break;
    default:
      break;
  }
}

//--------------------------------------------------------------------
void SkeletonWidget2D::onMouseRelease(Qt::MouseButtons button, const QPoint &p, RenderView *view)
{
  if(view != m_view) return;

  switch (m_mode)
  {
    case Mode::MODIFY:
      if(button == Qt::LeftButton && m_moving)
      {
        m_widget->stop();
        emit modified(m_widget->getSkeleton());
        emit updateWidgets();
        m_moving = false;
      }
      break;
    case Mode::CREATE:
    case Mode::ERASE:
    case Mode::MARK:
    default:
      break;
  }
}

//--------------------------------------------------------------------
void SkeletonWidget2D::stop()
{
  if(m_mode == Mode::CREATE)
  {
    m_widget->setIgnoreCursor(false);
    m_widget->stop();

    if (m_widget->numberOfPoints() < 2)
    {
      // not allowed strokes of only one point.
      m_widget->Initialize();
    }
    else
    {
      emit modified(m_widget->getSkeleton());
      emit updateWidgets();
    }
  }
}

//--------------------------------------------------------------------
void SkeletonWidget2D::setStroke(const Core::SkeletonStroke &stroke)
{
  if(m_widget)
  {
    m_widget->setStroke(stroke);
  }
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

    disconnect(this, SIGNAL(updateWidgets()), m_handler.get(), SLOT(updateRepresentations()));

    m_handler->removeWidget(this);

    m_view = nullptr;
  }
}

//--------------------------------------------------------------------
void SkeletonWidget2D::setMode(Mode mode)
{
  m_mode = mode;
  switch(m_mode)
  {
    case Mode::CREATE:
      m_widget->setCurrentOperationMode(vtkSkeletonWidget::Define);
      break;
    case Mode::ERASE:
      m_widget->setCurrentOperationMode(vtkSkeletonWidget::Delete);
      break;
    case Mode::MARK:
      m_widget->setCurrentOperationMode(vtkSkeletonWidget::Mark);
      break;
    case Mode::MODIFY:
    default:
      m_widget->setCurrentOperationMode(vtkSkeletonWidget::Manipulate);
      break;
  }
}

//--------------------------------------------------------------------
void SkeletonWidget2D::updateRepresentation()
{
  m_widget->setRepresentationWidth(m_settings->width());
  m_widget->setRepresentationShowText(m_settings->showAnnotations());
  m_widget->setRepresentationTextSize(m_settings->annotationsSize());
  m_widget->BuildRepresentation();
}

//--------------------------------------------------------------------
void SkeletonWidget2D::setRepresentationTextColor(const QColor &color)
{
  m_widget->setRepresentationTextColor(color);
}
