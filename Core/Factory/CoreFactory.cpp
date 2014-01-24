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

#include "CoreFactory.h"

#include "Core/Analysis/Sample.h"
#include "Core/Analysis/Channel.h"
#include <Core/Analysis/Segmentation.h>
#include <Core/Utils/TemporalStorage.h>

using namespace EspINA;

//------------------------------------------------------------------------
CoreFactory::CoreFactory(SchedulerSPtr scheduler)
: m_scheduler{scheduler}
{
  QDir tmpDir = QDir::tempPath();
  tmpDir.mkpath("espina");
  tmpDir.cd("espina");

  m_defaultStorage = TemporalStorageSPtr{new TemporalStorage(tmpDir)};
}


//-----------------------------------------------------------------------------
CoreFactory::~CoreFactory()
{
}

//-----------------------------------------------------------------------------
void CoreFactory::registerFilterFactory(FilterFactorySPtr factory)
throw (Factory_Already_Registered_Exception)
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
  return SampleSPtr{new Sample(name)};
}

//-----------------------------------------------------------------------------
FilterSPtr CoreFactory::createFilter(OutputSList inputs, const Filter::Type& type) const
throw (Unknown_Type_Exception)
{
  FilterSPtr filter;

  if (m_filterFactories.contains(type))
  {
    filter = m_filterFactories[type]->createFilter(inputs, type, m_scheduler);
    filter->setStorage(m_defaultStorage);
  } else
  {
    throw Unknown_Type_Exception();
  }

  return filter;
}


//-----------------------------------------------------------------------------
EspINA::ChannelSPtr CoreFactory::createChannel(FilterSPtr filter, Output::Id output) const
{
  ChannelSPtr channel{new Channel(filter, output)};

  channel->setStorage(m_defaultStorage);

  return channel;
}

//-----------------------------------------------------------------------------
void CoreFactory::registerExtensionFactory(ChannelExtensionFactorySPtr factory)
throw (Factory_Already_Registered_Exception)
{
  for(auto extension : factory->providedExtensions())
  {
    if (m_channelExtensionFactories.contains(extension)) throw Factory_Already_Registered_Exception();

    m_channelExtensionFactories[extension] = factory;
  }
}

//-----------------------------------------------------------------------------
ChannelExtensionSPtr CoreFactory::createChannelExtension(ChannelExtension::Type type, const State& state)
{
  ChannelExtensionSPtr extension;

  if (m_channelExtensionFactories.contains(type))
  {
    extension = m_channelExtensionFactories[type]->createChannelExtension(type, state);
  } else
  {
    throw Unknown_Type_Exception();
  }

  return extension;
}

//-----------------------------------------------------------------------------
SegmentationSPtr CoreFactory::createSegmentation(FilterSPtr filter, Output::Id output) const
{
  SegmentationSPtr segmentation{new Segmentation(filter, output)};

  segmentation->setStorage(m_defaultStorage);

  return segmentation;
}

//-----------------------------------------------------------------------------
void CoreFactory::registerExtensionFactory(SegmentationExtensionFactorySPtr factory)
throw (Factory_Already_Registered_Exception)
{
  for(auto extension : factory->providedExtensions())
  {
    if (m_segmentationExtensionFactories.contains(extension)) throw Factory_Already_Registered_Exception();

    m_segmentationExtensionFactories[extension] = factory;
  }
}

//-----------------------------------------------------------------------------
SegmentationExtensionSPtr CoreFactory::createSegmentationExtension(SegmentationExtension::Type type, const State& state)
{
  SegmentationExtensionSPtr extension;

  if (m_segmentationExtensionFactories.contains(type))
  {
    extension = m_segmentationExtensionFactories[type]->createSegmentationExtension(type, state);
  } else
  {
    throw Unknown_Type_Exception();
  }

  return extension;
}