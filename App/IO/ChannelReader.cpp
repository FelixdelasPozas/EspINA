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
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Sample.h>
#include <Core/Utils/TemporalStorage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>

using namespace EspINA;
using namespace EspINA::IO;


const Filter::Type VOLUMETRIC_STREAM_READER    = "ChannelReader::VolumetricStreamReader";
const Filter::Type ESPINA_1_3_2_CHANNEL_READER = "Channel Reader";

//------------------------------------------------------------------------
FilterTypeList ChannelReader::providedFilters() const
{
  FilterTypeList filters;

  filters << VOLUMETRIC_STREAM_READER << ESPINA_1_3_2_CHANNEL_READER;

  return filters;
}

//------------------------------------------------------------------------
FilterSPtr ChannelReader::createFilter(OutputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const throw (Unknown_Filter_Exception)
{
  if (filter != VOLUMETRIC_STREAM_READER 
   && filter != ESPINA_1_3_2_CHANNEL_READER) throw Unknown_Filter_Exception();

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
AnalysisSPtr ChannelReader::read(const QFileInfo& file,
                                 CoreFactorySPtr  factory,
                                 ErrorHandlerSPtr handler)
{
  AnalysisSPtr analysis{new Analysis()};

  auto sample = factory->createSample("Fake Sample");

  analysis->add(sample);

  auto filter = factory->createFilter<VolumetricStreamReader>(OutputSList(), VOLUMETRIC_STREAM_READER);

//   if (file.fileName().contains(".tif"))
//   {
//     using VolumeReader = itk::ImageFileReader<itkVolumeType>;
//     using VolumeWriter = itk::ImageFileWriter<itkVolumeType>;
// 
//     VolumeReader::Pointer reader = VolumeReader::New();
//     reader->SetFileName(file.absoluteFilePath().toUtf8().data());
//     reader->Update();
// 
//     TemporalStorageSPtr storage = filter->storage();
// 
//     file = QFileInfo(storage->absoluteFilePath(file.baseName() + ".mhd"));
// 
//     VolumeWriter::Pointer writer = VolumeWriter::New();
//     writer->SetFileName(file.absoluteFilePath().toUtf8().data());
//     writer->SetInput(reader->GetOutput());
//     writer->Write();
//   }

  filter->setErrorHandler(handler);
  filter->setFileName(file);
  ChannelSPtr channel = factory->createChannel(filter, 0);
  channel->setName(file.fileName());

  analysis->add(channel);

  analysis->addRelation(sample, channel, Channel::STAIN_LINK);

  return analysis;
}
