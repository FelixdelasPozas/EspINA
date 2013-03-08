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
#include <Core/Model/Segmentation.h>
#include <Core/Model/EspinaFactory.h>
#include <Core/Extensions/SegmentationExtension.h>

#include <ui_InformationSelector.h>

using namespace EspINA;

class InformationSelector::GUI
: public  Ui::InformationSelector
{
};

//----------------------------------------------------------------------------
InformationSelector::InformationSelector(Segmentation::InfoTagList &tags,
                                         const QString  &qualifiedName,
                                         EspinaFactory  *factory,
                                         QWidget        *parent, 
                                         Qt::WindowFlags f)
: QDialog(parent, f)
, m_gui(new GUI())
, m_tags(tags)
{
  m_gui->setupUi(this);

  setWindowTitle(tr("%1 Information").arg(qualifiedName));

  foreach(Segmentation::InformationExtension extension, factory->segmentationExtensions())
  {
    if (extension->validTaxonomy(qualifiedName))
    {
      QTreeWidgetItem *extensionNode = new QTreeWidgetItem(m_gui->treeWidget);
      extensionNode->setText(0, extension->id());
      extensionNode->setData(0, Qt::CheckStateRole, Qt::Checked);
      extensionNode->setData(0, Qt::UserRole, Qt::Checked);

      foreach(Segmentation::InfoTag tag, extension->availableInformations())
      {
        QTreeWidgetItem *tagNode = new QTreeWidgetItem(extensionNode);
        tagNode->setData(0, Qt::CheckStateRole, Qt::Checked);
        tagNode->setText(0, tag);
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
    if ((*it)->parent())
      m_tags << (*it)->data(0,Qt::DisplayRole).toString();
    ++it;
  }

  QDialog::accept();
}

#include <QDebug>

//----------------------------------------------------------------------------
void InformationSelector::updateCheckState(QTreeWidgetItem *item, int column)
{
  if (item->data(column, Qt::UserRole).toInt() != item->checkState(column))
  {
    for (int i = 0; i < item->childCount(); ++i)
    {
      item->child(i)->setCheckState(column, item->checkState(column));
      updateCheckState(item->child(i), column);
    }
    item->setData(column, Qt::UserRole, item->checkState(column));
  }
}
