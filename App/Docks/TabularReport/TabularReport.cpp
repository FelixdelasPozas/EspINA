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

#ifdef TEST_ESPINA_MODELS
#include "common/model/ModelTest.h"
#endif

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
    int role = left.column()>0?Qt::DisplayRole:Qt::UserRole+1;
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
{
  #ifdef TEST_ESPINA_MODELS
  m_modelTester = QSharedPointer<ModelTest>(new ModelTest(m_model.data()));
  #endif

  QVBoxLayout *layout = new QVBoxLayout();
  QScrollArea *scrollArea = new QScrollArea();
  QWidget *widget = new QWidget();
  //layout->setMargin(0);
  QPalette pal = this->palette();
  pal.setColor(QPalette::Base, pal.color(QPalette::Background));
  this->setPalette(pal);
  m_layout = new QVBoxLayout();
  widget->setLayout(m_layout);

  scrollArea->setWidget(widget);
  scrollArea->setWidgetResizable(true);
  layout->addWidget(scrollArea);
  setLayout(layout);

//   tableView->setModel(m_sort.data());
//   tableView->setSortingEnabled(true);
//   tableView->sortByColumn(0, Qt::AscendingOrder);
//   //   tableView->horizontalHeader()->setMovable(true);
//   tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
// 
//   connect(tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
//           this, SLOT(updateSelection(QItemSelection,QItemSelection)));
//   connect(m_viewManager, SIGNAL(selectionChanged(ViewManager::Selection, bool)),
//           this, SLOT(updateSelection(ViewManager::Selection)));
}

//------------------------------------------------------------------------
TabularReport::~TabularReport()
{
}

//------------------------------------------------------------------------
void TabularReport::rowsInserted(const QModelIndex &parent, int start, int end)
{
  if (parent.isValid() && !parent.parent().isValid())
  {
    ModelItemPtr       parentItem = indexPtr(parent);
    TaxonomyElementPtr taxonomy   = taxonomyElementPtr(parentItem);
    for (int row = start; row <= end; ++row)
    {
      if (!m_entries.contains(parentItem))
      {
        QString taxonomyName = taxonomy->qualifiedName();

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

        entry->tableView->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
        entry->tableView->setModel(sortFilter);
        QModelIndex rootIndex = sortFilter->mapFromSource(parent);
        sortFilter->setData(rootIndex, tags, InformationTagsRole);
        entry->tableView->setRootIndex(rootIndex);
        entry->tableView->setSortingEnabled(true);

        QStandardItemModel *header = new QStandardItemModel(1, tags.size(), this);
        header->setHorizontalHeaderLabels(tags);
        entry->tableView->horizontalHeader()->setModel(header);

        m_entries[parentItem] = entry;

        bool ordered = false;
        int  i = 0;
        while (!ordered && i < m_layout->count())
        {
          Entry *entry = dynamic_cast<Entry *>(m_layout->itemAt(i)->widget());
          TaxonomyElementPtr currentTaxonomy = taxonomyElementPtr(m_entries.key(entry));
          if (currentTaxonomy->qualifiedName() > taxonomyName)
            ordered = true;
          else
            i++;
        }

        m_layout->insertWidget(i, entry);
      }
    }
  }
}

//------------------------------------------------------------------------
void TabularReport::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
  if (parent == QModelIndex())
  {
    for (int row = start; row <= end; ++row)
    {
      ModelItemPtr child = indexPtr(model()->index(row, 0));

      Entry *entry = m_entries[child];
      m_layout->removeWidget(entry);
      delete entry;

      m_entries.remove(child);
    }
  }
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
}


//------------------------------------------------------------------------
void TabularReport::updateSelection(ViewManager::Selection selection)
{
//   if (!isVisible())
//     return;
// 
//   //   qDebug() << "Update Data Selection from Selection Manager";
//     tableView->blockSignals(true);
//     tableView->selectionModel()->blockSignals(true);
//     tableView->selectionModel()->reset();
//     tableView->setSelectionMode(QAbstractItemView::MultiSelection);
//     foreach(PickableItemPtr item, selection)
//     {
//       QModelIndex selIndex = index(item);
//       if (selIndex.isValid())
//       {
//         tableView->selectRow(selIndex.row());
//       }
//     }
//     tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
//     tableView->selectionModel()->blockSignals(false);
//     tableView->blockSignals(false);
//     // Center the view at the first selected item
//     if (!selection.isEmpty())
//     {
//       QModelIndex currentIndex = index(selection.first());
//       tableView->selectionModel()->setCurrentIndex(currentIndex, QItemSelectionModel::Select);
//       tableView->scrollTo(currentIndex);
//     }
//     // Update all visible items
//     tableView->viewport()->update();
}

//------------------------------------------------------------------------
void TabularReport::updateSelection(QItemSelection selected, QItemSelection deselected)
{
//   ViewManager::Selection selection;
// 
//   foreach(QModelIndex index, tableView->selectionModel()->selectedRows())
//   {
//     ModelItemPtr sItem = item(index);
//     if (EspINA::SEGMENTATION == sItem->type())
//       selection << pickableItemPtr(sItem);
//   }
// 
//   m_viewManager->setSelection(selection);
}
