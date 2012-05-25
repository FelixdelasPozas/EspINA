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


#include "TaxonomyProxy.h"

#include "common/EspinaCore.h"

#include <QMimeData>
#include <QPixmap>

typedef QSet<ModelItem *> SegSet;

//------------------------------------------------------------------------
TaxonomyProxy::TaxonomyProxy(QObject* parent)
    : QAbstractProxyModel(parent)
{
}

//------------------------------------------------------------------------
TaxonomyProxy::~TaxonomyProxy()
{
}

//------------------------------------------------------------------------
void TaxonomyProxy::setSourceModel(EspinaModel *sourceModel)
{
  m_model = sourceModel;
  connect(sourceModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
          this, SLOT(sourceRowsInserted(const QModelIndex&, int, int)));
  connect(sourceModel, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
          this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));
  connect(sourceModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
          this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
  connect(sourceModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
	  this,SLOT(sourceDataChanged(QModelIndex,QModelIndex)));
  QAbstractProxyModel::setSourceModel(sourceModel);
}

//------------------------------------------------------------------------
QVariant TaxonomyProxy::data(const QModelIndex& proxyIndex, int role) const
{
  if (!proxyIndex.isValid())
    return QVariant();

  ModelItem *item = indexPtr(proxyIndex);
  switch (item->type())
  {
    case ModelItem::TAXONOMY:
    {
      if (Qt::DisplayRole == role)
      {
	TaxonomyNode *taxonomy = dynamic_cast<TaxonomyNode *>(item);
	int numSegs = numSegmentations(taxonomy);
	QString suffix = (numSegs>0)?QString(" (%1)").arg(numSegmentations(taxonomy)):QString();
	return item->data(role).toString() + suffix;
      } else
	return item->data(role);
    }
    case ModelItem::SEGMENTATION:
      if (Qt::DecorationRole == role)
      {
	Segmentation *seg = dynamic_cast<Segmentation *>(item);
	QPixmap segIcon(3,16);
	segIcon.fill(seg->data(role).value<QColor>());
	return segIcon;
      }else
	return item->data(role);
    default:
      Q_ASSERT(false);
  }

  return QAbstractProxyModel::data(proxyIndex, role);
}



//------------------------------------------------------------------------
bool TaxonomyProxy::hasChildren(const QModelIndex& parent) const
{
  return rowCount(parent) > 0 && columnCount(parent) > 0;
}


//------------------------------------------------------------------------
int TaxonomyProxy::rowCount(const QModelIndex& parent) const
{
  if (!parent.isValid())
    return m_rootTaxonomies.size();

  // Cast to base type
  ModelItem *parentItem = indexPtr(parent);
  int rows = 0;

  if (ModelItem::TAXONOMY == parentItem->type())
  {
    TaxonomyNode *taxonomy = dynamic_cast<TaxonomyNode *>(parentItem);
    rows = numTaxonomies(taxonomy) + numSegmentations(taxonomy);
  }

  return rows;
}

//------------------------------------------------------------------------
QModelIndex TaxonomyProxy::index(int row, int column, const QModelIndex& parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  if (!parent.isValid())
    return mapFromSource(m_model->index(row,column,m_model->taxonomyRoot()));

  ModelItem *parentItem = indexPtr(parent);
  Q_ASSERT(ModelItem::TAXONOMY == parentItem->type());
  TaxonomyNode *parentTaxonomy = dynamic_cast<TaxonomyNode *>(parentItem);
  Q_ASSERT(parentTaxonomy);

  int subTaxonomies = numTaxonomies(parentTaxonomy);
  if (row < subTaxonomies)
  {
    return mapFromSource(m_model->index(row, column, m_model->taxonomyIndex(parentTaxonomy)));
  }

  int segRow = row - subTaxonomies;
  Q_ASSERT(segRow < numSegmentations(parentTaxonomy));
  ModelItem *internalPtr = m_segmentations[parentTaxonomy][segRow];

  return createIndex(row, column, internalPtr);
}

//------------------------------------------------------------------------
QModelIndex TaxonomyProxy::parent(const QModelIndex& child) const
{
  if (!child.isValid())
    return QModelIndex();

  ModelItem *childItem = indexPtr(child);
  Q_ASSERT(childItem);

  QModelIndex parent;
  switch (childItem->type())
  {
    case ModelItem::TAXONOMY:
    {
      TaxonomyNode *childTaxonmy = dynamic_cast<TaxonomyNode *>(childItem);
      QModelIndex sourceIndex  = m_model->taxonomyIndex(childTaxonmy);
      QModelIndex sourceParent = m_model->parent(sourceIndex);
      parent = mapFromSource(sourceParent);
      break;
    }
    case ModelItem::SEGMENTATION:
    {
      foreach(TaxonomyNode *taxonomy, m_segmentations.keys())
      {
	if (m_segmentations[taxonomy].contains(childItem))
	{
	  parent = mapFromSource(m_model->taxonomyIndex(taxonomy));
	}
      }
      break;
    }
    default:
      Q_ASSERT(false);
  }

  return parent;
}

//------------------------------------------------------------------------
QModelIndex TaxonomyProxy::mapFromSource(const QModelIndex& sourceIndex) const
{
  if (!sourceIndex.isValid())
    return QModelIndex();

  if (sourceIndex == m_model->taxonomyRoot() ||
      sourceIndex == m_model->sampleRoot()   ||
      sourceIndex == m_model->channelRoot()  ||
      sourceIndex == m_model->filterRoot()   || 
      sourceIndex == m_model->segmentationRoot())
    return QModelIndex();

  QModelIndex proxyIndex;
  ModelItem *sourceItem = indexPtr(sourceIndex);
  switch(sourceItem->type())
  {
    case ModelItem::TAXONOMY:
    {
      //Taxonomies are shown in the same order than in the original model
      proxyIndex = createIndex(sourceIndex.row(), sourceIndex.column(), sourceIndex.internalPointer());
      break;
    }
    case ModelItem::SEGMENTATION:
    {
      Segmentation *seg = dynamic_cast<Segmentation *>(sourceItem);
      Q_ASSERT(seg);
      TaxonomyNode *taxonomy = seg->taxonomy();
      if (taxonomy)
      {
	int row = m_segmentations[taxonomy].indexOf(seg);
	if (row >= 0)
	{
	  row += numTaxonomies(taxonomy);
	  proxyIndex = createIndex(row, 0, sourceIndex.internalPointer());
	}
      }
      break;
    }
    default:
      proxyIndex = QModelIndex();
  }

  return proxyIndex;
}

//------------------------------------------------------------------------
QModelIndex TaxonomyProxy::mapToSource(const QModelIndex& proxyIndex) const
{
  if (!proxyIndex.isValid())
    return QModelIndex();

  ModelItem *proxyItem = indexPtr(proxyIndex);
  Q_ASSERT(proxyItem);

  QModelIndex sourceIndex;
  switch (proxyItem->type())
  {
    case ModelItem::TAXONOMY:
    {
      TaxonomyNode *proxyTaxonomy = dynamic_cast<TaxonomyNode *>(proxyItem);
      Q_ASSERT(proxyTaxonomy);
      sourceIndex = m_model->taxonomyIndex(proxyTaxonomy);
      break;
    }
    case ModelItem::SEGMENTATION:
    {
      Segmentation *proxySeg = dynamic_cast<Segmentation *>(proxyItem);
      Q_ASSERT(proxySeg);
      sourceIndex = m_model->segmentationIndex(proxySeg);
      break;
    }
    default:
      Q_ASSERT(false);
  }

  return sourceIndex;
}

//------------------------------------------------------------------------
Qt::ItemFlags TaxonomyProxy::flags(const QModelIndex& index) const
{
  if (!index.isValid())
    return QAbstractProxyModel::flags(index);

  Qt::ItemFlags f = QAbstractProxyModel::flags(index);
  ModelItem *sourceItem = indexPtr(index);
  if (ModelItem::TAXONOMY == sourceItem->type())
      f = f | Qt::ItemIsDropEnabled;
  else if (ModelItem::SEGMENTATION == sourceItem->type())
    f = f | Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled;

  return f;
}


//------------------------------------------------------------------------
bool TaxonomyProxy::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
  ModelItem *parentItem = indexPtr(parent);


  TaxonomyNode *selectedTaxonomy = NULL;
  if (ModelItem::TAXONOMY == parentItem->type())
  {
    selectedTaxonomy = dynamic_cast<TaxonomyNode *>(parentItem);
  } else if (ModelItem::SEGMENTATION == parentItem->type())
  {
    Segmentation *seg = dynamic_cast<Segmentation *>(parentItem);
    selectedTaxonomy = seg->taxonomy();
  }
  Q_ASSERT(selectedTaxonomy);

  // Recover dragged item information
  QByteArray encoded = data->data("application/x-qabstractitemmodeldatalist");
  QDataStream stream(&encoded, QIODevice::ReadOnly);

  QList<Segmentation *> draggedSegs;

  while (!stream.atEnd())
  {
    int row, col;
    QMap<int,  QVariant> roleDataMap;
    stream >> row >> col >> roleDataMap;

    QString segName = roleDataMap[Qt::DisplayRole].toString();
    Segmentation *seg = findSegmentation(segName);
    Q_ASSERT(seg);
    m_model->changeTaxonomy(seg, selectedTaxonomy);
  }

  return true;
}

//------------------------------------------------------------------------
int TaxonomyProxy::numSegmentations(QModelIndex taxIndex, bool recursive) const
{
  ModelItem *item = indexPtr(taxIndex);
  if (ModelItem::TAXONOMY != item->type())
    return 0;

  TaxonomyNode *taxonomy = dynamic_cast<TaxonomyNode *>(item);
  int total = numSegmentations(taxonomy);
  if (recursive)
  {
    int numTax = numTaxonomies(taxonomy);
    for (int subTax = 0; subTax < numTax; subTax++)
    {
      total += numSegmentations(index(subTax, 0, taxIndex), true);
    }
  }
  return total;
}

//------------------------------------------------------------------------
int TaxonomyProxy::numTaxonomies(QModelIndex taxIndex) const
{
  ModelItem *item = indexPtr(taxIndex);
  if (ModelItem::TAXONOMY != item->type())
    return 0;

  TaxonomyNode *taxonomy = dynamic_cast<TaxonomyNode *>(item);
  return numTaxonomies(taxonomy);
}

//------------------------------------------------------------------------
QModelIndexList TaxonomyProxy::segmentations(QModelIndex taxIndex, bool recursive) const
{
  QModelIndexList segs;

  int start = numTaxonomies(taxIndex);
  int end = start + numSegmentations(taxIndex) - 1;
  if (recursive)
  {
    for (int tax = 0; tax < start; tax++)
      segs << segmentations(index(tax, 0, taxIndex), true);
  }
  if (start <= end)
    segs << proxyIndices(taxIndex, start, end);

  return segs;
}


// QList<TaxonomyNode *> TaxonomyProxy::indexTaxonomies(int row, int column, const QModelIndex& parent)
// {
//   QList<TaxonomyNode *> res;
//   QModelIndex sourceIndex = m_model->index(row, 0, parent);
//   ModelItem *sourceItem = indexPtr(sourceIndex);
//   Q_ASSERT(ModelItem::TAXONOMY == sourceItem->type());
//   TaxonomyNode *taxonomy = dynamic_cast<TaxonomyNode *>(sourceItem);
//   Q_ASSERT(taxonomy);
//   res << taxonomy;
// 
//   int numChildren = m_model->rowCount(sourceIndex);
//   for (int r=0; r < numChildren; r++)
//   {
//     res << indexTaxonomies(r, 0, sourceIndex);
//   }
// 
//   return res();
// }

//------------------------------------------------------------------------
QModelIndexList TaxonomyProxy::sourceIndices(const QModelIndex& parent, int start, int end) const
{
  QModelIndexList res;
//   static int indent = 0;

//   QString tab = std::string(indent*2,' ').c_str();
//   qDebug() <<  tab << parent.data(Qt::DisplayRole).toString() << m_model->rowCount(parent) << start << end;
  for (int row = start; row <= end; row++)
  {
    QModelIndex sourceIndex = m_model->index(row, 0, parent);
//     qDebug() << tab << "  " << sourceIndex.data(Qt::DisplayRole).toString();
    res << sourceIndex;

//     indent++;
    int numChildren = m_model->rowCount(sourceIndex);
    if (numChildren > 0)
      res << sourceIndices(sourceIndex,0,numChildren - 1);
//     indent--;
  }

  return res;
}

//------------------------------------------------------------------------
QModelIndexList TaxonomyProxy::proxyIndices(const QModelIndex& parent, int start, int end) const
{
  QModelIndexList res;
  for (int row = start; row <= end; row++)
  {
    QModelIndex proxyIndex = index(row, 0, parent);
    res << proxyIndex;

    int numChildren = rowCount(proxyIndex);
    if (numChildren > 0)
      res << proxyIndices(proxyIndex,0,numChildren - 1);
  }

  return res;
}

//------------------------------------------------------------------------
void TaxonomyProxy::sourceRowsInserted(const QModelIndex& sourceParent, int start, int end)
{
  if (!sourceParent.isValid())
    return;

  if (sourceParent == m_model->sampleRoot() ||
    sourceParent == m_model->channelRoot()  ||
    sourceParent == m_model->filterRoot())
    return;

  if (sourceParent == m_model->taxonomyRoot())
  {
    beginInsertRows(QModelIndex(), start, end);
    for (int row = start; row <= end; row++)
    {
      ModelItem *sourceRow = indexPtr(m_model->index(row, 0, sourceParent));
      Q_ASSERT(ModelItem::TAXONOMY == sourceRow->type());
      TaxonomyNode *taxonomy = dynamic_cast<TaxonomyNode *>(sourceRow);
      Q_ASSERT(taxonomy);
      m_rootTaxonomies << taxonomy;
      m_numTaxonomies[taxonomy] = taxonomy->subElements().size();
//        m_taxonomies << indexTaxonomies(row, 0, sourceParent);
    }
    endInsertRows();
    return;
  }

  if (sourceParent == m_model->segmentationRoot())
  {
    QMap<TaxonomyNode *, QList<ModelItem *> > relations;
    for (int row=start; row <= end; row++)
    {
      QModelIndex sourceIndex = m_model->index(row, 0, sourceParent);
      ModelItem *sourceItem = indexPtr(sourceIndex);
      Q_ASSERT(ModelItem::SEGMENTATION == sourceItem->type());
      Segmentation *seg = dynamic_cast<Segmentation *>(sourceItem);
      TaxonomyNode *taxonomy = seg->taxonomy();
      if (taxonomy)
	relations[taxonomy] << sourceItem;
    }
    foreach(TaxonomyNode *taxonomy, relations.keys())
    {
      int numTaxs = numTaxonomies(taxonomy);
      int numSegs = numSegmentations(taxonomy);
      int startRow = numTaxs + numSegs;
      int endRow = startRow + relations[taxonomy].size() - 1;
      QModelIndex proxyTaxonomy = mapFromSource(m_model->taxonomyIndex(taxonomy));
      beginInsertRows(proxyTaxonomy, startRow, endRow);
      m_segmentations[taxonomy] << relations[taxonomy];
      endInsertRows();
    }
    return;
  }

  ModelItem *parentItem = indexPtr(sourceParent);
  if (ModelItem::TAXONOMY == parentItem->type())
  {
    //This is the same code than above, TODO create a function to simplify the code
    beginInsertRows(mapFromSource(sourceParent), start, end);
    for (int row = start; row <= end; row++)
    {
      ModelItem *sourceRow = indexPtr(m_model->index(row, 0, sourceParent));
      Q_ASSERT(ModelItem::TAXONOMY == sourceRow->type());
      TaxonomyNode *taxonomy = dynamic_cast<TaxonomyNode *>(sourceRow);
      Q_ASSERT(taxonomy);
      if (!taxonomy->parentNode()->name().isEmpty())
	m_numTaxonomies[taxonomy->parentNode()] += 1;
      m_numTaxonomies[taxonomy] = taxonomy->subElements().size();
    }
    endInsertRows();
  }
}

//------------------------------------------------------------------------
void TaxonomyProxy::sourceRowsAboutToBeRemoved(const QModelIndex& sourceParent, int start, int end)
{
  if (!sourceParent.isValid())
    return;

  if (sourceParent == m_model->sampleRoot() ||
    sourceParent == m_model->channelRoot()  ||
    sourceParent == m_model->filterRoot())
    return;

  if (sourceParent == m_model->segmentationRoot())
  {
    for (int row=start; row <= end; row++)
    {
      QModelIndex   sourceIndex = m_model->index(row, 0, sourceParent);
      QModelIndex   proxyIndex  = mapFromSource(sourceIndex);
      ModelItem    *item = indexPtr(sourceIndex);
      Segmentation *seg  = dynamic_cast<Segmentation *>(item);
      TaxonomyNode *taxonomy = seg->taxonomy();
      Q_ASSERT(taxonomy);
      int segRow = m_segmentations[taxonomy].indexOf(item);
      if (segRow >= 0)
      {
	int row = numTaxonomies(taxonomy) + segRow;
	beginRemoveRows(proxyIndex.parent(), row, row);
	m_segmentations[taxonomy].removeAt(segRow);
	endRemoveRows();
      }
    }
    return;
  }else
    // Handles taxonomyRoot and taxonomyNodes
//   if (sourceParent == m_model->taxonomyRoot())
  {
    beginRemoveRows(mapFromSource(sourceParent), start, end);
    for (int row=start; row <= end; row++)
    {
      ModelItem *item = indexPtr(m_model->index(row, 0, sourceParent));
      TaxonomyNode *taxonomy = dynamic_cast<TaxonomyNode *>(item);
      removeTaxonomy(taxonomy);
    }
    endRemoveRows();
    return;
  }
/* 
  QModelIndex sourceIndex = m_model->index(start, 0, sourceParent);
  QModelIndex proxyIndex  = mapFromSource(sourceIndex);
  ModelItem *item = indexPtr(sourceIndex);
  Q_ASSERT(ModelItem::TAXONOMY == item->type());
  beginRemoveRows(proxyIndex.parent(), proxyIndex.row(),proxyIndex.row());
  for (int row=start; row <= end; row++)
  {
    item = indexPtr(m_model->index(row, 0, sourceParent));
    TaxonomyNode *taxonomy = dynamic_cast<TaxonomyNode *>(item);
    removeTaxonomy(taxonomy);
  }
  endRemoveRows();*/
}

//------------------------------------------------------------------------
void TaxonomyProxy::sourceRowsRemoved(const QModelIndex& sourceParent, int start, int end)
{
}

//------------------------------------------------------------------------
bool TaxonomyProxy::indices(const QModelIndex& topLeft, const QModelIndex& bottomRight, QModelIndexList& result)
{
  result << topLeft;

  if (topLeft == bottomRight)
    return true;

  for (int r = 0; r < m_model->rowCount(topLeft); r++)
  {
    if (indices(topLeft.child(r, 0), bottomRight, result))
      return true;
  }

  for (int r = topLeft.row(); r < m_model->rowCount(topLeft.parent()); r++)
    if (indices(topLeft.sibling(r,0), bottomRight, result))
      return true;

  return false;
}

//------------------------------------------------------------------------
Segmentation* TaxonomyProxy::findSegmentation(QString name)
{
  foreach (TaxonomyNode *tax, m_segmentations.keys())
    foreach(ModelItem *seg, m_segmentations[tax])
      if (seg->data(Qt::DisplayRole) == name)
	return dynamic_cast<Segmentation *>(seg);

  return NULL;
}


//------------------------------------------------------------------------
void TaxonomyProxy::sourceDataChanged(const QModelIndex& sourceTopLeft, const QModelIndex& sourceBottomRight)
{
  QModelIndexList sources;

  indices(sourceTopLeft, sourceBottomRight, sources);

  foreach(QModelIndex source, sources)
  {
    bool indexChanged = true;
    if (source.parent() == m_model->segmentationRoot())
    {
      ModelItem *sourceItem = indexPtr(source);
      Segmentation *seg = dynamic_cast<Segmentation*>(sourceItem);
      TaxonomyNode *prevTaxonomy = NULL;
      foreach(TaxonomyNode *taxonomy, m_segmentations.keys())
      {
	if (m_segmentations[taxonomy].contains(sourceItem))
	{
	  indexChanged = taxonomy != seg->taxonomy();
	  prevTaxonomy = taxonomy;
	  break;
	}
      }
      if (prevTaxonomy && indexChanged)
      {
	QModelIndex source = mapFromSource(m_model->taxonomyIndex(prevTaxonomy));
	QModelIndex destination = mapFromSource(m_model->taxonomyIndex(seg->taxonomy()));
	int currentRow = numTaxonomies(prevTaxonomy) + m_segmentations[prevTaxonomy].indexOf(sourceItem);
	int newRow = rowCount(destination);
	beginMoveRows(source, currentRow, currentRow, destination, newRow);
	m_segmentations[prevTaxonomy].removeOne(sourceItem);
	m_segmentations[seg->taxonomy()] << sourceItem;
	endMoveRows();
      }
    }
    QModelIndex proxyIndex = mapFromSource(source);
    if (proxyIndex.isValid())
    {
      emit dataChanged(proxyIndex, proxyIndex);
    }
  }
}

//------------------------------------------------------------------------
bool idOrdered(Segmentation *seg1, Segmentation *seg2)
{
  return seg1->number() < seg2->number();
}

//------------------------------------------------------------------------
void TaxonomyProxy::removeTaxonomy(TaxonomyNode* taxonomy)
{
  // First remove its subtaxonomies
  foreach(TaxonomyNode *subTax, taxonomy->subElements())
  {
    removeTaxonomy(subTax);
  }

  m_rootTaxonomies.removeOne(taxonomy); //Safe even if it's not root taxonomy
  m_numTaxonomies.remove(taxonomy);
  m_segmentations.remove(taxonomy);
  if (!taxonomy->parentNode()->name().isEmpty())
    m_numTaxonomies[taxonomy->parentNode()] -= 1;
}

//------------------------------------------------------------------------
int TaxonomyProxy::numSegmentations(TaxonomyNode* taxonomy) const
{
  return m_segmentations[taxonomy].size();
}
//------------------------------------------------------------------------
int TaxonomyProxy::numTaxonomies(TaxonomyNode* taxonomy) const
{
  return m_numTaxonomies[taxonomy];
  // We can't rely on source's model to deal with row counting
//   int count = m_model->rowCount(m_model->taxonomyIndex(taxonomy));
//   Q_ASSERT(taxonomy->subElements().size() == count);//TODO: If never crashed replace model access with taxonomy method
//   return count;
}
