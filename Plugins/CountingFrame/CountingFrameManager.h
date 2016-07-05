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
#include "Extensions/CountingFrameExtension.h"
#include <CountingFrames/CountingFrame.h>

// ESPINA
#include <GUI/Types.h>
#include <Support/Context.h>

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
      /** \brief CountingFrame class constructor.
       * \param[in] context application context reference.
       *
       */
      CountingFrameManager(Support::Context &context);

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

    private slots:
      /** \brief Helper method to invalidate segmentation representations.
       * \param[in] cf pointer of the Counting Frame finished applying.
       *
       */
      void onCountingFrameApplied(CountingFrame *cf);

    private:
      Support::Context                 &m_context;        /** application context.                  */
      QMap<CountingFrame *, ChannelPtr> m_countingFrames; /** maps counting frame with its channel. */
    };
  }
}

#endif // ESPINA_CF_EXTENSIONFACTORY_H
