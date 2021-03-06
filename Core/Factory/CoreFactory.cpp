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
#include <Core/Analysis/Extensions.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Utils/EspinaException.h>
#include <Core/Utils/SupportedFormats.h>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Core::Utils;

//------------------------------------------------------------------------
CoreFactory::CoreFactory(SchedulerSPtr scheduler)
: m_scheduler         {scheduler}
, m_defaultStorage    {nullptr}
, m_temporalStorageDir{nullptr}
{
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
    if (m_filterFactories.contains(filter))
    {
      auto what    = QObject::tr("Attempt to register an already registered filter: %1").arg(filter);
      auto details = QObject::tr("CoreFactory::registerFilterFactory() -> Filter type already registered: %1").arg(filter);

      throw EspinaException(what, details);
    }

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

  if (!type.isEmpty() && m_filterFactories.contains(type))
  {
    filter = m_filterFactories[type]->createFilter(inputs, type, m_scheduler);
    filter->setStorage(defaultStorage());
  }
  else
  {
	QString filterType = type.isEmpty() ? "Empty":type;

    auto what    = QObject::tr("Unable to create filter: %1").arg(filterType);
    auto details = QObject::tr("CoreFactory::createFilter() -> Unknown filter: %1").arg(filterType);

    throw EspinaException(what, details);
  }

  return filter;
}


//-----------------------------------------------------------------------------
ESPINA::ChannelSPtr CoreFactory::createChannel(FilterSPtr filter, Output::Id id) const
{
  auto input = getInput(filter, id);

  auto channel = std::make_shared<Channel>(input);
  channel->setStorage(defaultStorage());

  return channel;
}

//-----------------------------------------------------------------------------
void CoreFactory::registerExtensionFactory(StackExtensionFactorySPtr factory)
{
  for(auto extension : factory->providedExtensions())
  {
    if (availableStackExtensions().contains(extension))
    {
      auto what    = QObject::tr("Attempt to register an already registered stack extension: %1").arg(extension);
      auto details = QObject::tr("CoreFactory::registerExtensionFactory(stack extension) -> Stack extension type already registered: %1").arg(extension);

      throw EspinaException(what, details);
    }

    m_stackExtensionFactories[extension] = factory;
  }
}

//-----------------------------------------------------------------------------
StackExtension::TypeList CoreFactory::availableStackExtensions() const
{
  return m_stackExtensionFactories.keys();
}

//-----------------------------------------------------------------------------
Core::StackExtensionSPtr CoreFactory::createStackExtension(const StackExtension::Type      &type,
                                                           const StackExtension::InfoCache &cache,
                                                           const State &state)
{
  StackExtensionSPtr extension = nullptr;

  if (availableStackExtensions().contains(type))
  {
    extension = m_stackExtensionFactories[type]->createExtension(type, cache, state);
  }
  else
  {
    auto what    = QObject::tr("Unable to create stack extension: %1").arg(type);
    auto details = QObject::tr("CoreFactory::createStackExtension() -> Unknown extension type: %1").arg(type);

    throw EspinaException(what, details);
  }

  return extension;
}

//-----------------------------------------------------------------------------
SegmentationSPtr CoreFactory::createSegmentation(FilterSPtr filter, Output::Id id) const
{
  auto input = getInput(filter, id);

  auto segmentation = std::make_shared<Segmentation>(input);
  segmentation->setStorage(defaultStorage());

  return segmentation;
}

//-----------------------------------------------------------------------------
void CoreFactory::registerExtensionFactory(SegmentationExtensionFactorySPtr factory)
{
  for(auto extension : factory->providedExtensions())
  {
    if (m_segmentationExtensionFactories.contains(extension))
    {
      auto what    = QObject::tr("Attempt to register an already registered segmentation extension: %1").arg(extension);
      auto details = QObject::tr("CoreFactory::registerExtensionFactory(seg extension) -> Segmentation extension type already registered: %1").arg(extension);

      throw EspinaException(what, details);
    }

    m_segmentationExtensionFactories[extension] = factory;
  }
}

//-----------------------------------------------------------------------------
SegmentationExtension::TypeList CoreFactory::availableSegmentationExtensions() const
{
  return m_segmentationExtensionFactories.keys();
}

//-----------------------------------------------------------------------------
SegmentationExtensionSPtr CoreFactory::createSegmentationExtension(const SegmentationExtension::Type      &type,
                                                                   const SegmentationExtension::InfoCache &cache,
                                                                   const State &state)
{
  SegmentationExtensionSPtr extension = nullptr;

  if (availableSegmentationExtensions().contains(type))
  {
    extension = m_segmentationExtensionFactories[type]->createExtension(type, cache, state);
  }
  else
  {
    auto what    = QObject::tr("Unable to create segmentation extension: %1").arg(type);
    auto details = QObject::tr("CoreFactory::createSegmentationExtension() -> Unknown extension type: %1").arg(type);

    throw EspinaException(what, details);
  }

  return extension;
}

//-----------------------------------------------------------------------------
TemporalStorageSPtr CoreFactory::createTemporalStorage() const
{
  return std::make_shared<TemporalStorage>(m_temporalStorageDir);
}

//-----------------------------------------------------------------------------
void CoreFactory::setTemporalDirectory(const QDir &directory)
{
  auto newDirectory = directory.exists() ? directory : QDir::temp();

  if(!m_temporalStorageDir || (newDirectory.absolutePath() != m_temporalStorageDir->absolutePath()))
  {
    for(auto storage: TemporalStorage::s_Storages)
    {
      auto baseDirectory = storage->rootDirectory();
      if((baseDirectory.absolutePath() != newDirectory.absolutePath()) && !storage->move(newDirectory.absolutePath(), false))
      {
        auto what = QObject::tr("Unable to move files from %1 to %2").arg(baseDirectory.absolutePath()).arg(newDirectory.absolutePath());
        auto details = QObject::tr("CoreFactory::setTemporalDirectory() -> Unable to move files from %1 to %2").arg(baseDirectory.absolutePath()).arg(newDirectory.absolutePath());

        throw EspinaException(what, details);
      }
    }

    if(m_temporalStorageDir)
    {
      delete m_temporalStorageDir;
    }

    m_temporalStorageDir = new QDir(newDirectory);
  }
}

//-----------------------------------------------------------------------------
TemporalStorageSPtr CoreFactory::defaultStorage() const
{
  if(!m_defaultStorage)
  {
    m_defaultStorage = createTemporalStorage();
  }

  return m_defaultStorage;
}

//-----------------------------------------------------------------------------
void CoreFactory::registerAnalysisReader(AnalysisReaderSPtr reader)
{
  auto extensions = reader->supportedFileExtensions();

  for(auto description : extensions.keys())
  {
    for(auto fileExtension : extensions[description])
    {
      m_readerExtensions[fileExtension] << reader;
    }
  }

  m_readers << reader;
}

//-----------------------------------------------------------------------------
const AnalysisReaderSList CoreFactory::readers(const QFileInfo& file) const
{
  return m_readerExtensions[file.suffix()];
}

//-----------------------------------------------------------------------------
Core::Utils::SupportedFormats CoreFactory::supportedFileExtensions()
{
  SupportedFormats extensions;

  QStringList supportedExtensions;

  for(auto extension : m_readerExtensions.keys())
  {
    supportedExtensions << extension;
  }

  extensions.addFormat(QObject::tr("All Supported Files"), supportedExtensions);

  for(auto loader : m_readers)
  {
    auto loaderExtensions = loader->supportedFileExtensions();
    for (auto it = loaderExtensions.begin(); it != loaderExtensions.end(); ++it)
    {
      extensions.addFormat(it.key(), it.value());
    }
  }

  return extensions;
}
