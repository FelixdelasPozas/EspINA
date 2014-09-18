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

// Qt
#include <ui_InformationSelector.h>

using namespace ESPINA;

class InformationSelector::GUI
: public  Ui::InformationSelector
{
};

//----------------------------------------------------------------------------
InformationSelector::InformationSelector(const InformationSelector::GroupedInfo &available,
                                         InformationSelector::GroupedInfo       &selection,
                                         QWidget                                *parent,
                                         Qt::WindowFlags                         flags)
: QDialog    {parent, flags}
, m_gui      {new GUI()}
, m_selection{selection}
{
  m_gui->setupUi(this);

  setWindowTitle(tr("Select Analysis' Information"));

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
      state = Qt::PartiallyChecked;
    else if (hasCheckedChildren)
      state = Qt::Checked;
    else
      state = Qt::Unchecked;

    groupNode->setData(0, Qt::UserRole,       state);
    groupNode->setData(0, Qt::CheckStateRole, state);
  }

  connect(m_gui->treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
          this, SLOT(updateCheckState(QTreeWidgetItem*,int)));

  connect(m_gui->acceptChanges, SIGNAL(clicked(bool)),
          this, SLOT(accept()));
  connect(m_gui->rejectChanges, SIGNAL(clicked(bool)),
          this, SLOT(reject()));
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
    QTreeWidgetItem *node       = (*it);
    QTreeWidgetItem *parentNode = node->parent();
    if (parentNode)
      m_selection[parentNode->text(0)] << node->data(0,Qt::DisplayRole).toString();
    ++it;
  }

  QDialog::accept();
}

//----------------------------------------------------------------------------
void InformationSelector::updateCheckState(QTreeWidgetItem *item, int column, bool updateParent)
{
  if (item->data(column, Qt::UserRole).toInt() != item->checkState(column))
  {
    for (int i = 0; i < item->childCount(); ++i)
    {
      item->child(i)->setCheckState(column, item->checkState(column));
      updateCheckState(item->child(i), column, false);
    }
    item->setData(column, Qt::UserRole, item->checkState(column));

    QTreeWidgetItem *parentNode = item->parent();
    if (parentNode && updateParent && parentNode->parent())
    {
      Qt::CheckState state = item->checkState(column);
      int i = 0;
      while (state == item->checkState(column) && i < parentNode->childCount())
      {
        if (state != parentNode->child(i)->checkState(0))
          state = Qt::PartiallyChecked;
        ++i;
      }
      parentNode->setCheckState(column, state);
      parentNode->setData(column, Qt::UserRole, state);
    }
  }
}
