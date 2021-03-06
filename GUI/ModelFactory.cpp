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
using namespace ESPINA::Core;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI;

//------------------------------------------------------------------------
ModelFactory::ModelFactory(CoreFactorySPtr factory,
                           SchedulerSPtr scheduler)
: m_factory  {factory}
, m_scheduler{scheduler}
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
void ModelFactory::registerExtensionFactory(StackExtensionFactorySPtr factory)
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
  m_factory->registerAnalysisReader(reader);
}

//------------------------------------------------------------------------
StackExtension::TypeList ModelFactory::availableStackExtensions() const
{
  return m_factory->availableStackExtensions();
}

//------------------------------------------------------------------------
SegmentationExtension::TypeList ModelFactory::availableSegmentationExtensions() const
{
  return m_factory->availableSegmentationExtensions();
}

//------------------------------------------------------------------------
SupportedFormats ModelFactory::supportedFileExtensions()
{
  return m_factory->supportedFileExtensions();
}

//------------------------------------------------------------------------
AnalysisReaderSList ModelFactory::readers(const QFileInfo& file)
{
  return m_factory->readers(file);
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
StackExtensionSPtr ModelFactory::createStackExtension(const StackExtension::Type &type)
{
  return m_factory->createStackExtension(type);
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
  return SegmentationAdapterSPtr{new SegmentationAdapter(segmentation)};
}

//------------------------------------------------------------------------
void ModelFactory::setTemporalDirectory(const QDir &directory)
{
  m_factory->setTemporalDirectory(directory);
}

//------------------------------------------------------------------------
TemporalStorageSPtr ModelFactory::createTemporalStorage()
{
  return m_factory->createTemporalStorage();
}
