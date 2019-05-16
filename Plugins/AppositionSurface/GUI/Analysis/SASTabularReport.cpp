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
#include <Core/Utils/EspinaException.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/Widgets/InformationSelector.h>
#include <GUI/Dialogs/DefaultDialogs.h>

// Qt
#include <QAbstractItemView>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include "../../Plugin.h"

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::Core::Utils;

//----------------------------------------------------------------------------
void SASTabularReport::createCategoryEntry(const QString& category)
{
  bool found = false;
  int  i = 0;

  while (!found && i < m_tabs->count())
  {
    found = m_tabs->tabText(i) >= category;

    if (!found) i++;
  }

  auto factory = m_context.factory();

  if (m_tabs->tabText(i) != category)
  {
    auto entry = new Entry(category, m_model, factory, m_tabs);

    connect(entry, SIGNAL(informationReadyChanged()),
            this,  SLOT(updateExportStatus()));

    auto infoProxy = new SASInformationProxy(m_model, m_sasTags, factory->scheduler());
    infoProxy->setCategory(category);
    infoProxy->setFilter(&m_filter);
    infoProxy->setSourceModel(m_model);
    connect (infoProxy, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
             this, SLOT(rowsRemoved(const QModelIndex &, int, int)));
    entry->setProxy(infoProxy);

    auto sortFilter = new DataSortFilter();
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

  for (auto type : m_factory->availableSegmentationExtensions())
  {
    if (!isSASExtensions(type))
    {
      auto prototype = m_factory->createSegmentationExtension(type);

      if(prototype->validCategory("SAS") || prototype->validCategory("Synapse"))
      {
        info[type] << keyValues(prototype->availableInformation());
      }
    }
  }

  auto sasExtensionPrototype = m_factory->createSegmentationExtension(AppositionSurfaceExtension::TYPE);

  info[AppositionSurfaceExtension::TYPE] << keyValues(sasExtensionPrototype->availableInformation());

  // in case we have extensions not registered in the factory add them too.
  for (auto item : m_proxy->displayedItems())
  {
    Q_ASSERT(isSegmentation(item));

    auto segmentation = segmentationPtr(item);

    for (auto extension : segmentation->readOnlyExtensions())
    {
      if(!info.keys().contains(extension->type()))
      {
        info[extension->type()] << keyValues(extension->availableInformation());
      }
      else
      {
        // fix for extensions with variable keys like stereological inclusion.
        for(auto key: extension->availableInformation())
        {
          if(!info[extension->type()].contains(key.value()))
          {
            info[extension->type()] << key.value();
          }
        }
      }
    }
  }

  for (auto type : info.keys())
  {
    info[type].removeDuplicates();
  }

  return info;
}

//------------------------------------------------------------------------
void SASTabularReport::exportInformation()
{
  auto title      = tr("Export SAS Data");
  auto suggestion = tr("Raw information with SAS data.xls");
  auto formats    = SupportedFormats().addExcelFormat()
                                      .addCSVFormat();

  auto fileName   = DefaultDialogs::SaveFile(title, formats, QDir::homePath(), ".xls", suggestion, this);

  if (fileName.isEmpty()) return;

  if (!fileName.endsWith(".csv", Qt::CaseInsensitive) && !fileName.endsWith(".xls", Qt::CaseInsensitive))
  {
    fileName += tr(".xls");
  }

  if (fileName.endsWith(".csv", Qt::CaseInsensitive))
  {
    try
    {
      exportToCSV(fileName);
    }
    catch (const EspinaException &e)
    {
      auto message = tr("Unable to export %1").arg(fileName);
      DefaultDialogs::InformationMessage(message, title, e.details(), this);
    }
  }
  else
  {
    if (fileName.endsWith(".xls", Qt::CaseInsensitive))
    {
      try
      {
        exportToXLS(fileName);
      }
      catch (const EspinaException &e)
      {
        auto message = tr("Unable to export %1").arg(fileName);
        DefaultDialogs::InformationMessage(message, title, e.details(), this);
      }
    }
  }
}

//------------------------------------------------------------------------
void SASTabularReport::Entry::setInformation(InformationSelector::GroupedInfo extensionInformations, SegmentationExtension::InformationKeyList informationOrder)
{
  for (auto item :  m_proxy->displayedItems())
  {
    auto segmentation = segmentationPtr(item);

    if (AppositionSurfacePlugin::isValidSynapse(segmentation))
    {
      for(auto extensionType : extensionInformations.keys())
      {
        try
        {
          if(!isSASExtensions(extensionType))
          {
            retrieveOrCreateSegmentationExtension(segmentation, extensionType, m_factory);
          }
          else
          {
            auto sas = AppositionSurfacePlugin::segmentationSAS(segmentation);
            if(sas)
            {
              retrieveOrCreateSegmentationExtension(sas, AppositionSurfaceExtension::TYPE, m_factory);
            }
          }
        }
        catch(...)
        {
          // nothing to do, either the extensions is read-only or doesn't exist and that information will be reported as unavailable later.
        }
      }
    }
  }

  SegmentationExtension::InformationKeyList keys;

  keys << InformationProxy::NameKey() << informationOrder;

  m_proxy->setInformationTags(keys);

  QStringList headerLabels;
  for (auto key : keys)
  {
    auto label = isSASExtensions(key.extension()) ? AppositionSurfaceExtension::addSASPrefix(key.value()) : key.value();
    headerLabels << label;
  }

  auto header = new QStandardItemModel(1, keys.size(), this);
  header->setHorizontalHeaderLabels(headerLabels);
  tableView->horizontalHeader()->setModel(header);
}

//------------------------------------------------------------------------
SegmentationExtension::KeyList SASTabularReport::Entry::keyValues(const Extension< Segmentation >::InformationKeyList& keys) const
{
  SegmentationExtension::KeyList values;

  for (auto &key : keys)
  {
    values << key.value();
  }

  return values;
}

//------------------------------------------------------------------------
bool SASTabularReport::Entry::isSASExtensions(const SegmentationExtension::Type& type) const
{
  return type == AppositionSurfaceExtension::TYPE;
}

//------------------------------------------------------------------------
void SASTabularReport::Entry::extractInformation()
{
  auto title      = tr("Export %1 Data").arg(m_category);
  auto suggestion = QString("%1 SAS information.xls").arg(m_category.replace("/","-"));
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
      auto message = tr("Unable to export %1").arg(fileName);
      DefaultDialogs::InformationMessage(message, title, e.details(), this);
    }
  }
  else
  {
    if (fileName.endsWith(".xls", Qt::CaseInsensitive))
    {
      try
      {
        exportToXLS(fileName);
      }
      catch(const EspinaException &e)
      {
        auto message = tr("Unable to export %1").arg(fileName);
        DefaultDialogs::InformationMessage(message, title, e.details(), this);
      }
    }
  }
}
