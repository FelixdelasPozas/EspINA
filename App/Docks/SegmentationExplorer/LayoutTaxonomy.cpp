/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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


#include "LayoutTaxonomy.h"
#include <Undo/ChangeTaxonomyCommand.h>
#include <Undo/MoveTaxonomiesCommand.h>
#include <Undo/AddTaxonomyElement.h>

#include <Core/Model/Segmentation.h>
#include <Core/Model/Taxonomy.h>

#include <QMessageBox>
#include <QUndoStack>

using namespace EspINA;

//------------------------------------------------------------------------
bool TaxonomyLayout::SortFilter::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
  ModelItemPtr leftItem  = indexPtr(left);
  ModelItemPtr rightItem = indexPtr(right);

  if (leftItem->type() == rightItem->type())
    if (EspINA::SEGMENTATION == leftItem->type())
      return sortSegmentationLessThan(leftItem, rightItem);
    else
      return leftItem->data(Qt::DisplayRole).toString() < rightItem->data(Qt::DisplayRole).toString();
    else
      return leftItem->type() == EspINA::TAXONOMY;
}

//------------------------------------------------------------------------
TaxonomyLayout::TaxonomyLayout(CheckableTreeView *view,
                               EspinaModel       *model,
                               QUndoStack        *undoStack)
: Layout (view, model, undoStack)
, m_proxy(new TaxonomyProxy())
, m_sort (new SortFilter())
{
  m_proxy->setSourceModel(m_model);
  m_sort->setSourceModel(m_proxy.data());
  m_sort->setDynamicSortFilter(true);

  connect(m_proxy.data(), SIGNAL(segmentationsDragged(SegmentationList,TaxonomyElementPtr)),
          this,           SLOT  (segmentationsDragged(SegmentationList,TaxonomyElementPtr)));
  connect(m_proxy.data(), SIGNAL(taxonomiesDragged(TaxonomyElementList,TaxonomyElementPtr)),
          this,           SLOT  (taxonomiesDragged(TaxonomyElementList,TaxonomyElementPtr)));
}

//------------------------------------------------------------------------
TaxonomyLayout::~TaxonomyLayout()
{
  qDebug() << "Destroying Taxonomy Layout";
}

//------------------------------------------------------------------------
void TaxonomyLayout::createSpecificControls(QHBoxLayout *specificControlLayout)
{
  QPushButton *createTaxonomy = new QPushButton();
  createTaxonomy->setIcon(QIcon(":espina/create_node.png"));
  createTaxonomy->setIconSize(QSize(22,22));
  createTaxonomy->setBaseSize(32, 32);
  createTaxonomy->setMaximumSize(32, 32);
  createTaxonomy->setMinimumSize(32, 32);
  createTaxonomy->setFlat(true);

  connect(createTaxonomy, SIGNAL(clicked(bool)),
          this, SLOT(createTaxonomy()));

  specificControlLayout->addWidget(createTaxonomy);

  QPushButton *createSubTaxonomy = new QPushButton();
  createSubTaxonomy->setIcon(QIcon(":espina/create_subnode.png"));
  createSubTaxonomy->setIconSize(QSize(22,22));
  createSubTaxonomy->setBaseSize(32, 32);
  createSubTaxonomy->setMaximumSize(32, 32);
  createSubTaxonomy->setMinimumSize(32, 32);
  createSubTaxonomy->setFlat(true);

  connect(createSubTaxonomy, SIGNAL(clicked(bool)),
          this, SLOT(createSubTaxonomy()));

  specificControlLayout->addWidget(createSubTaxonomy);
}

//------------------------------------------------------------------------
SegmentationList TaxonomyLayout::deletedSegmentations(QModelIndexList selection)
{
  QSet<SegmentationPtr> toDelete;
  foreach(QModelIndex index, selection)
  {
    index = m_sort->mapToSource(index);
    ModelItemPtr item = indexPtr(index);
    switch (item->type())
    {
      case EspINA::SEGMENTATION:
      {
        SegmentationPtr seg = segmentationPtr(item);
        toDelete << seg;
        break;
      }
      case EspINA::TAXONOMY:
      {
        int totalSeg = m_proxy->numSegmentations(index, true);
        int directSeg = m_proxy->numSegmentations(index);

        if (totalSeg == 0)
          continue;

        TaxonomyElementPtr taxonmy = taxonomyElementPtr(item);
        QMessageBox msgBox;
        msgBox.setText(SEGMENTATION_MESSAGE.arg(taxonmy->qualifiedName()));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);

        if (directSeg > 0)
        {
          if (directSeg < totalSeg)
          {
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::YesAll |  QMessageBox::No);
            msgBox.setText(MIXED_MESSAGE.arg(taxonmy->qualifiedName()));
          }
        } else
        {
          msgBox.setText(RECURSIVE_MESSAGE.arg(taxonmy->qualifiedName()));
          msgBox.setStandardButtons(QMessageBox::YesAll |  QMessageBox::No);
        }

        bool recursive = false;
        switch (msgBox.exec())
        {
          case QMessageBox::YesAll:
            recursive = true;
          case QMessageBox::Yes:
          {
            QModelIndexList subSegs = m_proxy->segmentations(index, recursive);
            foreach(QModelIndex subIndex, subSegs)
            {
              ModelItemPtr subItem = indexPtr(subIndex);
              SegmentationPtr seg = segmentationPtr(subItem);
              toDelete << seg;
            }
            break;
          }
          default:
            break;
        }
        break;
      }
          default:
            Q_ASSERT(false);
            break;
    }
  }

  return toDelete.toList();
}

//------------------------------------------------------------------------
void TaxonomyLayout::createTaxonomy()
{
  ModelItemPtr taxonomyItem = item(m_view->currentIndex());

  if (EspINA::TAXONOMY == taxonomyItem->type())
  {
    QString name = tr("New Taxonomy");

    TaxonomyElementPtr taxonomy = taxonomyElementPtr(taxonomyItem);
    if (taxonomy->element(name).isNull())
    {
      m_undoStack->beginMacro("Create Taxonomy");
      m_undoStack->push(new AddTaxonomyElement(taxonomy->parent(), name, m_model));
      m_undoStack->endMacro();
    }
  }
}

//------------------------------------------------------------------------
void TaxonomyLayout::createSubTaxonomy()
{
  ModelItemPtr taxonomyItem = item(m_view->currentIndex());

  if (EspINA::TAXONOMY == taxonomyItem->type())
  {
    QString name = tr("New Taxonomy");

    TaxonomyElementPtr taxonomy = taxonomyElementPtr(taxonomyItem);
    if (taxonomy->element(name).isNull())
    {
      m_undoStack->beginMacro("Create Taxonomy");
      m_undoStack->push(new AddTaxonomyElement(taxonomy, name, m_model));
      m_undoStack->endMacro();
    }
  }
}

//------------------------------------------------------------------------
void TaxonomyLayout::segmentationsDragged(SegmentationList   segmentations,
                                          TaxonomyElementPtr taxonomy)
{
  m_undoStack->beginMacro(tr("Change Segmentation's Taxonomy"));
  m_undoStack->push(new ChangeTaxonomyCommand(segmentations, taxonomy, m_model));
  m_undoStack->endMacro();
}


//------------------------------------------------------------------------
void TaxonomyLayout::taxonomiesDragged(TaxonomyElementList subTaxonomies,
                                       TaxonomyElementPtr  taxonomy)
{
  m_undoStack->beginMacro(tr("Modify Taxonomy"));
  m_undoStack->push(new MoveTaxonomiesCommand(subTaxonomies, taxonomy, m_model));
  m_undoStack->endMacro();
}
