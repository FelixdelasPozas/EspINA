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
#include "common/editor/BrushSelector.h"
#include "common/model/Segmentation.h"

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
  class DrawCommand;
  class EraseCommand;
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
  void startDrawOperation(QAction *);
  void drawSegmentation(IPicker::PickList msel);
  void stopDrawing();
  void stateChanged(BrushSelector::State state);
  void combineSegmentations();
  void substractSegmentations();
  void erodeSegmentations();
  void dilateSegmentations();
  void openSegmentations();
  void closeSegmentations();
  void fillHoles();
  void cancelDrawOperation();

  SegmentationList selectedSegmentations();

private:
  void startPencilDrawing();
  void startContourDrawing();

private:
  ActionSelector *m_actionGroup;
  QAction *m_pencilDisc;
  QAction *m_pencilSphere;
  QAction *m_contour;
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
  BrushSelector   *m_brush;
  ContourSelector *m_contourSelector;
  Filter          *m_currentSource;
  Segmentation    *m_currentSeg;
};

#endif // EDITORTOOLBAR_H
