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

#include <GUI/ViewManager.h>
#include <Core/Model/Proxies/InformationProxy.h>
#include <QSortFilterProxyModel>
#include <QAbstractItemView>
#include <QVBoxLayout>

class QTableView;

namespace EspINA
{
  class TabularReport
  : public QAbstractItemView
  {
    Q_OBJECT

    class Entry;

  public:
    explicit TabularReport(EspinaFactory *factory,
                           ViewManager *viewManager,
                           QWidget *parent = 0,
                           Qt::WindowFlags f = 0);
    virtual ~TabularReport();

    virtual int horizontalOffset() const{ return 0;}
    virtual QModelIndex indexAt(const QPoint &point) const {return QModelIndex();}
    virtual bool isIndexHidden(const QModelIndex &index) const {return false;}
    virtual QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers){return QModelIndex();}
    virtual void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible){}
    virtual void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command){}
    virtual int verticalOffset() const{return 0;} 
    virtual QRect visualRect(const QModelIndex &index) const {return QRect();}
    virtual QRegion visualRegionForSelection(const QItemSelection &selection) const {return QRegion();}

    virtual void rowsInserted(const QModelIndex &parent, int start, int end);
    virtual void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);

    virtual void setModel(QAbstractItemModel *model);

  protected:
    virtual bool event(QEvent *event);
    virtual void reset();

  protected slots:
    void updateRepresentation(const QModelIndex &index);
    void updateSelection(ViewManager::Selection selection);
    void updateSelection(QItemSelection selected, QItemSelection deselected);

  private:
    void resizeTableViews(QTableView *table, const int numRows = 1);

  private:
    EspinaFactory *m_factory;
    ViewManager   *m_viewManager;

    QStringList  m_query;
    QVBoxLayout *m_layout;
    QMap<QString, Entry *> m_entries;

    bool m_multiSelection;
  };

} // namespace EspINA

#endif // DATAVIEW_H
