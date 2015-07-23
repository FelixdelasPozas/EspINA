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
#include "Model/SegmentationAdapter.h"

#include <Core/Factory/AnalysisReader.h>
#include <Core/Factory/FilterFactory.h>
#include <Core/Factory/CoreFactory.h>
#include <GUI/View/RepresentationInvalidator.h>

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
     * \param[in] factory core factory smart pointer.
     * \param[in] scheduler scheduler smart pointer.
     *
     */
    explicit ModelFactory(CoreFactorySPtr factory = CoreFactorySPtr(),
                          SchedulerSPtr scheduler = SchedulerSPtr(),
                          GUI::View::RepresentationInvalidator *invalidator = nullptr);

    /** \brief ModelFactory class destructor.
     *
     */
    ~ModelFactory();

    /** \brief Registers an analysis reader in the factory.
     * \param[in] reader to be registered
     *
     */
    void registerAnalysisReader(AnalysisReaderSPtr reader);

    /** \brief Registers a filter factory in the factory.
     * \param[in] factory filter factory smart pointer.
     *
     */
    void registerFilterFactory (FilterFactorySPtr  factory);


    /** \brief Registers a channel extension factory in the factory.
     * \param[in] factory channel extension factory smart pointer.
     *
     */
    void registerExtensionFactory(ChannelExtensionFactorySPtr factory);

    /** \brief Registers a segmentation extension factory in the factory.
     * \param[in] factory segmentation extension factory smart pointer.
     *
     */
    void registerExtensionFactory(SegmentationExtensionFactorySPtr factory);

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
    GUI::SupportedFormats supportedFileExtensions();

    /** \brief Returns the list of raw pointers of the readers registered in the factory for a given file.
     * \param[in] file QFileInfo object.
     *
     */
    AnalysisReaderSList readers(const QFileInfo& file);

    /** \brief Reads a data file and returns an analysis.
     * \param[in] reader analysis reader raw pointer.
     * \param[in] file QFileInfo object.
     * \param[in] handler smart pointer of the error handler to use.
     *
     */
    AnalysisSPtr read(AnalysisReaderSPtr reader, const QFileInfo& file, ErrorHandlerSPtr handler = ErrorHandlerSPtr())
    { return reader->read(file, m_factory, handler); }

    /** \brief Creates and returns a new sample adapter.
     * \param[in] name name of the sample.
     *
     */
    SampleAdapterSPtr createSample(const QString& name = QString()) const;

    /** \brief Creates and returns a filter of the specified type.
     * \param[in] inputs list of input smart pointers.
     * \param[in] type type of the filter to return.
     *
     */
    template<typename T>
    std::shared_ptr<T> createFilter(InputSList inputs, const Filter::Type &type) const
    {
      return m_factory->createFilter<T>(inputs, type);
    }

    /** \brief Creates and returns a channel adapter given the filter and the output id.
     * \param[in] filter filter adapter smart pointer.
     * \param[in] output id of the output of the given filter.
     *
     */
    ChannelAdapterSPtr createChannel(FilterSPtr filter, Output::Id output) const;

    /** \brief Creates and returns a channel extension of the given type.
     * \param[in] type channel extension type.
     *
     */
    ChannelExtensionSPtr createChannelExtension(const ChannelExtension::Type &type);

    /** \brief Creates and returns a segmentation adapter from a given filter and an output id.
     * \param[in] filter filter adapter smart pointer.
     * \param[in] output id of the output of the given filter.
     *
     */
    SegmentationAdapterSPtr createSegmentation(FilterSPtr filter, Output::Id output) const;

    /** \brief Creates and returns a segmentation extension of the given type.
     * \param[in] type segmentation extension type.
     *
     */
    SegmentationExtensionSPtr createSegmentationExtension(const SegmentationExtension::Type &type);

    /** \brief Returns the adapter for the given sample.
     * \param[in] sample sample smart pointer to adapt.
     *
     */
    SampleAdapterSPtr adaptSample(SampleSPtr sample) const;

    /** \brief Returns the adapter for the given channel and filter adapter.
     * \param[in] filter filter adapter smart pointer.
     * \param[in] channel channel smart pointer to adapt.
     *
     */
    ChannelAdapterSPtr adaptChannel(ChannelSPtr channel) const;

    /** \brief Returns the adapter for the given segmentation and filter adapter.
     * \param[in] filter filter adapter smart pointer.
     * \param[in] segmentation segmentation smart pointer to adapt.
     *
     */
    SegmentationAdapterSPtr adaptSegmentation(SegmentationSPtr segmentation) const;

    /** \brief Returns the smart pointer of the scheduler used in the factory.
     *
     */
    SchedulerSPtr scheduler() const
    { return m_scheduler; }

  private:
    using Invalidator = GUI::View::RepresentationInvalidator;

    CoreFactorySPtr m_factory;
    SchedulerSPtr   m_scheduler;
    Invalidator    *m_invalidator;

    AnalysisReaderSList                m_readers;
    QMap<QString, AnalysisReaderSList> m_readerExtensions;
  };

  using ModelFactorySPtr = std::shared_ptr<ModelFactory>;

  template<typename Extensible>
  void addSegmentationExtension(Extensible segmentation, const QString &type, ModelFactorySPtr factory)
  {
    auto extensions = segmentation->extensions();

    if (!extensions->hasExtension(type))
    {
      if (factory->availableSegmentationExtensions().contains(type))
      {
        auto extension = factory->createSegmentationExtension(type);

        if(extension->validCategory(segmentation->category()->classificationName()))
        {
          extensions->add(extension);
        }
        else
        {
          qWarning() << segmentation->data().toString() << "doesn't support"<< type << "extensions";
        }
      }
      else
      {
        qWarning() << type << " extension is not available";
      }
    }
  }

  template<typename Extensible>
  SegmentationExtensionSPtr retrieveOrCreateExtension(Extensible segmentation, const QString &type, ModelFactorySPtr factory)
  {
    addSegmentationExtension(segmentation, type, factory);

    return segmentation->readOnlyExtensions()[type];
  }

}// namespace ESPINA

#endif // ESPINA_CORE_FACTORY_H
