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


#ifndef SEGMENTATIONEXPLORER_H
#define SEGMENTATIONEXPLORER_H

//----------------------------------------------------------------------------
// File:    SegmentationExplorer.h
// Purpose: Dock widget to manage segmentations in the model
//----------------------------------------------------------------------------
#include <QDockWidget>

#include "GUI/QtWidget/IEspinaView.h"
#include <ui_SegmentationExplorer.h>

#include <GUI/ViewManager.h>

class SegmentationInspector;
class EspinaModel;
class QUndoStack;

#include "EspinaConfig.h"
#ifdef TEST_ESPINA_MODELS
class ModelTest;
#endif

class SegmentationExplorer
: public QDockWidget
, public IEspinaView
{
  Q_OBJECT
  class GUI;
public:
  class Layout;

public:
  explicit SegmentationExplorer(EspinaModel *model,
                                QUndoStack  *undoStack,
                                ViewManager *vm,
                                QWidget *parent = 0);
  virtual ~SegmentationExplorer();

protected:
  void addLayout(const QString id, Layout *proxy);

protected slots:
  void changeLayout(int index);
  void focusOnSegmentation(const QModelIndex &index);
  void showInformation();
  void deleteSegmentations();
  void segmentationsDeleted(const QModelIndex parent, int start, int end);
  void updateSelection(ViewManager::Selection selection);
  void updateSelection(QItemSelection selected, QItemSelection deselected);

  void releaseInspectorResources(SegmentationInspector *inspector);

  virtual ISettingsPanel* settingsPanel();
  virtual void updateSegmentationRepresentations();
  virtual void updateSelection();

protected:
  GUI *m_gui;

  EspinaModel    *m_baseModel;
  QUndoStack     *m_undoStack;
  ViewManager    *m_viewManager;

  QStringList     m_layoutNames;
  QList<Layout *> m_layouts;
  Layout         *m_layout;

  QMap<Segmentation *, SegmentationInspector *> m_inspectors;

private:
#ifdef TEST_ESPINA_MODELS
  QSharedPointer<ModelTest>   m_modelTester;
#endif
};

#endif // SEGMENTATIONEXPLORER_H
