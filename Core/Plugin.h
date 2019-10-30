/*

 Copyright (C) 2018 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef CORE_PLUGIN_H_
#define CORE_PLUGIN_H_

#include "Core/EspinaCore_Export.h"

// EspINA
#include <Core/Factory/AnalysisReader.h>
#include <Core/Factory/ExtensionFactory.h>
#include <Core/Factory/FilterFactory.h>
#include <Core/MultiTasking/Scheduler.h>

// Qt
#include <QObject>
#include <QList>

class QPluginLoader;

namespace ESPINA
{
  namespace Core
  {
    /** \class Plugin
     * \brief Interface for plugins at core level (Core plugins).
     *
     */
    class EspinaCore_EXPORT CorePlugin
    : public QObject
    {
        Q_OBJECT
      public:
        /** \brief Plugin class virtual destructor.
         *
         */
        virtual ~CorePlugin()
        {}

        /** \brief Returns the name of the plugin.
         *
         */
        virtual const QString name() const = 0;

        /** \brief Returns a brief description of the plugin.
         *
         */
        virtual const QString description() const = 0;

        /** \brief Returns the author/organization responsible of the plugin.
         *
         */
        virtual const QString organization() const = 0;

        /** \brief Returns an email address to contact the authors/organization of the plugin.
         *
         */
        virtual const QString maintainer() const = 0;

        /** \brief Intializes the plugin in the case of using only Core instances and not application plugins.
         * \param[in] scheduler Application task scheduler.
         *
         */
        virtual void init(SchedulerSPtr scheduler = nullptr) = 0;

        /** \brief Returns a list of channel extension factories.
         *
         *  Whenever this plugin provides a channel extension, it should provide
         *  a factory to obtain such extensions, otherwise read only information will
         *  be available after loading them.
         */
        virtual StackExtensionFactorySList channelExtensionFactories() const
        { return StackExtensionFactorySList(); }

        /** \brief Returns a list of segmentation extension factories.
         *
         *  Whenever this plugin provides a segmentation extension, it should provide
         *  a factory to obtain such extensions, otherwise read only information will
         *  be available after loading them.
         */
        virtual SegmentationExtensionFactorySList segmentationExtensionFactories() const
        { return SegmentationExtensionFactorySList(); }

        /** \brief Returns a list of filter factories provided by the plugin.
         *
         */
        virtual FilterFactorySList filterFactories() const
        { return FilterFactorySList(); }

        /** \brief Returns a list of analysis readers provided by this plugin.
         *
         */
        virtual AnalysisReaderSList analysisReaders() const
        { return AnalysisReaderSList(); }
    };

    /** \brief Loads the plugins in the given path to the given core factory.
     *
     */
    QList<QPluginLoader *> loadPlugins(const QDir &path, CoreFactory *factory);

    /** \brief Adds the components of the plugin to the given core factory. Return true on success
     * and false otherwise.
     * \param[in] instance Plugin instance returned from QPluginLoader class.
     * \param[in] factory Core factory pointer.
     *
     */
    bool loadPlugin(QObject *instance, CoreFactory* factory);

    /** \brief Unloads the given plugins.
     *
     */
    void unloadPlugins(const QList<QPluginLoader *> plugins);

  } // namespace Core
} // namespace ESPINA

Q_DECLARE_INTERFACE(ESPINA::Core::CorePlugin, "es.upm.cesvima.ESPINA.Core.Plugin/1.0")

#endif // CORE_PLUGIN_H_
