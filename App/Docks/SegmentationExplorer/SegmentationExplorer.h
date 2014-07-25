/*
    
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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


#ifndef ESPINA_SEGMENTATION_EXPLORER_H
#define ESPINA_SEGMENTATION_EXPLORER_H

//----------------------------------------------------------------------------
// File:    SegmentationExplorer.h
// Purpose: Dock widget to manage segmentations in the model
//----------------------------------------------------------------------------
#include <Support/DockWidget.h>

#include <ui_SegmentationExplorer.h>

class QUndoStack;

#include <QStringListModel>

namespace ESPINA
{
  class SegmentationInspector;

  class SegmentationExplorer
  : public DockWidget
  , public SelectableView
  {
    Q_OBJECT
    class GUI;
  public:
    class Layout;

  public:
    explicit SegmentationExplorer(ModelAdapterSPtr model,
                                  ModelFactorySPtr factory,
                                  ViewManagerSPtr  viewManager,
                                  QUndoStack      *undoStack,
                                  QWidget         *parent = 0);
    virtual ~SegmentationExplorer();

    virtual void updateRepresentations(ChannelAdapterList list){}

    virtual void updateRepresentations(SegmentationAdapterList list){}

    virtual void updateRepresentations() {}

    virtual void reset(); // slot

  protected:
    virtual void onSelectionSet(SelectionSPtr selection);

    void addLayout(const QString id, Layout *proxy);

    virtual bool eventFilter(QObject *sender, QEvent* e);

    // update segmentation explorer gui depending on selected indexes
    void updateGUI(const QModelIndexList &selectedIndexes);

  protected slots:
    void changeLayout(int index);

    void deleteSelectedItems();

    void showSelectedItemsInformation();

    void focusOnSegmentation(const QModelIndex &index);

    void onModelSelectionChanged(QItemSelection selected, QItemSelection deselected);

    void updateSearchFilter();

    void onSelectionChanged();

    void onItemModified();

  protected:
    ModelAdapterSPtr m_baseModel;
    ViewManagerSPtr  m_viewManager;
    QUndoStack      *m_undoStack;

    GUI *m_gui;
    QStringList      m_layoutNames;
    QStringListModel m_layoutModel;
    QList<Layout *>  m_layouts;
    Layout          *m_layout;
  };

} // namespace ESPINA

#endif // ESPINA_SEGMENTATION_EXPLORER_H