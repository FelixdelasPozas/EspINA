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

using namespace std;
using namespace ESPINA;

//------------------------------------------------------------------------
ModelFactory::ModelFactory(CoreFactorySPtr factory,
                           SchedulerSPtr scheduler,
                           GUI::View::RepresentationInvalidator *invalidator)
: m_factory(factory)
, m_scheduler(scheduler)
, m_invalidator(invalidator)
{
  if (!m_factory)
  {
    m_factory = make_shared<CoreFactory>();
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
void ModelFactory::registerAnalysisReader(AnalysisReaderSPtr reader)
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
AnalysisReaderSList ModelFactory::readers(const QFileInfo& file)
{
  return m_readerExtensions[file.suffix()];
}

//------------------------------------------------------------------------
SampleAdapterSPtr ModelFactory::createSample(const QString& name) const
{
  auto sample = m_factory->createSample(name);

  return adaptSample(sample);
}

//------------------------------------------------------------------------
ChannelAdapterSPtr ModelFactory::createChannel(FilterSPtr filter, Output::Id output) const
{
  auto channel = m_factory->createChannel(filter, output);

  return adaptChannel(channel);
}

//------------------------------------------------------------------------
ChannelExtensionSPtr ModelFactory::createChannelExtension(const ChannelExtension::Type &type)
{
  return m_factory->createChannelExtension(type);
}

//------------------------------------------------------------------------
SegmentationAdapterSPtr ModelFactory::createSegmentation(FilterSPtr filter, Output::Id output) const
{
  auto segmentation = m_factory->createSegmentation(filter, output);

  return adaptSegmentation(segmentation);
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
ChannelAdapterSPtr ModelFactory::adaptChannel(ChannelSPtr channel) const
{
  return ChannelAdapterSPtr{new ChannelAdapter(channel)};
}

//------------------------------------------------------------------------
SegmentationAdapterSPtr ModelFactory::adaptSegmentation(SegmentationSPtr segmentation) const
{
  SegmentationAdapterSPtr adaptedSegmentation(new SegmentationAdapter(segmentation));

  QObject::connect(adaptedSegmentation.get(), SIGNAL(representationsInvalidated(ViewItemAdapterPtr)),
                   m_invalidator,             SLOT(invalidateRepresentations(ViewItemAdapterPtr)));

  return adaptedSegmentation;
}