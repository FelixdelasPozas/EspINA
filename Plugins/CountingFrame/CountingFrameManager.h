/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#ifndef ESPINA_CF_EXTENSION_FACTORY_H
#define ESPINA_CF_EXTENSION_FACTORY_H

#include <Extensions/ExtensionFactory.h>

#include <CountingFrames/CountingFrame.h>
#include <GUI/Model/ChannelAdapter.h>

namespace EspINA {
  namespace CF {

    enum Type
    {
      ADAPTIVE,
      RECTANGULAR
    };

    class CountingFrameManager
    : public QObject
    , public ExtensionFactory
    {
      Q_OBJECT

    public:
      virtual ChannelExtensionSPtr createChannelExtension(ChannelExtension::Type type);

      virtual SegmentationExtensionSPtr createSegmentationExtension(SegmentationExtension::Type type);

      void createAdaptiveCF(ChannelAdapterPtr channel,
                            Nm inclusion[3],
                            Nm exclusion[3]);

      void createRectangularCF(ChannelAdapterPtr channel,
                               Nm inclusion[3],
                               Nm exclusion[3]);

      void deleteCountingFrame(CountingFrame *cf);

      CountingFrameList countingFrames() const
      { return m_countingFrames; }

    signals:
      void countingFrameCreated(CountingFrame *);
      void countingFrameDeleted(CountingFrame *);

    private:
      CountingFrameList m_countingFrames;
    };
  }
}

#endif // ESPINA_CF_EXTENSIONFACTORY_H
