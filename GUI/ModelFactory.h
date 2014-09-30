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

#ifndef ESPINA_MODEL_FACTORY_H
#define ESPINA_MODEL_FACTORY_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include "GUI/Model/FilterAdapter.h"
#include "Model/SegmentationAdapter.h"
#include "Representations/RepresentationFactoryGroup.h"
#include <Core/Factory/AnalysisReader.h>
#include <Core/Factory/FilterFactory.h>
#include <Core/Factory/CoreFactory.h>

// C++
#include <memory>

// Qt
#include <QStringList>
#include <QMap>

namespace ESPINA
{
  class ExtensionProvider;

  class SampleAdapter;
  using SampleAdapterSPtr = std::shared_ptr<SampleAdapter>;

  class ChannelAdapter;
  using ChannelAdapterSPtr = std::shared_ptr<ChannelAdapter>;

  class EspinaGUI_EXPORT ModelFactory
  {
  public:
  	/** \brief ModelFactory class constructor.
  	 * \param[in] factory, core factory smart pointer.
  	 * \param[in] scheduler, scheduler smart pointer.
  	 *
  	 */
    explicit ModelFactory(CoreFactorySPtr factory = CoreFactorySPtr(),
                          SchedulerSPtr scheduler = SchedulerSPtr());

    /** \brief ModelFactory class destructor.
     *
     */
    ~ModelFactory();

    /** \brief Registers an analysis reader in the factory.
     * \param[in] reader, analysis reader raw pointer.
     *
     */
    void registerAnalysisReader(AnalysisReaderPtr reader);

    /** \brief Registers a filter factory in the factory.
     * \param[in] factory, filter factory smart pointer.
     *
     */
    void registerFilterFactory (FilterFactorySPtr  factory);

    /** \brief Registers a channel extension factory in the factory.
     * \param[in] factory, channel extension factory smart pointer.
     *
     */
    void registerExtensionFactory(ChannelExtensionFactorySPtr factory);

    /** \brief Registers a segmentation extension factory in the factory.
     * \param[in] factory, segmentation extension factory smart pointer.
     *
     */
    void registerExtensionFactory(SegmentationExtensionFactorySPtr factory);

    /** \brief Registers a channel representation factory in the factory.
     * \param[in] factory, channel representation factory smart pointer.
     *
     */
    void registerChannelRepresentationFactory(RepresentationFactorySPtr factory);

    /** \brief Registers a segmentation representation factory in the factory.
     * \param[in] factory, segmentation representation factory smart pointer.
     *
     */
    void registerSegmentationRepresentationFactory(RepresentationFactorySPtr factory);

    /** \brief Returns the list of channel extension types the factory can create.
     *
     */
    ChannelExtensionTypeList availableChannelExtensions() const;

    /** \brief Returns the list of segmentation extension types the factory can create.
     *
     */
    SegmentationExtensionTypeList availableSegmentationExtensions() const;

    /** \brief Returns the list of file extensions the factory can read.
     *
     */
    FileExtensions supportedFileExtensions();

    /** \brief Returns the list of raw pointers of the readers registered in the factory for a given file.
     * \param[in] file, QFileInfo object.
     *
     */
    AnalysisReaderList readers(const QFileInfo& file);

    /** \brief Reads a data file and returns an analysis.
     * \param[in] reader, analysis reader raw pointer.
     * \param[in] file, QFileInfo object.
     * \param[in] handler, smart pointer of the error handler to use.
     *
     */
    AnalysisSPtr read(AnalysisReaderPtr reader, const QFileInfo& file, ErrorHandlerSPtr handler = ErrorHandlerSPtr())
    { return reader->read(file, m_factory, handler); }

    /** \brief Creates and returns a new sample adapter.
     * \param[in] name, name of the sample.
     *
     */
    SampleAdapterSPtr createSample(const QString& name = QString()) const;

    /** \brief Creates and returns a filter of the specified type.
     * \param[in] inputs, list of input smart pointers.
     * \param[in] type, type of the filter to return.
     *
     */
    template<typename T>
    std::shared_ptr<FilterAdapter<T>> createFilter(InputSList inputs, Filter::Type type) const
    {
      std::shared_ptr<T> filter{m_factory->createFilter<T>(inputs, type)};
      return std::shared_ptr<FilterAdapter<T>>(new FilterAdapter<T>(filter));
    }

    /** \brief Creates and returns a channel adapter given the filter and the output id.
     * \param[in] filter, filter adapter smart pointer.
     * \param[in] output, id of the output of the given filter.
     *
     */
    ChannelAdapterSPtr createChannel(FilterAdapterSPtr filter, Output::Id output) const;

    /** \brief Creates and returns a channel extension of the given type.
     * \param[in] type, channel extension type.
     *
     */
    ChannelExtensionSPtr createChannelExtension(const ChannelExtension::Type &type);

    /** \brief Creates and returns a segmentation adapter from a given filter and an output id.
     * \param[in] filter, filter adapter smart pointer.
     * \param[in] output, id of the output of the given filter.
     *
     */
    SegmentationAdapterSPtr createSegmentation(FilterAdapterSPtr filter, Output::Id output) const;

    /** \brief Creates and returns a segmentation extension of the given type.
     * \param[in] type, segmentation extension type.
     *
     */
    SegmentationExtensionSPtr createSegmentationExtension(const SegmentationExtension::Type &type);

    /** \brief Returns the adapter for the given sample.
     * \param[in] sample, sample smart pointer to adapt.
     *
     */
    SampleAdapterSPtr adaptSample(SampleSPtr sample) const;

    /** \brief Returns the adapter for the given filter.
     * \param[in] filter, filter smart pointer to adapt.
     *
     */
    FilterAdapterSPtr  adaptFilter(FilterSPtr filter) const;

    /** \brief Returns the adapter for the given channel and filter adapter.
     * \param[in] filter, filter adapter smart pointer.
     * \param[in] channel, channel smart pointer to adapt.
     *
     */
    ChannelAdapterSPtr adaptChannel(FilterAdapterSPtr filter, ChannelSPtr channel) const;

    /** \brief Returns the adapter for the given segmentation and filter adapter.
     * \param[in] filter, filter adapter smart pointer.
     * \param[in] segmentation, segmentation smart pointer to adapt.
     *
     */
    SegmentationAdapterSPtr adaptSegmentation(FilterAdapterSPtr filter, SegmentationSPtr segmentation) const;

    /** \brief Returns the channel representation factory smart pointer.
     *
     */
    RepresentationFactorySPtr channelRepresentationFactory() const
    { return m_channelRepresentationFactory; }

    /** \brief Returns the segmentation representation factory smart pointer.
     *
     */
    RepresentationFactorySPtr segmentationRepresentationFactory() const
    { return m_segmentationRepresentationFactory; }

    /** \brief Returns the smart pointer of the scheduler used in the factory.
     *
     */
    SchedulerSPtr scheduler() const
    { return m_scheduler; }

  private:
    CoreFactorySPtr m_factory;
    SchedulerSPtr   m_scheduler;
    RepresentationFactoryGroupSPtr m_channelRepresentationFactory;
    RepresentationFactoryGroupSPtr m_segmentationRepresentationFactory;

    QMap<QString, AnalysisReaderList> m_readerExtensions;
    AnalysisReaderList m_readers;
  };

  using ModelFactorySPtr = std::shared_ptr<ModelFactory>;
}// namespace ESPINA

#endif // ESPINA_CORE_FACTORY_H
