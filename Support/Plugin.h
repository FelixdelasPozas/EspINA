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
#include <Core/Factory/ExtensionFactory.h>
#include <Core/Factory/AnalysisReader.h>
#include <Core/Factory/FilterFactory.h>
#include <Core/Plugin.h>

// Qt
#include <QtPlugin>
#include <QStringList>

class QUndoStack;
class QAction;

namespace ESPINA
{
  namespace Support
  {
    using ColorEngineSwitchSList = QList<Widgets::ColorEngineSwitchSPtr>;
    using MenuEntry              = QPair<QStringList, QAction *>;

    enum class ToolCategory
    {
      SESSION,
      EXPLORE,
      ROI,
      SEGMENT,
      EDIT,
      VISUALIZE,
      ANALYZE
    };

    using CategorizedTool = QPair<ToolCategory, Widgets::ToolSPtr>;

    /** \class Plugin
     * \brief Interface for plugins at application level (EspINA plugins).
     *
     */
    class EspinaSupport_EXPORT AppPlugin
    : public Core::CorePlugin
    {
      Q_OBJECT
    public:
      /** \brief Plugin class virtual destructor.
       *
       */
      virtual ~AppPlugin()
      {}

      /** \brief Gives the plugin the neccesary objects to initialize itself.
       *        Must be called before any other plugin method.
       *
       */
      virtual void init(Context &context) = 0;

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
       *  Each tool is assigned to one of the available categories
       */
      virtual QList<CategorizedTool> tools() const
      { return QList<CategorizedTool>(); }

      /** \brief Returns a list of reports provided by the plugin.
       *
       */
      virtual ReportSList reports() const
      { return ReportSList(); }

      /** \brief Returns a list of settings panels provided by the plugin.
       *
       */
      virtual Settings::SettingsPanelSList settingsPanels() const
      { return Settings::SettingsPanelSList(); }

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

  } // namespace Support
} // namespace ESPINA

Q_DECLARE_INTERFACE(ESPINA::Support::AppPlugin, "es.upm.cesvima.ESPINA.Plugin/2.0")

#endif // ESPINA_PLUGIN_H
