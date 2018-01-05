/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include "SkeletonToolsEventHandler.h"
#include "SkeletonToolsUtils.h"
#include "ConnectionPointsTemporalRepresentation2D.h"
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>
#include <Extensions/EdgeDistances/ChannelEdges.h>
#include <GUI/View/RenderView.h>
#include <GUI/View/View2D.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/Model/SegmentationAdapter.h>

// Qt
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QMenu>
#include <QLabel>
#include <QWidgetAction>

// VTK
#include <vtkPolyData.h>

using namespace ESPINA;
using namespace ESPINA::Support;
using namespace ESPINA::Extensions;
using namespace ESPINA::SkeletonToolsUtils;
using namespace ESPINA::GUI::View::Widgets::Skeleton;
using namespace ESPINA::GUI::Representations::Managers;

//------------------------------------------------------------------------
class EdgesCreatorHelper
: public Task
{
  public:
    explicit EdgesCreatorHelper(ChannelEdgesSPtr edges, Support::Context &context)
    : Task(context.scheduler())
    , m_edges{edges}
    {};

    virtual ~EdgesCreatorHelper()
    {};

    virtual void run()
    { m_edges->channelEdges(); }

  private:
    ChannelEdgesSPtr m_edges;
};

//------------------------------------------------------------------------
SkeletonToolsEventHandler::SkeletonToolsEventHandler(Context &context)
: SkeletonEventHandler()
, WithContext     (context)
, m_contextMenu   {nullptr}
, m_connectionMenu{nullptr}
, m_checkCollision{false}
, m_cancelled     {false}
, m_edges         {nullptr}
, m_plane         {Plane::UNDEFINED}
{
  connect(this, SIGNAL(eventHandlerInUse(bool)), this, SLOT(onHandlerUseChanged(bool)));
}

//------------------------------------------------------------------------
SkeletonToolsEventHandler::~SkeletonToolsEventHandler()
{
}

//------------------------------------------------------------------------
bool SkeletonToolsEventHandler::filterEvent(QEvent* e, RenderView* view)
{
  auto me = dynamic_cast<QMouseEvent *>(e);

  if(view->type() == ViewType::VIEW_3D) return false;
  m_plane = view2D_cast(view)->plane();

  static bool isHandlingMenu = false;
  static bool hasCollision = false;
  if(isHandlingMenu) return false;
  bool processed = false;

  switch(e->type())
  {
    case QEvent::MouseButtonPress:
      if(!m_tracking && me && !QApplication::keyboardModifiers().testFlag(Qt::ControlModifier))
      {
        auto position = me->pos();

        if(me->button() == Qt::RightButton)
        {
          emit clearConnections();
          break;
        }

        if ((me->button() == Qt::LeftButton) && (m_mode == SkeletonEventHandler::Mode::CREATE) && m_contextMenu)
        {
          if(m_track.isEmpty())
          {
            processed = true;
            isHandlingMenu = true;
            m_contextMenu->setParent(view);
            m_contextMenu->exec(position);
            isHandlingMenu = false;

            if (m_cancelled)
            {
              m_cancelled = false;
              return true;
            }

            auto bounds  = getActiveChannel()->bounds();
            auto point   = view->worldEventPosition();
            if(!contains(bounds, point)) break;

            auto spacing = getActiveChannel()->output()->spacing();
            Nm tolerance = 100;
            for(auto i: {0,1,2})
            {
              if(i == normalCoordinateIndex(m_plane)) continue;
              tolerance = std::min(tolerance, spacing[i]);
            }
            tolerance *=3; // three pixels wide at least in the smallest direction.

            if(m_edges && m_edges->areEdgesAvailable() && m_edges->isPointOnEdge(point, tolerance))
            {
              emit addEntryPoint(point);
            }
          }
          else
          {
            if(hasCollision && m_connectionMenu)
            {
              processed = true;
              isHandlingMenu = true;
              m_connectionMenu->setParent(view);
              m_connectionMenu->exec(position);
              isHandlingMenu = false;
              hasCollision = false;

              if (m_cancelled) // can be cancelled leaving the view.
              {
                m_cancelled = false;
                return true;
              }
            }
          }
        }

        if (m_checkCollision)
        {
          auto point = view->worldEventPosition();

          auto candidates = getModel()->contains(point);
          for (auto candidate: candidates)
          {
            auto seg = std::dynamic_pointer_cast<SegmentationAdapter>(candidate);
            if (seg && hasVolumetricData(seg->output()) && seg->category()->classificationName().startsWith("Synapse", Qt::CaseInsensitive))
            {
              if(isSegmentationVoxel(readLockVolume(seg->output()), point))
              {
                m_point = point;
                emit addConnectionPoint(m_point);
                view->refresh();
                hasCollision = true;
              }
            }
          }
        }
      }
      break;
    case QEvent::Leave:
      if(m_contextMenu && m_contextMenu->isVisible())
      {
        m_cancelled = true;
        m_contextMenu->close();
      }
      break;
    default:
      break;
  }

  auto result = SkeletonEventHandler::filterEvent(e, view);

  if(processed)
  {
    // if we've processed the event with the menu we need a mouse release event to interact correctly.
    auto releaseEvent = new QMouseEvent(QEvent::MouseButtonRelease, me->pos(), me->button(), me->buttons(), me->modifiers());
    result = SkeletonEventHandler::filterEvent(releaseEvent, view);
  }

  return result;
}

//------------------------------------------------------------------------
void SkeletonToolsEventHandler::onActionSelected(QAction *action)
{
  auto menu = qobject_cast<QMenu *>(sender());
  if(menu)
  {
    menu->close();
    menu->setParent(nullptr);
    int index = 0;
    auto actions = menu->actions();

    for(int i = 0; i < actions.size(); ++i)
    {
      if(actions.at(i) == action) break;
      ++index;
    }

    --index; // the first action is the title.

    auto strokes = STROKES[m_category]; // Q_ASSERT(m_category is valid)
    if(menu == m_connectionMenu)
    {
      if(index < strokes.size())
      {
        emit signalConnection(m_category, index, m_plane);
      }
      else
      {
        emit removeConnectionPoint(m_point);
      }
    }
    else
    {
      if(index < strokes.size())
      {
        emit selectedStroke(index);
      }
    }

    m_cancelled = (index >= strokes.size());
  }
}

//------------------------------------------------------------------------
void SkeletonToolsEventHandler::setStrokesCategory(const QString &category)
{
  if(m_contextMenu)
  {
    if(m_contextMenu->isVisible()) m_contextMenu->close();
    m_contextMenu->setParent(nullptr);
    disconnect(m_contextMenu, SIGNAL(triggered(QAction *)), this, SLOT(onActionSelected(QAction *)));
    delete m_contextMenu;

    if(m_connectionMenu)
    {
      if(m_connectionMenu->isVisible()) m_connectionMenu->close();
      m_connectionMenu->setParent(nullptr);
      disconnect(m_connectionMenu, SIGNAL(triggered(QAction *)), this, SLOT(onActionSelected(QAction *)));
      delete m_connectionMenu;
    }

    m_contextMenu = nullptr;
    m_connectionMenu = nullptr;
  }

  m_category = category;

  if(STROKES.keys().contains(m_category))
  {
    m_checkCollision = m_category.startsWith("Dendrite", Qt::CaseInsensitive) || m_category.startsWith("Axon", Qt::CaseInsensitive);

    m_contextMenu = SkeletonToolsUtils::createStrokesContextMenu("Start stroke", m_category);
    connect(m_contextMenu, SIGNAL(triggered(QAction *)), this, SLOT(onActionSelected(QAction *)));

    if(m_checkCollision)
    {
      m_connectionMenu = SkeletonToolsUtils::createStrokesContextMenu("Connection Type", m_category);
      connect(m_connectionMenu, SIGNAL(triggered(QAction *)), this, SLOT(onActionSelected(QAction *)));
    }
  }
}

//------------------------------------------------------------------------
void SkeletonToolsEventHandler::onHandlerUseChanged(bool enabled)
{
  if(!enabled && m_contextMenu && m_contextMenu->isVisible())
  {
    m_contextMenu->close();
    m_contextMenu->setParent(nullptr);
  }

  if(!enabled && m_connectionMenu && m_connectionMenu->isVisible())
  {
    m_connectionMenu->close();
    m_connectionMenu->setParent(nullptr);
  }

  if(enabled)
  {
    auto stack = getActiveChannel();
    if(stack)
    {
      m_edges = stack->readOnlyExtensions()->get<ChannelEdges>();
      if(!m_edges->areEdgesAvailable())
      {
        auto helper = std::make_shared<EdgesCreatorHelper>(m_edges, getContext());

        Task::submit(helper);
      }
    }
  }
  else
  {
    m_edges = nullptr;
  }
}
