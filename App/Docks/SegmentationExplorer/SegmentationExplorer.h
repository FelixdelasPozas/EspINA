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

namespace EspINA
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
                                  ViewManagerSPtr  viewManager,
                                  QUndoStack      *undoStack,
                                  QWidget         *parent = 0);
    virtual ~SegmentationExplorer();

    virtual void initDockWidget(ModelAdapterSPtr model,
                                QUndoStack      *undoStack,
                                ViewManagerSPtr  viewManager){}

    virtual void reset(); // slot

  protected:
    void addLayout(const QString id, Layout *proxy);

    virtual bool eventFilter(QObject *sender, QEvent* e);

    // update segmentation explorer gui depending on selected indexes
    void updateGUI(const QModelIndexList &selectedIndexes);

  protected slots:
    void changeLayout(int index);

    void deleteSelectedItems();
    void showSelectedItemsInformation();

    void focusOnSegmentation(const QModelIndex &index);

    virtual Selection currentSelection() const{}//TODO

    virtual void updateSelection(){}//TODO

    void onModelSelectionChanged(QItemSelection selected, QItemSelection deselected);

    virtual void updateRepresentations(ChannelAdapterList list){}//TODO

    virtual void updateRepresentations(SegmentationAdapterList list){}//TODO

    virtual void updateRepresentations(){}//TODO

    void updateSearchFilter();

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

} // namespace EspINA

#endif // ESPINA_SEGMENTATION_EXPLORER_H