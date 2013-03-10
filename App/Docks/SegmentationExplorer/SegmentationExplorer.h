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
#include <Core/Interfaces/IDockWidget.h>

#include "GUI/QtWidget/IEspinaView.h"
#include <ui_SegmentationExplorer.h>

#include <GUI/ViewManager.h>

class QUndoStack;

#include "EspinaConfig.h"
#include <Core/Model/EspinaModel.h>
#include <QStringListModel>

#ifdef TEST_ESPINA_MODELS
class ModelTest;
#endif

namespace EspINA
{
  class SegmentationInspector;

  class SegmentationExplorer
  : public IDockWidget
  , public IEspinaView
  {
    Q_OBJECT
    class GUI;
  public:
    class Layout;

  public:
    explicit SegmentationExplorer(EspinaModel *model,
                                  QUndoStack  *undoStack,
                                  ViewManager *viewManager,
                                  QWidget     *parent = 0);
    virtual ~SegmentationExplorer();

    virtual void initDockWidget(EspinaModel *model,
                                QUndoStack  *undoStack,
                                ViewManager *viewManager);

    virtual void reset(); // slot

  protected:
    void addLayout(const QString id, Layout *proxy);

    virtual bool eventFilter(QObject *sender, QEvent* e);

  protected slots:
    void changeLayout(int index);

    void deleteSelectedItems();
    void showSelectedItemsInformation();

    void focusOnSegmentation(const QModelIndex &index);

    void updateSelection(ViewManager::Selection selection);
    void updateSelection(QItemSelection selected, QItemSelection deselected);

    virtual void updateSegmentationRepresentations(SegmentationList list = SegmentationList());
    virtual void updateChannelRepresentations(ChannelList list = ChannelList());
    virtual void updateSelection();

  protected:
    EspinaModel *m_baseModel;
    QUndoStack  *m_undoStack;
    ViewManager *m_viewManager;

    GUI *m_gui;
    QStringList      m_layoutNames;
    QStringListModel m_layoutModel;
    QList<Layout *>  m_layouts;
    Layout          *m_layout;

  private:
    #ifdef TEST_ESPINA_MODELS
    QSharedPointer<ModelTest>   m_modelTester;
    #endif
  };

} // namespace EspINA

#endif // SEGMENTATIONEXPLORER_H
