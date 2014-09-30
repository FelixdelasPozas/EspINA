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
#include "ModelFactory.h"
#include "GUI/Model/ChannelAdapter.h"
#include "GUI/Model/SampleAdapter.h"
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Sample.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Factory/CoreFactory.h>

using namespace ESPINA;

//------------------------------------------------------------------------
ModelFactory::ModelFactory(CoreFactorySPtr factory,
                           SchedulerSPtr scheduler)
: m_factory(factory)
, m_scheduler(scheduler)
, m_channelRepresentationFactory(new RepresentationFactoryGroup(scheduler))
, m_segmentationRepresentationFactory(new RepresentationFactoryGroup(scheduler))
{
  if (!m_factory)
  {
    m_factory = CoreFactorySPtr{new CoreFactory()};
  }

}

//------------------------------------------------------------------------
ModelFactory::~ModelFactory()
{
}

//------------------------------------------------------------------------
void ModelFactory::registerFilterFactory(FilterFactorySPtr factory)
{
  m_factory->registerFilterFactory(factory);
}

//------------------------------------------------------------------------
void ModelFactory::registerExtensionFactory(ChannelExtensionFactorySPtr factory)
{
  m_factory->registerExtensionFactory(factory);
}

//------------------------------------------------------------------------
void ModelFactory::registerExtensionFactory(SegmentationExtensionFactorySPtr factory)
{
  m_factory->registerExtensionFactory(factory);
}

//------------------------------------------------------------------------
void ModelFactory::registerAnalysisReader(AnalysisReaderPtr reader)
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

//------------------------------------------------------------------------
void ModelFactory::registerChannelRepresentationFactory(RepresentationFactorySPtr factory)
{
  m_channelRepresentationFactory->addRepresentationFactory(factory);
}

//------------------------------------------------------------------------
void ModelFactory::registerSegmentationRepresentationFactory(RepresentationFactorySPtr factory)
{
  m_segmentationRepresentationFactory->addRepresentationFactory(factory);
}

//------------------------------------------------------------------------
ChannelExtensionTypeList ModelFactory::availableChannelExtensions() const
{
  return m_factory->availableChannelExtensions();
}

//------------------------------------------------------------------------
SegmentationExtensionTypeList ModelFactory::availableSegmentationExtensions() const
{
  return m_factory->availableSegmentationExtensions();
}

//------------------------------------------------------------------------
FileExtensions ModelFactory::supportedFileExtensions()
{
  FileExtensions extensions;

  for(auto extension : m_readerExtensions.keys())
  {
    extensions << QString("*.%1").arg(extension);
  }

  extensions = QStringList(QObject::tr("All Supported Files (%1)").arg(extensions.join(" ")));

  for(auto loader : m_readers)
  {
    extensions << loader->fileExtensionDescriptions();
  }

  return extensions;
}

//------------------------------------------------------------------------
AnalysisReaderList ModelFactory::readers(const QFileInfo& file)
{
  return m_readerExtensions[file.suffix()];
}

//------------------------------------------------------------------------
SampleAdapterSPtr ModelFactory::createSample(const QString& name) const
{
  SampleSPtr sample{m_factory->createSample(name)};

  return adaptSample(sample);
}

//------------------------------------------------------------------------
ChannelAdapterSPtr ModelFactory::createChannel(FilterAdapterSPtr filter, Output::Id output) const
{
  ChannelSPtr channel{m_factory->createChannel(filter->adaptedFilter(), output)};

  return adaptChannel(filter, channel);
}

//------------------------------------------------------------------------
ChannelExtensionSPtr ModelFactory::createChannelExtension(const ChannelExtension::Type &type)
{
  return m_factory->createChannelExtension(type);
}

//------------------------------------------------------------------------
SegmentationAdapterSPtr ModelFactory::createSegmentation(FilterAdapterSPtr filter, Output::Id output) const
{
  SegmentationSPtr segmentation{m_factory->createSegmentation(filter->adaptedFilter(), output)};

  return adaptSegmentation(filter, segmentation);
}

//------------------------------------------------------------------------
SegmentationExtensionSPtr ModelFactory::createSegmentationExtension(const SegmentationExtension::Type &type)
{
  return m_factory->createSegmentationExtension(type);
}

//------------------------------------------------------------------------
SampleAdapterSPtr ModelFactory::adaptSample(SampleSPtr sample) const
{
  return SampleAdapterSPtr{new SampleAdapter(sample)};
}

//------------------------------------------------------------------------
FilterAdapterSPtr ModelFactory::adaptFilter(FilterSPtr filter) const
{
  return FilterAdapterSPtr{new FilterAdapter<Filter>(filter)};
}

//------------------------------------------------------------------------
ChannelAdapterSPtr ModelFactory::adaptChannel(FilterAdapterSPtr filter, ChannelSPtr channel) const
{
  ChannelAdapterSPtr adapter{new ChannelAdapter(filter, channel)};
  adapter->setRepresentationFactory(m_channelRepresentationFactory);

  return adapter;
}

//------------------------------------------------------------------------
SegmentationAdapterSPtr ModelFactory::adaptSegmentation(FilterAdapterSPtr filter, SegmentationSPtr segmentation) const
{
  SegmentationAdapterSPtr adapter{new SegmentationAdapter(filter, segmentation)};
  adapter->setRepresentationFactory(m_segmentationRepresentationFactory);

  return adapter;
}

