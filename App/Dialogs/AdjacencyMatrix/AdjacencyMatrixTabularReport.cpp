/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

 ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include <Dialogs/AdjacencyMatrix/AdjacencyMatrixTabularReport.h>
#include <Core/Utils/ListUtils.hxx>
#include <Core/Utils/SupportedFormats.h>
#include <GUI/Dialogs/DefaultDialogs.h>

// Qt
#include <QStandardItem>
#include <QStandardItemModel>
#include <QItemDelegate>

// C++
#include <cstring>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI;

//--------------------------------------------------------------------
AdjacencyMatrixTabularReport::AdjacencyMatrixTabularReport(SegmentationAdapterList segmentations, Support::Context& context)
: TabularReport  (context)
, m_segmentations{segmentations}
{
  createEntry(segmentations, context.model());

  m_exportButton->setToolTip(tr("Save Adjacency Matrix Data"));
  m_exportButton->setEnabled(true);

  resize(sizeHint());
  setMinimumSize(600, 400);
  adjustSize();
  move(parentWidget()->window()->frameGeometry().topLeft() + parentWidget()->window()->rect().center() - rect().center());
}

//--------------------------------------------------------------------
void AdjacencyMatrixTabularReport::exportInformation()
{
  auto title      = tr("Export Adjacency Matrix");
  auto suggestion = tr("Adjacency Matrix.xls");
  auto formats    = SupportedFormats().addExcelFormat().addCSVFormat();
  auto fileName   = DefaultDialogs::SaveFile(title, formats, QDir::homePath(), ".xls", suggestion, this);

  if (fileName.isEmpty()) return;

  if(!fileName.endsWith(".csv", Qt::CaseInsensitive) && !fileName.endsWith(".xls", Qt::CaseInsensitive))
  {
    fileName += tr(".xls");
  }

  if (fileName.endsWith(".csv", Qt::CaseInsensitive))
  {
    try
    {
      exportToCSV(fileName);
    }
    catch(const EspinaException &e)
    {
      DefaultDialogs::InformationMessage(tr("Unable to export %1").arg(fileName), title, e.details(), this);
    }
  }
  else if (fileName.endsWith(".xls", Qt::CaseInsensitive))
  {
    try
    {
      exportToXLS(fileName);
    }
    catch(const EspinaException &e)
    {
      DefaultDialogs::InformationMessage(tr("Unable to export %1").arg(fileName), title, e.details(), this);
    }
  }
}

//--------------------------------------------------------------------
void AdjacencyMatrixTabularReport::createEntry(const SegmentationAdapterList segmentations, const ModelAdapterSPtr model)
{
  auto entry = new Entry(segmentations, model, this);
  entry->refreshInformation->setVisible(false);
  entry->selectInformation->setVisible(false);
  entry->progressBar->setVisible(false);
  entry->progressLabel->setVisible(false);
  entry->exportInformation->setToolTip(tr("Save Adjacency Matrix Data"));

  connect(entry->tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this,                               SLOT(updateSelection(QItemSelection,QItemSelection)));

  m_tabs->insertTab(0, entry, tr("Adjacency Matrix"));
}

//--------------------------------------------------------------------
void AdjacencyMatrixTabularReport::updateSelection(QItemSelection selected, QItemSelection deselected)
{
  SegmentationAdapterList selectedSegs;

  auto entry = dynamic_cast<Entry *>(m_tabs->widget(0));
  if(entry)
  {
    for (auto index : entry->tableView->selectionModel()->selectedIndexes())
    {
      if (index.column() == 0 || !index.isValid())
        continue;

      auto horizontalItem = entry->horizontalHeaders().at(index.column() - 1);
      auto verticalItem = entry->verticalHeaders().at(index.row());

      if (!selectedSegs.contains(horizontalItem)) selectedSegs << horizontalItem;
      if (!selectedSegs.contains(verticalItem))   selectedSegs << verticalItem;
    }

    if (!selectedSegs.isEmpty())
    {
      blockSignals(true);
      getSelection(m_context)->set(selectedSegs);
      blockSignals(false);
    }
  }
}

//--------------------------------------------------------------------
AdjacencyMatrixTabularReport::Entry::Entry(const SegmentationAdapterList segmentations, const ModelAdapterSPtr model, QWidget* parent)
: TabularReport::Entry("Distances(nm)", nullptr, nullptr, parent)
{
  auto table = new QStandardItemModel(tableView);

  auto sortFilter = new DataSortFilter();
  sortFilter->setSourceModel(table);
  sortFilter->setDynamicSortFilter(true);

  tableView->setSelectionBehavior(QAbstractItemView::SelectItems);
  tableView->setItemDelegate(new QItemDelegate());
  tableView->setAlternatingRowColors(true);

  m_horizontalHeaders = toRawList<SegmentationAdapter>(model->segmentations());
  if(!segmentations.isEmpty())
  {
    m_verticalHeaders = segmentations;
  }
  else
  {
    m_verticalHeaders = m_horizontalHeaders;
  }

  // get values
  int connectionsVertical[m_verticalHeaders.size()];
  int connectionsHorizontal[m_horizontalHeaders.size()];
  std::memset(connectionsVertical, 0, sizeof(int)*m_verticalHeaders.size());
  std::memset(connectionsHorizontal, 0, sizeof(int)*m_horizontalHeaders.size());

  int tableValues[m_verticalHeaders.size()][m_horizontalHeaders.size()];
  for(int i = 0; i < m_verticalHeaders.size(); ++i)
  {
    auto seg_i = model->smartPointer(m_verticalHeaders.at(i));
    for(int j = 0; j < m_horizontalHeaders.size(); ++j)
    {
      tableValues[i][j] = model->connections(seg_i, model->smartPointer(m_horizontalHeaders.at(j))).size();
      connectionsVertical[i] += tableValues[i][j];
      connectionsHorizontal[j] += tableValues[i][j];
    }
  }

  // remove empty table values.
  SegmentationAdapterList toShowVertical;
  for(int i = 0; i < m_verticalHeaders.size(); ++i)
  {
    if(connectionsVertical[i] != 0) toShowVertical << m_verticalHeaders.at(i);
  }

  SegmentationAdapterList toShowHorizontal;
  for(int j = 0; j < m_horizontalHeaders.size(); ++j)
  {
    if(connectionsHorizontal[j] != 0) toShowHorizontal << m_horizontalHeaders.at(j);
  }

  int column = 0;
  int row = 0;

  table->setRowCount(toShowVertical.size());
  table->setColumnCount(1 + toShowHorizontal.size());

  for (auto from: toShowVertical)
  {
    auto segItem = new QStandardItem(QString(from->data().toString()));
    auto font = segItem->font();
    font.setBold(true);
    segItem->setFont(font);
    segItem->setEditable(false);
    segItem->setSelectable(false);
    segItem->setDragEnabled(false);
    segItem->setDropEnabled(false);
    table->setItem(row, column++, segItem);

    for (auto to: toShowHorizontal)
    {
      QStandardItem *item = nullptr;
      if(from != to)
      {
        item = new QStandardItem(QString::number(tableValues[m_verticalHeaders.indexOf(from)][m_horizontalHeaders.indexOf(to)]));
        item->setEditable(false);
        item->setDragEnabled(false);
        item->setDropEnabled(false);
        item->setToolTip(tr("Connections between %1 and %2.").arg(from->data().toString()).arg(to->data().toString()));
      }
      else
      {
        item = new QStandardItem();
        item->setEditable(false);
        item->setDragEnabled(false);
        item->setDropEnabled(false);
        item->setBackground(QColor{192,192,192});
      }

      table->setItem(row, column++, item);
    }

    column = 0;
    ++row;
  }

  table->setHeaderData(0, Qt::Horizontal, "Segmentation");
  for(int i = 0; i < toShowHorizontal.size(); ++i) table->setHeaderData(i + 1, Qt::Horizontal, toShowHorizontal.at(i)->data().toString());

  tableView->setModel(sortFilter);
}

//--------------------------------------------------------------------
void AdjacencyMatrixTabularReport::Entry::extractInformation()
{
  m_category = tr("Adjacency Matrix");
  if(m_verticalHeaders.size() == 1)
  {
    m_category += " " + m_verticalHeaders.first()->data().toString();
  }

  TabularReport::Entry::extractInformation();
}
