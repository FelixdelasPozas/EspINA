/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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


#include "taxonomyProxy.h"

#include <data/modelItem.h>
#include <data/taxonomy.h>
#include "espina.h"
#include "products.h"

//Debug
#include <assert.h>
#include <QDebug>
#include <iostream>
//------------------------------------------------------------------------
TaxonomyProxy::TaxonomyProxy(QObject* parent)
    : QAbstractProxyModel(parent)
{
  updateSegmentations();
}

//------------------------------------------------------------------------
TaxonomyProxy::~TaxonomyProxy()
{
}

//------------------------------------------------------------------------
void TaxonomyProxy::setSourceModel(QAbstractItemModel* sourceModel)
{
  QAbstractProxyModel::setSourceModel(sourceModel);
  updateSegmentations();
  connect(sourceModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
          this, SLOT(sourceRowsInserted(const QModelIndex&, int, int)));
  connect(sourceModel, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
          this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));
  connect(sourceModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
          this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
}

//------------------------------------------------------------------------
int TaxonomyProxy::rowCount(const QModelIndex& parent) const
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());

  updateSegmentations();

  if (!parent.isValid())
    return 3;

  if (parent == mapFromSource(model->taxonomyRoot()))
    return model->rowCount(model->taxonomyRoot());

  if (parent == mapFromSource(model->sampleRoot()))
    return 0;

  if (parent == mapFromSource(model->segmentationRoot()))
    return 0;

  // Cast to base type
  IModelItem *parentItem = static_cast<IModelItem *>(parent.internalPointer());
  // Check if Taxonomy Item
  TaxonomyNode *parentTax = dynamic_cast<TaxonomyNode *>(parentItem);
  if (parentTax)
  {
    int numSegs = m_taxonomySegs[parentTax].size();
    return parentTax->getSubElements().size() + numSegs;
  }
  // Otherwise Samples and Segmentations have no children
  return 0;
}

//------------------------------------------------------------------------
QModelIndex TaxonomyProxy::index(int row, int column, const QModelIndex& parent) const
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());

  if (!hasIndex(row, column, parent))
    return QModelIndex();

  if (!parent.isValid())
  {
    assert(row < 3);
    if (row == 0)
      return mapFromSource(model->taxonomyRoot());
    if (row == 1)
      return mapFromSource(model->sampleRoot());
    if (row == 2)
      return mapFromSource(model->segmentationRoot());
  }

  // Segmentation can't be parent index
  IModelItem *parentItem = static_cast<IModelItem *>(parent.internalPointer());
  // Checks if parent is Taxonomy
  TaxonomyNode *parentTax = dynamic_cast<TaxonomyNode *>(parentItem);
  if (parentTax)
  {
    IModelItem *element;
    //int subTaxonomies = numOfSubTaxonomies(taxItem);
    int subTaxonomies = parentTax->getSubElements().size();
    if (row < subTaxonomies)
      element = parentTax->getSubElements()[row];
    else
      element = m_taxonomySegs[parentTax][row-subTaxonomies];

    return createIndex(row, column, element);
  }
  // Otherwise, invalid index
  assert(false);
  return QModelIndex();
}

//------------------------------------------------------------------------
QModelIndex TaxonomyProxy::parent(const QModelIndex& child) const
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());
  assert(model);

  if (!child.isValid())
    return QModelIndex();

  if (child == mapFromSource(model->taxonomyRoot())
      || child == mapFromSource(model->sampleRoot())
      || child == mapFromSource(model->segmentationRoot()))
    return QModelIndex();

  IModelItem *childItem = static_cast<IModelItem *>(child.internalPointer());
  assert(childItem);
  // Checks if Taxonomy
  TaxonomyNode *childNode = dynamic_cast<TaxonomyNode *>(childItem);
  if (childNode)
  {
    QModelIndex sourceNode = model->taxonomyIndex(childNode);
    QModelIndex sourceParent = model->parent(sourceNode);
    return mapFromSource(sourceParent);
    //std::cout << "Getting parent of " << childNode->getName().toStdString() << std::endl;
    TaxonomyNode *tax = static_cast<TaxonomyNode *>(model->taxonomyRoot().internalPointer());
    TaxonomyNode *parentNode = tax->getParent(childNode->getName());
    //std::cout << "\tParent is " << parentNode->getName().toStdString() << std::endl;
  }

  // Otherwise is a segmentation
  Segmentation *childSeg = dynamic_cast<Segmentation *>(childItem);
  if (childSeg)
    return mapFromSource(model->taxonomyIndex(childSeg->taxonomy()));

  assert(false);
  return QModelIndex();
}

//------------------------------------------------------------------------
QModelIndex TaxonomyProxy::mapFromSource(const QModelIndex& sourceIndex) const
{
  if (!sourceIndex.isValid())
    return QModelIndex();

  return createIndex(sourceIndex.row(), sourceIndex.column(), sourceIndex.internalPointer());
}

//------------------------------------------------------------------------
QModelIndex TaxonomyProxy::mapToSource(const QModelIndex& proxyIndex) const
{
  if (!proxyIndex.isValid())
    return QModelIndex();

  IModelItem *proxyItem = static_cast<IModelItem *>(proxyIndex.internalPointer());
  TaxonomyNode *proxyTax = dynamic_cast<TaxonomyNode *>(proxyItem);
  if (proxyTax)
    return sourceModel()->index(proxyIndex.row(), proxyIndex.column(), proxyIndex.parent());
  Segmentation *proxySeg = dynamic_cast<Segmentation *>(proxyItem);
  if (proxySeg)
  {
    EspINA *model = dynamic_cast<EspINA *>(sourceModel());
    return model->segmentationIndex(proxySeg);
  }
  return QModelIndex();
}

//------------------------------------------------------------------------
void TaxonomyProxy::sourceRowsInserted(const QModelIndex& sourceParent, int start, int end)
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());

  if (sourceParent == model->segmentationRoot())
  {
    updateSegmentations();
    //Look for modified segmentations
    for (int r = start; r <= end; r++)
    {
      QModelIndex sourceIndex = model->index(r, 0, sourceParent);
      IModelItem *sourceItem = static_cast<IModelItem *>(sourceIndex.internalPointer());
      Segmentation *sourceSeg = dynamic_cast<Segmentation *>(sourceItem);
      assert(sourceSeg);
      TaxonomyNode *segTax = sourceSeg->taxonomy();
      QModelIndex taxIndex = mapFromSource(model->taxonomyIndex(segTax));
      int begin = segTax->getSubElements().size();
      int end = rowCount(taxIndex) - 1;
      beginInsertRows(taxIndex, end, end);
      endInsertRows();
    }
  }
}

//------------------------------------------------------------------------
void TaxonomyProxy::sourceRowsAboutToBeRemoved(const QModelIndex& sourceParent, int start, int end)
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());

  if (sourceParent == model->segmentationRoot())
  {
    for (int r = start; r <= end; r++)
    {
      // Need to find its parent before deletion
      QModelIndex sourceIndex = model->index(r, 0, sourceParent);
#ifdef ESPINA_DEBUG
      IModelItem *sourceItem = static_cast<IModelItem *>(sourceIndex.internalPointer());
      Segmentation *sourceSeg = dynamic_cast<Segmentation *>(sourceItem);
      assert(sourceSeg);
#endif
      QModelIndex proxyIndex = mapFromSource(sourceIndex);
      beginRemoveRows(proxyIndex.parent(),proxyIndex.row(),proxyIndex.row());
      endRemoveRows();
    }
  }
}

//------------------------------------------------------------------------
void TaxonomyProxy::sourceRowsRemoved(const QModelIndex& sourceParent, int start, int end)
{
  updateSegmentations();
}

//------------------------------------------------------------------------
void TaxonomyProxy::updateSegmentations() const
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());
  if (!model)
    return;

  m_taxonomySegs.clear();
  int rows = model->rowCount(model->segmentationRoot());
  for (int row = 0; row < rows; row++)
  {
    QModelIndex segIndex = model->index(row, 0, model->segmentationRoot());
    IModelItem *segItem = static_cast<IModelItem *>(segIndex.internalPointer());
    assert(segItem);
    Segmentation *seg = dynamic_cast<Segmentation *>(segItem);
    assert(seg);
    m_taxonomySegs[seg->taxonomy()].push_back(seg);
  }
}





