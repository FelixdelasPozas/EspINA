/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>
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


#ifndef DATAVIEW_H
#define DATAVIEW_H

#include <QWidget>
#include <ui_DataView.h>

#include <GUI/ViewManager.h>
#include <Core/Model/Proxies/InformationProxy.h>
#include <QSortFilterProxyModel>

#ifdef TEST_ESPINA_MODELS
class ModelTest;
#endif

namespace EspINA
{
  class DataView
  : public QWidget
  , Ui::DataView
  {
    Q_OBJECT
  public:
    explicit DataView(EspinaModel *model,
                      ViewManager *vm,
                      QWidget *parent = 0,
                      Qt::WindowFlags f = 0);
    virtual ~DataView();

  protected:
    QModelIndex index(ModelItemPtr item) const;
    ModelItemPtr item(QModelIndex index) const;

  protected slots:
    void defineQuery();
    void extractInformation();
    void updateSelection(ViewManager::Selection selection);
    void updateSelection(QItemSelection selected, QItemSelection deselected);

  private:
    EspinaModel                          *m_baseModel;
    ViewManager                          *m_viewManager;
    QSharedPointer<InformationProxy>      m_model;
    QSharedPointer<QSortFilterProxyModel> m_sort;

    #ifdef TEST_ESPINA_MODELS
    QSharedPointer<ModelTest>             m_modelTester;
    #endif

    QStringList m_query;
  };

} // namespace EspINA

#endif // DATAVIEW_H
