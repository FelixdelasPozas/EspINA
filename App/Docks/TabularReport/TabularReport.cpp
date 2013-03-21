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


#include "TabularReport.h"
#include "TabularReportEntry.h"

#include "Dialogs/TabularReport/QueryView.h"

// Espina
#include <Core/Model/EspinaModel.h>
#include <Core/Model/Segmentation.h>
#include <Core/Model/EspinaFactory.h>
#include <Core/Model/QtModelUtils.h>
#include <Core/Extensions/SegmentationExtension.h>

// Qt
#include <QFileDialog>
#include <QLabel>
#include <QScrollArea>
#include <QStandardItemModel>
#include <QKeyEvent>


using namespace EspINA;

//------------------------------------------------------------------------
class DataSortFiler
: public QSortFilterProxyModel
{
public:
  DataSortFiler(QObject *parent = 0) 
  : QSortFilterProxyModel(parent) {}

protected:
//   virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
//   {
//     if (!source_parent.isValid())
//       return true;
// 
//     bool accepted = false;
// 
//     QModelIndex proxyIndex = source_parent.child(source_row, 0);
//     if (proxyIndex == m_root)
//       accepted = true;
//     else if (proxyIndex.parent() == m_root && QtModelUtils::isLeafNode(proxyIndex))
//       accepted = true;
//     else
//     {
//       QModelIndex parent = source_parent.parent();
//       while (!accepted && parent.isValid())
//       {
//         accepted = parent == m_root ;
//         parent   = parent.parent();
//       }
//     }
// 
//     return accepted;
//   }

  //------------------------------------------------------------------------
  virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const
  {
    int role = left.column()>0?Qt::DisplayRole:Qt::DisplayRole;
    bool ok1, ok2;
    double lv = left.data(role).toDouble(&ok1);
    double rv = right.data(role).toDouble(&ok2);

    if (ok1 && ok2)
      return lv < rv;
    else
      return left.data(role).toString() < right.data(role).toString();
  }
};


//------------------------------------------------------------------------
TabularReport::TabularReport(EspinaFactory  *factory,
                             ViewManager    *viewmManager,
                             QWidget        *parent,
                             Qt::WindowFlags f)
: m_factory(factory)
, m_viewManager(viewmManager)
, m_layout(new QVBoxLayout())
, m_multiSelection(false)
{
  QVBoxLayout *layout = new QVBoxLayout();
  QScrollArea *scrollArea = new QScrollArea();
  QWidget *widget = new QWidget();
  m_layout->setMargin(0);
  QPalette pal = this->palette();
  pal.setColor(QPalette::Base, pal.color(QPalette::Background));
  this->setPalette(pal);
  widget->setLayout(m_layout);

  scrollArea->setWidget(widget);
  scrollArea->setWidgetResizable(true);
  layout->addWidget(scrollArea);
  setLayout(layout);

  connect(m_viewManager, SIGNAL(selectionChanged(ViewManager::Selection, bool)),
          this, SLOT(updateSelection(ViewManager::Selection)));
}

//------------------------------------------------------------------------
TabularReport::~TabularReport()
{
}

//------------------------------------------------------------------------
void TabularReport::rowsInserted(const QModelIndex &parent, int start, int end)
{
  if (parent == QModelIndex())
  {
    for (int row = start; row <= end; ++row)
    {
      QModelIndex child = model()->index(row, 0);
      int end = model()->rowCount(child) - 1;
      if (end >= 0)
        rowsInserted(child, 0, end);
    }
  } else if (parent.isValid() && !parent.parent().isValid())
  {
    QString taxonomyName = parent.data(Qt::DisplayRole).toString();
    if (!m_entries.contains(taxonomyName))
    {
      Entry *entry = new Entry(taxonomyName, m_factory);
      entry->title->setText(taxonomyName);

      DataSortFiler *sortFilter = new DataSortFiler(entry);
      sortFilter->setSourceModel(model());
      sortFilter->setDynamicSortFilter(true);
      sortFilter->setSortRole(Qt::DisplayRole);

      QStringList tags;
      tags << tr("Name") << tr("Taxonomy");
      foreach(Segmentation::InformationExtension extension, m_factory->segmentationExtensions())
      {
        if (extension->validTaxonomy(taxonomyName))
        {
          tags << extension->availableInformations();
        }
      }

      QStandardItemModel *header = new QStandardItemModel(1, tags.size(), this);
      header->setHorizontalHeaderLabels(tags);

      QModelIndex rootIndex = sortFilter->mapFromSource(parent);
      sortFilter->setData(rootIndex, tags, InformationTagsRole);

      QTableView *tableView = entry->tableView;
      tableView->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
      tableView->setModel(sortFilter);
      tableView->setRootIndex(rootIndex);
      tableView->setSortingEnabled(true);
      tableView->sortByColumn(3);

      tableView->horizontalHeader()->setModel(header);
      connect(tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
              this, SLOT(updateSelection(QItemSelection,QItemSelection)));


      m_entries[taxonomyName] = entry;

      bool ordered = false;
      int  i = 0;
      while (!ordered && i < m_layout->count())
      {
        Entry *exisintEntry = dynamic_cast<Entry *>(m_layout->itemAt(i)->widget());
        QString currentTaxonomy = m_entries.key(exisintEntry);
        if (currentTaxonomy > taxonomyName)
          ordered = true;
        else
          i++;
      }

      m_layout->insertWidget(i, entry);
    }

    const int numRows = parent.model()->rowCount(parent);
    QTableView *table = m_entries[taxonomyName]->tableView;
    resizeTableViews(table, numRows);
  }
}

//------------------------------------------------------------------------
void TabularReport::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
  if (parent == QModelIndex())
  {
    for (int row = start; row <= end; ++row)
    {
      QString child = model()->index(row, 0).data(Qt::DisplayRole).toString();

      Entry *entry = m_entries[child];
      m_layout->removeWidget(entry);
      delete entry;

      m_entries.remove(child);
    }
  } else if (parent.isValid() && !parent.parent().isValid())
  {
    for (int row = start; row <= end; ++row)
    {
      QString taxonomyName = parent.data(Qt::DisplayRole).toString();
      QTableView *table = m_entries[taxonomyName]->tableView;
      QModelIndex entryRoot = table->rootIndex();
      const int numRows = table->model()->rowCount(entryRoot) - 1;
      resizeTableViews(table, numRows);
    }
  }
}


//------------------------------------------------------------------------
void TabularReport::setModel(QAbstractItemModel *model)
{
  QAbstractItemView::setModel(model);
}

//------------------------------------------------------------------------
bool TabularReport::event(QEvent *event)
{
  if ( QEvent::KeyPress   == event->type() 
    || QEvent::KeyRelease == event->type())
  {
    QKeyEvent *ke = dynamic_cast<QKeyEvent *>(event);
    m_multiSelection = ke->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
  }

  return QAbstractItemView::event(event);
}

//------------------------------------------------------------------------
void TabularReport::reset()
{
  foreach(Entry *entry, m_entries)
  {
    m_layout->removeWidget(entry);
    delete entry;
}
  m_entries.clear();

  QAbstractItemView::reset();

  rowsInserted(rootIndex(), 0, model()->rowCount(rootIndex())-1);
}


//------------------------------------------------------------------------
void TabularReport::updateSelection(ViewManager::Selection selection)
{
  if (!isVisible())
    return;

  foreach(Entry *entry, m_entries)
  {
    QTableView *tableView = entry->tableView;

    tableView->blockSignals(true);
    tableView->selectionModel()->blockSignals(true);
    tableView->selectionModel()->reset();
    tableView->setSelectionMode(QAbstractItemView::MultiSelection);
  }

  foreach(PickableItemPtr item, selection)
  {
    if (EspINA::SEGMENTATION == item->type())
    {
      SegmentationPtr segmentation = segmentationPtr(item);
      Entry *entry = m_entries.value(segmentation->taxonomy()->qualifiedName(), NULL);
      if (entry)
      {
        QTableView *tableView = entry->tableView;
        QAbstractItemModel *model = tableView->model();
        QModelIndex rootIndex = tableView->rootIndex();

        for (int row = 0; row < model->rowCount(rootIndex); ++row)
        {
          QModelIndex index = model->index(row, 0, rootIndex);
          if (index.data().toString() == item->data().toString())
          {
            tableView->selectRow(row);
            if (item == selection.first())
            {
              tableView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::Select);
              //TODO: Scroll scrollArea to show this row
            }
            break;
          }
        }
      }
    }
  }

  foreach(Entry *entry, m_entries)
  {
    QTableView *tableView = entry->tableView;

    tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tableView->selectionModel()->blockSignals(false);
    tableView->blockSignals(false);

    // Update all visible items
    tableView->viewport()->update();
  }
}

//------------------------------------------------------------------------
void TabularReport::updateSelection(QItemSelection selected, QItemSelection deselected)
{
  ViewManager::Selection selection;

  if (m_multiSelection)
  {
    foreach(Entry *entry, m_entries)
    {
      QTableView *tableView = entry->tableView;

      QSortFilterProxyModel *sorFilter = dynamic_cast<QSortFilterProxyModel *>(tableView->model());
      foreach(QModelIndex index, tableView->selectionModel()->selectedRows())
      {
        ModelItemPtr sItem = indexPtr(sorFilter->mapToSource(index));
        if (EspINA::SEGMENTATION == sItem->type())
          selection << pickableItemPtr(sItem);
      }
    }
  } else
  {
    QItemSelectionModel *selectionModel = dynamic_cast<QItemSelectionModel *>(sender());

    QTableView *tableView = NULL;
    foreach(Entry *entry, m_entries)
    {
      if (entry->tableView->selectionModel() == selectionModel)
      {
        tableView = entry->tableView;
        break;
      }
    }

    QAbstractProxyModel   *proxyModel = dynamic_cast<QAbstractProxyModel *>(model());
    QSortFilterProxyModel *sorFilter = dynamic_cast<QSortFilterProxyModel *>(tableView->model());
    foreach(QModelIndex index, tableView->selectionModel()->selectedRows())
    {
      ModelItemPtr sItem = indexPtr(proxyModel->mapToSource(sorFilter->mapToSource(index)));
      if (EspINA::SEGMENTATION == sItem->type())
        selection << pickableItemPtr(sItem);
    }
  }

  m_viewManager->setSelection(selection);
}

//------------------------------------------------------------------------
void TabularReport::resizeTableViews(QTableView *table, const int numRows)
{
  int rowHeight   = table->rowHeight(0);
  int tableHeight = (numRows * rowHeight)
                  + table->horizontalHeader()->height()
                  + 2 * table->frameWidth() 
                  + 0.5* rowHeight;

  table->setMinimumHeight(tableHeight);
  table->setMaximumHeight(tableHeight);
}
