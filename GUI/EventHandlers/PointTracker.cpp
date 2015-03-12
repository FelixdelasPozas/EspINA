/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "PointTracker.h"

#include <GUI/View/RenderView.h>
#include <QMouseEvent>
#include <QApplication>

using namespace ESPINA;

//------------------------------------------------------------------------
PointTracker::PointTracker()
: m_tracking          {false}
, m_interpolation     {true}
, m_maxDistance2      {10}
, m_distanceHasBeenSet{false}
, m_view              {nullptr}
{
}

//------------------------------------------------------------------------
bool PointTracker::filterEvent(QEvent *e, RenderView *view)
{
  QMouseEvent *me = static_cast<QMouseEvent *>(e);

  switch(e->type())
  {
    case QEvent::Enter:
    case QEvent::MouseButtonPress:
      if(!m_tracking && !QApplication::keyboardModifiers().testFlag(Qt::ControlModifier))
      {
        if (me->button() == Qt::LeftButton)
        {
          m_tracking = true;
          startTrack(me->pos(), view);
          return true;
        }
      }
      break;
    case QEvent::MouseMove:
      if (m_tracking)
      {
        updateTrack(me->pos());
        return true;
      }
      break;
    case QEvent::MouseButtonRelease:
    case QEvent::Leave:
      if (m_tracking)
      {
        m_tracking = false;
        stopTrack(me->pos());
        return true;
      }
      break;
    default:
      break;
  }

  return false;
}

//------------------------------------------------------------------------
void PointTracker::setInterpolation(bool active)
{
  m_interpolation = active;
}

//------------------------------------------------------------------------
void PointTracker::setMaximumPointDistance(Nm distance)
{
  m_distanceHasBeenSet = true;
  m_maxDistance2 = distance * distance;
  qDebug() << "set max point distance" << distance;
}

//------------------------------------------------------------------------
bool PointTracker::isTracking() const
{
  return m_tracking;
}

//------------------------------------------------------------------------
void PointTracker::startTrack(const QPoint &pos, RenderView *view)
{
  if(!m_distanceHasBeenSet)
  {
    auto res    = view->sceneResolution();
    auto minRes = 4*std::min(res[0], std::min(res[1], res[2]));
    m_maxDistance2 = minRes*minRes;
  }

  m_track.clear();
  m_view = view;

  m_track << view->worldEventPosition(pos);

  emit trackStarted(m_track, view);
}

//------------------------------------------------------------------------
void PointTracker::updateTrack(const QPoint &pos)
{
  m_updatedTrack.clear();

  auto point = m_view->worldEventPosition(pos);

  if(m_interpolation)
  {
    m_updatedTrack << interpolate(m_track.last(), point);
  }
  else
  {
    m_updatedTrack << point;
  }

  emit trackUpdated(m_updatedTrack);

  m_track << m_updatedTrack;
}

//------------------------------------------------------------------------
void PointTracker::stopTrack(const QPoint &pos)
{
  updateTrack(pos);

  emit trackStopped(m_track, m_view);
}

//------------------------------------------------------------------------
PointTracker::Track PointTracker::interpolate(const NmVector3 &point1, const NmVector3 &point2)
{
  Track track;
  auto distance = distance2(point1, point2);

  if(distance > m_maxDistance2)
  {
    int chunks = sqrt(static_cast<int>(distance/m_maxDistance2));

    Q_ASSERT(chunks > 0);

    double vector[3] = { point2[0]-point1[0], point2[1]-point1[1], point2[2]-point1[2] };
    double delta[3]  = { vector[0]/chunks, vector[1]/chunks, vector[2]/chunks };

    for(auto i = 0; i < chunks - 1; ++i)
    {
      auto pointCenter = NmVector3{point1[0] + (delta[0] * i),
                                   point1[1] + (delta[1] * i),
                                   point1[2] + (delta[2] * i)};
      track << pointCenter;
    }
  }

  track << point2;

  return track;
}

//------------------------------------------------------------------------
Nm PointTracker::distance2(const NmVector3 &p1, const NmVector3 &p2)
{
  double point1[3] = { p1[0], p1[1], p1[2] };
  double point2[3] = { p2[0], p2[1], p2[2] };

  return vtkMath::Distance2BetweenPoints(point1, point2);
}