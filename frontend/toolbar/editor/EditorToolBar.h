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


#ifndef EDITORTOOLBAR_H
#define EDITORTOOLBAR_H

#include <QToolBar>
#include <pluginInterfaces/IFactoryExtension.h>
#include <pluginInterfaces/IFilterCreator.h>

// EspINA
#include "common/model/Segmentation.h"
#include "Brush.h"

class Channel;
class ITool;
class Brush;
class ViewManager;
class ActionSelector;
class ContourSelector;
class ContourWidget;
class EspinaModel;
class FreeFormSource;
class QAction;
class QUndoStack;
class ModelItem;

class EditorToolBar
: public QToolBar
, public IFactoryExtension
, public IFilterCreator
{
  Q_OBJECT
  Q_INTERFACES(IFactoryExtension IFilterCreator)
  class FreeFormCommand;
  class CODECommand;//CloseOpenDilateErode Command

public:
  class Settings;
  class SettingsPanel;

public:
  explicit EditorToolBar(EspinaModel *model,
                         QUndoStack *undoStack,
                         ViewManager *vm,
                         QWidget *parent = 0);

  virtual void initFactoryExtension(EspinaFactory* factory);

  virtual Filter* createFilter(const QString filter,
                               Filter::NamedInputs inputs,
                               const ModelItem::Arguments args);
protected slots:
  void changeCircularBrushMode(Brush::BrushMode mode);
  void changeSphericalBrushMode(Brush::BrushMode mode);
  void changeDrawTool(QAction *action);
  void cancelDrawOperation();
  void changeSplitTool(QAction *action);
  void cancelSplitOperation();
  void combineSegmentations();
  void substractSegmentations();
  void closeSegmentations();
  void openSegmentations();
  void dilateSegmentations();
  void erodeSegmentations();
  void fillHoles();
  void updateAvailableOperations();

  SegmentationList selectedSegmentations();

private:
  void initDrawTools();
  void initSplitTools();
  void initMorphologicalTools();
  void initCODETools();
  void initFillTool();

private:
  ActionSelector *m_drawToolSelector;
  QMap<QAction *, ITool *> m_drawTools;

  ActionSelector *m_splitToolSelector;
  QMap<QAction *, ITool *> m_splitTools;

  QAction *m_addition;
  QAction *m_substraction;

  QAction *m_erode;
  QAction *m_dilate;
  QAction *m_open;
  QAction *m_close;

  QAction *m_fill;
  ContourWidget *m_contourWidget;

  EspinaModel *m_model;
  QUndoStack  *m_undoStack;
  ViewManager *m_viewManager;

  Settings        *m_settings;
};

#endif // EDITORTOOLBAR_H
