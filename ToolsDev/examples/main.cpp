/*

 Copyright (C) 2016 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <Core/Analysis/Analysis.h>
#include <Core/Analysis/Extensions.h>
#include <Core/Analysis/ItemExtension.hxx>
#include <Core/Analysis/Segmentation.h>
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Filters/VolumetricStreamReader.h>
#include <Core/Analysis/Sample.h>
#include <Core/Analysis/Query.h>
#include <Core/Factory/AnalysisReader.h>
#include <Core/Factory/CoreFactory.h>
#include <Core/Factory/FilterFactory.h>
#include <Core/IO/DataFactory/RawDataFactory.h>
#include <Core/IO/SegFile.h>
#include <Core/Types.h>

// Qt
#include <QString>

// C++
#include <iostream>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::IO;

void usage(const char *appName)
{
  std::cout << "Usage: " << appName << " SEG_file_1 [SEG_file_2] ... [SEG_file_N]\n";
}

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

class ChannelReader
: public FilterFactory
{
  public:
  static const Filter::Type VOLUMETRIC_STREAM_READER; /** channel reader signature. */
  static const Filter::Type ESPINA_1_3_2_CHANNEL_READER; /** channel reader old signature. */

  virtual QString type() const
  { return "ChannelReader";}

  virtual FilterTypeList providedFilters() const
  {
    FilterTypeList filters;

    filters << VOLUMETRIC_STREAM_READER << ESPINA_1_3_2_CHANNEL_READER;

    return filters;
  }

  virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const
  {
    static auto dataFactory = std::make_shared<UpdateFilterDataFactory>();

    if (filter != VOLUMETRIC_STREAM_READER && filter != ESPINA_1_3_2_CHANNEL_READER)
    {
      auto what    = QObject::tr("Unable to create filter: %1").arg(filter);
      auto details = QObject::tr("ChannelReader::createFilter() -> Unknown filter type: %1").arg(filter);
      throw Core::Utils::EspinaException(what, details);
    }

    auto reader = std::make_shared<VolumetricStreamReader>(inputs, VOLUMETRIC_STREAM_READER, scheduler);
    reader->setDataFactory(dataFactory); //FIX: Temporal fix to create output during seg file load

    return reader;
  }
};

const Filter::Type ChannelReader::VOLUMETRIC_STREAM_READER    = "ChannelReader::VolumetricStreamReader";
const Filter::Type ChannelReader::ESPINA_1_3_2_CHANNEL_READER = "Channel Reader";

int main(int argc, char **argv)
{
  int errors = 0;
  if(argc == 1)
  {
    usage(argv[0]);
    return errors;
  }

  // These informations will only be available if contained within the SEG file (generated and thus stored in the
  // file) or, if not present in the file, the corresponding extension has been registered in the Core factory.
  // In the latter case the information will be computed by the extension.
  auto size   = SegmentationExtension::InformationKey("MorphologicalInformation", "Size");
  auto area   = SegmentationExtension::InformationKey("MorphologicalInformation", "Surface Area");
  auto volume = SegmentationExtension::InformationKey("MorphologicalInformation", "Physical Size");

  // EspINA core factory object.
  auto factory = std::make_shared<CoreFactory>(SchedulerSPtr());

  // Register the Stack loading filter.
  factory->registerFilterFactory(std::make_shared<ChannelReader>());

  for(int i = 1; i < argc; ++i)
  {
    // Process all files passed as arguments.
    QFileInfo file(QString(argv[i]));
    try
    {
      // Analysis loading.
      auto analysis = SegFile::load(file, factory);

      // Data extraction for each stack.
      for(auto stack: analysis->channels())
      {
        // Stack image, can be manipulated using the itk iterators.
        auto image = readLockVolume(stack->output())->itkImage();

        // Some stack basic information.
        std::cout << "Channel (Stack) ---------" << std::endl;
        std::cout << "Spacing          : " << stack->output()->spacing().toString().toStdString() << std::endl;
        std::cout << "Name             : " << stack->name().toStdString() << std::endl;
        std::cout << "Bounds           : " << stack->bounds().toString().toStdString() << std::endl;
        std::cout << "Nº segmentations : " << QueryRelations::segmentationsOnChannelSample(stack).size() << std::endl;
        std::cout << "Belongs to sample: " << QueryRelations::sample(stack)->name().toStdString() << std::endl;
        std::cout << "Spacing          : " << stack->output()->spacing().toString().toStdString() << std::endl;
      }

      // Data extraction for each segmentation.
      for(auto segmentation: analysis->segmentations())
      {
        // Segmentation image, can be manipulated using the itk iterators.
        auto image = readLockVolume(segmentation->output())->itkImage();

        // Some basic segmentation information.
        std::cout << "Segmentation ------------" << std::endl;
        std::cout << "Name    : " << segmentation->name().toStdString() << std::endl;
        std::cout << "Alias   : " << segmentation->alias().toStdString() << std::endl;
        std::cout << "Number  : " << segmentation->number() << std::endl;
        std::cout << "Category: " << segmentation->category()->name().toStdString() << std::endl;
        std::cout << "Bounds  : " << segmentation->bounds().toString().toStdString() << std::endl;
        std::cout << "Size    : " << segmentation->information(size).toInt() << " voxels" << std::endl;
        std::cout << "Volume  : " << segmentation->information(volume).toDouble() << " nm^3" << std::endl;
        std::cout << "Area    : " << segmentation->information(area).toDouble() << " nm^2" << std::endl;
      }
    }
    catch(const Utils::EspinaException &e)
    {
      ++errors;
      // catch and inform of any error.
      qDebug() << QString("Unable to load file %1. Error: %2.").arg(file.fileName()).arg(e.details());
    }
  }

  return errors;
}

