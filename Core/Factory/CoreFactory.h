/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

    This program is free software: you can redistribute it and/or modify
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


#ifndef ESPINA_CORE_FACTORY_H
#define ESPINA_CORE_FACTORY_H

#include "EspinaCore_Export.h"

#include "Core/Factory/FilterFactory.h"
#include "Core/Factory/ChannelExtensionFactory.h"
#include "Core/Factory/SegmentationExtensionFactory.h"

#include <QStringList>
#include <QMap>

namespace EspINA
{

  class EspinaCore_EXPORT CoreFactory
  {
  public:
    struct Factory_Already_Registered_Exception{};
    struct Unknown_Type_Exception{};

  public:
    explicit CoreFactory(SchedulerSPtr scheduler = SchedulerSPtr());
    ~CoreFactory();

    void registerFilterFactory(FilterFactorySPtr factory) throw (Factory_Already_Registered_Exception);

    FilterSPtr createFilter(OutputSList inputs, const Filter::Type& type) const throw (Unknown_Type_Exception);

    template<typename T>
    std::shared_ptr<T> createFilter(OutputSList inputs, Filter::Type type) const
    {
      auto filter = std::shared_ptr<T>{new T(inputs, type, m_scheduler)};
      filter->setStorage(m_defaultStorage);
      return filter;
    }

    SampleSPtr createSample(const QString& name = QString()) const;

    ChannelSPtr createChannel(FilterSPtr filter, Output::Id output) const;

    void registerExtensionFactory(ChannelExtensionFactorySPtr factory) throw (Factory_Already_Registered_Exception);

    ChannelExtensionTypeList availableChannelExtensions() const;

    ChannelExtensionSPtr createChannelExtension(ChannelExtension::Type type, const State &state = State());

    SegmentationSPtr createSegmentation(FilterSPtr filter, Output::Id output) const;

    void registerExtensionFactory(SegmentationExtensionFactorySPtr factory) throw (Factory_Already_Registered_Exception);

    SegmentationExtensionTypeList availableSegmentationExtensions() const;

    SegmentationExtensionSPtr createSegmentationExtension(SegmentationExtension::Type type, const State &state = State());

    void setPresistentStorage(TemporalStorageSPtr storage)
    { m_defaultStorage = storage; }

  private:
    SchedulerSPtr m_scheduler;
    TemporalStorageSPtr m_defaultStorage;

    QMap<Filter::Type,
         FilterFactorySPtr>                m_filterFactories;

    QMap<ChannelExtension::Type,
         ChannelExtensionFactorySPtr>      m_channelExtensionFactories;

    QMap<SegmentationExtension::Type,
         SegmentationExtensionFactorySPtr> m_segmentationExtensionFactories;
  };

}// namespace EspINA

#endif // ESPINA_CORE_FACTORY_H
