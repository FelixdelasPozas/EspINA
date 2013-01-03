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


#include "SegmentationContextualMenu.h"

#include <Core/Model/EspinaModel.h>
#include <Core/Model/Taxonomy.h>
#include <Core/Model/Segmentation.h>

#include <QWidgetAction>
#include <QTreeView>
#include <QHeaderView>
#include <QStringList>
#include <QStringListModel>
#include <QListView>

using namespace EspINA;

//------------------------------------------------------------------------
SegmentationContextualMenu::SegmentationContextualMenu(EspinaModel *model,
                                                       SegmentationList selection,
                                                       QWidget         *parent)
: QMenu(parent)
, m_segmentations(selection)
{
  QMenu         *changeTaxonomyMenu = new QMenu(tr("Change Taxonomy"));
  QWidgetAction *taxonomyListAction = new QWidgetAction(changeTaxonomyMenu);
  QTreeView     *taxonomyList       = new QTreeView();
  taxonomyList->header()->setVisible(false);
  taxonomyList->setModel(model);
  taxonomyList->setRootIndex(model->taxonomyRoot());
  taxonomyList->expandAll();
  connect(taxonomyList, SIGNAL(clicked(QModelIndex)),
          this, SLOT(changeTaxonomyClicked(QModelIndex)));
  taxonomyListAction->setDefaultWidget(taxonomyList);
  changeTaxonomyMenu->addAction(taxonomyListAction);
  this->addMenu(changeTaxonomyMenu);

  m_changeFinalNode = this->addAction(tr("Set level of detail"));
  m_changeFinalNode->setCheckable(true);
  connect(m_changeFinalNode, SIGNAL(triggered()),
          this, SLOT(changeFinalFlag()));

  QAction *deleteSegs = this->addAction(tr("Delete"));
  connect (deleteSegs, SIGNAL(triggered(bool)),
           this, SLOT(deleteSementationsClicked()));

  bool enabled = false;
  SegmentationList ancestors, successors;
  foreach (SegmentationPtr seg, m_segmentations)
  {
    enabled |= seg->IsFinalNode();
    foreach(SegmentationSPtr ancestor, seg->componentOf())
      ancestors <<  ancestor.data();
    foreach(SegmentationSPtr successor, seg->components())
      successors << successor.data();
  }

  foreach(SegmentationPtr seg, ancestors)
  {
    if (m_segmentations.contains(seg))
    {
      ancestors.removeAll(seg);
      break;
    }
    m_segmentations.append(seg);
    foreach(SegmentationSPtr ancestor, seg->componentOf())
      ancestors <<  ancestor.data();

    enabled |= seg->IsFinalNode();
  }

  foreach(SegmentationPtr seg, successors)
  {
    if (m_segmentations.contains(seg))
    {
      successors.removeAll(seg);
      break;
    }
    m_segmentations.append(seg);
    foreach(SegmentationSPtr successor, seg->components())
      successors << successor.data();

    enabled |= seg->IsFinalNode();
  }

  m_changeFinalNode->setChecked(enabled);
}

//------------------------------------------------------------------------
void SegmentationContextualMenu::changeTaxonomyClicked(const QModelIndex& index)
{
  this->hide();

  ModelItemPtr taxItem = indexPtr(index);
  Q_ASSERT(EspINA::TAXONOMY == taxItem->type());
  TaxonomyElementPtr taxonomy = taxonomyElementPtr(taxItem);
  emit changeTaxonomy(taxonomy);
}

//------------------------------------------------------------------------
void SegmentationContextualMenu::changeFinalFlag()
{
  this->hide();
  emit changeFinalNode(m_changeFinalNode->isChecked());
}


//------------------------------------------------------------------------
void SegmentationContextualMenu::deleteSementationsClicked()
{
  this->hide();
  emit deleteSegmentations();
}

//------------------------------------------------------------------------
void SegmentationContextualMenu::setSelection(SegmentationList list)
{
  this->m_segmentations = list;
}
