/*
 *
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include "TabularReport.h"
#include "TabularReportEntry.h"
#include <GUI/Widgets/CheckableTableView.h>
#include <GUI/Model/Proxies/InformationProxy.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <Support/Utils/xlsUtils.h>

// Qt
#include <QFileDialog>
#include <QLabel>
#include <QScrollArea>
#include <QStandardItemModel>
#include <QKeyEvent>
#include <QMessageBox>
#include <QTableView>

using namespace ESPINA;
using namespace ESPINA::Support;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::Core::Utils;
using namespace xlslib_core;

//------------------------------------------------------------------------
TabularReport::TabularReport(Support::Context &context,
                             QWidget          *parent,
                             Qt::WindowFlags   flags)
: QAbstractItemView(parent)
, m_context       (context)
, m_model         {nullptr}
, m_tabs          {new QTabWidget()}
, m_multiSelection{false}
{
  auto general = new QHBoxLayout();
  general->setAlignment(Qt::AlignRight);

  m_exportButton = new QPushButton();
  QIcon saveIcon = qApp->style()->standardIcon(QStyle::SP_DialogSaveButton);
  m_exportButton->setIcon(saveIcon);
  m_exportButton->setFlat(false);
  m_exportButton->setEnabled(false);
  m_exportButton->setToolTip("Save All Data");
  m_exportButton->setFlat(true);
  m_exportButton->setIconSize(QSize(22,22));
  m_exportButton->setMinimumSize(32,32);
  m_exportButton->setMaximumSize(32,32);
  connect(m_exportButton, SIGNAL(clicked(bool)),
          this, SLOT(exportInformation()));

  general->addWidget(m_exportButton);

  auto layout = new QVBoxLayout();
  auto pal    = this->palette();

  pal.setColor(QPalette::Base, pal.color(QPalette::Background));
  this->setPalette(pal);

  layout->addWidget(m_tabs);
  layout->addLayout(general);

  setLayout(layout);

  connect(getSelection(context).get(), SIGNAL(selectionStateChanged(SegmentationAdapterList)),
          this,                        SLOT(updateSelection(SegmentationAdapterList)));
}

//------------------------------------------------------------------------
TabularReport::~TabularReport()
{
  QStringList currentEntries;

  for (int i = 0; i < m_tabs->count(); ++i)
  {
    currentEntries << m_tabs->tabText(i).replace("/",">");
  }

  // Remove old extras
  if (m_model)
  {
    auto storage = m_model->storage();
    for (auto data : storage->snapshots(extraPath(), TemporalStorage::Mode::NORECURSIVE))
    {
      QFileInfo file(data.first);

      if (!currentEntries.contains(file.baseName()))
      {
        QDir dir;
        dir.remove(file.absoluteFilePath());
      }
    }
  }

  removeTabsAndWidgets();
  delete m_tabs;
}

//------------------------------------------------------------------------
void TabularReport::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
  QAbstractItemView::dataChanged(topLeft, bottomRight);
  if (topLeft.isValid())
  {
    auto item = itemAdapter(topLeft);
    if (isSegmentation(item))
    {
      auto segmentation = segmentationPtr(item);

      if (acceptSegmentation(segmentation))
      {
        createCategoryEntry(segmentation->category()->classificationName());
      }
    }
  }
}

//------------------------------------------------------------------------
void TabularReport::rowsInserted(const QModelIndex &parent, int start, int end)
{
  if (parent == m_model->segmentationRoot())
  {
    for (int row = start; row <= end; ++row)
    {
      auto item = itemAdapter(parent.child(row, 0));

      auto segmentation = segmentationPtr(item);
      if (acceptSegmentation(segmentation))
      {
        createCategoryEntry(segmentation->category()->classificationName());
      }
    }
  }
}

//------------------------------------------------------------------------
void TabularReport::setModel(ModelAdapterSPtr model)
{
  m_model = model;

  QAbstractItemView::setModel(model.get());
}

//------------------------------------------------------------------------
void TabularReport::setFilter(SegmentationAdapterList segmentations)
{
  m_filter = segmentations;

  reset();
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
  removeTabsAndWidgets();

  QAbstractItemView::reset();

  if (m_model)
  {
    rowsInserted(m_model->segmentationRoot(), 0, model()->rowCount(m_model->segmentationRoot())-1);

    if (!m_filter.isEmpty())
    {
      QString tabText = m_filter.last()->category()->classificationName();
      tabText.detach();

      for (int i = 0; i < m_tabs->count(); ++i)
      {
        if (m_tabs->tabText(i) == tabText)
        {
          m_tabs->setCurrentIndex(i);
          break;
        }
      }
    }
  }
}

//------------------------------------------------------------------------
void TabularReport::indexDoubleClicked(QModelIndex index)
{
  QModelIndex sourceIndex = mapToSource(index);

  auto sItem        = itemAdapter(sourceIndex);

  if(sItem)
  {
    auto segmentation = segmentationPtr(sItem);
    auto bounds       = segmentation->output()->bounds();

    m_context.viewState().focusViewOn(centroid(bounds));

    emit doubleClicked(sourceIndex);
  }
}

//------------------------------------------------------------------------
void TabularReport::updateRepresentation(const QModelIndex &index)
{
  auto sItem = itemAdapter(mapToSource(index));

  if(sItem)
  {
    auto segmentation = segmentationPtr(sItem);

    segmentation->invalidateRepresentations();
  }
}

//------------------------------------------------------------------------
void TabularReport::updateSelection(SegmentationAdapterList selection)
{
  if (!isVisible() || signalsBlocked())
    return;

  blockSignals(true);
  for (int i = 0; i < m_tabs->count(); ++i)
  {
    auto entry     = dynamic_cast<Entry *>(m_tabs->widget(i));
    auto tableView = entry->tableView;

    tableView->selectionModel()->clear();
  }

  for(auto segmentation : selection)
  {
    Entry *entry = nullptr;
    int i = 0;
    for (i = 0; i < m_tabs->count(); ++i)
    {
      if (m_tabs->tabText(i) == segmentation->category()->classificationName())
      {
        entry = dynamic_cast<Entry *>(m_tabs->widget(i));
        break;
      }
    }

    if (entry)
    {
      QTableView *tableView = entry->tableView;
      QAbstractItemModel *model = tableView->model();
      QModelIndex rootIndex = tableView->rootIndex();

      for (int row = 0; row < model->rowCount(rootIndex); ++row)
      {
        QModelIndex index = model->index(row, 0, rootIndex);
        if (index.data().toString() == segmentation->data().toString())
        {
          tableView->selectRow(row);
          if (segmentation == selection.first())
          {
            tableView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::Select);
            tableView->scrollTo(index);
            m_tabs->setCurrentIndex(i);
          }
          break;
        }
      }
    }
  }

  for (int i = 0; i < m_tabs->count(); ++i)
  {
    auto entry     = dynamic_cast<Entry *>(m_tabs->widget(i));
    auto tableView = entry->tableView;

    // Update all visible items
    tableView->viewport()->update();
  }
  blockSignals(false);
}

//------------------------------------------------------------------------
void TabularReport::updateSelection(QItemSelection selected, QItemSelection deselected)
{
  if (signalsBlocked())
  {
    return;
  }

  ViewItemAdapterList selectedItems;

  if (m_multiSelection)
  {
    for (int i = 0; i < m_tabs->count(); ++i)
    {
      auto entry = dynamic_cast<Entry *>(m_tabs->widget(i));
      auto tableView = entry->tableView;
      auto sortFilter = dynamic_cast<QSortFilterProxyModel *>(tableView->model());

      for(auto index : tableView->selectionModel()->selectedRows())
      {
        auto sItem = itemAdapter(sortFilter->mapToSource(index));

        if (isSegmentation(sItem))
        {
          selectedItems << segmentationPtr(sItem);
        }
      }
    }
  } else
  {
    auto selectionModel = dynamic_cast<QItemSelectionModel *>(sender());

    QTableView *tableView = nullptr;
    for (int i = 0; i < m_tabs->count(); ++i)
    {
      auto entry = dynamic_cast<Entry *>(m_tabs->widget(i));
      if (entry && (entry->tableView->selectionModel() == selectionModel))
      {
        tableView = entry->tableView;
        break;
      }
    }

    if(tableView && selectionModel)
    {
      auto sortFilter = dynamic_cast<DataSortFilter *>(tableView->model());
      if(sortFilter)
      {
        for(auto index : selectionModel->selectedRows())
        {
          auto sItem = itemAdapter(sortFilter->mapToSource(index));
          if (sItem && isSegmentation(sItem))
          {
            selectedItems << segmentationPtr(sItem);
          }
        }
      }
    }
  }

  blockSignals(true);
  getSelection(m_context)->set(selectedItems);
  blockSignals(false);
}

//------------------------------------------------------------------------
void TabularReport::rowsRemoved(const QModelIndex &parent, int start, int end)
{
  InformationProxy *proxy = dynamic_cast<InformationProxy *>(sender());

  for (int i = 0; i < m_tabs->count(); ++i)
  {
    if (m_tabs->tabText(i) == proxy->category() && proxy->rowCount() == 0)
    {
      QWidget *tab = m_tabs->widget(i);
      m_tabs->removeTab(i);
      delete tab;
      break;
    }
  }
}

//------------------------------------------------------------------------
void TabularReport::exportInformation()
{
  auto title      = tr("Export Raw Data");
  auto suggestion = tr("raw information.xls");
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

//------------------------------------------------------------------------
void TabularReport::updateExportStatus()
{
  bool enabled = true;

  for (int i = 0; i < m_tabs->count(); ++i)
  {
    Entry *entry = dynamic_cast<Entry *>(m_tabs->widget(i));

    enabled &= entry->exportInformation->isEnabled();
  }

  m_exportButton->setEnabled(enabled);
}

//------------------------------------------------------------------------
bool TabularReport::acceptSegmentation(const SegmentationAdapterPtr segmentation)
{
  return m_filter.isEmpty() || m_filter.contains(segmentation);
}

//------------------------------------------------------------------------
void TabularReport::createCategoryEntry(const QString &category)
{
  bool found = false;
  int  i = 0;
  while (!found && i < m_tabs->count())
  {
    if (m_tabs->tabText(i) >= category)
    {
      found = true;
    }
    else
    {
      i++;
    }
  }

  if (m_tabs->tabText(i) != category)
  {
    auto entry = new Entry(category, m_model, factory(), m_tabs);

    connect(entry, SIGNAL(informationReadyChanged()),
            this,  SLOT(updateExportStatus()));

    InformationProxy *infoProxy = new InformationProxy(factory()->scheduler());
    infoProxy->setCategory(category);
    infoProxy->setFilter(&m_filter);
    infoProxy->setSourceModel(m_model);
    connect (infoProxy, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
             this,      SLOT(rowsRemoved(const QModelIndex &, int, int)));
    entry->setProxy(infoProxy);

    DataSortFilter *sortFilter = new DataSortFilter();
    sortFilter->setSourceModel(infoProxy);
    sortFilter->setDynamicSortFilter(true);

    CheckableTableView *tableView = entry->tableView;
    tableView->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    tableView->setModel(sortFilter);
    tableView->setSortingEnabled(true);

    connect(tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(updateSelection(QItemSelection,QItemSelection)));
    connect(tableView, SIGNAL(itemStateChanged(QModelIndex)),
            this, SLOT(updateRepresentation(QModelIndex)));
    connect(tableView, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(indexDoubleClicked(QModelIndex)));

    m_tabs->insertTab(i, entry, category);
  }

  updateExportStatus();
}

//------------------------------------------------------------------------
void TabularReport::exportToCSV(const QFileInfo &filename)
{
  for (int i = 0; i < m_tabs->count(); ++i)
  {
    auto entry   = dynamic_cast<Entry *>(m_tabs->widget(i));

    auto csvFile = filename.dir().absoluteFilePath(filename.baseName() + "-" + m_tabs->tabText(i).replace("/","-") + ".csv");

    QFile file(csvFile);

    if(!file.open(QIODevice::WriteOnly|QIODevice::Text) || !file.isWritable() || !file.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ReadOther|QFile::WriteOther))
    {
      auto what    = tr("exportToCSV: can't save file '%1'.").arg(filename.absoluteFilePath());
      auto details = tr("Cause of failure: %1").arg(file.errorString());

      throw EspinaException(what, details);
    }

    QTextStream out(&file);

    for (int r = 0; r < entry->rowCount(); r++)
    {
      for (int c = 0; c < entry->columnCount(); c++)
      {
        if (c)
        {
          out << ",";
        }
        out << entry->value(r, c).toString();
      }
      out << "\n";
    }
    file.close();
  }
}

//------------------------------------------------------------------------
void TabularReport::exportToXLS(const QString &filename)
{
  workbook wb;

  for (int i = 0; i < m_tabs->count(); ++i)
  {
    auto entry = dynamic_cast<Entry *>(m_tabs->widget(i));
    worksheet *sheet = wb.sheet(m_tabs->tabText(i).replace("/",">").toStdString());

    for (int r = 0; r < entry->rowCount(); ++r)
    {
      for (int c = 0; c < entry->columnCount(); ++c)
      {
        createCell(sheet, r, c, entry->value(r,c));
      }
    }
  }

  auto result = wb.Dump(filename.toStdString());

  if(result != NO_ERRORS)
  {
    auto what    = tr("exportToXLS: can't save file '%1'.").arg(filename);
    auto details = tr("Cause of failure: %1").arg(result == FILE_ERROR ? "file error" : "general error");

    throw EspinaException(what, details);
  }
}

//------------------------------------------------------------------------
QModelIndex TabularReport::mapToSource(const QModelIndex &index)
{
  auto sortFilter = dynamic_cast<const QSortFilterProxyModel *>(index.model());

  return sortFilter->mapToSource(index);
}

//------------------------------------------------------------------------
QModelIndex TabularReport::mapFromSource(const QModelIndex &index, QSortFilterProxyModel *sortFilter)
{
  return sortFilter->mapFromSource(index);
}

//------------------------------------------------------------------------
void TabularReport::removeTabsAndWidgets()
{
  while (m_tabs->count())
  {
    delete m_tabs->widget(0);
  }
}

//------------------------------------------------------------------------
ModelFactorySPtr TabularReport::factory() const
{
  return m_context.factory();
}
