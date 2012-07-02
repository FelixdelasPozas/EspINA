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

#include <EspinaCore.h>
#include <QFileDialog>
#include "QueryView.h"

#ifdef DEBUG
  #include "common/model/ModelTest.h"
#include <model/Segmentation.h>
#endif

//------------------------------------------------------------------------
DataView::DataView(QWidget* parent, Qt::WindowFlags f)
: QWidget(parent, f)
, m_model(new InformationProxy())
, m_sort (new QSortFilterProxyModel())
{
  setupUi(this);

  EspinaModel *model = EspinaCore::instance()->model().data();
  m_model->setSourceModel(model);
  m_sort->setSourceModel(m_model.data());
  m_sort->setDynamicSortFilter(true);
  m_sort->setSortRole(Qt::UserRole+1);

#ifdef DEBUG
  m_modelTester = QSharedPointer<ModelTest>(new ModelTest(m_model.data()));
#endif

  tableView->setModel(m_sort.data());
  tableView->setSortingEnabled(true);
  tableView->sortByColumn(0, Qt::AscendingOrder);
//   tableView->horizontalHeader()->setMovable(true);

  QIcon iconSave = qApp->style()->standardIcon(QStyle::SP_DialogSaveButton);
  writeDataToFile->setIcon(iconSave);
  connect(writeDataToFile, SIGNAL(clicked()),
	  this,SLOT(extractInformation()));
  connect(changeQuery, SIGNAL(clicked()),
	  this,SLOT(defineQuery()));
  connect(tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
	  this, SLOT(updateSelection(QItemSelection,QItemSelection)));
  connect(m_sort.data(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
	  this, SLOT(updateSelection(QModelIndex)));
}

//------------------------------------------------------------------------
DataView::~DataView()
{

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
void DataView::updateSelection(QModelIndex index)
{
  if (index.isValid())
  {
    QString data = index.data().toString();
    ModelItem *item = indexPtr(m_sort->mapToSource(index));
    if (ModelItem::SEGMENTATION == item->type())
    {
      tableView->blockSignals(true);
      Segmentation *seg = dynamic_cast<Segmentation *>(item);
      if (seg->isSelected())
      {
	int row = index.row();
// 	tableView->selectionModel()->select(index.sibling(row,0), QItemSelectionModel::SelectCurrent);
	tableView->selectionModel()->setCurrentIndex(index.sibling(row,0),QItemSelectionModel::Select);
	for (int c = 1; c < m_sort->columnCount(); c++)
	  tableView->selectionModel()->select(index.sibling(row,c), QItemSelectionModel::Select);
// 	tableView->selectRow(index.row());
      }
      else
      {
	int row = index.row();
	for (int c = 0; c < m_sort->columnCount(); c++)
	  tableView->selectionModel()->select(index.sibling(row,c), QItemSelectionModel::Deselect);
      }
      tableView->blockSignals(false);
    }
  }
}

//------------------------------------------------------------------------
void DataView::updateSelection(QItemSelection selected, QItemSelection deselected)
{
  m_sort->blockSignals(true);
  foreach(QModelIndex index, selected.indexes())
  {
    if (index.column() == 0)
      m_sort->setData(index, true, Segmentation::SelectionRole);
  }

  foreach(QModelIndex index, deselected.indexes())
  {
    if (index.column() == 0)
      m_sort->setData(index, false, Segmentation::SelectionRole);
  }
  m_sort->blockSignals(false);
}
