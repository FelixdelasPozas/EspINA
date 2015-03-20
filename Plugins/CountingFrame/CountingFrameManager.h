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

#include <CountingFrames/CountingFrame.h>
#include "Extensions/CountingFrameExtension.h"
#include <GUI/Model/ChannelAdapter.h>
#include <GUI/Model/SegmentationAdapter.h>

namespace ESPINA {
  namespace CF {

    class CountingFrameManager
    : public QObject
    {
      Q_OBJECT

    public:
      CountingFrameExtensionSPtr createExtension(SchedulerSPtr scheduler,
                                                 const State& state = State()) const;

      CountingFrameList countingFrames() const
      { return m_countingFrames.keys(); }

      void registerCountingFrame(CountingFrame *cf);

      void unregisterCountingFrame(CountingFrame *cf);

      CountingFrame::Id defaultCountingFrameId(const QString& constraint) const;

      CountingFrame::Id suggestedId(const CountingFrame::Id id) const;

    signals:
      void countingFrameCreated(CountingFrame *);
      void countingFrameDeleted(CountingFrame *);

    private:
      QMap<CountingFrame *, ChannelPtr> m_countingFrames;
    };
  }
}

#endif // ESPINA_CF_EXTENSIONFACTORY_H
