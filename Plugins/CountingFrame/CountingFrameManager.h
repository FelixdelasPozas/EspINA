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

#include "CountingFramePlugin_Export.h"

// Plugin
#include "Extensions/CountingFrameExtension.h"
#include <CountingFrames/CountingFrame.h>

// ESPINA
#include <GUI/Types.h>
#include <Support/Context.h>
#include <Tasks/ApplyCountingFrame.h>

// VTK
#include <vtkSmartPointer.h>
class vtkPolyData;

// Qt
#include <QMutex>

namespace ESPINA
{
  namespace CF
  {
    class CountingFramePlugin_EXPORT CountingFrameManager
    : public QObject
    {
      Q_OBJECT

    public:
      /** \brief CountingFrame class constructor.
       *
       */
      CountingFrameManager();

      /** \brief Returns the list of created counting frames.
       *
       */
      CountingFrameList countingFrames() const;

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
      CountingFrame::Id suggestedId(const CountingFrame::Id &id) const;

      /** \brief Sets the context of the application. Only needed if executed in a graphical environment.
       * \param[in] context Application context reference.
       *
       */
      void setContext(Support::Context &context)
      { m_context = &context; }

      /** \brief Sets the extension factory for stereological inclusion extension to be used when applying the a CF.
       *
       */
      void setExtensionFactory(Core::SegmentationExtensionFactorySPtr factory)
      { m_factory = factory; }

    signals:
      void countingFrameCreated(CountingFrame *cf);
      void countingFrameDeleted(CountingFrame *cf);

    private slots:
      /** \brief Helper method to invalidate segmentation representations.
       * \param[in] cf pointer of the Counting Frame finished applying.
       *
       */
      void onCountingFrameApplied();

      void applyCountingFrame(CountingFrame *cf);

    private:
      void checkApplies();

      struct CFData
      {
          ChannelPtr channel;   /** CF channel.                                   */
          bool       needApply; /** true if needs to be applied, false otherwise. */

          CFData(): channel{nullptr}, needApply{false}{};
      };

      Support::Context                      *m_context;        /** application context.                       */
      Core::SegmentationExtensionFactorySPtr m_factory;        /** stereological inclusion extension factory. */
      QMap<CountingFrame *, CFData>          m_countingFrames; /** maps counting frame with its channel.      */

      ApplyCountingFrameSPtr m_applyTask;                 /** task to apply the counting frame to the constrained segmentations. */
      QMutex                 m_taskMutex;                 /** mutex to protect task variable.                                    */

    };
  }
}

#endif // ESPINA_CF_EXTENSIONFACTORY_H
