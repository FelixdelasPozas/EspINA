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
#include <Core/Interfaces/IFactoryExtension.h>
#include <Core/Interfaces/IFilterCreator.h>

//Forward declarations
class SeedGrowSegmentationTool;
class ActionSelector;
class DefaultVOIAction;
class ThresholdAction;
class IPicker;
class QAction;
class ViewManager;
class QUndoStack;
class EspinaModel;

/// Seed Growing Segmenation Plugin
class SeedGrowSegmentation
: public QToolBar
, public IFactoryExtension
, public IFilterCreator
{
public:
  class Settings;

private:
  class SettingsPanel;

  Q_OBJECT
  Q_INTERFACES(IFactoryExtension IFilterCreator)

public:
  SeedGrowSegmentation(EspinaModel *model,
                       QUndoStack  *undoStack,
                       ViewManager *vm,
                       QWidget *parent=NULL);
  virtual ~SeedGrowSegmentation();

  virtual void initFactoryExtension(EspinaFactory* factory);

  virtual Filter* createFilter(const QString              &filter,
                               const Filter::NamedInputs  &inputs,
                               const ModelItem::Arguments &args);

protected slots:
  /// Change picker
  void changePicker(QAction *action);
  void cancelSegmentationOperation();

  void batchMode();

private:
  void addPixelSelector(QAction *action, IPicker *handler);
  void buildSelectors();

private:
  EspinaModel      *m_model;
  QUndoStack       *m_undoStack;
  ViewManager      *m_viewManager;

  ThresholdAction  *m_threshold;
  DefaultVOIAction *m_useDefaultVOI;
  ActionSelector   *m_pickerSelector;

  QMap<QAction *, IPicker *> m_selectors;
  SeedGrowSegmentationTool * m_tool;

  Settings      *m_settings;
  SettingsPanel *m_settingsPanel;
};

#endif// SEEDGROWINGSEGMENTATION_H
