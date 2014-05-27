/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This program is free software: you can redistribute it and/or modify
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
#ifndef ESPINA_SEGMHA_IMPORTER_H
#define ESPINA_SEGMHA_IMPORTER_H

#include "SegmhaImporterPlugin_Export.h"

#include <Support/Plugin.h>
#include "SegmhaReader.h"

#include <QUndoCommand>

namespace EspINA
{

  /// Segmha Reader Plugin
  class SegmhaImporterPlugin_EXPORT SegmhaImporterPlugin
  : public Plugin
  {
    Q_OBJECT
    Q_INTERFACES
    (
      EspINA::Plugin
    )
    //     class UndoCommand
    //     : public QUndoCommand
    //     {
    //     public:
    //       static const Filter::FilterType FILTER_TYPE;
    //     public:
    //       explicit UndoCommand(SampleSPtr          sample,
    //                            ChannelSPtr         channel,
    //                            SegmhaImporterFilterSPtr filter,
    //                            EspinaModel         *model,
    //                            QUndoCommand        *parent = NULL);
    //       virtual void redo();
    //       virtual void undo();
    //
    //     private:
    //       EspinaModel         *m_model;
    //       SampleSPtr          m_sample;
    //       ChannelSPtr         m_channel;
    //       SegmhaImporterFilterSPtr m_filter;
    //       SegmentationSList   m_segs;
    //     };

  public:
    explicit SegmhaImporterPlugin();
    virtual ~SegmhaImporterPlugin();

    virtual void init(ModelAdapterSPtr model,
                      ViewManagerSPtr  viewManager,
                      ModelFactorySPtr factory,
                      SchedulerSPtr    scheduler,
                      QUndoStack*      undoStack);

    virtual NamedColorEngineSList colorEngines();

    virtual QList< ToolGroup* > toolGroups();

    virtual QList<DockWidget *> dockWidgets();

    virtual ChannelExtensionFactorySList channelExtensionFactories() const;

    virtual SegmentationExtensionFactorySList segmentationExtensionFactories() const;

    virtual RendererSList renderers();

    virtual SettingsPanelSList settingsPanels();


  private:
    ModelAdapterSPtr m_model;
    ViewManagerSPtr  m_viewManager;
    ModelFactorySPtr m_factory;
    SchedulerSPtr    m_scheduler;
    QUndoStack*      m_undoStack;

    SegmhaReaderSPtr m_reader;
  };

} // namespace EspINA

#endif// SEGMHAIMPORTER_H
