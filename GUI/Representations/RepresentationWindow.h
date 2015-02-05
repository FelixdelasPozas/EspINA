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

#ifndef ESPINA_REPRESENTATIONWINDOW_H
#define ESPINA_REPRESENTATIONWINDOW_H

#include "RepresentationUpdater.h"
#include <QPair>
#include <QList>

namespace ESPINA
{

  class RepresentationWindow
  : public QObject
  {
    Q_OBJECT

    using Cursor = QPair<RepresentationUpdaterSPtr, int>;

  public:
    explicit RepresentationWindow(SchedulerSPtr scheduler, unsigned windowSize);

    QList<Cursor> moveCurrent(int distance);

    RepresentationUpdaterSPtr current() const;

    RepresentationUpdaterSList all() const;

    RepresentationUpdaterSList behind() const;

    RepresentationUpdaterSList ahead() const;

    RepresentationUpdaterSList closestBehind() const;

    RepresentationUpdaterSList closestAhead() const;

    RepresentationUpdaterSList closest() const
    { return closestBehind() + closestAhead(); }

    RepresentationUpdaterSList furtherBehind() const;

    RepresentationUpdaterSList furtherAhead() const;

    RepresentationUpdaterSList further() const
    { return furtherBehind() + furtherAhead(); }

    void incrementBuffer();

    void decrementBuffer();

    int size() const;

  signals:
    void currentUpdaterFinished();

  private:
    RepresentationUpdaterSList aheadFrom(int pos, int length) const;

    RepresentationUpdaterSList behindOf(int pos, int length) const;

    int closestDistance() const;

    int furtherDistance() const;

    unsigned nextPosition(int pos) const;

    unsigned prevPosition(int pos) const;

    unsigned innerPosition(int pos) const;

  private slots:
    void onTaskFinish();

  private:
    const unsigned BUFFER_INCREMENT = 4;

    SchedulerSPtr m_scheduler;

    int m_currentPos;
    int m_witdh;
    RepresentationUpdaterSList m_buffer;
  };

}

#endif // ESPINA_REPRESENTATIONWINDOW_H
