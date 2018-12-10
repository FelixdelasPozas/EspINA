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
using namespace ESPINA::Core;
using namespace ESPINA::Support;
using namespace ESPINA::Extensions;
using namespace ESPINA::SkeletonToolsUtils;
using namespace ESPINA::GUI::View::Widgets::Skeleton;
using namespace ESPINA::GUI::Representations::Managers;

//------------------------------------------------------------------------
SkeletonToolsEventHandler::SkeletonToolsEventHandler(Context &context)
: SkeletonEventHandler()
, WithContext     (context)
, m_operation     {OperationMode::NORMAL}
, m_strokeMenu    {nullptr}
, m_connectionMenu{nullptr}
, m_cancelled     {false}
, m_plane         {Plane::UNDEFINED}
, m_isStartPoint  {true}
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

  if((m_connectionMenu && m_connectionMenu->isVisible()) || (m_strokeMenu && m_strokeMenu->isVisible())) return false;

  bool usedMenu = false;

  switch(e->type())
  {
    case QEvent::MouseButtonPress:
      if(!m_tracking && me && !QApplication::keyboardModifiers().testFlag(Qt::ControlModifier))
      {
        if(me->button() == Qt::RightButton)
        {
          emit clearConnections();
          m_operation = OperationMode::NORMAL;
          break;
        }

        if(me->button() != Qt::LeftButton) break;

        auto position = me->pos();
        auto point    = view->worldEventPosition();
        auto spacing  = view->sceneResolution();
        for(auto i: {0,1,2}) point[i] = std::round(point[i]/spacing[i])*spacing[i];

        if ((m_mode == SkeletonEventHandler::Mode::CREATE) && m_strokeMenu)
        {
          if(m_track.isEmpty())
          {
            emit checkStartNode(point);

            if(!m_isStartPoint) break;

            fixMenuPositionForView(m_strokeMenu, view, position);
            m_strokeMenu->setParent(view);
            m_strokeMenu->exec(position);
            usedMenu = true;

            if (m_cancelled)
            {
              m_cancelled = false;
              return true;
            }

            if(isCollision(point))
            {
              m_point = point;
              emit addConnectionPoint(m_point);
              view->refresh();
              m_operation = OperationMode::COLLISION_START;
            }
            break;
          }
          else
          {
            if(m_operation == OperationMode::COLLISION_START)
            {
              m_operation = OperationMode::NORMAL;
              if(isDendrite())
              {
                if(m_lastStroke.name.compare("Synapse on shaft", Qt::CaseInsensitive) == 0)
                {
                  emit changeStrokeTo(m_category, 0, m_plane); // Shaft
                  return true;
                }

                if(m_lastStroke.name.startsWith("Synapse on", Qt::CaseInsensitive) && m_lastStroke.name.contains("spine", Qt::CaseInsensitive))
                {
                  emit changeStrokeTo(m_category, 1, m_plane); // Spine
                  return true;
                }
              }
              else
              {
                if(isAxon() && m_lastStroke.name.startsWith("Synapse", Qt::CaseInsensitive))
                {
                  emit changeStrokeTo(m_category, 0, m_plane); // Shaft
                  return true;
                }
              }
            }

            if(m_operation == OperationMode::COLLISION_MIDDLE && m_connectionMenu)
            {
              m_operation = OperationMode::NORMAL;
              if(m_lastStroke.name.startsWith("Shaft"))
              {
                if(isDendrite())
                {
                  emit signalConnection(m_category, 3, m_plane); // Synapse on shaft.
                }
                else
                {
                  emit signalConnection(m_category, 1, m_plane); // Synapse en passant.
                }
              }
              else
              {
                fixMenuPositionForView(m_connectionMenu, view, position);
                m_connectionMenu->setParent(view);
                m_connectionMenu->exec(position);
                usedMenu = true;

                if (m_cancelled) // can be cancelled leaving the view.
                {
                  m_cancelled = false;
                  return true;
                }
              }

              break;
            }

            if(isCollision(point))
            {
              m_point = point;
              emit addConnectionPoint(m_point);
              view->refresh();
              m_operation = OperationMode::COLLISION_MIDDLE;
            }
          }
        }
      }
      break;
    case QEvent::Leave:
      if(m_strokeMenu && m_strokeMenu->isVisible())
      {
        m_cancelled = true;
        m_strokeMenu->close();
      }
      if(m_connectionMenu && m_connectionMenu->isVisible())
      {
        m_cancelled = true;
        m_connectionMenu->close();
      }
      m_operation = OperationMode::NORMAL;
      break;
    default:
      break;
  }

  auto result = SkeletonEventHandler::filterEvent(e, view);

  if(usedMenu)
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
        m_lastStroke = strokes.at(index);
        emit selectedStroke(index);
      }
      else
      {
        m_lastStroke = SkeletonStroke();
      }
    }

    m_cancelled = (index >= strokes.size());
  }
}

//------------------------------------------------------------------------
void SkeletonToolsEventHandler::setStrokesCategory(const QString &category)
{
  if(m_strokeMenu)
  {
    if(m_strokeMenu->isVisible()) m_strokeMenu->close();
    m_strokeMenu->setParent(nullptr);
    disconnect(m_strokeMenu, SIGNAL(triggered(QAction *)), this, SLOT(onActionSelected(QAction *)));
    delete m_strokeMenu;

    if(m_connectionMenu)
    {
      if(m_connectionMenu->isVisible()) m_connectionMenu->close();
      m_connectionMenu->setParent(nullptr);
      disconnect(m_connectionMenu, SIGNAL(triggered(QAction *)), this, SLOT(onActionSelected(QAction *)));
      delete m_connectionMenu;
    }

    m_strokeMenu = nullptr;
    m_connectionMenu = nullptr;
  }

  m_category = category;

  if(STROKES.keys().contains(m_category))
  {
    if(STROKES[m_category].size() != 1)
    {
      m_strokeMenu = SkeletonToolsUtils::createStrokesContextMenu("Start Trace", m_category);
      connect(m_strokeMenu, SIGNAL(triggered(QAction *)), this, SLOT(onActionSelected(QAction *)));

      if(isSpecialCategory())
      {
        m_connectionMenu = SkeletonToolsUtils::createStrokesContextMenu("Connection Type", m_category);
        connect(m_connectionMenu, SIGNAL(triggered(QAction *)), this, SLOT(onActionSelected(QAction *)));
      }
    }
  }
}

//------------------------------------------------------------------------
void SkeletonToolsEventHandler::onHandlerUseChanged(bool enabled)
{
  if(!enabled)
  {
    if(m_strokeMenu && m_strokeMenu->isVisible())
    {
      m_cancelled = true;
      m_strokeMenu->close();
      m_strokeMenu->setParent(nullptr);
    }

    if(m_connectionMenu && m_connectionMenu->isVisible())
    {
      m_cancelled = true;
      m_connectionMenu->close();
      m_connectionMenu->setParent(nullptr);
    }

    m_operation = OperationMode::NORMAL;
  }
}

//------------------------------------------------------------------------
bool SkeletonToolsEventHandler::isCollision(const NmVector3& point) const
{
  if(isSpecialCategory())
  {
    auto candidates = getModel()->contains(point);
    for (auto candidate: candidates)
    {
      auto seg = std::dynamic_pointer_cast<SegmentationAdapter>(candidate);

      if (seg                                                                              &&
          hasVolumetricData(seg->output())                                                 &&
          seg->category()->classificationName().startsWith("Synapse", Qt::CaseInsensitive) &&
          isSegmentationVoxel(readLockVolume(seg->output()), point))
      {
        return true;
      }
    }
  }

  return false;
}

//------------------------------------------------------------------------
void SkeletonToolsEventHandler::fixMenuPositionForView(const QMenu *menu, const RenderView* view, QPoint& position)
{
  auto size = view->size();
  auto hint = menu->sizeHint();

  if(size.height() - position.y() < hint.height())
  {
    position.setY(size.height() - hint.height() - 10);
  }

  if(size.width() - position.x() < hint.width())
  {
    position.setX(size.width() - hint.width() - 10);
  }
}
