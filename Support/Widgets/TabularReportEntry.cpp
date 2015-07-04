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
#include <Support/Settings/EspinaSettings.h>
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
        progressBarOption.state = QStyle::State_Enabled;
        progressBarOption.direction = QApplication::layoutDirection();
        progressBarOption.rect = option.rect;
        progressBarOption.fontMetrics = QApplication::fontMetrics();
        progressBarOption.minimum = 0;
        progressBarOption.maximum = 100;
        progressBarOption.textAlignment = Qt::AlignCenter;
        progressBarOption.textVisible = true;

        progressBarOption.progress = progress;
        progressBarOption.text = QString("%1%").arg(progressBarOption.progress);

//         progressBarOption.text = QString("%1: %2%%").arg(index.data(Qt::DisplayRole).toString())
//                                                     .arg(progressBarOption.progress);

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
                            ModelFactorySPtr factory)
: QWidget()
, m_category(category)
, m_model(model)
, m_factory(factory)
, m_proxy(nullptr)
{
  setupUi(this);

  tableView->horizontalHeader()->setMovable(true);
  tableView->setItemDelegate(new InformationDelegate());

  connect(tableView->horizontalHeader(),SIGNAL(sectionMoved(int,int,int)),
          this, SLOT(saveSelectedInformation()));

  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

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
  return m_proxy->informationTags().size();
}

//------------------------------------------------------------------------
QVariant TabularReport::Entry::value(int row, int column) const
{
  QVariant result;

  if (row < rowCount() && column < columnCount())
  {
    if (row == 0)
      result = m_proxy->informationTags()[column];
    else
      result = tableView->model()->index(row - 1, column, tableView->rootIndex()).data();
  }

  return result;
}


//------------------------------------------------------------------------
void TabularReport::Entry::paintEvent(QPaintEvent* event)
{
  refreshGUI();
  QWidget::paintEvent(event);
}

//------------------------------------------------------------------------
void TabularReport::Entry::changeDisplayedInformation()
{
  auto available = availableInformation();

  auto selection = lastDisplayedInformation();

  InformationSelector tagSelector(available, selection, tr("Select Analysis' Information"), this);

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

  //qDebug() << "New order: " << informationOrder;
  QByteArray selectedInformation = informationOrder.join("\n").toUtf8();
  m_model->storage()->saveSnapshot(SnapshotData(selectedInformationFile(), selectedInformation));
}

//------------------------------------------------------------------------
void TabularReport::Entry::extractInformation()
{
  auto title      = tr("Export %1 Data").arg(m_category);
  auto suggestion = QString("%1.xls").arg(m_category.replace("/","-"));
  auto formats    = SupportedFormats().addExcelFormat().addCSVFormat();
  auto fileName   = DefaultDialogs::SaveFile(title, formats, "", ".xls", suggestion);

  if (fileName.isEmpty())
    return;

  // some users are used to not enter an extension, and expect a default xls output.
  if(!fileName.toLower().endsWith(".csv") && !fileName.toLower().endsWith(".xls"))
    fileName += tr(".xls");

  bool exported = false;

  if (fileName.toLower().endsWith(".csv"))
  {
    exported = exportToCSV(fileName);
  }
  else if (fileName.toLower().endsWith(".xls"))
  {
    exported = exportToXLS(fileName);
  }

  if (!exported)
  {
    auto message = tr("Couldn't export %1").arg(fileName);
    DefaultDialogs::InformationMessage(title, message);
  }
}

//------------------------------------------------------------------------
void TabularReport::Entry::refreshAllInformation()
{
  int c = m_proxy->columnCount() - 1;

  if (m_proxy->informationTags()[c] == tr("Category"))
  {
    --c; // Category tag doesn't span task
  }

  for (int r = 1; r <= m_proxy->rowCount(); ++r) {
    auto data = value(r, c);
  }
}

//------------------------------------------------------------------------
void TabularReport::Entry::refreshGUI()
{
  int  progress   = m_proxy->progress();
  bool inProgress = (progress < 100);

  if (m_proxy->informationTags().size() == 1 || (m_proxy->informationTags().size() == 2 && m_proxy->informationTags()[1] == tr("Category")))
  {
    inProgress = false;
  }

  progressLabel->setVisible(inProgress);
  progressBar->setVisible(inProgress);
  progressBar->setValue(progress);

  if (exportInformation->isEnabled() == inProgress) {
    exportInformation->setEnabled(!inProgress);

    emit informationReadyChanged();
  }

  tableView->viewport()->update();
}

//------------------------------------------------------------------------
bool TabularReport::Entry::exportToCSV(const QString &filename)
{
  QFile file(filename);

  file.open(QIODevice::WriteOnly |  QIODevice::Text);

  QTextStream out(&file);

  for (int r = 0; r < rowCount(); r++)
  {
    for (int c = 0; c < columnCount(); c++)
    {
      if (c)
        out << ",";
      out << value(r, c).toString();
    }
    out << "\n";
  }
  file.close();

  return true;
}

//------------------------------------------------------------------------
bool TabularReport::Entry::exportToXLS(const QString &filename)
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

  wb.Dump(filename.toStdString());

  return true;
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
QStringList TabularReport::Entry::lastInformationOrder()
{
  QStringList informationTags, availableInformationTags;

  auto entriesFile = TabularReport::extraPath(m_category + ".xml");
  auto groupedInfo = availableInformation();

  for (auto group : groupedInfo.keys())
  {
    availableInformationTags << groupedInfo[group];
  }

  QString selectedInformation(m_model->storage()->snapshot(selectedInformationFile()));

  for (auto tag : selectedInformation.split("\n", QString::SkipEmptyParts))
  {
    if (availableInformationTags.contains(tag))
    {
      informationTags << tag;
    }
  }

  return informationTags;
}

//------------------------------------------------------------------------
InformationSelector::GroupedInfo TabularReport::Entry::lastDisplayedInformation()
{
  InformationSelector::GroupedInfo info, available;

  available = availableInformation();

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

  if (info.isEmpty())
  {
    info[SEGMENTATION_GROUP]  << tr("Category");
  }

  return info;
}

//------------------------------------------------------------------------
void TabularReport::Entry::setInformation(InformationSelector::GroupedInfo extensionInformations, QStringList informationOrder)
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

  QStringList tags;
  tags << tr("Name") << informationOrder;
  m_proxy->setInformationTags(tags);

  auto header = new QStandardItemModel(1, tags.size(), this);
  header->setHorizontalHeaderLabels(tags);
  tableView->horizontalHeader()->setModel(header);
}


//------------------------------------------------------------------------
QStringList TabularReport::Entry::updateInformationOrder(InformationSelector::GroupedInfo extensionInformation)
{
  QStringList oldInformationList     = lastInformationOrder();
  QStringList orderedInformationList = oldInformationList;

  QStringList newInformationList = information(extensionInformation);

  for (auto oldInformation : oldInformationList)
  {
    if (!newInformationList.contains(oldInformation))
    {
      orderedInformationList.removeAll(oldInformation);
    }
  }

  for (auto newInformation : newInformationList)
  {
    if (!orderedInformationList.contains(newInformation))
    {
      orderedInformationList << newInformation;
    }
  }

  return orderedInformationList;
}

//------------------------------------------------------------------------
QStringList TabularReport::Entry::information(InformationSelector::GroupedInfo extensionInformations)
{
  QStringList informations;

  for (auto extension : extensionInformations)
  {
    for (auto information : extension)
    {
      if (!informations.contains(information))
      {
        informations << information;
      }
    }
  }

  return informations;
}
