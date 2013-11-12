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


#ifndef ESPINA_MODEL_FACTORY_H
#define ESPINA_MODEL_FACTORY_H

#include "EspinaGUI_Export.h"

#include "GUI/Model/FilterAdapter.h"
#include "Model/SegmentationAdapter.h"
#include "Representations/RepresentationFactoryGroup.h"
#include <Core/Factory/AnalysisReader.h>
#include <Core/Factory/FilterFactory.h>

#include <memory>

#include <QStringList>
#include <QMap>

namespace EspINA
{
  class ExtensionProvider;

  class SampleAdapter;
  using SampleAdapterSPtr = std::shared_ptr<SampleAdapter>;
  
  class ChannelAdapter;
  using ChannelAdapterSPtr = std::shared_ptr<ChannelAdapter>;

//   class FilterAdapter;
//   using FilterAdapterSPtr = std::shared_ptr<FilterAdapter>;

  //const QString CHANNEL_FILES = QObject::tr("Channel Files (*.mha *.mhd *.tif *.tiff)");
  //const QString SEG_FILES     = QObject::tr("Espina Analysis (*.seg)");

  class EspinaGUI_EXPORT ModelFactory
  {
  public:
    explicit ModelFactory(SchedulerSPtr scheduler);
    ~ModelFactory();

    void registerAnalysisReader(AnalysisReaderPtr reader);
    void registerFilterFactory (FilterFactoryPtr  factory);
    void registerChannelRepresentationFactory(RepresentationFactorySPtr factory);
    void registerSegmentationRepresentationFactory(RepresentationFactorySPtr factory);

    FileExtensions supportedFileExtensions();

    AnalysisReaderList readers(const QFileInfo& file);

    AnalysisSPtr read(AnalysisReaderPtr reader, const QFileInfo& file, ErrorHandlerPtr handler = nullptr)
    { return reader->read(file, m_factory, handler); }

    SampleAdapterSPtr createSample(const QString& name = QString()) const;

    template<typename T>
    std::shared_ptr<FilterAdapter<T>> createFilter(OutputSList inputs, Filter::Type type) const
    {
      std::shared_ptr<T> filter{new T(inputs, type, m_scheduler)};
      return std::shared_ptr<FilterAdapter<T>>(new FilterAdapter<T>(filter));
    }

    ChannelAdapterSPtr createChannel(FilterAdapterSPtr filter, Output::Id output) const;

    SegmentationAdapterSPtr createSegmentation(FilterAdapterSPtr filter, Output::Id output) const;

    SampleAdapterSPtr adaptSample(SampleSPtr sample) const;

    FilterAdapterSPtr  adaptFilter(FilterSPtr filter) const;

    ChannelAdapterSPtr adaptChannel(FilterAdapterSPtr filter, ChannelSPtr channel) const;

    SegmentationAdapterSPtr adaptSegmentation(FilterAdapterSPtr filter, SegmentationSPtr segmentation) const;

    RepresentationFactorySPtr channelRepresentationFactory() const
    { return m_channelRepresentationFactory; }

    RepresentationFactorySPtr segmentationRepresentationFactory() const
    { return m_segmentationRepresentationFactory; }

  private:
    SchedulerSPtr   m_scheduler;
    CoreFactorySPtr m_factory;
    RepresentationFactoryGroupSPtr m_channelRepresentationFactory;
    RepresentationFactoryGroupSPtr m_segmentationRepresentationFactory;

    QMap<QString, AnalysisReaderList> m_readerExtensions;
    AnalysisReaderList m_readers;
  };

  using ModelFactorySPtr = std::shared_ptr<ModelFactory>;
}// namespace EspINA

#endif // ESPINA_CORE_FACTORY_H
