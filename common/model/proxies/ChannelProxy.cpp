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

#include "ChannelProxy.h"

#include <common/EspinaCore.h>
#include <QPixmap>
#include <QSet>
#include <model/Channel.h>

typedef QSet<ModelItem *> ChannelSet;

//------------------------------------------------------------------------
ChannelProxy::ChannelProxy(QObject* parent)
: QAbstractProxyModel(parent)
{
}

//------------------------------------------------------------------------
ChannelProxy::~ChannelProxy()
{

}

//------------------------------------------------------------------------
void ChannelProxy::setSourceModel(EspinaModel* sourceModel)
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
QVariant ChannelProxy::data(const QModelIndex& proxyIndex, int role) const
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
	int numSegs = numChannels(sample);
	QString suffix = (numSegs>0)?QString(" (%1)").arg(numChannels(sample)):QString();
	return item->data(role).toString() + suffix;
      } else if (Qt::DecorationRole == role)
	return QColor(Qt::blue);
      else
	return item->data(role);
    }
    case ModelItem::CHANNEL:
      if (Qt::DecorationRole == role)
      {
	QPixmap channelIcon(3,16);
	channelIcon.fill(proxyIndex.parent().data(role).value<QColor>());
	return channelIcon;
      }else
	return item->data(role);
    default:
      Q_ASSERT(false);
  }

  return QAbstractProxyModel::data(proxyIndex, role);
}

//------------------------------------------------------------------------
int ChannelProxy::rowCount(const QModelIndex& parent) const
{
  if (!parent.isValid())
    return m_samples.size();

  // Cast to base type
  ModelItem *parentItem = indexPtr(parent);
  int rows = 0;
  if (ModelItem::SAMPLE == parentItem->type())
  {
    Sample *sample = dynamic_cast<Sample *>(parentItem);
    rows = numSubSamples(sample) + numChannels(sample);
  }
  return rows;
}

//------------------------------------------------------------------------
QModelIndex ChannelProxy::index(int row, int column, const QModelIndex& parent) const
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

  int channelRow = row - subSamples;
  Q_ASSERT(channelRow < numChannels(parentSample));
  ModelItem *internalPtr = m_channels[parentSample][channelRow];

  return createIndex(row, column, internalPtr);
}

//------------------------------------------------------------------------
QModelIndex ChannelProxy::parent(const QModelIndex& child) const
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
    case ModelItem::CHANNEL:
    {
      foreach(Sample *sample, m_channels.keys())
      {
	if (m_channels[sample].contains(childItem))
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
QModelIndex ChannelProxy::mapFromSource(const QModelIndex& sourceIndex) const
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
    case ModelItem::CHANNEL:
    {
      Channel *channel = dynamic_cast<Channel *>(sourceItem);
      Q_ASSERT(channel);
      ModelItem::Vector samples = channel->relatedItems(ModelItem::IN, "mark");
      if (samples.size() > 0)
      {
	Sample *sample = dynamic_cast<Sample *>(samples[0]);
	int row = m_channels[sample].indexOf(sourceItem);
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
QModelIndex ChannelProxy::mapToSource(const QModelIndex& proxyIndex) const
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
    case ModelItem::CHANNEL:
    {
      Channel *proxyChannel = dynamic_cast<Channel *>(proxyItem);
      Q_ASSERT(proxyChannel);
      sourceIndex = m_model->channelIndex(proxyChannel);
      break;
    }
    default:
      Q_ASSERT(false);
  }

  return sourceIndex;
}

//------------------------------------------------------------------------
int ChannelProxy::numChannels(QModelIndex sampleIndex, bool recursive) const
{
  ModelItem *item = indexPtr(sampleIndex);
  if (ModelItem::SAMPLE != item->type())
    return 0;

  Sample *sample = dynamic_cast<Sample *>(item);
  int total = numChannels(sample);
  return total;
}

//------------------------------------------------------------------------
int ChannelProxy::numSubSamples(QModelIndex sampleIndex) const
{
  ModelItem *item = indexPtr(sampleIndex);
  if (ModelItem::SAMPLE != item->type())
    return 0;

  Sample *sample = dynamic_cast<Sample *>(item);
  return numSubSamples(sample);
}

//------------------------------------------------------------------------
QModelIndexList ChannelProxy::channels(QModelIndex sampleIndex, bool recursive) const
{
  QModelIndexList res;

  int start = numSubSamples(sampleIndex);
  int end = start + numChannels(sampleIndex) - 1;
  if (recursive)
  {
    for (int tax = 0; tax < start; tax++)
      res << channels(index(tax, 0, sampleIndex), true);
  }
  if (start <= end)
    res << proxyIndices(sampleIndex, start, end);

  return res;
}

//------------------------------------------------------------------------
void ChannelProxy::sourceRowsInserted(const QModelIndex& sourceParent, int start, int end)
{
  if (!sourceParent.isValid())
    return;

  if (sourceParent == m_model->taxonomyRoot() ||
      sourceParent == m_model->segmentationRoot() ||
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

  if (sourceParent == m_model->channelRoot())
  {
    QMap<Sample *, QList<ModelItem *> > relations;
    for (int child=start; child <= end; child++)
    {
      QModelIndex sourceIndex = m_model->index(child, 0, sourceParent);
      ModelItem *sourceItem = indexPtr(sourceIndex);
      Q_ASSERT(ModelItem::CHANNEL == sourceItem->type());
      Channel *channel = dynamic_cast<Channel *>(sourceItem);
      Sample *sample = origin(channel);
      if (sample)
	relations[sample] << sourceItem;
    }
    foreach(Sample *sample, relations.keys())
    {
      int numSamples = numSubSamples(sample);
      int nChannels = numChannels(sample);
      int startRow = numSamples + nChannels;
      int endRow = startRow + relations[sample].size() - 1;
      QModelIndex proxySample = mapFromSource(m_model->sampleIndex(sample));
      beginInsertRows(proxySample, startRow, endRow);
      m_channels[sample] << relations[sample];
      endInsertRows();
    }
    return;
  }
}

//------------------------------------------------------------------------
void ChannelProxy::sourceRowsAboutToBeRemoved(const QModelIndex& sourceParent, int start, int end)
{
  if (!sourceParent.isValid())
    return;

  if (sourceParent == m_model->taxonomyRoot() ||
      sourceParent == m_model->segmentationRoot()  ||
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
      m_channels.remove(sample);
      endRemoveRows();
      break;
    }
    default:
      break;
  }
}


//------------------------------------------------------------------------
void ChannelProxy::sourceRowsRemoved(const QModelIndex& sourceParent, int start, int end)
{
}

//------------------------------------------------------------------------
bool ChannelProxy::indices(const QModelIndex& topLeft, const QModelIndex& bottomRight, QModelIndexList& result)
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
QModelIndexList ChannelProxy::proxyIndices(const QModelIndex& parent, int start, int end) const
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
// void debugSet(QString name, QSet<ModelItem *> set)
// {
//   qDebug() << name;
//   foreach(ModelItem *item, set)
//     qDebug() << item->data(Qt::DisplayRole).toString();
// }

//------------------------------------------------------------------------
void ChannelProxy::sourceDataChanged(const QModelIndex& sourceTopLeft, const QModelIndex& sourceBottomRight)
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
	ModelItem::Vector channels = sample->relatedItems(ModelItem::OUT, "mark");
	ChannelSet prevChannels = m_channels[sample].toSet();
// 	debugSet("Previous Channels", prevSegs);
	ChannelSet currentChannels = channels.toSet();
// 	debugSet("Current Channels", currentSegs);
	// We need to copy currentSegs to avoid emptying it
	ChannelSet newChannels = ChannelSet(currentChannels).subtract(prevChannels);
// 	debugSet("Channels to be added", newSegs);
	ChannelSet remChannels = ChannelSet(prevChannels).subtract(currentChannels);
// 	debugSet("Channels to be removed", remSegs);
	
	if (remChannels.size() > 0)
	{
	  foreach(ModelItem *seg, remChannels)
	  {
	    int row = m_channels[sample].indexOf(seg);
	    beginRemoveRows(proxyIndex, row, row);
	    m_channels[sample].removeAt(row);
	    endRemoveRows();
	  }
	}
	if (newChannels.size() > 0)
	{
	  int start = m_channels[sample].size();
	  int end = start + newChannels.size() - 1;
	  beginInsertRows(proxyIndex, start, end);
	  m_channels[sample] << newChannels.toList();
	  endInsertRows();
	}
      }
      emit dataChanged(proxyIndex, proxyIndex);
    }
  }
}

//------------------------------------------------------------------------
Sample* ChannelProxy::origin(Channel* channel) const
{
  Sample *sample = NULL;
  ModelItem::Vector samples = channel->relatedItems(ModelItem::IN, "mark");
  if (samples.size() > 0)
    sample = dynamic_cast<Sample *>(samples[0]);

  return sample;
}

//------------------------------------------------------------------------
int ChannelProxy::numChannels(Sample* sample) const
{
  return m_channels[sample].size();
}

//------------------------------------------------------------------------
int ChannelProxy::numSubSamples(Sample* sample) const
{
  return m_model->rowCount(m_model->sampleIndex(sample));
}