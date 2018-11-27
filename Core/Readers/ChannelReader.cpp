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

// ESPINA
#include "ChannelReader.h"
#include <EspinaConfig.h>
#include <Core/Factory/CoreFactory.h>
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Sample.h>
#include <Core/Analysis/Output.h>
#include <Core/Analysis/Filters/VolumetricStreamReader.h>
#include <Core/Utils/EspinaException.h>
#include <Core/Utils/TemporalStorage.h>
#include <Core/IO/DataFactory/RawDataFactory.h>

#if USE_METADONA
  #include <Producer.h>
  #include <IRODS_Storage.h>
  #include <Support/Metadona/Coordinator.h>
  #include <Support/Metadona/StorageFactory.h>
#endif

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::IO;

const Filter::Type ChannelReader::VOLUMETRIC_STREAM_READER    = "ChannelReader::VolumetricStreamReader";
const Filter::Type ChannelReader::ESPINA_1_3_2_CHANNEL_READER = "Channel Reader";

class UpdateFilterDataFactory
: public DataFactory
{
  public:
    virtual DataSPtr createData(OutputSPtr output, TemporalStorageSPtr storage, const QString& path, QXmlStreamAttributes info) override
    {
      DataSPtr data;

      if ("VolumetricData" == info.value("type"))
      {
        data = fetchData(output);
      }
      else
      {
        data = m_fetchData.createData(output, storage, path, info);
      }

      return data;
    }

  private:
    DefaultVolumetricDataSPtr fetchData(OutputSPtr output)
    {
      output->filter()->update();
      return writeLockVolume(output, DataUpdatePolicy::Ignore);
    }

    RawDataFactory m_fetchData;
};

//------------------------------------------------------------------------
const FilterTypeList ChannelReader::providedFilters() const
{
  FilterTypeList filters;

  filters << VOLUMETRIC_STREAM_READER << ESPINA_1_3_2_CHANNEL_READER;

  return filters;
}

//------------------------------------------------------------------------
FilterSPtr ChannelReader::createFilter(InputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const
{
  static auto dataFactory = std::make_shared<UpdateFilterDataFactory>();

  if (filter != VOLUMETRIC_STREAM_READER && filter != ESPINA_1_3_2_CHANNEL_READER)
  {
    auto what    = QObject::tr("Unable to create filter: %1").arg(filter);
    auto details = QObject::tr("ChannelReader::createFilter() -> Unknown filter type: %1").arg(filter);
    throw EspinaException(what, details);
  }

  auto reader = std::make_shared<VolumetricStreamReader>(inputs, VOLUMETRIC_STREAM_READER, scheduler);
  reader->setDataFactory(dataFactory); //FIX: Temporal fix to create output during seg file load

  return reader;
}

//------------------------------------------------------------------------
const AnalysisReader::ExtensionList ChannelReader::supportedFileExtensions() const
{
  ExtensionList supportedExtensions;

  Extensions extensions;
  extensions << "mha" << "mhd" << "tiff" << "tif";

  supportedExtensions["Channel Files"] = extensions;

  return supportedExtensions;
}

//------------------------------------------------------------------------
AnalysisSPtr ChannelReader::read(const QFileInfo& file,
                                 CoreFactorySPtr  factory,
                                 ProgressReporter *reporter,
                                 ErrorHandlerSPtr handler)
{
  auto analysis = std::make_shared<Analysis>();

  analysis->setStorage(factory->createTemporalStorage());

  auto sampleName = QString("Unknown Sample");

  QString channelMetadata;

#if USE_METADONA
  if(!StorageFactory::supportedStorages().isEmpty())
  {
    Coordinator coordinator;

    auto storage = StorageFactory::newStorage(StorageFactory::Type::IRODS);

    Metadona::Producer producer(storage);

    try
    {
      auto metadata = storage->metadata(file.absoluteFilePath().toStdString());

      if (metadata.empty())
      {
        metadata = producer.generateFrom("Specimen", coordinator);

        storage->setMetadata(file.absoluteFilePath().toStdString(), metadata);
      }

      std::cout << Metadona::dump(metadata) << std::endl;
      channelMetadata = Metadona::dump(metadata).c_str();

      sampleName = metadata.at(0).id().c_str();
    }
    catch(...)
    {

    }
  }
#endif

  auto sample = factory->createSample(sampleName);

  analysis->add(sample);

  auto filter = factory->createFilter<VolumetricStreamReader>(InputSList(), VOLUMETRIC_STREAM_READER);
  filter->setErrorHandler(handler);
  filter->setFileName(file);
  filter->setStorage(factory->defaultStorage());
  filter->update();

  auto channel = factory->createChannel(filter, 0);
  channel->setName(file.fileName());
  channel->setMetadata(channelMetadata);

  analysis->add(channel);

  analysis->addRelation(sample, channel, Channel::STAIN_LINK);

  return analysis;
}
