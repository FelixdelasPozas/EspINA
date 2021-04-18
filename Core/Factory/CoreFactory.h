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
#include <Core/Analysis/Extensible.hxx>
#include <Core/Factory/AnalysisReader.h>
#include <Core/Factory/FilterFactory.h>
#include <Core/Factory/ExtensionFactory.h>
#include <Core/Analysis/Extensions.h>
#include <Core/Utils/TemporalStorage.h>
#include <Core/Utils/EspinaException.h>

// Qt
#include <QStringList>
#include <QMap>
#include <QDir>

namespace ESPINA
{
  /** \class CoreFactory
   * \brief Factory for core model objects.
   */
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
      void registerExtensionFactory(Core::StackExtensionFactorySPtr factory);

      /** \brief Returns the list of channel extensions types this factory can create.
       *
       *  Extension state is restored using cache and state data.
       */
      Core::StackExtension::TypeList availableStackExtensions() const;

      /** \brief Create a channel extension.
       * \param[in] type channel extension type.
       * \param[in] cache cache object.
       * \param[in] state state object.
       *
       *  Extension state is restored using cache and state data.
       */
      Core::StackExtensionSPtr createStackExtension(const Core::StackExtension::Type      &type,
                                                    const Core::StackExtension::InfoCache &cache = Core::StackExtension::InfoCache(),
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
      void registerExtensionFactory(Core::SegmentationExtensionFactorySPtr factory);

      /** \brief Returns a list of segmentation extension types that this factory can create.
       *
       */
      Core::SegmentationExtension::TypeList availableSegmentationExtensions() const;

      /** \brief Create an extension instance of the given type.
       *
       *  Extension state is restored using cache and state data.
       */
      Core::SegmentationExtensionSPtr createSegmentationExtension(const Core::SegmentationExtension::Type      &type,
                                                                  const Core::SegmentationExtension::InfoCache &cache = Core::SegmentationExtension::InfoCache(),
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

      /** \brief Returns the factory thread scheduler.
       *
       */
      SchedulerSPtr scheduler() const
      { return m_scheduler; }

      /** \brief Returns the default temporal storage for the factory.
       *
       */
      TemporalStorageSPtr defaultStorage() const;

      /** \brief Registers an analysis reader in the factory.
       * \param[in] reader to be registered
       *
       */
      void registerAnalysisReader(AnalysisReaderSPtr reader);

      /** \brief Returns the list of raw pointers of the readers registered in the factory for a given file.
       * \param[in] file QFileInfo object.
       *
       */
      const AnalysisReaderSList readers(const QFileInfo &file) const;

      /** \brief Returns the list of file extensions the factory can read.
       *
       */
      Core::Utils::SupportedFormats supportedFileExtensions();

    private:

      SchedulerSPtr               m_scheduler;           /** task scheduler.                   */
      mutable TemporalStorageSPtr m_defaultStorage;      /** factory default temporal storage. */
      QDir                       *m_temporalStorageDir;  /** directory for temporal storage.   */

      QMap<Filter::Type, FilterFactorySPtr>                                           m_filterFactories;
      QMap<Core::StackExtension::Type, Core::StackExtensionFactorySPtr>               m_stackExtensionFactories;
      QMap<Core::SegmentationExtension::Type, Core::SegmentationExtensionFactorySPtr> m_segmentationExtensionFactories;
      AnalysisReaderSList                                                             m_readers;
      QMap<QString, AnalysisReaderSList>                                              m_readerExtensions;
  };

  //--------------------------------------------------------------------
  template<typename Extensible, typename Factory>
  Core::SegmentationExtensionSPtr retrieveOrCreateSegmentationExtension(Extensible item, const Core::SegmentationExtension::Type &type, Factory factory)
  {
    auto extensions = item->extensions(); // get extensions lock for the rest of the method to avoid concurrency problems.

    if(!extensions->hasExtension(type))
    {
      if(!factory->availableSegmentationExtensions().contains(type))
      {
        auto message = QObject::tr("Unknown or read-only segmentation extension type %1").arg(type);
        auto details = QObject::tr("CoreFactory::retrieveOrCreateSegmentationExtension() -> ") + message;

        throw Core::Utils::EspinaException(message, details);
      }

      auto extension = factory->createSegmentationExtension(type);

      if(extension->validCategory(item->category()->classificationName()))
      {
        if(extension->validData(item->output()))
        {
          extensions->add(extension);
        }
        else
        {
          auto message = QObject::tr("Segmentation %1 supports but can't make use of %2 extension.").arg(item->number()).arg(type);
          auto details = QObject::tr("CoreFactory::retrieveOrCreateSegmentationExtension() -> ") + message;

          throw Core::Utils::EspinaException(message, details);
        }
      }
      else
      {
        auto message = QObject::tr("Segmentation %1 doesn't support %2 extensions.").arg(item->number()).arg(type);
        auto details = QObject::tr("CoreFactory::retrieveOrCreateSegmentationExtension() -> ") + message;

        throw Core::Utils::EspinaException(message, details);
      }
    }

    return extensions[type];
  }

  //--------------------------------------------------------------------
  template<typename Extension, typename Extensible, typename Factory>
  std::shared_ptr<Extension> retrieveOrCreateSegmentationExtension(Extensible item, Factory factory)
  {
    auto extension = retrieveOrCreateSegmentationExtension(item, Extension::TYPE, factory);
    return std::dynamic_pointer_cast<Extension>(extension);
  }

  //--------------------------------------------------------------------
  template<typename Extensible, typename Factory>
  Core::StackExtensionSPtr retrieveOrCreateStackExtension(Extensible item, const Core::StackExtension::Type &type, Factory factory)
  {
    auto extensions = item->extensions(); // get extensions lock for the rest of the method to avoid concurrency problems.

    if(!extensions->hasExtension(type))
    {
      if(!factory->availableStackExtensions().contains(type))
      {
        auto message = QObject::tr("Unknown or read-only stack extension type %1").arg(type);
        auto details = QObject::tr("CoreFactory::retrieveOrCreateSegmentationExtension() -> ") + message;

        throw Core::Utils::EspinaException(message, details);
      }

      auto extension = factory->createStackExtension(type);
      extensions->add(extension);
    }

    return extensions[type];
  }

  //--------------------------------------------------------------------
  template<typename Extension, typename Extensible, typename Factory>
  std::shared_ptr<Extension> retrieveOrCreateStackExtension(Extensible item, Factory factory)
  {
    auto extension = retrieveOrCreateStackExtension(item, Extension::TYPE, factory);
    return std::dynamic_pointer_cast<Extension>(extension);
  }

}// namespace ESPINA

#endif // ESPINA_CORE_FACTORY_H
