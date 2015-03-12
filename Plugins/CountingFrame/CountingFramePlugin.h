/*
 *    
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ESPINA_COUNTING_FRAME_PLUGIN_H
#define ESPINA_COUNTING_FRAME_PLUGIN_H

#include "CountingFramePlugin_Export.h"

#include <Support/Plugin.h>

#include "CountingFrameManager.h"

namespace ESPINA
{
  namespace CF
  {
    class CountingFramePlugin_EXPORT CountingFramePlugin
    : public Plugin
    {
      Q_OBJECT
      Q_INTERFACES(ESPINA::Plugin)

    public:
      explicit CountingFramePlugin();
      virtual ~CountingFramePlugin();

      virtual void init(ModelAdapterSPtr model,
                        ViewManagerSPtr  viewManager,
                        ModelFactorySPtr factory,
                        SchedulerSPtr    scheduler,
                        QUndoStack      *undoStack);

      virtual NamedColorEngineSList colorEngines() const;

      virtual QList<ToolGroup *> toolGroups() const;

      virtual QList<DockWidget *> dockWidgets() const;

      virtual ChannelExtensionFactorySList channelExtensionFactories() const;

      virtual SegmentationExtensionFactorySList segmentationExtensionFactories() const;

      virtual FilterFactorySList filterFactories() const;


      virtual SettingsPanelSList settingsPanels() const;

      virtual QList<MenuEntry> menuEntries() const;

      virtual AnalysisReaderSList analysisReaders() const;

    public slots:
      virtual void onAnalysisClosed();

    private:
      CountingFrameManager m_manager;
      ModelAdapterSPtr     m_model;
      ViewManagerSPtr      m_viewManager;
      SchedulerSPtr        m_scheduler;
      QUndoStack          *m_undoStack;

      NamedColorEngine     m_colorEngine;
      DockWidget *         m_dockWidget;
      ChannelExtensionFactorySPtr m_channelExtensionFactory;
      SegmentationExtensionFactorySPtr m_segmentationExtensionFactory;
//       RendererSPtr m_renderer3d;
//       RendererSPtr m_renderer2d;
    };
  }
} // namespace ESPINA

#endif // ESPINA_COUNTING_FRAME_PLUGIN_H
