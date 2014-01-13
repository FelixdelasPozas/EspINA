/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
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

#include "InformationSelector.h"

#include <ui_InformationSelector.h>

using namespace EspINA;

class InformationSelector::GUI
: public  Ui::InformationSelector
{
};

//----------------------------------------------------------------------------
InformationSelector::InformationSelector(InformationTagsByCategory &tags,
                                         ModelFactorySPtr           factory,
                                         QWidget                   *parent,
                                         Qt::WindowFlags            flags)
: QDialog(parent, flags)
, m_gui(new GUI())
, m_factory(factory)
, m_tags(tags)
{
  m_gui->setupUi(this);

  setWindowTitle(tr("Select Analysis' Information"));

  for(auto category : tags.keys())
  {
    auto categoryNode = new QTreeWidgetItem(m_gui->treeWidget);
    categoryNode->setText(0, category);

    for(auto extension : m_factory->segsegmentationExtensions())
    {
      if (extension->validTaxonomy(qualifiedName))
      {
        QTreeWidgetItem *extensionNode = new QTreeWidgetItem(categoryNode);
        extensionNode->setText(0, extension->id());

        bool hasCheckedChildren   = false;
        bool hasUnCheckedChildren = false;

        foreach(Segmentation::InfoTag tag, extension->availableInformations())
        {
          QTreeWidgetItem *tagNode = new QTreeWidgetItem(extensionNode);
          bool selected = m_tags[category].contains(tag);

          hasCheckedChildren   |=  selected;
          hasUnCheckedChildren |= !selected;

          Qt::CheckState state = selected?Qt::Checked:Qt::Unchecked;
          tagNode->setText(0, tag);
          tagNode->setData(0, Qt::UserRole,       state);
          tagNode->setData(0, Qt::CheckStateRole, state);
        }
        Qt::CheckState state;
        if (hasCheckedChildren && hasUnCheckedChildren)
          state = Qt::PartiallyChecked;
        else if (hasCheckedChildren)
          state = Qt::Checked;
        else
          state = Qt::Unchecked;

        extensionNode->setData(0, Qt::UserRole,       state);
        extensionNode->setData(0, Qt::CheckStateRole, state);
      }
    }
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
  m_tags.clear();

  QTreeWidgetItemIterator it(m_gui->treeWidget, QTreeWidgetItemIterator::Checked);
  while (*it)
  {
    QTreeWidgetItem *node       = (*it);
    QTreeWidgetItem *parentNode = node->parent();
    if (parentNode && parentNode->parent())
      m_tags[parentNode->parent()->text(0)] << node->data(0,Qt::DisplayRole).toString();
    ++it;
  }

  QDialog::accept();
}

#include <QDebug>

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
