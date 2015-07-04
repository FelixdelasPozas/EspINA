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

#ifndef ESPINA_PLUGIN_H
#define ESPINA_PLUGIN_H

#include "Support/EspinaSupport_Export.h"

// ESPINA
#include <GUI/Types.h>
#include <Support/Types.h>
#include <Core/Factory/SegmentationExtensionFactory.h>
#include <Core/Factory/ChannelExtensionFactory.h>
#include <Core/Factory/AnalysisReader.h>
#include <Core/Factory/FilterFactory.h>

// Qt
#include <QtPlugin>
#include <QStringList>

class QUndoStack;

namespace ESPINA
{
  namespace Support
  {
    using ColorEngineSwitchSList = QList<Widgets::ColorEngineSwitchSPtr>;
    using MenuEntry              = QPair<QStringList, QAction *>;

    enum class ToolCategory
    {
      EXPLORE,
      RESTRICT,
      SEGMENT,
      REFINE,
      VISUALIZE,
      ANALYZE
    };

    using CategorizedTool = QPair<ToolCategory, Widgets::ToolSPtr>;

    class EspinaSupport_EXPORT Plugin
    : public QObject
    {
      Q_OBJECT
    public:
      /** \brief Plugin class virtual destructor.
       *
       */
      virtual ~Plugin()
      {}

      /** \brief Gives the plugin the neccesary objects to initilize itself.
       *        Must be called before any other plugin method.
       *
       */
      virtual void init(Context &context) = 0;

      /** \brief Returns a list of channel extension factories.
       *
       *  Whenever this plugin provides a channel extension, it should provide
       *  a factory to obtain such extensions, otherwise read only information will
       *  be available after loading them.
       */
      virtual ChannelExtensionFactorySList channelExtensionFactories() const
      { return ChannelExtensionFactorySList(); }

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

      /** \brief Returns a list of analysis readers provided by the plugin.
       *
       */
      virtual AnalysisReaderSList analysisReaders() const
      { return AnalysisReaderSList(); }

      /** \brief Returns a list of color engines provided by the plugin.
       *
       */
      virtual ColorEngineSwitchSList colorEngines() const
      { return ColorEngineSwitchSList(); }

      /** \brief Returns a list of ToolGroups provided by the plugin.
       *
       */
      virtual RepresentationFactorySList representationFactories() const
      { return RepresentationFactorySList(); }

      /** \brief Returns a list of tools provided by the plugin.
       *
       *  Each tool is asigned to one of the available categories
       */
      virtual QList<CategorizedTool> tools() const
      { return QList<CategorizedTool>(); }

      /** \brief Returns a list of settings panels provided by the plugin.
       *
       */
      virtual Settings::SettingsPanelSList settingsPanels() const
      { return Settings::SettingsPanelSList(); }

      /** \brief Returns a list of menu entries to add to the main application.
       *
       */
      virtual QList<MenuEntry> menuEntries() const
      { return QList<MenuEntry>(); }

    public slots:
      /** \brief Perform operations when an analysis is closed.
       *
       * Use to free resources.
       *
       */
      virtual void onAnalysisClosed()
      {}

      /** \brief Perform operations when an analysis changes.
       *
       * Use to free resources or reevaluate values.
       *
       */
      virtual void onAnalysisChanged()
      {}
    };
  }
} // namespace ESPINA

Q_DECLARE_INTERFACE(ESPINA::Support::Plugin, "es.upm.cesvima.ESPINA.Plugin/1.4")

#endif // ESPINA_PLUGIN_H
