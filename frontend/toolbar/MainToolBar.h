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

//----------------------------------------------------------------------------
// File:    MainToolBar.h
// Purpose: Provide tool buttons to:
//          - Toggle Segmentation Visibility
//          - Select Active Taxonomy
//          - Remove Segmentation by clicking on them
//----------------------------------------------------------------------------
#ifndef MAINTOOLBAR_H
#define MAINTOOLBAR_H

#include <QToolBar>

#include "common/tools/IPicker.h"

#include <QModelIndex>

class PixelSelector;
class EspinaModel;
class QComboBox;
class QTreeView;
class QUndoStack;
class ViewManager;

class MainToolBar
: public QToolBar
{
  Q_OBJECT
public:
  explicit MainToolBar(EspinaModel *model,
                       QUndoStack  *undoStack,
                       ViewManager *vm,
                       QWidget* parent = 0);

public slots:
  void setShowSegmentations(bool visible);

protected slots:
  void setActiveTaxonomy(QModelIndex index);
  void setActiveTaxonomy(QString taxonomy);
  void updateTaxonomy(QModelIndex left, QModelIndex right);
  void removeSegmentation(bool active);
  void removeSelectedSegmentation(IPicker::PickList msel);
  void toggleCrosshair(bool);
  void abortSelection();

signals:
  void showSegmentations(bool);

private:
  EspinaModel   *m_model;
  QUndoStack    *m_undoStack;
  ViewManager   *m_viewManager;

  QAction       *m_toggleSegVisibility, *m_removeSegmentation, *m_toggleCrosshair;
  QComboBox     *m_taxonomySelector;
  QTreeView     *m_taxonomyView;
  PixelSelector *m_selector;
};

#endif // MAINTOOLBAR_H
