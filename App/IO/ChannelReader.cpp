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

#include "ChannelReader.h"
#include <Filters/VolumetricStreamReader.h>
#include <Core/Factory/CoreFactory.h>

using namespace EspINA;
using namespace EspINA::IO;


const Filter::Type VOLUMETRIC_STREAM_READER = "ChannelReader::VolumetricStreamReader";

//------------------------------------------------------------------------
FilterTypeList ChannelReader::providedFilters() const
{
  FilterTypeList filters;

  filters << VOLUMETRIC_STREAM_READER;

  return filters;
}

//------------------------------------------------------------------------
FilterSPtr ChannelReader::createFilter(OutputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const
{
  if (filter != VOLUMETRIC_STREAM_READER) throw Filter_Not_Provided_Exception();

  return FilterSPtr{new VolumetricStreamReader(inputs, filter, scheduler)};
}

//------------------------------------------------------------------------
AnalysisReader::ExtensionList ChannelReader::supportedFileExtensions() const
{
  ExtensionList supportedExtensions;

  Extensions extensions;
  extensions << "mha" << "mhd" << "tiff" << "tif";

  supportedExtensions["Channel Files"] = extensions;

  return supportedExtensions;
}

//------------------------------------------------------------------------
AnalysisSPtr ChannelReader::read(const QFileInfo file,
                                 CoreFactorySPtr factory,
                                 ErrorHandlerPtr handler)
{
  AnalysisSPtr analysis{new Analysis()};

  auto sample = factory->createSample("Fake Sample");

  analysis->add(sample);

  auto filter = factory->createFilter<VolumetricStreamReader>(OutputSList(), VOLUMETRIC_STREAM_READER);
  filter->setFileName(file);
  ChannelSPtr channel = factory->createChannel(filter, 0);

  analysis->add(channel);

  return analysis;
}
