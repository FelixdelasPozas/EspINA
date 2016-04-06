/*
 * 
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_CF_EXTENSION_FACTORY_H
#define ESPINA_CF_EXTENSION_FACTORY_H

// Plugin
#include <CountingFrames/CountingFrame.h>
#include "Extensions/CountingFrameExtension.h"
#include <GUI/Types.h>

// VTK
#include <vtkSmartPointer.h>
class vtkPolyData;

// Qt
#include <QMutex>

namespace ESPINA
{
  namespace CF
  {
    class CountingFrameManager
    : public QObject
    {
      Q_OBJECT

    public:
      /** \brief Creates and returns a counting frame extension.
       * \param[in] scheduler task scheduler.
       * \param[in] state extension state.
       *
       */
      CountingFrameExtensionSPtr createExtension(SchedulerSPtr scheduler,
                                                 const State  &state = State()) const;

      /** \brief Returns the list of created counting frames.
       *
       */
      CountingFrameList countingFrames() const
      { return m_countingFrames.keys(); }

      /** \brief Adds the given counting frame to the map of counting frames.
       * \param[in] cf counting frame object pointer.
       *
       */
      void registerCountingFrame(CountingFrame *cf);

      /** \brief Removes the given counting frame from the map of counting frames.
       * \param[in] cf counting frame object pointer.
       *
       */
      void unregisterCountingFrame(CountingFrame *cf);

      /** \brief Returns the default id for the given constraint, or "Global" if the constraint is empty.
       *
       */
      CountingFrame::Id defaultCountingFrameId(const QString& constraint) const;

      /** \brief Suggest a counting frame id for a new counting frame taking into account the existing counting frames' ids.
       *
       */
      CountingFrame::Id suggestedId(const CountingFrame::Id id) const;

    signals:
      void countingFrameCreated(CountingFrame *cf);
      void countingFrameDeleted(CountingFrame *cf);

    private:
      QMap<CountingFrame *, ChannelPtr> m_countingFrames;      /** maps counting frame with its channel. */
    };
  }
}

#endif // ESPINA_CF_EXTENSIONFACTORY_H
