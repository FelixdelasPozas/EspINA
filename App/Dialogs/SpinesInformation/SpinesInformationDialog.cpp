/*

 Copyright (C) 2018 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <App/Dialogs/SpinesInformation/SpinesInformationDialog.h>
#include <App/Dialogs/SkeletonInspector/SkeletonInspector.h>
#include <Core/Factory/CoreFactory.h>
#include <Core/Utils/SupportedFormats.h>
#include <Core/Utils/ListUtils.hxx>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Widgets/ToolButton.h>
#include <GUI/Widgets/Styles.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <Support/Settings/Settings.h>
#include <Support/Utils/xlsUtils.h>

// Qt
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QTabWidget>
#include <QSpacerItem>
#include <QTableWidget>
#include <QApplication>

using namespace ESPINA;
using namespace ESPINA::Extensions;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::GUI::Widgets;
using namespace xlslib_core;

const QString SETTINGS_GROUP = "Spines Information Dialog";

//--------------------------------------------------------------------
SpinesInformationDialog::SpinesInformationDialog(SegmentationAdapterList input, Support::Context& context)
: QDialog{DefaultDialogs::defaultParentWidget(), Qt::WindowFlags{Qt::WindowMinMaxButtonsHint|Qt::WindowCloseButtonHint}}
, Support::WithContext(context)
, m_segmentations{input}
{
  connectSignals();

  createGUI();

  computeInformation();

  ESPINA_SETTINGS(settings);

  settings.beginGroup(SETTINGS_GROUP);
  resize(settings.value("size", QSize (400, 200)).toSize());
  move  (settings.value("pos",  QPoint(200, 200)).toPoint());
  settings.endGroup();
}

//--------------------------------------------------------------------
void SpinesInformationDialog::closeEvent(QCloseEvent* event)
{
  ESPINA_SETTINGS(settings);

  settings.beginGroup(SETTINGS_GROUP);
  settings.setValue("size", size());
  settings.setValue("pos", pos());
  settings.endGroup();

  QDialog::closeEvent(event);
}

//--------------------------------------------------------------------
void SpinesInformationDialog::connectSignals()
{
  auto model = getModel().get();

  // only watched if the dialog is for all segmentations in the model.
  if(m_segmentations.isEmpty())
  {
    connect(model, SIGNAL(segmentationsAdded(ViewItemAdapterSList)),
            this,  SLOT(onSegmentationsAdded(ViewItemAdapterSList)));
    connect(model, SIGNAL(segmentationsAboutToBeRemoved(ViewItemAdapterSList)),
            this,  SLOT(onSegmentationsRemoved(ViewItemAdapterSList)));
  }

  connect(model, SIGNAL(connectionAdded(Connection)),
          this,  SLOT(onConnectionModified(Connection)));
  connect(model, SIGNAL(connectionRemoved(Connection)),
          this,  SLOT(onConnectionModified(Connection)));

  for(auto segmentation: m_segmentations)
  {
    connect(segmentation, SIGNAL(outputModified()),
            this,         SLOT(onOutputModified()));
  }
}

//--------------------------------------------------------------------
void SpinesInformationDialog::onExportButtonClicked()
{
  auto title        = tr("Export spines information");
  auto suggestion   = QString("%spines-information.xls");
  auto formats      = SupportedFormats().addExcelFormat().addCSVFormat();
  auto fileName     = DefaultDialogs::SaveFile(title, formats, QDir::homePath(), ".xls", suggestion, this);

  if (fileName.isEmpty()) return;

  // some users are used to not enter an extension, and expect a default xls output.
  if(!fileName.endsWith(".csv", Qt::CaseInsensitive) && !fileName.endsWith(".xls", Qt::CaseInsensitive))
  {
    fileName += tr(".xls");
  }

  if (fileName.endsWith(".csv", Qt::CaseInsensitive))
  {
    try
    {
      saveToCSV(fileName);
    }
    catch(const EspinaException &e)
    {
      auto message = tr("Couldn't export %1").arg(fileName);
      DefaultDialogs::InformationMessage(message, title, e.details(), this);
    }
  }
  else if (fileName.endsWith(".xls", Qt::CaseInsensitive))
  {
    try
    {
      saveToXLS(fileName);
    }
    catch(const EspinaException &e)
    {
      auto message = tr("Couldn't export %1").arg(fileName);
      DefaultDialogs::InformationMessage(message, title, e.details(), this);
    }
  }
}

//--------------------------------------------------------------------
void SpinesInformationDialog::onOutputModified()
{
  refreshTable();
}

//--------------------------------------------------------------------
void SpinesInformationDialog::onSegmentationsAdded(ViewItemAdapterSList items)
{
  bool needsRefresh = false;

  for(auto item: items)
  {
    auto segmentation = segmentationPtr(item.get());
    if(segmentation && segmentation->category()->classificationName().startsWith("Dendrite", Qt::CaseInsensitive))
    {
      m_segmentations << segmentation;

      auto extension = retrieveOrCreateSegmentationExtension<DendriteSkeletonInformation>(segmentation, getFactory());
      auto spines    = extension->spinesInformation();
      m_spinesMap.insert(segmentation, spines);

      needsRefresh = true;
    }
  }

  if(needsRefresh) refreshTable();
}

//--------------------------------------------------------------------
void SpinesInformationDialog::onSegmentationsRemoved(ViewItemAdapterSList items)
{
  bool needsRefresh = false;
  for(auto item: items)
  {
    auto segmentation = segmentationPtr(item.get());
    if(segmentation && m_segmentations.contains(segmentation))
    {
      m_segmentations.removeAll(segmentation);
      m_spinesMap.remove(segmentation);

      needsRefresh = true;
    }
  }

  if(needsRefresh) refreshTable();
}

//--------------------------------------------------------------------
void ESPINA::SpinesInformationDialog::createGUI()
{
  setWindowTitle(tr("Spines Information Report"));
  setWindowIconText(":/espina/espina.svg");
  auto dialogLayout = new QVBoxLayout();

  auto general = new QHBoxLayout();
  general->setAlignment(Qt::AlignRight);

  auto exportButton = new QPushButton();
  QIcon saveIcon = qApp->style()->standardIcon(QStyle::SP_DialogSaveButton);
  exportButton->setIcon(saveIcon);
  exportButton->setToolTip("Save All Data");
  exportButton->setFlat(true);
  exportButton->setIconSize(QSize(22,22));
  exportButton->setMinimumSize(32,32);
  exportButton->setMaximumSize(32,32);

  connect(exportButton, SIGNAL(clicked(bool)),
          this, SLOT(onExportButtonClicked()));

  general->addWidget(exportButton);

  auto frame = new QFrame();
  frame->setFrameShape(QFrame::Shape::StyledPanel);
  auto tabLayout = new QVBoxLayout();

  auto tab = new QTabWidget();

  auto headers = QString("Name;Parent dendrite;Complete;Branched;Length (Nm);Num of synapses;Num asymmetric;Num asymmetric on head;Num asymmetric on neck;");
  headers     += QString("Num symmetric;Num symmetric on head;Num symmetric on neck;Num of contacted axons;Num of inhibitory axons;Num of excitatory axons");

  m_table  = new QTableWidget();
  m_table->verticalHeader()->hide();
  m_table->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
  m_table->setColumnCount(14);
  m_table->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
  m_table->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
  m_table->setHorizontalHeaderLabels(headers.split(";"));
  m_table->horizontalHeader()->setStretchLastSection(true);

  connect(m_table, SIGNAL(cellDoubleClicked(int, int)),
          this,    SLOT(focusOnActor(int)));

  tab->insertTab(0, m_table, tr("Spines information"));

  tabLayout->addWidget(tab);
  tabLayout->addLayout(general);
  frame->setLayout(tabLayout);

  dialogLayout->addWidget(frame);

  auto acceptButton = new QDialogButtonBox(QDialogButtonBox::Ok);

  connect(acceptButton, SIGNAL(accepted()),
          this,         SLOT(close()));

  dialogLayout->addWidget(acceptButton);
  setLayout(dialogLayout);

  window()->resize(layout()->sizeHint());
  window()->adjustSize();
}

//--------------------------------------------------------------------
void ESPINA::SpinesInformationDialog::saveToCSV(const QString& filename)
{
  QFile file(filename);

  if(!file.open(QIODevice::WriteOnly|QIODevice::Text) || !file.isWritable() || !file.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ReadOther|QFile::WriteOther))
  {
    auto what    = tr("exportToCSV: can't save file '%1'.").arg(filename);
    auto details = tr("Cause of failure: %1").arg(file.errorString());

    throw EspinaException(what, details);
  }

  QTextStream out(&file);

  for (int c = 0; c < m_table->columnCount(); c++)
  {
    if(c) out << ",";
    out << m_table->horizontalHeaderItem(c)->data(Qt::DisplayRole).toString();
  }

  for (int r = 0; r < m_table->rowCount(); r++)
  {
    for (int c = 0; c < m_table->columnCount(); c++)
    {
      if(c) out << ",";
      out << m_table->item(r, c)->data(Qt::DisplayRole).toString().remove('\'');
    }
  }
  file.close();
}

//--------------------------------------------------------------------
void SpinesInformationDialog::onConnectionModified(Connection connection)
{
  if(m_segmentations.contains(connection.item1.get()) ||
     m_segmentations.contains(connection.item2.get()))
  {
    refreshTable();
  }
}

//--------------------------------------------------------------------
void SpinesInformationDialog::focusOnActor(int row)
{
  auto segmentationName = m_table->item(row, 1)->data(Qt::DisplayRole).toString();

  for(auto segmentation: m_segmentations)
  {
    if(segmentation->data(Qt::DisplayRole).toString() == segmentationName)
    {
      getSelection()->set(SegmentationAdapterList{segmentation});
      getViewState().focusViewOn(centroid(segmentation->bounds()));
    }
  }
}

//--------------------------------------------------------------------
void SpinesInformationDialog::saveToXLS(const QString& filename)
{
  workbook wb;

  auto excelSheet = wb.sheet(std::string{"Spines information"});

  for (int c = 0; c < m_table->columnCount(); ++c)
  {
    createCell(excelSheet, 0, c, m_table->horizontalHeaderItem(c)->data(Qt::DisplayRole).toString());
  }

  for (int r = 0; r < m_table->rowCount(); ++r)
  {
    for (int c = 0; c < m_table->columnCount(); ++c)
    {
      createCell(excelSheet, r, c, m_table->item(r, c)->data(Qt::DisplayRole).toString().remove('\''));
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

//--------------------------------------------------------------------
void SpinesInformationDialog::computeInformation()
{
  if(m_segmentations.isEmpty())
  {
    for(auto segmentation: getModel()->segmentations())
    {
      if(segmentation->category()->classificationName().startsWith("Dendrite", Qt::CaseInsensitive))
      {
        m_segmentations << segmentation.get();
      }
    }
  }

  qSort(m_segmentations.begin(), m_segmentations.end(), lessThan<SegmentationAdapterPtr>);

  refreshTable();

  m_table->resizeColumnsToContents();
  m_table->setAlternatingRowColors(true);
  m_table->setSortingEnabled(true);
  m_table->sortByColumn(1, Qt::SortOrder::AscendingOrder);
}

//--------------------------------------------------------------------
void SpinesInformationDialog::refreshTable()
{
  m_table->setRowCount(0);
  m_table->setUpdatesEnabled(false);

  int row = 0;
  Item *item = nullptr;
  for(auto segmentation: m_segmentations)
  {
    auto extension = retrieveOrCreateSegmentationExtension<DendriteSkeletonInformation>(segmentation, getFactory());
    auto spines    = extension->spinesInformation();
    m_spinesMap.insert(segmentation, spines);

    m_table->setRowCount(row + spines.size());
    for(auto spine: spines)
    {
      item = new Item(spine.name);
      item->setTextAlignment(Qt::AlignLeft);
      m_table->setItem(row, 0, item);
      item = new Item(spine.parentName);
      item->setTextAlignment(Qt::AlignLeft);
      m_table->setItem(row, 1, item);
      item = new Item(spine.complete ? "yes":"no");
      item->setTextAlignment(Qt::AlignCenter);
      m_table->setItem(row, 2, item);
      item = new Item(spine.branched ? "yes":"no");
      item->setTextAlignment(Qt::AlignCenter);
      m_table->setItem(row, 3, item);
      item = new Item(QString::number(spine.length));
      item->setTextAlignment(Qt::AlignCenter);
      m_table->setItem(row, 4, item);
      item = new Item(QString::number(spine.numSynapses));
      item->setTextAlignment(Qt::AlignCenter);
      m_table->setItem(row, 5, item);
      item = new Item(QString::number(spine.numAsymmetric));
      item->setTextAlignment(Qt::AlignCenter);
      m_table->setItem(row, 6, item);
      item = new Item(QString::number(spine.numAsymmetricHead));
      item->setTextAlignment(Qt::AlignCenter);
      m_table->setItem(row, 7, item);
      item = new Item(QString::number(spine.numAsymmetricNeck));
      item->setTextAlignment(Qt::AlignCenter);
      m_table->setItem(row, 8, item);
      item = new Item(QString::number(spine.numSymmetric));
      item->setTextAlignment(Qt::AlignCenter);
      m_table->setItem(row, 9, item);
      item = new Item(QString::number(spine.numSymmetricHead));
      item->setTextAlignment(Qt::AlignCenter);
      m_table->setItem(row, 10, item);
      item = new Item(QString::number(spine.numSymmetricNeck));
      item->setTextAlignment(Qt::AlignCenter);
      m_table->setItem(row, 11, item);
      item = new Item(QString::number(spine.numAxons));
      item->setTextAlignment(Qt::AlignCenter);
      m_table->setItem(row, 12, item);
      item = new Item(QString::number(spine.numAxonsInhibitory));
      item->setTextAlignment(Qt::AlignCenter);
      m_table->setItem(row, 13, item);
      item = new Item(QString::number(spine.numAxonsExcitatory));
      item->setTextAlignment(Qt::AlignCenter);
      m_table->setItem(row, 14, item);

      ++row;
    }
  }

  m_table->setUpdatesEnabled(true);
}