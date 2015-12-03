/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include "CoreFactory.h"
#include "Core/Analysis/Sample.h"
#include "Core/Analysis/Channel.h"
#include <Core/Analysis/Segmentation.h>
#include <Core/Utils/TemporalStorage.h>

using namespace ESPINA;

//------------------------------------------------------------------------
CoreFactory::CoreFactory(SchedulerSPtr scheduler)
: m_scheduler{scheduler}
{
  m_defaultStorage = std::make_shared<TemporalStorage>();
}


//-----------------------------------------------------------------------------
CoreFactory::~CoreFactory()
{
}

//-----------------------------------------------------------------------------
void CoreFactory::registerFilterFactory(FilterFactorySPtr factory)
{
  for(auto filter : factory->providedFilters())
  {
    if (m_filterFactories.contains(filter)) throw Factory_Already_Registered_Exception();

    m_filterFactories[filter] = factory;
  }
}


//-----------------------------------------------------------------------------
SampleSPtr CoreFactory::createSample(const QString& name) const
{
  return std::make_shared<Sample>(name);
}

//-----------------------------------------------------------------------------
FilterSPtr CoreFactory::createFilter(InputSList inputs, const Filter::Type& type) const
{
  FilterSPtr filter;

  if (m_filterFactories.contains(type))
  {
    filter = m_filterFactories[type]->createFilter(inputs, type, m_scheduler);
    filter->setStorage(m_defaultStorage);
  }
  else
  {
    throw CoreFactory::Unknown_Type_Exception();
  }

  return filter;
}


//-----------------------------------------------------------------------------
ESPINA::ChannelSPtr CoreFactory::createChannel(FilterSPtr filter, Output::Id id) const
{
  auto input = getInput(filter, id);

  auto channel = std::make_shared<Channel>(input);

  channel->setStorage(m_defaultStorage);

  return channel;
}

//-----------------------------------------------------------------------------
void CoreFactory::registerExtensionFactory(ChannelExtensionFactorySPtr factory)
{
  for(auto extension : factory->providedExtensions())
  {
    if (m_channelExtensionFactories.contains(extension)) throw Factory_Already_Registered_Exception();

    m_channelExtensionFactories[extension] = factory;
  }
}

//-----------------------------------------------------------------------------
ChannelExtensionTypeList CoreFactory::availableChannelExtensions() const
{
  return m_channelExtensionFactories.keys();
}

//-----------------------------------------------------------------------------
ChannelExtensionSPtr CoreFactory::createChannelExtension(const ChannelExtension::Type      &type,
                                                         const ChannelExtension::InfoCache &cache,
                                                         const State &state)
{
  ChannelExtensionSPtr extension;

  if (m_channelExtensionFactories.contains(type))
  {
    extension = m_channelExtensionFactories[type]->createChannelExtension(type, cache, state);
  } else
  {
    throw Unknown_Type_Exception();
  }

  return extension;
}

//-----------------------------------------------------------------------------
SegmentationSPtr CoreFactory::createSegmentation(FilterSPtr filter, Output::Id id) const
{
  auto input = getInput(filter, id);

  auto segmentation = std::make_shared<Segmentation>(input);

  segmentation->setStorage(m_defaultStorage);

  return segmentation;
}

//-----------------------------------------------------------------------------
void CoreFactory::registerExtensionFactory(SegmentationExtensionFactorySPtr factory)
{
  for(auto extension : factory->providedExtensions())
  {
    if (m_segmentationExtensionFactories.contains(extension)) throw Factory_Already_Registered_Exception();

    m_segmentationExtensionFactories[extension] = factory;
  }
}

//-----------------------------------------------------------------------------
SegmentationExtensionTypeList CoreFactory::availableSegmentationExtensions() const
{
  return m_segmentationExtensionFactories.keys();
}

//-----------------------------------------------------------------------------
SegmentationExtensionSPtr CoreFactory::createSegmentationExtension(const SegmentationExtension::Type      &type,
                                                                   const SegmentationExtension::InfoCache &cache,
                                                                   const State &state)
{
  SegmentationExtensionSPtr extension;

  if (m_segmentationExtensionFactories.contains(type))
  {
    extension = m_segmentationExtensionFactories[type]->createSegmentationExtension(type, cache, state);
  } else
  {
    throw Unknown_Type_Exception();
  }

  return extension;
}
