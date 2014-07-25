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
#ifndef ESPINA_SEGMHA_IMPORTER_H
#define ESPINA_SEGMHA_IMPORTER_H

#include "SegmhaImporterPlugin_Export.h"

// Plugin
#include "SegmhaReader.h"

// ESPINA
#include <Support/Plugin.h>

// Qt
#include <QUndoCommand>

namespace ESPINA
{
  class SegmhaImporterPlugin_EXPORT SegmhaImporterPlugin
  : public Plugin
  {
    Q_OBJECT
    Q_INTERFACES(ESPINA::Plugin)

  public:
    explicit SegmhaImporterPlugin();
    virtual ~SegmhaImporterPlugin();

    virtual void init(ModelAdapterSPtr model,
                      ViewManagerSPtr  viewManager,
                      ModelFactorySPtr factory,
                      SchedulerSPtr    scheduler,
                      QUndoStack*      undoStack);

    /* \brief Implements Plugin::colorEngines().
     *
     */
    virtual NamedColorEngineSList colorEngines() const;

    /* \brief Implements Plugin::toolGroups().
     *
     */
    virtual QList< ToolGroup* > toolGroups() const;

    /* \brief Implements Plugin::dockWidgets().
     *
     */
    virtual QList<DockWidget *> dockWidgets() const;

    /* \brief Implements Plugin::channelExtensionFactories().
     *
     */
    virtual ChannelExtensionFactorySList channelExtensionFactories() const;

    /* \brief Implements Plugin::segmentationExtensionFactories().
     *
     */
    virtual SegmentationExtensionFactorySList segmentationExtensionFactories() const;

    /* \brief Implements Plugin::filterFactories().
     *
     */
    virtual FilterFactorySList filterFactories() const;

    /* \brief Implements Plugin::analysisReaders().
     *
     */
    virtual AnalysisReaderSList analysisReaders() const;

    /* \brief Implements Plugin::renderers().
     *
     */
    virtual RendererSList renderers() const;

    /* \brief Implements Plugin::settingsPanels().
     *
     */
    virtual SettingsPanelSList settingsPanels() const;

    /* \brief Implements Plugin::menuEntries().
     *
     */
    virtual QList<MenuEntry> menuEntries() const;

  private:
    ModelAdapterSPtr m_model;
    ViewManagerSPtr  m_viewManager;
    ModelFactorySPtr m_factory;
    SchedulerSPtr    m_scheduler;
    QUndoStack*      m_undoStack;

    SegmhaReaderSPtr  m_reader;
    FilterFactorySPtr m_filterFactory;
  };

  class SegmhaFilterFactory
  : public FilterFactory
  {
    public:
      virtual ~SegmhaFilterFactory()
      {};

      virtual FilterSPtr createFilter(InputSList          inputs,
                                      const Filter::Type& filter,
                                      SchedulerSPtr       scheduler) const throw (Unknown_Filter_Exception);

      virtual FilterTypeList providedFilters() const;
  };

} // namespace ESPINA

#endif// SEGMHAIMPORTER_H
