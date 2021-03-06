/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#include "InformationSelector.h"
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Model/CategoryAdapter.h>

// Qt
#include <ui_InformationSelector.h>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::GUI;

class InformationSelector::UI
: public Ui::InformationSelector
{
};

//----------------------------------------------------------------------------
InformationSelector::InformationSelector(const InformationSelector::GroupedInfo &available,
                                         InformationSelector::GroupedInfo       &selection,
                                         const QString                          &title,
                                         const bool                             exclusive,
                                         QWidget                               *parent)
: QDialog(parent)
, m_gui      {new UI()}
, m_exclusive{exclusive}
, m_selection(selection)
{
  m_gui->setupUi(this);

  setWindowTitle(title);

  for(auto group : available.keys())
  {
    auto groupNode = new QTreeWidgetItem(m_gui->treeWidget);
    groupNode->setText(0, group);

    bool hasCheckedChildren   = false;
    bool hasUnCheckedChildren = false;

    for(auto info : available[group])
    {
      auto infoNode = new QTreeWidgetItem(groupNode);
      bool selected = m_selection[group].contains(info);

      hasCheckedChildren   |=  selected;
      hasUnCheckedChildren |= !selected;

      Qt::CheckState state = selected?Qt::Checked:Qt::Unchecked;
      infoNode->setText(0, info);
      infoNode->setData(0, Qt::UserRole,       state);
      infoNode->setData(0, Qt::CheckStateRole, state);
    }

    Qt::CheckState state;
    if (hasCheckedChildren && hasUnCheckedChildren)
    {
      state = Qt::PartiallyChecked;
    }
    else if (hasCheckedChildren)
    {
      state = Qt::Checked;
    }
    else
    {
      state = Qt::Unchecked;
    }

    groupNode->setData(0, Qt::UserRole,       state);
    if (!m_exclusive)
    {
      groupNode->setData(0, Qt::CheckStateRole, state);
    }
  }

  connect(m_gui->treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
          this,              SLOT(onItemClicked(QTreeWidgetItem*,int)));

  connect(m_gui->acceptChanges, SIGNAL(clicked(bool)),
          this,                 SLOT(accept()));

  connect(m_gui->rejectChanges, SIGNAL(clicked(bool)),
          this,                 SLOT(reject()));
}

//----------------------------------------------------------------------------
InformationSelector::~InformationSelector()
{
  delete m_gui;
}

//----------------------------------------------------------------------------
void InformationSelector::accept()
{
  m_selection.clear();

  QTreeWidgetItemIterator it(m_gui->treeWidget, QTreeWidgetItemIterator::Checked);
  while (*it)
  {
    auto node       = (*it);
    auto parentNode = node->parent();

    if (parentNode)
    {
      m_selection[parentNode->text(0)] << node->data(0,Qt::DisplayRole).toString();
    }

    ++it;
  }

  QDialog::accept();
}

//----------------------------------------------------------------------------
void InformationSelector::onItemClicked(QTreeWidgetItem* item, const int column)
{
  if (m_exclusive)
  {
    if (item->parent())
    {
      unselectItems();
      item->setCheckState(column, Qt::Checked);
    }
  }
  else
  {
    updateCheckState(item, column, true);
  }
}

//----------------------------------------------------------------------------
void InformationSelector::unselectItems()
{
  QTreeWidgetItemIterator it(m_gui->treeWidget, QTreeWidgetItemIterator::Checked);
  while (*it)
  {
    (*it)->setCheckState(0, Qt::Unchecked);

    ++it;
  }
}

//----------------------------------------------------------------------------
void InformationSelector::updateCheckState(QTreeWidgetItem *item, const int column, const bool updateParent)
{
  if (item->data(column, Qt::UserRole).toInt() != item->checkState(column))
  {
    for (int i = 0; i < item->childCount(); ++i)
    {
      item->child(i)->setCheckState(column, item->checkState(column));
      updateCheckState(item->child(i), column, false);
    }
    item->setData(column, Qt::UserRole, item->checkState(column));

    auto parentNode = item->parent();
    if (parentNode && updateParent)
    {
      Qt::CheckState state = item->checkState(column);
      int i = 0;
      while (state == item->checkState(column) && i < parentNode->childCount())
      {
        if (state != parentNode->child(i)->checkState(0))
        {
          state = Qt::PartiallyChecked;
        }
        ++i;
      }
      parentNode->setCheckState(column, state);
      parentNode->setData(column, Qt::UserRole, state);
    }
  }
}

//----------------------------------------------------------------------------
bool validForSegmentations(const SegmentationExtensionPtr extension, const SegmentationAdapterList segmentations)
{
  bool result = !segmentations.isEmpty();

  auto isValid = [&result, &extension](const SegmentationAdapterPtr seg)
  {
    result &= (extension->validCategory(seg->category()->classificationName())) &&
              (extension->validData(seg->output()));
  };
  std::for_each(segmentations.cbegin(), segmentations.cend(), isValid);

  return result;
}

//----------------------------------------------------------------------------
InformationSelector::GroupedInfo GUI::availableInformation(ModelFactorySPtr factory)
{
  InformationSelector::GroupedInfo info;

  auto addInformation = [&info, &factory](const Core::SegmentationExtension::Type &t)
  {
    const auto extension = factory->createSegmentationExtension(t);
    for (auto key : extension->availableInformation())
    {
      info[t] << key.value();
    }
  };
  const auto types = factory->availableSegmentationExtensions();
  std::for_each(types.cbegin(), types.cend(), addInformation);

  return info;
}

//----------------------------------------------------------------------------
InformationSelector::GroupedInfo GUI::availableInformation(const SegmentationAdapterList segmentations, ModelFactorySPtr factory)
{
  InformationSelector::GroupedInfo info;

  const auto types = factory->availableSegmentationExtensions();

  auto addInformation = [&info, &factory, &segmentations](const Core::SegmentationExtension::Type &t)
  {
    auto extension = factory->createSegmentationExtension(t);

    if (validForSegmentations(extension.get(), segmentations))
    {
      for (auto key : extension->availableInformation())
      {
        info[t] << key.value();
      }
    }
  };
  std::for_each(types.cbegin(), types.cend(), addInformation);

  auto addSegmentationInformation = [&info, &factory](const SegmentationAdapterPtr seg)
  {
    auto extensions = seg->readOnlyExtensions();

    for(const auto &extension: extensions)
    {
      if(!info.keys().contains(extension->type()))
      {
        const auto keys =  extension->availableInformation();
        for (const auto key : keys)
        {
          info[key.extension()] << key;
        }
      }
      else
      {
        // fix for extensions with variable keys like stereological inclusion (variable number of counting frames).
        const auto keys = extension->availableInformation();
        for(const auto &key: keys)
        {
          if(!info[key.extension()].contains(key))
          {
            info[key.extension()] << key;
          }
        }
      }
    }
  };
  std::for_each(segmentations.cbegin(), segmentations.cend(), addSegmentationInformation);

  auto removeDuplicates = [&info](const QString &tag)
  {
    info[tag].removeDuplicates();
  };
  const auto keys = info.keys();
  std::for_each(keys.cbegin(), keys.cend(), removeDuplicates);

  return info;
}
