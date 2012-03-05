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
#include <EspinaCore.h>
#include <QPixmap>


SampleProxy::SampleProxy(QObject* parent)
    : QAbstractProxyModel(parent)
{
}

SampleProxy::~SampleProxy()
{

}

//------------------------------------------------------------------------
void SampleProxy::setSourceModel(EspinaModel* sourceModel)
{
  QAbstractProxyModel::setSourceModel(sourceModel);
  m_model = sourceModel;
  updateSegmentations();
  connect(sourceModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
          this, SLOT(sourceRowsInserted(const QModelIndex&, int, int)));
  connect(sourceModel, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
          this, SLOT(sourceRowsRemoved(QModelIndex, int, int)));
  connect(sourceModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
          this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex, int, int)));
  connect(sourceModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
          this, SLOT(sourceDataChanged(const QModelIndex &,const QModelIndex &)));
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
	return item->data(role).toString() + QString(" (%1)").arg(numberOfSegmentations(sample));
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
    rows = numberOfSubSamples(sample) + numberOfSegmentations(sample);
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

  int subSamples = numberOfSubSamples(parentSample);
  if (row < subSamples)
    return mapFromSource(m_model->index(row, column, m_model->sampleIndex(parentSample)));

  int segRow = row - subSamples;
  Q_ASSERT(segRow < numberOfSegmentations(parentSample));
  ModelItem *internalPtr = m_sampleSegs[parentSample][segRow];

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
      Segmentation *seg = dynamic_cast<Segmentation *>(childItem);
      Q_ASSERT(seg);
      parent = mapFromSource(m_model->sampleIndex(origin(seg)));
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

  if (sourceIndex == m_model->taxonomyRoot())
    return QModelIndex();

  if (sourceIndex == m_model->sampleRoot())
    return QModelIndex();

  if (sourceIndex == m_model->channelRoot())
    return QModelIndex();

  if (sourceIndex == m_model->filterRoot())
    return QModelIndex();

  if (sourceIndex == m_model->segmentationRoot())
    return QModelIndex();


  ModelItem *sourceItem = indexPtr(sourceIndex);
  QModelIndex proxyIndex;
  switch (sourceItem->type())
  {
    case ModelItem::SAMPLE:
    {
      Sample *sample = dynamic_cast<Sample *>(sourceItem);
      Q_ASSERT(sample);
      //Samples are shown in the same order than in the original model
      proxyIndex = createIndex(sourceIndex.row(), sourceIndex.column(), sourceIndex.internalPointer());
      break;
    }
    case ModelItem::CHANNEL:
      break;
    case ModelItem::SEGMENTATION:
    {
      //Segmentations
      Segmentation *seg = dynamic_cast<Segmentation *>(sourceItem);
      Q_ASSERT(seg);
      ModelItem::Vector samples = seg->relatedItems(ModelItem::IN, "where");
      if (samples.size() > 0)
      {
	Sample *sample = dynamic_cast<Sample *>(samples[0]);
	int row = m_sampleSegs[sample].indexOf(seg);
	if (row >= 0)
	  proxyIndex = createIndex(row, 0, sourceIndex.internalPointer());
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
    ModelItem *sourceRow = indexPtr(m_model->index(start, 0, sourceParent));
    Q_ASSERT(ModelItem::SAMPLE == sourceRow->type());
    Sample *sample = dynamic_cast<Sample *>(sourceRow);
    Q_ASSERT(sample);
    m_samples << sample;
    endInsertRows();
    return;
  }

  if (sourceParent == m_model->segmentationRoot())
  {
    QMap<Sample *, QList<Segmentation *> > relations;
    for (int child=start; child <= end; child++)
    {
      QModelIndex sourceIndex = m_model->index(child, 0, sourceParent);
      ModelItem *sourceItem = indexPtr(sourceIndex);
      Q_ASSERT(ModelItem::SEGMENTATION == sourceItem->type());
      Segmentation *seg = dynamic_cast<Segmentation *>(sourceItem);
      Sample *sample = origin(seg);
      if (sample)
	relations[sample] << seg;
    }
    foreach(Sample *sample, relations.keys())
    {
      int numSubSamples = numberOfSubSamples(sample);
      int numSegs = numberOfSegmentations(sample);
      int startRow = numSubSamples + numSegs;
      int endRow = startRow + relations[sample].size();
      QModelIndex sourceSample = mapFromSource(m_model->sampleIndex(sample));
      beginInsertRows(sourceSample, startRow, endRow);
      m_sampleSegs[sample] << relations[sample];
      endInsertRows();
    }
    return;
  }

  ModelItem *sourceItem = indexPtr(sourceParent);
  if (sourceItem->type() != ModelItem::SAMPLE)
    return;
}

//------------------------------------------------------------------------
void SampleProxy::sourceRowsAboutToBeRemoved(const QModelIndex& sourceParent, int start, int end)
{
/*
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());

  if (sourceParent == model->sampleRoot())
  {
    beginRemoveRows(mapFromSource(sourceParent),start,end);
  } else if (sourceParent == model->segmentationRoot())
  {
    assert(start == end);//TODO: Add support for multiple deletions
    // Need to find its parent before deletion
    QModelIndex sourceIndex = model->index(start, 0, sourceParent);
    IModelItem *sourceItem = static_cast<IModelItem *>(sourceIndex.internalPointer());
    Segmentation *sourceSeg = dynamic_cast<Segmentation *>(sourceItem);
    Sample *segParent = sourceSeg->origin();
    int row = m_sampleSegs[segParent].indexOf(sourceSeg);
    QModelIndex proxyIndex = mapFromSource(sourceIndex);
    beginRemoveRows(proxyIndex.parent(),row,row);
  }
*/
}


//------------------------------------------------------------------------
void SampleProxy::sourceRowsRemoved(const QModelIndex& sourceParent, int start, int end)
{
/*
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());

  if (sourceParent == model->sampleRoot())
    endRemoveRows();
  
  if (sourceParent == model->segmentationRoot())
  {
    updateSegmentations();
    endRemoveRows();
  }
*/
}

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
	QList<Segmentation *> newSegmentations;
	foreach(ModelItem *output, segs)
	{
	  Q_ASSERT(ModelItem::SEGMENTATION == output->type());
	  Segmentation *seg = dynamic_cast<Segmentation *>(output);
	  if (!m_sampleSegs[sample].contains(seg))
	    newSegmentations << seg;
	}
	int start = m_sampleSegs[sample].size();
	int end = start + newSegmentations.size() - 1;
	beginInsertRows(proxyIndex, start, end);
	m_sampleSegs[sample] << newSegmentations;
	endInsertRows();
      }
      emit dataChanged(proxyIndex, proxyIndex);
    }
  }
}



//------------------------------------------------------------------------
void SampleProxy::updateSegmentations() const
{
  m_sampleSegs.clear();
  int rows = m_model->rowCount(m_model->segmentationRoot());
  for (int row = 0; row < rows; row++)
  {
    QModelIndex segIndex = m_model->index(row, 0, m_model->segmentationRoot());
    ModelItem *segItem = indexPtr(segIndex);
    Q_ASSERT(segItem);
    Segmentation *seg = dynamic_cast<Segmentation *>(segItem);
    Q_ASSERT(seg);
    ModelItem::Vector samples = segItem->relatedItems(ModelItem::IN, "where");
    if (samples.size() > 0)
    {
      Sample *sample = dynamic_cast<Sample *>(samples[0]);
      m_sampleSegs[sample].push_back(seg);
    }
  }
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
int SampleProxy::numberOfSegmentations(Sample* sample) const
{
  return m_sampleSegs[sample].size();
}

//------------------------------------------------------------------------
int SampleProxy::numberOfSubSamples(Sample* sample) const
{
  return m_model->rowCount(m_model->sampleIndex(sample));
}