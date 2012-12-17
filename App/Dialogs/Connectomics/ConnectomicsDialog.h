/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2012  Laura Fernandez Soria <laura.fernandez@ctb.upm.es>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CONNECTOMICSDIALOG_H
#define CONNECTOMICSDIALOG_H

// Qt
#include <QDialog>
#include <QStringListModel>
#include <QWidget>
#include <QSortFilterProxyModel>

// EspINA
#include <GUI/ViewManager.h>
#include <Core/Model/Proxies/InformationProxy.h>
#include <Core/Model/RelationshipGraph.h>
#include <Core/Model/Segmentation.h>
#include <Core/Model/Proxies/ConnectomicProxy.h>

#include "ui_ConnectomicsDialog.h"

namespace EspINA
{
  class ConnectomicsDialog
  : public QDialog
  , Ui::ConnectomicsDialog
  {
    Q_OBJECT
  public:
    explicit ConnectomicsDialog(EspinaModelPtr  model,
                                ViewManager    *vm,
                                QWidget        *parent = 0,
                                Qt::WindowFlags flags  = 0);
    virtual ~ConnectomicsDialog() {};

    void generateConectomicGraph(EspinaModel *m_model);
    Segmentation itemSelectedProxy(int row, const QModelIndex& sourceParent) const;
    //   QModelIndex mapToSource(const QModelIndex &proxy) const;

  protected slots:
    void showGraphConnectomicsInformation (QModelIndex index);
    void updateSelection(ViewManager::Selection selection);
    void updateSelection(QItemSelection selected, QItemSelection deselected);
    void updateSelectionAux(ViewManager::Selection selection);
    void updateSelectionAux(QItemSelection selected, QItemSelection deselected);
    //   void showGraphConnectomicsInformationAux(QModelIndex index);

  private:
    EspinaModelPtr m_model;

    QStringListModel connectedSegModel;
    ViewManager    *m_viewManager;
    QList<QListView*> m_listView;
    ViewManager::Selection selection;
  };

} // namespace EspINA

#endif // CONNECTOMICSDIALOG_H
