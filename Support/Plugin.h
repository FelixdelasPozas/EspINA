/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#ifndef ESPINA_PLUGIN_H
#define ESPINA_PLUGIN_H

#include "Support/EspinaSupport_Export.h"

#include <GUI/Model/ModelAdapter.h>
#include <GUI/Representations/Renderers/Renderer.h>
#include <GUI/ColorEngines/ColorEngine.h>
#include <Support/DockWidget.h>
#include <Support/Settings/SettingsPanel.h>
#include <Support/ViewManager.h>
#include "ToolGroup.h"

#include <QtPlugin>

class QUndoStack;

namespace EspINA
{
  using NamedColorEngine      = QPair<QString, ColorEngineSPtr>;
  using NamedColorEngineSList = QList<NamedColorEngine>;
  using MenuEntry              = QPair<QStringList, QAction *>;

  class EspinaSupport_EXPORT Plugin
  : public QObject
  {
    Q_OBJECT
  public:
    virtual ~Plugin(){}

    virtual void init(ModelAdapterSPtr model,
                      ViewManagerSPtr  viewManager,
                      ModelFactorySPtr factory,
                      SchedulerSPtr    scheduler,
                      QUndoStack      *undoStack) = 0;

    /** \brief Returns a list of channel extension factories
     *
     *  Whenever this plugin provides a channel extension, it should provide
     *  a factory to obtain such extensions, otherwise read only information will
     *  be available after loading them.
     */
    virtual ChannelExtensionFactorySList channelExtensionFactories() const = 0;

    /** \brief Returns a list of segmentation extension factories
     *
     *  Whenever this plugin provides a segmentation extension, it should provide
     *  a factory to obtain such extensions, otherwise read only information will
     *  be available after loading them.
     */
    virtual SegmentationExtensionFactorySList segmentationExtensionFactories() const = 0;

    /* \brief Returns a list of filter factories.
     *
     */
    virtual FilterFactorySList filterFactories() const = 0;

    /* \brief Returns a list of analysis readers.
     *
     */
    virtual AnalysisReaderSList analysisReaders() const = 0;

    /* \brief Returns a list of color engines.
     *
     */
    virtual NamedColorEngineSList colorEngines() const = 0;

    /* \brief Returns a list of ToolGroups.
     *
     */
    virtual QList<ToolGroup *> toolGroups() const = 0;

    /* \brief Returns a list of Dock Widgets.
     *
     */
    virtual QList<DockWidget *> dockWidgets() const = 0;

    /* \brief Returns a list of Renderers.
     *
     */
    virtual RendererSList renderers() const = 0;

    /* \brief Returns a list of settings panels.
     *
     */
    virtual SettingsPanelSList settingsPanels() const = 0;

    /* \brief Returns a list of menu entries to add to the main application.
     *
     */
    virtual QList<MenuEntry> menuEntries() const = 0;

  public slots:
    virtual void onAnalysisChanged() {}
//     /** \brief
//      */
//     virtual Snapshot snapshot() const = 0;
//
//     void setStorage(TemporalStorageSPtr storage)
//     { m_storage = storage; }
//
//     TemporalStorageSPtr storage() const
//     { return m_storage; }
//
//   private:
//     TemporalStorageSPtr m_storage;
  };

} // namespace EspINA

Q_DECLARE_INTERFACE(EspINA::Plugin, "es.upm.cesvima.EspINA.Plugin/1.0")

#endif // ESPINA_PLUGIN_H
