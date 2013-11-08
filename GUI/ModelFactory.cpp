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

#include "ModelFactory.h"

#include "GUI/Model/ChannelAdapter.h"
#include "GUI/Model/SampleAdapter.h"
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Sample.h>
#include <Core/Factory/CoreFactory.h>

using namespace EspINA;

//------------------------------------------------------------------------
ModelFactory::ModelFactory(SchedulerSPtr scheduler)
: m_scheduler(scheduler)
, m_factory(new CoreFactory(m_scheduler))
{

}

//------------------------------------------------------------------------
ModelFactory::~ModelFactory()
{

}

//------------------------------------------------------------------------
void ModelFactory::registerFilterFactory(FilterFactoryPtr factory)
{
  m_factory->registerFilter(factory);
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
FileExtensions ModelFactory::supportedFileExtensions()
{
  FileExtensions extensions;

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
  SampleSPtr sample{new Sample(name)};

  return SampleAdapterSPtr{new SampleAdapter(sample)};
}

//------------------------------------------------------------------------
ChannelAdapterSPtr ModelFactory::createChannel(FilterAdapterSPtr filter, Output::Id output) const
{
  ChannelSPtr channel{new Channel(filter->adaptedFilter(), output)};

  return ChannelAdapterSPtr{new ChannelAdapter(filter, channel)};
}