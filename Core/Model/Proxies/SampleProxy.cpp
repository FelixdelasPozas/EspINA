/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#include "SampleProxy.h"

// Espina
#include "Core/Model/EspinaModel.h"
#include "Core/Model/Sample.h"
#include "Core/Model/Segmentation.h"

// Qt
#include <QPixmap>
#include <QSet>

typedef QSet<ModelItem *> SegSet;

//------------------------------------------------------------------------
SampleProxy::SampleProxy(QObject* parent)
: QAbstractProxyModel(parent)
, m_model(NULL)
{
}

//------------------------------------------------------------------------
SampleProxy::~SampleProxy()
{
}

//------------------------------------------------------------------------
void SampleProxy::setSourceModel(EspinaModel* sourceModel)
{
  m_model = sourceModel;
  connect(sourceModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
          this, SLOT(sourceRowsInserted(const QModelIndex&, int, int)));
  connect(sourceModel, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
          this, SLOT(sourceRowsRemoved(QModelIndex, int, int)));
  connect(sourceModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
          this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex, int, int)));
  connect(sourceModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
          this, SLOT(sourceDataChanged(const QModelIndex &,const QModelIndex &)));
  QAbstractProxyModel::setSourceModel(sourceModel);
}

//------------------------------------------------------------------------
QVariant SampleProxy::data(const QModelIndex& proxyIndex, int role) const
{
  if (!proxyIndex.isValid())
    return QVariant();

  ModelItem *item = indexPtr(proxyIndex);
  switch (item->type())
  {
    case ModelItem::SAMPLE:
    {
      if (Qt::DisplayRole == role)
      {
	Sample *sample = dynamic_cast<Sample *>(item);
	int numSegs = numSegmentations(sample);
	QString suffix = (numSegs>0)?QString(" (%1)").arg(numSegmentations(sample)):QString();
	return item->data(role).toString() + suffix;
      } else if (Qt::DecorationRole == role)
	return QColor(Qt::blue);
      else
	return item->data(role);
    }
    case ModelItem::SEGMENTATION:
      if (Qt::DecorationRole == role)
      {
	QPixmap segIcon(3,16);
	segIcon.fill(proxyIndex.parent().data(role).value<QColor>());
	return segIcon;
      }else
	return item->data(role);
    default:
      Q_ASSERT(false);
  }

  return QAbstractProxyModel::data(proxyIndex, role);
}

//------------------------------------------------------------------------
bool SampleProxy::hasChildren(const QModelIndex& parent) const
{
  return rowCount(parent) > 0 && columnCount(parent) > 0;
}


//------------------------------------------------------------------------
int SampleProxy::rowCount(const QModelIndex& parent) const
{
  if (!parent.isValid())
    return m_samples.size();

  // Cast to base type
  ModelItem *parentItem = indexPtr(parent);
  int rows = 0;
  if (ModelItem::SAMPLE == parentItem->type())
  {
    Sample *sample = dynamic_cast<Sample *>(parentItem);
    rows = numSubSamples(sample) + numSegmentations(sample);
  }
  return rows;
}

//------------------------------------------------------------------------
QModelIndex SampleProxy::index(int row, int column, const QModelIndex& parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  if (!parent.isValid())
    return mapFromSource(m_model->index(row, column, m_model->sampleRoot()));

  ModelItem *parentItem = indexPtr(parent);
  Q_ASSERT(parentItem->type() == ModelItem::SAMPLE);
  Sample * parentSample = dynamic_cast<Sample *>(parentItem);
  Q_ASSERT(parentSample);

  int subSamples = numSubSamples(parentSample);
  if (row < subSamples)
    return mapFromSource(m_model->index(row, column, m_model->sampleIndex(parentSample)));

  int segRow = row - subSamples;
  Q_ASSERT(segRow < numSegmentations(parentSample));
  ModelItem *internalPtr = m_segmentations[parentSample][segRow];

  return createIndex(row, column, internalPtr);
}

//------------------------------------------------------------------------
QModelIndex SampleProxy::parent(const QModelIndex& child) const
{
  if (!child.isValid())
    return QModelIndex();

  ModelItem *childItem = indexPtr(child);
  Q_ASSERT(childItem);

  QModelIndex parent;
  // Checks if Sample
  switch (childItem->type())
  {
    case ModelItem::SAMPLE:
    {
      //TODO: Support nested samples
      parent = QModelIndex();
      break;
    }
    case ModelItem::SEGMENTATION:
    {
      foreach(Sample *sample, m_segmentations.keys())
      {
	if (m_segmentations[sample].contains(childItem))
	  parent = mapFromSource(m_model->sampleIndex(sample));
      }
      break;
    }
    default:
      Q_ASSERT(false);
  }

  return parent;
}

//------------------------------------------------------------------------
QModelIndex SampleProxy::mapFromSource(const QModelIndex& sourceIndex) const
{

  if (!sourceIndex.isValid())
    return QModelIndex();

  if (sourceIndex == m_model->taxonomyRoot() ||
      sourceIndex == m_model->sampleRoot()   ||
      sourceIndex == m_model->channelRoot()  ||
      sourceIndex == m_model->filterRoot()   ||
      sourceIndex == m_model->segmentationRoot())
    return QModelIndex();

  ModelItem *sourceItem = indexPtr(sourceIndex);
  QModelIndex proxyIndex;
  switch (sourceItem->type())
  {
    case ModelItem::SAMPLE:
    {
//       Sample *sample = dynamic_cast<Sample *>(sourceItem);
//       Q_ASSERT(sample);
      //Samples are shown in the same order than in the original model
      proxyIndex = createIndex(sourceIndex.row(), sourceIndex.column(), sourceIndex.internalPointer());
      break;
    }
    case ModelItem::SEGMENTATION:
    {
      Segmentation *seg = dynamic_cast<Segmentation *>(sourceItem);
      Q_ASSERT(seg);
      ModelItem::Vector samples = seg->relatedItems(ModelItem::IN, "where");
      if (samples.size() > 0)
      {
	Sample *sample = dynamic_cast<Sample *>(samples[0]);
	int row = m_segmentations[sample].indexOf(sourceItem);
	if (row >= 0)
	{
	  row += numSubSamples(sample);
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
QModelIndex SampleProxy::mapToSource(const QModelIndex& proxyIndex) const
{
  if (!proxyIndex.isValid())
    return QModelIndex();

  ModelItem *proxyItem = indexPtr(proxyIndex);
  Q_ASSERT(proxyItem);

  QModelIndex sourceIndex;
  switch (proxyItem->type())
  {
    case ModelItem::SAMPLE:
    {
      Sample *proxySample = dynamic_cast<Sample *>(proxyItem);
      Q_ASSERT(proxySample);
      sourceIndex = m_model->sampleIndex(proxySample);
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
int SampleProxy::numSegmentations(QModelIndex sampleIndex, bool recursive) const
{
  ModelItem *item = indexPtr(sampleIndex);
  if (ModelItem::SAMPLE != item->type())
    return 0;

  Sample *sample = dynamic_cast<Sample *>(item);
  int total = numSegmentations(sample);
  return total;
}

//------------------------------------------------------------------------
int SampleProxy::numSubSamples(QModelIndex sampleIndex) const
{
  ModelItem *item = indexPtr(sampleIndex);
  if (ModelItem::SAMPLE != item->type())
    return 0;

  Sample *sample = dynamic_cast<Sample *>(item);
  return numSubSamples(sample);
}

//------------------------------------------------------------------------
QModelIndexList SampleProxy::segmentations(QModelIndex sampleIndex, bool recursive) const
{
  QModelIndexList segs;

  int start = numSubSamples(sampleIndex);
  int end = start + numSegmentations(sampleIndex) - 1;
  if (recursive)
  {
    for (int tax = 0; tax < start; tax++)
      segs << segmentations(index(tax, 0, sampleIndex), true);
  }
  if (start <= end)
    segs << proxyIndices(sampleIndex, start, end);

  return segs;
}

//------------------------------------------------------------------------
void SampleProxy::sourceRowsInserted(const QModelIndex& sourceParent, int start, int end)
{
  if (!sourceParent.isValid())
    return;

  if (sourceParent == m_model->taxonomyRoot() ||
      sourceParent == m_model->channelRoot()  ||
      sourceParent == m_model->filterRoot())
    return;

  if (sourceParent == m_model->sampleRoot())
  {
    beginInsertRows(QModelIndex(), start, end);
    for (int row = start; row <= end; row++)
    {
      ModelItem *sourceRow = indexPtr(m_model->index(row, 0, sourceParent));
      Q_ASSERT(ModelItem::SAMPLE == sourceRow->type());
      Sample *sample = dynamic_cast<Sample *>(sourceRow);
      Q_ASSERT(sample);
      m_samples << sample;
    }
    endInsertRows();
    return;
  }

  if (sourceParent == m_model->segmentationRoot())
  {
    QMap<Sample *, QList<ModelItem *> > relations;
    for (int child=start; child <= end; child++)
    {
      QModelIndex sourceIndex = m_model->index(child, 0, sourceParent);
      ModelItem *sourceItem = indexPtr(sourceIndex);
      Q_ASSERT(ModelItem::SEGMENTATION == sourceItem->type());
      Segmentation *seg = dynamic_cast<Segmentation *>(sourceItem);
      Sample *sample = origin(seg);
      if (sample)
	relations[sample] << sourceItem;
    }
    foreach(Sample *sample, relations.keys())
    {
      int numSamples = numSubSamples(sample);
      int numSegs = numSegmentations(sample);
      int startRow = numSamples + numSegs;
      int endRow = startRow + relations[sample].size() - 1;
      QModelIndex proxySample = mapFromSource(m_model->sampleIndex(sample));
      beginInsertRows(proxySample, startRow, endRow);
      m_segmentations[sample] << relations[sample];
      endInsertRows();
    }
    return;
  }

//   ModelItem *sourceItem = indexPtr(sourceParent);
//   if (sourceItem->type() != ModelItem::SAMPLE)
//     return;
}

//------------------------------------------------------------------------
void SampleProxy::sourceRowsAboutToBeRemoved(const QModelIndex& sourceParent, int start, int end)
{
  if (!sourceParent.isValid())
    return;

  if (sourceParent == m_model->taxonomyRoot() ||
      sourceParent == m_model->channelRoot()  ||
      sourceParent == m_model->filterRoot())
    return;

  QModelIndex sourceIndex = m_model->index(start, 0, sourceParent);
  QModelIndex proxyIndex = mapFromSource(sourceIndex);
  ModelItem *item = indexPtr(sourceIndex);

  switch (item->type())
  {
    case ModelItem::SAMPLE:
    {
      beginRemoveRows(proxyIndex.parent(), start,end);
      Sample *sample = dynamic_cast<Sample *>(item);
      m_samples.removeOne(sample);
      m_segmentations.remove(sample);
      endRemoveRows();
      break;
    }
//     case ModelItem::SEGMENTATION:
//     {
//       // Need to find its parent before deletion
//       Segmentation *seg = dynamic_cast<Segmentation *>(item);
//       Sample *sample = origin(seg);
//       int row = m_segmentations[sample].indexOf(item);
//       if (row >= 0)
//       {
// 	beginRemoveRows(proxyIndex.parent(), row, row);
// 	m_segmentations[sample].removeAt(row);
// 	endRemoveRows();
//       }
//       break;
//     }
    default:
      // Ignore taxonomy nodes
      break;
  }
}


//------------------------------------------------------------------------
void SampleProxy::sourceRowsRemoved(const QModelIndex& sourceParent, int start, int end)
{
//   QModelIndex sourceIndex = m_model->index(start, 0, sourceParent);
// 
//   if (!sourceIndex.isValid())
//     return;
// 
//   ModelItem *item = indexPtr(sourceIndex);
// 
//   switch (item->type())
//   {
//     case ModelItem::SAMPLE:
//     {
// //       Sample *sample = dynamic_cast<Sample *>(item);
// //       m_samples.removeOne(sample);
// //       m_segmentations.remove(sample);
// //       endRemoveRows();
//       break;
//     }
//     case ModelItem::SEGMENTATION:
//     {
//       Segmentation *seg = dynamic_cast<Segmentation *>(item);
//       Sample *sample = origin(seg);
//       if (sample)
// 	m_segmentations[sample].removeOne(seg);
//       endRemoveRows();
//       break;
//     }
//     default:
//       // ignore other elements
//       break;
//   }
}

//------------------------------------------------------------------------
bool SampleProxy::indices(const QModelIndex& topLeft, const QModelIndex& bottomRight, QModelIndexList& result)
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

// //------------------------------------------------------------------------
// QModelIndexList SampleProxy::indices(const QModelIndex &parent, int start, int end)
// {
//   QModelIndexList res;
// //   static int indent = 0;
// 
// //   QString tab = std::string(indent*2,' ').c_str();
// //   qDebug() <<  tab << parent.data(Qt::DisplayRole).toString() << m_model->rowCount(parent) << start << end;
//   for (int row = start; row <= end; row++)
//   {
//     QModelIndex sourceIndex = m_model->index(row, 0, parent);
// //     qDebug() << tab << "  " << sourceIndex.data(Qt::DisplayRole).toString();
//     res << sourceIndex;
// 
// //     indent++;
//     int numChildren = m_model->rowCount(sourceIndex);
//     if (numChildren > 0)
//       res << indices(sourceIndex,0,numChildren - 1);
// //     indent--;
//   }
// 
//   return res;
// }

//------------------------------------------------------------------------
QModelIndexList SampleProxy::proxyIndices(const QModelIndex& parent, int start, int end) const
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
void debugSet(QString name, QSet<ModelItem *> set)
{
  qDebug() << name;
  foreach(ModelItem *item, set)
    qDebug() << item->data(Qt::DisplayRole).toString();
}

//------------------------------------------------------------------------
void SampleProxy::sourceDataChanged(const QModelIndex& sourceTopLeft, const QModelIndex& sourceBottomRight)
{
  QModelIndexList sources;
  indices(sourceTopLeft, sourceBottomRight, sources);

  foreach(QModelIndex source, sources)
  {
    QModelIndex proxyIndex = mapFromSource(source);
    if (proxyIndex.isValid())
    {
      ModelItem *proxyItem = indexPtr(proxyIndex);
      if (ModelItem::SAMPLE == proxyItem->type())
      {
	Sample *sample = dynamic_cast<Sample *>(proxyItem);
	ModelItem::Vector segs = sample->relatedItems(ModelItem::OUT, "where");
	SegSet prevSegs = m_segmentations[sample].toSet();
// 	debugSet("Previous Segmentations", prevSegs);
	SegSet currentSegs = segs.toSet();
// 	debugSet("Current Segmentations", currentSegs);
	// We need to copy currentSegs to avoid emptying it
	SegSet newSegs = SegSet(currentSegs).subtract(prevSegs);
// 	debugSet("Segmentations to be added", newSegs);
	SegSet remSegs = SegSet(prevSegs).subtract(currentSegs);
// 	debugSet("Segmentations to be removed", remSegs);
	
	if (remSegs.size() > 0)
	{
	  foreach(ModelItem *seg, remSegs)
	  {
	    int row = m_segmentations[sample].indexOf(seg);
	    beginRemoveRows(proxyIndex, row, row);
	    m_segmentations[sample].removeAt(row);
	    endRemoveRows();
	  }
	}
	if (newSegs.size() > 0)
	{
	  int start = m_segmentations[sample].size();
	  int end = start + newSegs.size() - 1;
	  beginInsertRows(proxyIndex, start, end);
	  m_segmentations[sample] << newSegs.toList();
	  endInsertRows();
	}
      }
      emit dataChanged(proxyIndex, proxyIndex);
    }
  }
}



//------------------------------------------------------------------------
void SampleProxy::updateSegmentations() const
{
//   m_segmentations.clear();
//   int rows = m_model->rowCount(m_model->segmentationRoot());
//   for (int row = 0; row < rows; row++)
//   {
//     QModelIndex segIndex = m_model->index(row, 0, m_model->segmentationRoot());
//     ModelItem *segItem = indexPtr(segIndex);
//     Q_ASSERT(segItem);
//     Segmentation *seg = dynamic_cast<Segmentation *>(segItem);
//     Q_ASSERT(seg);
//     ModelItem::Vector samples = segItem->relatedItems(ModelItem::IN, "where");
//     if (samples.size() > 0)
//     {
//       Sample *sample = dynamic_cast<Sample *>(samples[0]);
//       m_segmentations[sample].push_back(seg);
//     }
//   }
}

//------------------------------------------------------------------------
Sample* SampleProxy::origin(Segmentation* seg) const
{
  Sample *sample = NULL;
  ModelItem::Vector samples = seg->relatedItems(ModelItem::IN, "where");
  if (samples.size() > 0)
    sample = dynamic_cast<Sample *>(samples[0]);

  return sample;
}

//------------------------------------------------------------------------
int SampleProxy::numSegmentations(Sample* sample) const
{
  return m_segmentations[sample].size();
}

//------------------------------------------------------------------------
int SampleProxy::numSubSamples(Sample* sample) const
{
  return m_model->rowCount(m_model->sampleIndex(sample));
}