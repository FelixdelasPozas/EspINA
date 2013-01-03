/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#include "DataView.h"

#include "Dialogs/TabularReport/QueryView.h"

// Espina
#include <Core/Model/EspinaModel.h>
#include <Core/Model/Segmentation.h>

// Qt
#include <QFileDialog>

#ifdef TEST_ESPINA_MODELS
  #include "common/model/ModelTest.h"
#endif

using namespace EspINA;

class DataSortFiler
: public QSortFilterProxyModel
{
protected:
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
DataView::DataView(EspinaModel *model,
                   ViewManager *viewmManager,
                   QWidget     *parent,
                   Qt::WindowFlags f)
: QWidget(parent, f)
, m_baseModel(model)
, m_viewManager(viewmManager)
, m_model(new InformationProxy())
, m_sort (new DataSortFiler())
{
  setupUi(this);

  m_model->setSourceModel(m_baseModel);
  m_sort->setSourceModel(m_model.data());
  m_sort->setDynamicSortFilter(true);

#ifdef TEST_ESPINA_MODELS
  m_modelTester = QSharedPointer<ModelTest>(new ModelTest(m_model.data()));
#endif

  tableView->setModel(m_sort.data());
  tableView->setSortingEnabled(true);
  tableView->sortByColumn(0, Qt::AscendingOrder);
//   tableView->horizontalHeader()->setMovable(true);
  tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

  QIcon iconSave = qApp->style()->standardIcon(QStyle::SP_DialogSaveButton);
  writeDataToFile->setIcon(iconSave);
  connect(writeDataToFile, SIGNAL(clicked()),
	  this,SLOT(extractInformation()));
  connect(changeQuery, SIGNAL(clicked()),
	  this,SLOT(defineQuery()));
  connect(tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
	  this, SLOT(updateSelection(QItemSelection,QItemSelection)));
  connect(m_viewManager, SIGNAL(selectionChanged(ViewManager::Selection, bool)),
	  this, SLOT(updateSelection(ViewManager::Selection)));
}

//------------------------------------------------------------------------
DataView::~DataView()
{
}

//------------------------------------------------------------------------
QModelIndex DataView::index(ModelItemPtr item) const
{
  QModelIndex baseModelIndex = m_baseModel->index(item);
  QModelIndex informationIndex = m_model->mapFromSource(baseModelIndex);
  return m_sort->mapFromSource(informationIndex);
}

//------------------------------------------------------------------------
ModelItemPtr DataView::item(QModelIndex index) const
{
  return indexPtr(m_sort->mapToSource(index));
}

//------------------------------------------------------------------------
void DataView::defineQuery()
{
  QStringList query = m_model->availableInformation();
  QueryView *querySelector = new QueryView(query, this);
  querySelector->exec();
  m_model->setQuery(query);
}

//------------------------------------------------------------------------
void DataView::extractInformation()
{
  QString title   = tr("Export Segmentation Data");
  QString fileExt = tr("CSV Text File (*.csv)");
  QString fileName = QFileDialog::getSaveFileName(this, title, "", fileExt);

  if (fileName.isEmpty())
    return;

  QFile file(fileName);
  file.open(QIODevice::WriteOnly |  QIODevice::Text);
  QTextStream out(&file);
  out << m_model->query().join(",") << "\n";
  for (int r = 0; r < m_sort->rowCount(); r++)
  {
    for (int c = 0; c < m_sort->columnCount(); c++)
    {
      if (c)
        out << ",";
      out << m_sort->index(r,c).data().toString();
    }
    out << "\n";
  }
  file.close();
}

//------------------------------------------------------------------------
void DataView::updateSelection(ViewManager::Selection selection)
{
  if (!isVisible())
    return;

//   qDebug() << "Update Data Selection from Selection Manager";
  tableView->blockSignals(true);
  tableView->selectionModel()->blockSignals(true);
  tableView->selectionModel()->reset();
  tableView->setSelectionMode(QAbstractItemView::MultiSelection);
  foreach(PickableItemPtr item, selection)
  {
    QModelIndex selIndex = index(item);
    if (selIndex.isValid())
    {
      tableView->selectRow(selIndex.row());
    }
  }
  tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  tableView->selectionModel()->blockSignals(false);
  tableView->blockSignals(false);
  // Center the view at the first selected item
  if (!selection.isEmpty())
  {
    QModelIndex currentIndex = index(selection.first());
    tableView->selectionModel()->setCurrentIndex(currentIndex, QItemSelectionModel::Select);
    tableView->scrollTo(currentIndex);
  }
  // Update all visible items
  tableView->viewport()->update();
}

//------------------------------------------------------------------------
void DataView::updateSelection(QItemSelection selected, QItemSelection deselected)
{
  ViewManager::Selection selection;

  foreach(QModelIndex index, tableView->selectionModel()->selectedRows())
  {
    ModelItemPtr sItem = item(index);
    if (EspINA::SEGMENTATION == sItem->type())
      selection << pickableItemPtr(sItem);
  }

  m_viewManager->setSelection(selection);
}
