/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

// Plugin
#include "SASInformationProxy.h"
#include "SASTabularReport.h"

// ESPINA
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/Widgets/InformationSelector.h>

// Qt
#include <QAbstractItemView>
#include <QFileDialog>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>

const QString SEGMENTATION_GROUP = "Segmentation";
const QString SASTAG_PREPEND = QObject::tr("SAS ");

using namespace ESPINA::GUI::Model::Utils;

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
    int role = left.column() > 0 ? Qt::DisplayRole : ESPINA::TypeRole + 1;
    bool ok1, ok2;
    double lv = left.data(role).toDouble(&ok1);
    double rv = right.data(role).toDouble(&ok2);

    if (ok1 && ok2)
      return lv < rv;
    else
      return left.data(role).toString() < right.data(role).toString();
  }
};

namespace ESPINA
{
  //----------------------------------------------------------------------------
  void SASTabularReport::createCategoryEntry(const QString& category)
  {
    bool found = false;
    int  i = 0;
    while (!found && i < m_tabs->count())
    {
      if (m_tabs->tabText(i) >= category)
        found = true;
      else
        i++;
    }

    auto factory = m_context.factory();

    if (m_tabs->tabText(i) != category)
    {
      auto entry = new Entry(category, m_model, factory);

      connect(entry, SIGNAL(informationReadyChanged()),
              this,  SLOT(updateExportStatus()));

      auto infoProxy = new SASInformationProxy(m_model, m_sasTags, factory->scheduler());
      infoProxy->setCategory(category);
      infoProxy->setFilter(&m_filter);
      infoProxy->setSourceModel(m_model);
      connect (infoProxy, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
               this, SLOT(rowsRemoved(const QModelIndex &, int, int)));
      entry->setProxy(infoProxy);

      auto sortFilter = new DataSortFiler();
      sortFilter->setSourceModel(infoProxy);
      sortFilter->setDynamicSortFilter(true);

      auto tableView = entry->tableView;
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
  InformationSelector::GroupedInfo SASTabularReport::Entry::availableInformation()
  {
    InformationSelector::GroupedInfo info;

    info[SEGMENTATION_GROUP] << tr("Category");

    for (auto type : m_factory->availableSegmentationExtensions())
    {
      if(type == AppositionSurfaceExtension::TYPE)
        continue;

      auto extension = m_factory->createSegmentationExtension(type);
      info[type] << extension->availableInformations();
    }

    for(auto tag: m_factory->createSegmentationExtension(AppositionSurfaceExtension::TYPE)->availableInformations())
        info[QString(AppositionSurfaceExtension::TYPE).prepend(SASTAG_PREPEND)] << QString(tag).prepend(SASTAG_PREPEND);

    // in case we have extensions not registered in the factory add them too. Will be read-only extensions.
    for (auto item : m_proxy->displayedItems())
    {
      Q_ASSERT(isSegmentation(item));

      auto segmentation = segmentationPtr(item);

      for (auto extension : segmentation->extensions())
        info[extension->type()] << extension->availableInformations();
    }

    for (auto tag : info.keys())
      info[tag].removeDuplicates();

    return info;
  }

  //------------------------------------------------------------------------
  void SASTabularReport::exportInformation()
  {
    QString filter = tr("Excel File (*.xls)") + ";;" + tr("CSV Text File (*.csv)");
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Export Raw Data"),
                                                    QString("Raw information with SAS data.xls"),
                                                    filter);

    if (fileName.isEmpty())
      return;

    bool result = false;
    if (fileName.endsWith(".csv"))
    {
      result = exportToCSV(fileName);
    }
    else if (fileName.endsWith(".xls"))
    {
      result = exportToXLS(fileName);
    }

    if (!result)
      QMessageBox::warning(this, "ESPINA", tr("Unable to export %1").arg(fileName));
  }

  //------------------------------------------------------------------------
  void SASTabularReport::Entry::setInformation(InformationSelector::GroupedInfo extensionInformations, QStringList informationOrder)
  {
    for(auto extensionType : extensionInformations.keys())
    {
      if(extensionType.startsWith(SASTAG_PREPEND))
        continue;

      for (auto segmentation : m_model->segmentations())
      {
        if (!segmentation->hasExtension(extensionType))
        {
          if (m_factory->availableSegmentationExtensions().contains(extensionType))
          {
            auto extension = m_factory->createSegmentationExtension(extensionType);
            if(extension->validCategory(segmentation->category()->classificationName()))
              segmentation->addExtension(extension);
          }
          else if (extensionType != SEGMENTATION_GROUP)
          {
            qWarning() << extensionType << " is not available";
          }
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
  void SASTabularReport::Entry::extractInformation()
  {
    QString filter = tr("Excel File (*.xls)") + ";;" + tr("CSV Text File (*.csv)");
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Export %1 Data").arg(m_category),
                                                    QString("%1 with SAS data.xls").arg(m_category.replace("/","-")),
                                                    filter);

    if (fileName.isEmpty())
      return;

    // some users are used to not enter an extension, and expect a default xls output.
    if(!fileName.toLower().endsWith(".csv") && !fileName.toLower().endsWith(".xls"))
      fileName += tr(".xls");

    bool result = false;

    if (fileName.endsWith(".csv"))
    {
      result = exportToCSV(fileName);
    }
    else
    {
      result = exportToXLS(fileName);
    }

    if (!result)
      QMessageBox::warning(this, "ESPINA", tr("Couldn't export %1").arg(fileName));
  }

} // namespace ESPINA



