/*
    Copyright (c) 2013, Jorge Peña Pastor <jpena@cesvima.upm.es>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the <organization> nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "TabularReportEntry.h"

#include <Support/Utils/xlsUtils.h>
#include <GUI/Widgets/InformationSelector.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <Core/Utils/ListUtils.hxx>

#include <QStandardItemModel>
#include <QMessageBox>
#include <QItemDelegate>
#include <qvarlengtharray.h>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Model::Utils;
using namespace xlslib_core;

const QString SEGMENTATION_GROUP = "Segmentation";

class InformationDelegate
: public QItemDelegate
{
  virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
  {
    if (index.column() == 0)
    {
      int progress = index.data(Qt::UserRole).toInt();

      if (progress >= 0)
      {
        // Set up a QStyleOptionProgressBar to precisely mimic the
        // environment of a progress bar.
        QStyleOptionProgressBar progressBarOption;
        progressBarOption.state         = QStyle::State_Enabled;
        progressBarOption.direction     = QApplication::layoutDirection();
        progressBarOption.rect          = option.rect;
        progressBarOption.fontMetrics   = QApplication::fontMetrics();
        progressBarOption.minimum       = 0;
        progressBarOption.maximum       = 100;
        progressBarOption.textAlignment = Qt::AlignCenter;
        progressBarOption.textVisible   = true;
        progressBarOption.progress      = progress;
        progressBarOption.text          = QString("%1%").arg(progressBarOption.progress);

        // Draw the progress bar onto the view.
        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);

        return;
      }
    }

    QItemDelegate::paint(painter, option, index);
  }
};

//------------------------------------------------------------------------
TabularReport::Entry::Entry(const QString   &category,
                            ModelAdapterSPtr model,
                            ModelFactorySPtr factory,
                            QWidget         *parent)
: QWidget(parent)
, m_category(category)
, m_model(model)
, m_factory(factory)
, m_proxy(nullptr)
{
  setupUi(this);

  tableView->setItemDelegate(new InformationDelegate());
  tableView->horizontalHeader()->setMovable(true);
  tableView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  tableView->adjustSize();
  tableView->sortByColumn(0, Qt::AscendingOrder);
  tableView->horizontalHeader()->setSortIndicatorShown(true);

  connect(tableView->horizontalHeader(),SIGNAL(sectionMoved(int,int,int)),
          this, SLOT(saveSelectedInformation()));

  QIcon iconSave = qApp->style()->standardIcon(QStyle::SP_DialogSaveButton);
  exportInformation->setIcon(iconSave);

  connect(refreshInformation, SIGNAL(clicked(bool)),
          this,               SLOT(refreshAllInformation()));

  connect(exportInformation,    SIGNAL(clicked(bool)),
          this,               SLOT(extractInformation()));

  connect(selectInformation,  SIGNAL(clicked(bool)),
          this,               SLOT(changeDisplayedInformation()));

  tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
}

//------------------------------------------------------------------------
TabularReport::Entry::~Entry()
{
  if (m_proxy) delete m_proxy;
}

//------------------------------------------------------------------------
void TabularReport::Entry::setProxy(InformationProxy* proxy)
{
  if (m_proxy)
  {
    disconnect(m_proxy, SIGNAL(informationProgress()),
               this,    SLOT(refreshGUI()));
  }

  m_proxy = proxy;

  if (m_proxy)
  {
    connect(m_proxy, SIGNAL(informationProgress()),
            this,    SLOT(refreshGUI()));
  }

  setInformation(lastDisplayedInformation(), lastInformationOrder());

  refreshGUI();
}

//------------------------------------------------------------------------
int TabularReport::Entry::rowCount() const
{
  return tableView->model()->rowCount(tableView->rootIndex()) + 1;
}

//------------------------------------------------------------------------
int TabularReport::Entry::columnCount() const
{
  return m_proxy->availableInformation().size();
}

//------------------------------------------------------------------------
QVariant TabularReport::Entry::value(int row, int column) const
{
  QVariant result;

  if (row < rowCount() && column < columnCount())
  {
    int logicalIdx = tableView->horizontalHeader()->logicalIndex(column);

    if (row == 0)
    {
      result = m_proxy->availableInformation()[logicalIdx].value();
    }
    else
    {
      result = tableView->model()->index(row-1, logicalIdx, tableView->rootIndex()).data();
    }
  }

  return result;
}


//------------------------------------------------------------------------
void TabularReport::Entry::paintEvent(QPaintEvent* event)
{
  refreshGUIImplementation();
  QWidget::paintEvent(event);
}

//------------------------------------------------------------------------
void TabularReport::Entry::changeDisplayedInformation()
{
  auto available = availableInformation();

  auto selection = lastDisplayedInformation();

  InformationSelector tagSelector(available, selection, tr("Select Information"), false, this->parentWidget());

  if (tagSelector.exec() == QDialog::Accepted)
  {
    setInformation(selection, updateInformationOrder(selection));
  }
  saveSelectedInformation();
}

//------------------------------------------------------------------------
void TabularReport::Entry::saveSelectedInformation()
{
  QStringList informationOrder;

  for (int i = 0; i < tableView->horizontalHeader()->count(); ++i)
  {
    int logicalIdx = tableView->horizontalHeader()->logicalIndex(i);
    informationOrder << m_proxy->headerData(logicalIdx, Qt::Horizontal, Qt::DisplayRole).toString();
  }

  QByteArray selectedInformation = informationOrder.join("\n").toUtf8();
  m_model->storage()->saveSnapshot(SnapshotData(selectedInformationFile(), selectedInformation));
}

//------------------------------------------------------------------------
void TabularReport::Entry::extractInformation()
{
  auto title      = tr("Export %1 Data").arg(m_category);
  auto suggestion = QString("%1.xls").arg(m_category.replace("/","-"));
  auto formats    = SupportedFormats().addExcelFormat().addCSVFormat();
  auto fileName   = DefaultDialogs::SaveFile(title, formats, QDir::homePath(), ".xls", suggestion, this->parentWidget());

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
      exportToCSV(fileName);
    }
    catch(const EspinaException &e)
    {
      auto message = tr("Couldn't export %1").arg(fileName);
      DefaultDialogs::InformationMessage(message, title, e.details(), this->parentWidget());
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
      auto message = tr("Couldn't export %1").arg(fileName);
      DefaultDialogs::InformationMessage(message, title, e.details(), this->parentWidget());
    }
  }
}

//------------------------------------------------------------------------
void TabularReport::Entry::refreshAllInformation()
{
  int c = m_proxy->columnCount() - 1;

  if (m_proxy->availableInformation()[c].value() == tr("Category"))
  {
    --c; // Category tag doesn't spawn task
  }

  for (int r = 1; r <= m_proxy->rowCount(); ++r)
  {
    auto data = value(r, c);
  }
}

//------------------------------------------------------------------------
void TabularReport::Entry::refreshGUI()
{
  refreshGUIImplementation();
  tableView->viewport()->update();
}

//------------------------------------------------------------------------
void TabularReport::Entry::refreshGUIImplementation()
{
  int  progress   = m_proxy->progress();
  bool inProgress = (progress < 100);

  int informationSize = m_proxy->availableInformation().size();
  if (informationSize == 1 || (informationSize == 2 && m_proxy->availableInformation()[1].value() == tr("Category")))
  {
    inProgress = false;
  }

  progressLabel->setVisible(inProgress);
  progressBar->setVisible(inProgress);
  progressBar->setValue(progress);

  if (exportInformation->isEnabled() == inProgress)
  {
    exportInformation->setEnabled(!inProgress);

    emit informationReadyChanged();
  }
}

//------------------------------------------------------------------------
void TabularReport::Entry::exportToCSV(const QString &filename)
{
  QFile file(filename);

  if(!file.open(QIODevice::WriteOnly|QIODevice::Text) || !file.isWritable() || !file.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ReadOther|QFile::WriteOther))
  {
    auto what    = tr("exportToCSV: can't save file '%1'.").arg(filename);
    auto details = tr("Cause of failure: %1").arg(file.errorString());

    throw EspinaException(what, details);
  }

  QTextStream out(&file);

  for (int r = 0; r < rowCount(); r++)
  {
    for (int c = 0; c < columnCount(); c++)
    {
      if (c)
      {
        out << ",";
      }
      out << value(r, c).toString();
    }
    out << "\n";
  }
  file.close();
}

//------------------------------------------------------------------------
void TabularReport::Entry::exportToXLS(const QString &filename)
{
  workbook wb;

  worksheet *ws = wb.sheet(m_category.replace("/",">").toStdString());

  for (int r = 0; r < rowCount(); ++r)
  {
    for (int c = 0; c < columnCount(); ++c)
    {
      createCell(ws, r, c, value(r,c));
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
InformationSelector::GroupedInfo TabularReport::Entry::availableInformation()
{
  auto segmentations = toList<SegmentationAdapter>(m_proxy->displayedItems());
  auto availableInfo = GUI::availableInformation(segmentations, m_factory);

  availableInfo[SEGMENTATION_GROUP] << tr("Category");

  return availableInfo;
}

//------------------------------------------------------------------------
SegmentationExtension::InformationKeyList TabularReport::Entry::lastInformationOrder()
{
  SegmentationExtension::InformationKeyList informationTags, availableInformationTags;

  auto groupedInfo = availableInformation();

  for (auto extension : groupedInfo.keys())
  {
    for (auto value : groupedInfo[extension])
    {
      availableInformationTags <<  SegmentationExtension::InformationKey(extension, value);
    }
  }

  if(m_model->storage()->exists(selectedInformationFile()))
  {
    QString selectedInformation(m_model->storage()->snapshot(selectedInformationFile()));

    for (auto tag : selectedInformation.split("\n", QString::SkipEmptyParts))
    {
      for(auto key: availableInformationTags)
      {
        if(key.value() == tag)
        {
          informationTags << key;
        }
      }
    }
  }

  return informationTags;
}

//------------------------------------------------------------------------
InformationSelector::GroupedInfo TabularReport::Entry::lastDisplayedInformation()
{
  InformationSelector::GroupedInfo info, available;

  available = availableInformation();

  if(m_model->storage()->exists(selectedInformationFile()))
  {
    QString selectedInformation(m_model->storage()->snapshot(selectedInformationFile()));
    for (auto tag : selectedInformation.split("\n", QString::SkipEmptyParts))
    {
      for (auto extension : available.keys())
      {
        if (available[extension].contains(tag))
        {
          info[extension] << tag;
        }
      }
    }
  }

  if (info.isEmpty())
  {
    info[SEGMENTATION_GROUP]  << tr("Category");
  }

  return info;
}

//------------------------------------------------------------------------
void TabularReport::Entry::setInformation(InformationSelector::GroupedInfo extensionInformations, SegmentationExtension::InformationKeyList informationOrder)
{
  for(auto extensionType : extensionInformations.keys())
  {
    if (extensionType != SEGMENTATION_GROUP)
    {
      for (auto segmentation : m_model->segmentations())
      {
        addSegmentationExtension(segmentation, extensionType, m_factory);
      }
    }
  }

  SegmentationExtension::InformationKeyList keys;
  keys << InformationProxy::NameKey() << informationOrder;
  m_proxy->setInformationTags(keys);

  QStringList headerLabels;
  for (auto key : keys)
  {
    headerLabels << key.value();
  }

  auto header = new QStandardItemModel(1, keys.size(), this);
  header->setHorizontalHeaderLabels(headerLabels);
  tableView->horizontalHeader()->setModel(header);

  tableView->updateGeometry();
}

//------------------------------------------------------------------------
SegmentationExtension::InformationKeyList TabularReport::Entry::updateInformationOrder(InformationSelector::GroupedInfo extensionInformation)
{
  return information(extensionInformation);
}

//------------------------------------------------------------------------
SegmentationExtension::InformationKeyList TabularReport::Entry::information(InformationSelector::GroupedInfo extensionInformations)
{
  SegmentationExtension::InformationKeyList informations;

  for (auto extension : extensionInformations.keys())
  {
    for (auto value : extensionInformations[extension])
    {
      SegmentationExtension::InformationKey key(extension, value);
      if (!informations.contains(key))
      {
        informations << key;
      }
    }
  }

  return informations;
}
