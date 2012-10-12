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

#include "common/selection/IPicker.h"
#include <model/Taxonomy.h>
#include <model/Segmentation.h>
#include "common/pluginInterfaces/IFactoryExtension.h"
#include "common/pluginInterfaces/IFilterCreator.h"
#include "SeedGrowSelector.h"

#include <QSharedPointer>
#include <QUndoCommand>

class ViewManager;
class EspinaModel;
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
, public IFactoryExtension
, public IFilterCreator
{
  class SettingsPanel;
  class Settings;

  class UndoCommand
  : public QUndoCommand
  {
  public:
    explicit UndoCommand(Channel * channel,
                         SeedGrowSegmentationFilter *filter,
                         TaxonomyElement *taxonomy,
                         EspinaModel *model);
    virtual void redo();
    virtual void undo();
  private:
    EspinaModel                *m_model;
    Sample                     *m_sample;
    Channel                    *m_channel;
    SeedGrowSegmentationFilter *m_filter;
    Segmentation               *m_seg;
    TaxonomyElement            *m_taxonomy;
  };

  Q_OBJECT
  Q_INTERFACES(IFactoryExtension IFilterCreator)
public:
  SeedGrowSegmentation(EspinaModel *model,
                       QUndoStack *undoStack,
                       ViewManager *vm,
                       QWidget *parent=NULL);
  virtual ~SeedGrowSegmentation();

  virtual void initFactoryExtension(EspinaFactory* factory);

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
  void startSegmentation(IPicker::PickList msel);

  void batchMode();

signals:
//   void productCreated(Segmentation *);
  void selectionAborted(IPicker *);

private:
  void addPixelSelector(QAction *action, IPicker *handler);
  void buildSelectors();

private:
  EspinaModel      *m_model;
  QUndoStack       *m_undoStack;
  ViewManager      *m_viewManager;

  ThresholdAction  *m_threshold;
  DefaultVOIAction *m_useDefaultVOI;
  ActionSelector   *m_segment;

  QMap<QAction *, IPicker *> m_selectors;
  QSharedPointer<SeedGrowSelector>    m_selector;

  Settings      *m_settings;
  SettingsPanel *m_settingsPanel;
};

#endif// SEEDGROWINGSEGMENTATION_H