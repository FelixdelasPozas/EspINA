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


#ifndef ESPINA_TABULAR_REPORT_H
#define ESPINA_TABULAR_REPORT_H

#include <QWidget>

#include <QSortFilterProxyModel>
#include <QAbstractItemView>
#include <QVBoxLayout>
#include <Support/ViewManager.h>
#include <GUI/Model/ModelAdapter.h>

class QTableView;

namespace EspINA
{
  class TabularReport
  : public QAbstractItemView
  {
    Q_OBJECT

    class Entry;

  public:
    explicit TabularReport(ModelFactorySPtr factory,
                           ViewManagerSPtr  viewManager,
                           QWidget         *parent = 0,
                           Qt::WindowFlags  f = 0);
    virtual ~TabularReport();

    virtual int horizontalOffset() const
    { return 0;}

    virtual QModelIndex indexAt(const QPoint &point) const
    { return QModelIndex(); }

    virtual bool isIndexHidden(const QModelIndex &index) const
    { return false; }

    virtual QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
    { return QModelIndex(); }

    virtual void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible)
    {}

    virtual void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
    {}

    virtual int verticalOffset() const
    { return 0; }

    virtual QRect visualRect(const QModelIndex &index) const
    { return QRect(); }

    virtual QRegion visualRegionForSelection(const QItemSelection &selection) const
    {return QRegion();}

    virtual void setModel(ModelAdapterSPtr model);

    /// Only display segmentations' information. If segmentations is empty, then
    /// all segmentations' information is displayed
    virtual void setFilter(SegmentationAdapterList segmentations);

  protected:
    virtual void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    virtual void rowsInserted(const QModelIndex &parent, int start, int end);

    virtual bool event(QEvent *event);
    virtual void reset();

  public slots:
    void updateSelection(SegmentationAdapterList selection);

  protected slots:
    void indexDoubleClicked(QModelIndex index);
    void updateRepresentation(const QModelIndex &index);
    void updateSelection(QItemSelection selected, QItemSelection deselected);
    void rowsRemoved(const QModelIndex &parent, int start, int end);
    void exportInformation();

  private:
    bool acceptSegmentation(const SegmentationAdapterPtr segmentation);

    void createCategoryEntry(const QString &category);

    bool exportToCSV(const QFileInfo &filename);

    bool exportToXLS(const QString &filename);

    QModelIndex mapToSource(const QModelIndex &index);
    QModelIndex mapFromSource(const QModelIndex &index, QSortFilterProxyModel *sortFilter);

    void removeTabsAndWidgets();

  private:
    ModelAdapterSPtr m_model;
    ModelFactorySPtr m_factory;
    ViewManagerSPtr  m_viewManager;

    SegmentationAdapterList m_filter;
    QTabWidget *m_tabs;

    bool m_multiSelection;
  };

} // namespace EspINA

#endif // DATAVIEW_H
