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

#ifndef ESPINA_CORE_FACTORY_H
#define ESPINA_CORE_FACTORY_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include "Core/Factory/FilterFactory.h"
#include "Core/Factory/ChannelExtensionFactory.h"
#include "Core/Factory/SegmentationExtensionFactory.h"
#include <Core/Utils/TemporalStorage.h>

// Qt
#include <QStringList>
#include <QMap>
#include <QDir>

namespace ESPINA
{

  class EspinaCore_EXPORT CoreFactory
  {
  public:
    /** \brief CoreFactory class constructor.
     * \param[in] scheduler scheduler smart pointer.
     *
     */
    explicit CoreFactory(SchedulerSPtr scheduler = SchedulerSPtr());

    /** \brief CoreFactory class destructor.
     *
     */
    ~CoreFactory();

    /** \brief Registers a filter factory.
     * \param[in] factory filter factory smart pointer.
     *
     */
    void registerFilterFactory(FilterFactorySPtr factory);

    /** \brief Creates a filter given the inputs and the type.
     * \param[in] inputs list of input smart pointers.
     * \param[in] type filter type.
     *
     */
    FilterSPtr createFilter(InputSList inputs, const Filter::Type& type) const;

    /** \brief Convenience method to create filters with a single view item input
     * \param[in] input view item
     * \param[in] type filter type.
     *
     * This is a convenience method to create input
     */
    template<typename T>
    std::shared_ptr<T> createFilter(ViewItem *input, const Filter::Type &type) const
    {
      InputSList inputs;
      inputs << input->asInput();

      return createFilter<T>(inputs, type);
    }

    /** \brief Creates filter given the inputs and the type.
     * \param[in] inputs list of input smart pointers.
     * \param[in] type filter type.
     *
     */
    template<typename T>
    std::shared_ptr<T> createFilter(InputSList inputs, const Filter::Type &type) const
    {
      auto filter = std::make_shared<T>(inputs, type, m_scheduler);
      filter->setStorage(m_defaultStorage);
      return filter;
    }

    /** \brief Creates a sample.
     * \param[in] name sample name.
     *
     */
    SampleSPtr createSample(const QString& name = QString()) const;

    /** \brief Creates a channel.
     * \param[in] filter filter smart pointer.
     * \param[in] id output id.
     *
     */
    ChannelSPtr createChannel(FilterSPtr filter, Output::Id id) const;

    /** \brief Registers channel extension factory
     * \param[in] factory channel extension factory smart pointer.
     *
     *  From now on, CoreFactory can create all the channel extensions provided by
     *  the registered factory
     */
    void registerExtensionFactory(ChannelExtensionFactorySPtr factory);

    /** \brief Returns the list of channel extensions types this factory can create.
     *
     *  Extension state is restored using cache and state data.
     */
    ChannelExtensionTypeList availableChannelExtensions() const;

    /** \brief Create a channel extension.
     * \param[in] type channel extension type.
     * \param[in] cache cache object.
     * \param[in] state state object.
     *
     *  Extension state is restored using cache and state data.
     */
    ChannelExtensionSPtr createChannelExtension(const ChannelExtension::Type      &type,
                                                const ChannelExtension::InfoCache &cache = ChannelExtension::InfoCache(),
                                                const State &state = State());

    /** \brief Creates a segmentation.
     * \param[in] filte, filter smart pointer.
     * \param[in] id output id.
     *
     */
    SegmentationSPtr createSegmentation(FilterSPtr filter, Output::Id id) const;

    /** \brief Registers segmentation extension factory.
     * \param[in] factor segmentation extension factory smart pointer.
     *
     *  From now on, CoreFactory can create all the segmentation extensions provided by
     *  the registered factory.
     */
    void registerExtensionFactory(SegmentationExtensionFactorySPtr factory);

    /** \brief Returns a list of segmentation extension types that this factory can create.
     *
     */
    SegmentationExtensionTypeList availableSegmentationExtensions() const;

    /** \brief Create an extension instance of the given type.
     *
     *  Extension state is restored using cache and state data.
     */
    SegmentationExtensionSPtr createSegmentationExtension(const SegmentationExtension::Type      &type,
                                                          const SegmentationExtension::InfoCache &cache = SegmentationExtension::InfoCache(),
                                                          const State &state = State());

    /** \brief Sets temporal storage for the factory.
     * \param[in] storage temporal storage smart pointer.
     *
     */
    void setPersistentStorage(TemporalStorageSPtr storage)
    { m_defaultStorage = storage; }

    /** \brief Returns a temporal storage object.
     *
     */
    TemporalStorageSPtr createTemporalStorage() const;

    /** \brief Sets the directory for the created TemporalStorage objects.
     * \param[in] directory temporal directory.
     *
     */
    void setTemporalDirectory(const QDir &directory);

  private:
    /** \brief Returns the default temporal storage for the factory.
     *
     */
    TemporalStorageSPtr defaultStorage() const;

    SchedulerSPtr               m_scheduler;           /** task scheduler.                   */
    mutable TemporalStorageSPtr m_defaultStorage;      /** factory default temporal storage. */
    QDir                       *m_temporalStorageDir;  /** dir for temporal storage.         */

    QMap<Filter::Type, FilterFactorySPtr>                               m_filterFactories;
    QMap<ChannelExtension::Type, ChannelExtensionFactorySPtr>           m_channelExtensionFactories;
    QMap<SegmentationExtension::Type, SegmentationExtensionFactorySPtr> m_segmentationExtensionFactories;
  };

}// namespace ESPINA

#endif // ESPINA_CORE_FACTORY_H
