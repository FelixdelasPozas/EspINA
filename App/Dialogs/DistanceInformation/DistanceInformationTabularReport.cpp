/*
 * Copyright (C) 2016, Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>
 *
 * This file is part of ESPINA.
 *
 * ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// ESPINA
#include "DistanceInformationTabularReport.h"
#include <Core/Utils/ListUtils.hxx>
#include <GUI/Dialogs/DefaultDialogs.h>

// Qt
#include <QStandardItemModel>
#include <QItemDelegate>
#include <QItemSelectionModel>
#include <QModelIndex>

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::Core::Utils;

//--------------------------------------------------------------------
DistanceInformationTabularReport::DistanceInformationTabularReport(Support::Context& context,
                                                                   const SegmentationAdapterList &segmentations,
                                                                   const DistanceInformationOptionsDialog::Options &options,
                                                                   const DistanceInformationDialog::DistancesMap &distances)
: TabularReport  (context)
, m_segmentations(segmentations)
, m_options      (options)
, m_distances    (distances)
{
  if(m_options.tableType == DistanceInformationOptionsDialog::TableType::COMBINED)
  {
    createEntry(m_segmentations);
  }
  else
  {
    for(auto seg: segmentations)
    {
      SegmentationAdapterList list;
      list << seg;
      createEntry(list);
    }
  }

  m_exportButton->setToolTip(tr("Save Distances Data"));
  m_exportButton->setEnabled(true);

  resize(sizeHint());
  setMinimumSize(600, 400);
  adjustSize();
  move(parentWidget()->window()->frameGeometry().topLeft() + parentWidget()->window()->rect().center() - rect().center());
}

//--------------------------------------------------------------------
void DistanceInformationTabularReport::exportInformation()
{
  auto title      = tr("Export Segmentation Distances");
  auto suggestion = tr("Segmentation Distances.xls");
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
void DistanceInformationTabularReport::createEntry(const SegmentationAdapterList segmentations)
{
  auto entry = new Entry(segmentations, m_options, m_distances, this);
  entry->refreshInformation->setVisible(false);
  entry->selectInformation->setVisible(false);
  entry->progressBar->setVisible(false);
  entry->progressLabel->setVisible(false);
  entry->exportInformation->setToolTip(tr("Save Tab Distances Data"));

  connect(entry->tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this,                               SLOT(updateSelection(QItemSelection,QItemSelection)));

  QString title;
  if(segmentations.size() == 1)
  {
    title = segmentations.first()->data().toString();
  }
  else
  {
    title = tr("Distances (nm)");
  }

  m_tabs->insertTab(m_tabs->count(), entry, title);
}

//--------------------------------------------------------------------
void DistanceInformationTabularReport::updateSelection(QItemSelection selected, QItemSelection deselected)
{
  auto emitter = qobject_cast<QItemSelectionModel *>(sender());

  if(emitter)
  {
    for(int i = 0; i < m_tabs->count(); ++i)
    {
      auto entry = dynamic_cast<Entry *>(m_tabs->widget(i));
      if(entry && entry->tableView->selectionModel() == emitter)
      {
        SegmentationAdapterList selectedSegs;

        for(auto index: entry->tableView->selectionModel()->selectedIndexes())
        {
          if(index.column() == 0 || !index.isValid()) continue;

          auto horizontalItem = entry->horizontalHeaders().at(index.column()-1);
          auto verticalItem   = entry->verticalHeaders().at(index.row());
          if(!selectedSegs.contains(horizontalItem)) selectedSegs << horizontalItem;
          if(!selectedSegs.contains(verticalItem)) selectedSegs << verticalItem;
        }

        if(!selectedSegs.isEmpty())
        {
          blockSignals(true);
          getSelection(m_context)->set(selectedSegs);
          blockSignals(false);
        }
      }
    }
  }
}

//--------------------------------------------------------------------
DistanceInformationTabularReport::Entry::Entry(const SegmentationAdapterList                   segmentations,
                                               const DistanceInformationOptionsDialog::Options &options,
                                               const DistanceInformationDialog::DistancesMap   &distances,
                                               QWidget                                         *parent)
: TabularReport::Entry("Distances(nm)", nullptr, nullptr, parent)
{
  auto table = new QStandardItemModel(tableView);

  auto sortFilter = new DataSortFilter();
  sortFilter->setSourceModel(table);
  sortFilter->setDynamicSortFilter(true);

  tableView->setSelectionBehavior(QAbstractItemView::SelectItems);
  tableView->setItemDelegate(new QItemDelegate());
  tableView->setAlternatingRowColors(true);

  m_verticalHeaders = segmentations;

  // get headers
  if(segmentations.size() == 1)
  {
    m_horizontalHeaders = distances[segmentations.first()].keys();
  }
  else
  {
    QSet<SegmentationAdapterPtr> headers;

    for (auto from : segmentations)
      for (auto to : distances[from].keys())
        headers << to;

    m_horizontalHeaders = headers.toList();
  }

  // remove m_horizontalHeaders elements if there is a minimum distance.
  if(options.maxDistance != 0 || options.minDistance != 0)
  {
    QSet<SegmentationAdapterPtr> headers;

    for(auto from: segmentations)
    {
      for(auto to: m_horizontalHeaders)
      {
        auto dist = distances[from][to];
        bool accepted = true;

        if((options.maxDistance != 0) && (dist > options.maxDistance))
        {
          accepted = false;
        }

        if((options.minDistance != 0) && (dist < options.minDistance))
        {
          accepted = false;
        }

        if(accepted)
        {
          headers << to;
        }
      }
    }

    m_horizontalHeaders = headers.toList();
  }

  // sort headers
  sort(m_horizontalHeaders);
  sort(m_verticalHeaders);

  int column = 0;
  int row = 0;

  table->setRowCount(m_verticalHeaders.size());
  table->setColumnCount(1 + m_horizontalHeaders.size());

  for (auto from : m_verticalHeaders)
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

    for (auto to : m_horizontalHeaders)
    {
      QStandardItem *distItem = nullptr;

      if((options.category != CategoryAdapterSPtr()) &&
          !to->category()->classificationName().startsWith(options.category->classificationName(), Qt::CaseSensitive) &&
          !from->category()->classificationName().startsWith(options.category->classificationName(), Qt::CaseSensitive))
      {
        distItem = new QStandardItem("Not computed");
        distItem->setEditable(false);
        distItem->setDragEnabled(false);
        distItem->setDropEnabled(false);
        distItem->setData(QVariant::fromValue(Qt::blue), Qt::TextColorRole);
        distItem->setToolTip(tr("Does not comply with category selection."));
      }
      else
      {
        auto dist = distances[from][to];

        distItem = new QStandardItem(QString::number(dist));
        distItem->setEditable(false);
        distItem->setDragEnabled(false);
        distItem->setDropEnabled(false);

        if(dist == -1)
        {
          distItem->setData(tr("Error"), Qt::DisplayRole);
          distItem->setData(QVariant::fromValue(Qt::red), Qt::TextColorRole);
          distItem->setToolTip(tr("Error occurred when computing this distance."));
          distItem->setFont(font);
        }
        else
        {
          if( ((options.maxDistance != 0) && (dist > options.maxDistance)) ||
              ((options.minDistance != 0) && (dist < options.minDistance)) )
          {
            distItem->setData(QVariant::fromValue(Qt::red), Qt::TextColorRole);
            distItem->setToolTip(tr("Distance outside of given limits."));
          }
        }
      }

      table->setItem(row, column++, distItem);
    }

    column = 0;
    ++row;
  }

  table->setHeaderData(0, Qt::Horizontal, "Segmentation");
  for(int i = 0; i < m_horizontalHeaders.size(); ++i) table->setHeaderData(i + 1, Qt::Horizontal, m_horizontalHeaders.at(i)->data().toString());

  tableView->setModel(sortFilter);
}

//--------------------------------------------------------------------
void DistanceInformationTabularReport::Entry::extractInformation()
{
  if(m_verticalHeaders.size() == 1)
  {
    m_category = tr("Distances to ") + m_verticalHeaders.first()->data().toString();
  }

  TabularReport::Entry::extractInformation();
}
