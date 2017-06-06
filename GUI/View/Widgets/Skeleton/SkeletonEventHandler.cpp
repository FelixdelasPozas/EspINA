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
#include <GUI/View/Widgets/Skeleton/SkeletonEventHandler.h>
#include <GUI/View/RenderView.h>
#include <GUI/View/View2D.h>

// Qt
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>

using namespace ESPINA;
using namespace ESPINA::GUI::View::Widgets::Skeleton;

//------------------------------------------------------------------------
SkeletonEventHandler::SkeletonEventHandler()
: m_tracking          {false}
, m_interpolation     {true}
, m_maxDistance2      {10}
, m_minDistance2      {0}
, m_distanceHasBeenSet{false}
, m_view              {nullptr}
{
}

//------------------------------------------------------------------------
bool SkeletonEventHandler::filterEvent(QEvent* e, RenderView* view)
{
  auto me = dynamic_cast<QMouseEvent *>(e);
  auto ke = dynamic_cast<QKeyEvent *>(e);

  switch(e->type())
  {
    case QEvent::MouseButtonPress:
      if(!m_tracking && !QApplication::keyboardModifiers().testFlag(Qt::ControlModifier))
      {
        if (me && (me->button() == Qt::LeftButton))
        {
          m_tracking = true;
          m_view     = view;

          startTrack(me->pos());
          return true;
        }

        if (!m_track.isEmpty() && me && (me->button() == Qt::RightButton))
        {
          emit stopped();
          m_track.clear();
          m_view = nullptr;
          return true;
        }
      }
      break;
    case QEvent::MouseMove:
      if (me && (view == m_view))
      {
        if(m_tracking)
        {
          updateTrack(me->pos());
          return true;
        }
        else
        {
          if(!m_track.isEmpty())
          {
            emit cursorPosition(me->pos());
            return true;
          }
        }
      }
      break;
    case QEvent::MouseButtonRelease:
      if (m_tracking && me && (me->button() == Qt::LeftButton))
      {
        m_tracking = false;
        updateTrack(me->pos());
        return true;
      }
      break;
    case QEvent::Leave:
      if (m_tracking)
      {
        m_tracking = false;
        updateTrack(me->pos());

        emit stopped();
        m_track.clear();
        m_view = nullptr;
      }
      else
      {
        if (!m_track.isEmpty())
        {
          emit cancelled();
          m_track.clear();
          return false; // let the view also handle the event.
        }
      }
      break;
    case QEvent::KeyPress:
      if(ke && ke->key() == Qt::Key_Shift)
      {
        emit modifier(true);
      }
      break;
    case QEvent::KeyRelease:
      if(ke && ke->key() == Qt::Key_Shift)
      {
        emit modifier(false);
      }
      break;
      return true;
      break;
    default:
      break;
  }

  return false;
}

//------------------------------------------------------------------------
void SkeletonEventHandler::setInterpolation(bool active)
{
  m_interpolation = active;
}

//------------------------------------------------------------------------
void SkeletonEventHandler::setMaximumPointDistance(const Nm distance)
{
  m_distanceHasBeenSet = true;
  m_maxDistance2 = distance * distance;
  if(m_maxDistance2 < m_minDistance2) m_maxDistance2 = m_minDistance2;
}

//------------------------------------------------------------------------
void SkeletonEventHandler::setMinimumPointDistance(const Nm distance)
{
  m_distanceHasBeenSet = true;
  m_minDistance2 = distance * distance;
  if(m_minDistance2 > m_maxDistance2) m_maxDistance2 = m_minDistance2;
}

//------------------------------------------------------------------------
bool SkeletonEventHandler::isTracking() const
{
  return m_tracking;
}

//------------------------------------------------------------------------
void SkeletonEventHandler::startTrack(const QPoint &pos)
{
  if(m_view)
  {
    if(!m_distanceHasBeenSet)
    {
      // if not set, the default is minimum spacing, and max is 5* minimum
      auto res    = m_view->sceneResolution();
      auto view2d = view2D_cast(m_view);
      auto planeIndex = normalCoordinateIndex(view2d->plane());
      double minRes = VTK_DOUBLE_MAX;
      for(int i: {0,1,2})
      {
        if(i == planeIndex) continue;

        minRes = std::min(res[i], minRes);
      }
      m_minDistance2 = minRes*minRes;
      m_maxDistance2 = 5 * m_minDistance2;
    }

    m_track.clear();

    m_track << pos;

    emit started(m_track, m_view);
  }
}

//------------------------------------------------------------------------
void SkeletonEventHandler::updateTrack(const QPoint &pos)
{
  if(m_view)
  {
    m_updatedTrack.clear();

    auto viewPoint1 = m_view->worldEventPosition(pos);
    auto viewPoint2 = m_view->worldEventPosition(m_track.last());
    auto distance = distance2(viewPoint1, viewPoint2);

    if(distance < m_minDistance2)
    {
      emit cursorPosition(pos);
      return;
    }

    if(distance > m_maxDistance2 && m_interpolation)
    {
      m_updatedTrack << interpolate(m_track.last(), pos);
    }
    else
    {
      m_updatedTrack << pos;
    }

    emit updated(m_updatedTrack);

    m_track << m_updatedTrack;
  }
}

//------------------------------------------------------------------------
SkeletonEventHandler::Track SkeletonEventHandler::interpolate(const QPoint &point1, const QPoint &point2)
{
  Track track;

  if(m_view)
  {
    auto viewPoint1 = m_view->worldEventPosition(point1);
    auto viewPoint2 = m_view->worldEventPosition(point2);
    auto distance = distance2(viewPoint1, viewPoint2);

    if(distance > m_maxDistance2)
    {
      int chunks = sqrt(static_cast<int>(distance/m_maxDistance2));

      Q_ASSERT(chunks > 0);

      QPoint vector{ point2.x()-point1.x(), point2.y()-point1.y() };
      QPointF delta{ static_cast<double>(vector.x()/chunks), static_cast<double>(vector.y()/chunks) };

      for(auto i = 0; i < chunks - 1; ++i)
      {
        auto pointCenter = QPoint{static_cast<int>(point1.x() + (delta.x() * i)),
                                  static_cast<int>(point1.y() + (delta.y() * i))};
        track << pointCenter;
      }
    }

    track << point2;
  }

  return track;
}

//------------------------------------------------------------------------
Nm SkeletonEventHandler::distance2(const NmVector3 &p1, const NmVector3 &p2)
{
  double point1[3] = { p1[0], p1[1], p1[2] };
  double point2[3] = { p2[0], p2[1], p2[2] };

  return vtkMath::Distance2BetweenPoints(point1, point2);
}

