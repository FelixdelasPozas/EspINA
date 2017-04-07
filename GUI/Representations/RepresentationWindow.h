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

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include "RepresentationUpdater.h"

// Qt
#include <QPair>
#include <QList>

namespace ESPINA
{
  /** \class RepresentationWindow
   * \brief Manages a circular buffer position and movement for representations.
   *
   */
  class EspinaGUI_EXPORT RepresentationWindow
  : public QObject
  {
      Q_OBJECT

      using Cursor = QPair<RepresentationUpdaterSPtr, int>;

    public:
      /** \brief RepresentationWindow class constructor.
       * \param[in] scheduler task scheduler.
       * \param[in] pipeline pipeline of the representation.
       * \param[in] windowSize size of the window.
       *
       */
      explicit RepresentationWindow(SchedulerSPtr scheduler, RepresentationPipelineSPtr pipeline, unsigned windowSize);

      /** \brief Moves current position "distance" positions.
       * \param[in] distance int numerical value.
       *
       */
      QList<Cursor> moveCurrent(int distance);

      /** \brief Returns the updater in the current position.
       *
       */
      RepresentationUpdaterSPtr current() const;

      /** \brief Returns all updaters in the window.
       *
       */
      RepresentationUpdaterSList all() const;

      /** \brief Returns all the updaters behind the current position in the window.
       *
       */
      RepresentationUpdaterSList behind() const;

      /** \brief Returns all the updaters ahead the current position in the window.
       *
       */
      RepresentationUpdaterSList ahead() const;

      /** \brief Returns the closest updaters behind the current position in the window.
       *
       * NOTE: the closes distance is half the size of the window. Farther is the rest
       * of the window minus the closest.
       *
       */
      RepresentationUpdaterSList closestBehind() const;

      /** \brief Returns the closest updaters ahead the current position in the window.
       *
       * NOTE: the closes distance is half the size of the window. Farther is the rest
       * of the window minus the closest.
       *
       */
      RepresentationUpdaterSList closestAhead() const;

      /** \brief Returns the closest updaters around the current position in the window.
       *
       * NOTE: the closes distance is half the size of the window. Farther is the rest
       * of the window minus the closest.
       *
       */
      RepresentationUpdaterSList closest() const
      { return closestBehind() + closestAhead(); }

      /** \brief Returns the farther updaters behind the current position in the window.
       *
       * NOTE: the closes distance is half the size of the window. Farther is the rest
       * of the window minus the closest.
       *
       */
      RepresentationUpdaterSList fartherBehind() const;

      /** \brief Returns the farther updaters ahead the current position in the window.
       *
       * NOTE: the closes distance is half the size of the window, rounded integer.
       *
       */
      RepresentationUpdaterSList fartherAhead() const;

      /** \brief Returns the farther updaters around the current position in the window.
       *
       * NOTE: the closes distance is half the size of the window, rounded integer.
       *
       */
      RepresentationUpdaterSList farther() const
      { return fartherBehind() + fartherAhead(); }

      /** \brief Increments the size of the window buffer by BUFFER_INCREMENT positions.
       *
       */
      void incrementBuffer();

      /** \brief Increments the size of the window buffer by BUFFER_INCREMENT positions.
       *
       */
      void decrementBuffer();

      /** \brief Returns the size of the window.
       *
       */
      int size() const;

    signals:
      void actorsReady(const GUI::Representations::FrameCSPtr frame, RepresentationPipeline::Actors actors);

    private:
      /** \brief Returns the list of updaters ahead of the given position up to the given length.
       * \param[in] pos buffer position.
       * \param[in] length distance in positions.
       *
       */
      RepresentationUpdaterSList aheadFrom(int pos, int length) const;

      /** \brief Returns the list of updaters behind the given position up to the given length.
       * \param[in] pos buffer position.
       * \param[in] length distance in positions.
       *
       */
      RepresentationUpdaterSList behindOf(int pos, int length) const;

      /** \brief Returns the value of the closest distance.
       *
       */
      int closestDistance() const;

      /** \brief Returns the value in positions of the farthest distance.
       *
       */
      int fartherDistance() const;

      /** \brief Returns the next position from the given one.
       * \param[in] pos buffer position.
       *
       */
      unsigned int nextPosition(int pos) const;

      /** \brief Returns the previous position from the given one.
       * \param[in] pos buffer position.
       *
       */
      unsigned int prevPosition(int pos) const;

      /** \brief Returns the buffer position from the given one taking into account buffer
       * overflow/underflow.
       * \param[in] pos buffer position.
       *
       */
      unsigned int innerPosition(int pos) const;

    private:
      /** \brief For debug purposes.
       * \param[inout] debug QDebug stream.
       * \param[in] cursors list of updaters and associated distances from the current buffer position.
       *
       */
      friend QDebug operator<< (QDebug debug, const QList<RepresentationWindow::Cursor> &cursors);

      const unsigned BUFFER_INCREMENT = 4;     /** fixed buffer increment/decrement value.  */

      unsigned int               m_currentPos; /** current position of actual frame actors. */
      unsigned int               m_width;      /** representation's buffer width.           */
      RepresentationUpdaterSList m_buffer;     /** representation's buffer.                 */
  };

  QDebug EspinaGUI_EXPORT operator<< (QDebug debug, const QList<RepresentationWindow::Cursor> &cursors);

}

#endif // ESPINA_REPRESENTATIONWINDOW_H
