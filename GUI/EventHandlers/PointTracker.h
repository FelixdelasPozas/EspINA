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

#ifndef ESPINA_POINT_TRACKER_H
#define ESPINA_POINT_TRACKER_H

#include <GUI/View/EventHandler.h>
#include <Core/Utils/NmVector3.h>

namespace ESPINA
{

  class PointTracker
  : public EventHandler
  {
    Q_OBJECT

  public:
    using Track = QList<NmVector3>;

  public:
    explicit PointTracker();

    virtual bool filterEvent(QEvent *e, RenderView *view = nullptr);

    void setInterpolation(bool active);

    void setMinimumInterpolationDistance(Nm distance);

    bool isTracking() const;

  signals:
    void trackStarted(Track track, RenderView *view);
    void trackUpdated(Track track);
    void trackStopped(Track track, RenderView *view);;

  private:
    void startTrack(const QPoint &pos, RenderView *view);

    void updateTrack(const QPoint &pos);

    void stopTrack(const QPoint &pos);

    Track interpolate(const NmVector3 &p1, const NmVector3 &p2);

    Nm distance2(const NmVector3 &p1, const NmVector3 &p2);

  private:
    bool  m_tracking;
    int   m_interpolation;
    Nm    m_minDistance2;
    Track m_track;
    Track m_updatedTrack;

    RenderView *m_view;
  };

  using PointTrackerSPtr = std::shared_ptr<PointTracker>;
}

#endif // ESPINA_POINT_TRACKER_H
