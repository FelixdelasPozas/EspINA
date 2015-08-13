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
using namespace xlslib_core;

//------------------------------------------------------------------------
class DataSortFiler
: public QSortFilterProxyModel
{
public:
  DataSortFiler(QObject *parent = 0)
  : QSortFilterProxyModel(parent) {}

protected:
  virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const
  {
    int role = Qt::DisplayRole;
    auto ldata = left.data(role);
    auto rdata = right.data(role);

    if(left.column() != 0)
    {
      bool ok1, ok2;

      double lv = ldata.toDouble(&ok1);
      double rv = rdata.toDouble(&ok2);

      if (ok1 && ok2)
      {
        return lv < rv;
      }
    }

    // default for strings and data non convertible to numerical values.
    auto lstring = ldata.toString();
    auto rstring = rdata.toString();

    if(lstring.length() != rstring.length())
    {
      return lstring.length() < rstring.length();
    }

    return lstring < rstring;
  }
};


//------------------------------------------------------------------------
TabularReport::TabularReport(Support::Context &context,
                             QWidget          *parent,
                             Qt::WindowFlags   flags)
: m_context       (context)
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
  auto segmentation = segmentationPtr(sItem);
  auto bounds       = segmentation->output()->bounds();

  m_context.viewState().focusViewOn(centroid(bounds));

  emit doubleClicked(sourceIndex);
}

//------------------------------------------------------------------------
void TabularReport::updateRepresentation(const QModelIndex &index)
{
  auto sItem = itemAdapter(mapToSource(index));

  auto segmentation = segmentationPtr(sItem);

  segmentation->invalidateRepresentations();
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
      Entry *entry = dynamic_cast<Entry *>(m_tabs->widget(i));
      QTableView *tableView = entry->tableView;

      QSortFilterProxyModel *sortFilter = dynamic_cast<QSortFilterProxyModel *>(tableView->model());
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
    QItemSelectionModel *selectionModel = dynamic_cast<QItemSelectionModel *>(sender());

    QTableView *tableView = nullptr;
    for (int i = 0; i < m_tabs->count(); ++i)
    {
      Entry *entry = dynamic_cast<Entry *>(m_tabs->widget(i));
      if (entry->tableView->selectionModel() == selectionModel)
      {
        tableView = entry->tableView;
        break;
      }
    }

    DataSortFiler *sortFilter = dynamic_cast<DataSortFiler *>(tableView->model());
    for(auto index : tableView->selectionModel()->selectedRows())
    {
      auto sItem = itemAdapter(sortFilter->mapToSource(index));

      if (isSegmentation(sItem))
      {
        selectedItems << segmentationPtr(sItem);
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
  auto fileName   = DefaultDialogs::SaveFile(title, formats, "", ".xls", suggestion);

  if (fileName.isEmpty())
    return;

  bool result = false;
  if (fileName.toLower().endsWith(".csv"))
  {
    result = exportToCSV(fileName);
  }
  else if (fileName.toLower().endsWith(".xls"))
  {
    result = exportToXLS(fileName);
  }

  if (!result)
  {
    DefaultDialogs::InformationMessage(title, tr("Unable to export %1").arg(fileName));
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
    auto entry = new Entry(category, m_model, factory());

    connect(entry, SIGNAL(informationReadyChanged()),
            this,  SLOT(updateExportStatus()));

    InformationProxy *infoProxy = new InformationProxy(factory()->scheduler());
    infoProxy->setCategory(category);
    infoProxy->setFilter(&m_filter);
    infoProxy->setSourceModel(m_model);
    connect (infoProxy, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
             this,      SLOT(rowsRemoved(const QModelIndex &, int, int)));
    entry->setProxy(infoProxy);

    DataSortFiler *sortFilter = new DataSortFiler();
    sortFilter->setSourceModel(infoProxy);
    sortFilter->setDynamicSortFilter(true);

    CheckableTableView *tableView = entry->tableView;
    tableView->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    tableView->setModel(sortFilter);
    tableView->setSortingEnabled(true);
    //
    //       tableView->horizontalHeader()->setModel(header);
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
bool TabularReport::exportToCSV(const QFileInfo &filename)
{
  for (int i = 0; i < m_tabs->count(); ++i)
  {
    auto entry   = dynamic_cast<Entry *>(m_tabs->widget(i));

    auto csvFile = filename.dir().absoluteFilePath(filename.baseName() + "-" + m_tabs->tabText(i).replace("/","-") + ".csv");

    QFile file( csvFile);

    file.open(QIODevice::WriteOnly |  QIODevice::Text);

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

  return true;
}

//------------------------------------------------------------------------
bool TabularReport::exportToXLS(const QString &filename)
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

  wb.Dump(filename.toStdString());

  return true;
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
