/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#ifndef SEEDGROWINGSEGMENTATION_H
#define SEEDGROWINGSEGMENTATION_H

#include <QToolBar>

#include "common/pluginInterfaces/FilterFactory.h"
#include "common/selection/SelectionHandler.h"
#include <model/Taxonomy.h>
#include <model/Segmentation.h>
#include "SeedGrowSelector.h"

#include <QSharedPointer>
#include <QUndoCommand>

//Forward declarations
class ActionSelector;
class DefaultVOIAction;
class SeedGrowSegmentationFilter;
class ThresholdAction;
class SeedGrowSegmentationPanel;
class SeedGrowSegmentationSettings;
class Channel;


/// Seed Growing Segmenation Plugin
class SeedGrowSegmentation
: public QToolBar
, public FilterFactory
{
  class SettingsPanel;
  class Settings;

  class UndoCommand : public QUndoCommand
  {
  public:
    explicit UndoCommand(Channel * channel,
			SeedGrowSegmentationFilter *filter,
			TaxonomyNode *taxonomy);
    virtual void redo();
    virtual void undo();
  private:
    Sample                     *m_sample;
    Channel                    *m_channel;
    SeedGrowSegmentationFilter *m_filter;
    Segmentation               *m_seg;
    TaxonomyNode               *m_taxonomy;
  };

  Q_OBJECT
public:
  SeedGrowSegmentation(QWidget *parent=NULL);
  virtual ~SeedGrowSegmentation();

  virtual Filter* createFilter(const QString filter,
                               Filter::NamedInputs inputs,
                               const ModelItem::Arguments args);

protected slots:
  /// Wait for Seed Selection
  void waitSeedSelection(QAction *action);
  /// Abort current selection
  void abortSelection();
  void onSelectionAborted();
  /// Starts the segmentation filter putting using @msel as seed
  void startSegmentation(SelectionHandler::MultiSelection msel);

  void batchMode();

signals:
//   void productCreated(Segmentation *);
  void selectionAborted(SelectionHandler *);

private:
  void addPixelSelector(QAction *action, SelectionHandler *handler);
  void buildSelectors();

private:
  ThresholdAction  *m_threshold;
  DefaultVOIAction *m_useDefaultVOI;
  ActionSelector   *m_segment;
  QMap<QAction *, SelectionHandler *> m_selectors;
  QSharedPointer<SeedGrowSelector> m_selector;
  Settings      *m_settings;
  SettingsPanel *m_settingsPanel;
};

#endif// SEEDGROWINGSEGMENTATION_H