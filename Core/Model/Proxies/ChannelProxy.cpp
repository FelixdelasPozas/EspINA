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

// Espina
#include "Core/Model/Channel.h"
#include "Core/Model/EspinaModel.h"
#include "Core/Model/Sample.h"
#include "GUI/ViewManager.h"

// Qt
#include <QPixmap>
#include <QSet>
#include <QFont>

using namespace EspINA;

typedef QSet<ModelItemPtr> ChannelSet;

//------------------------------------------------------------------------
ChannelProxy::ChannelProxy(ViewManager *vm, QObject* parent)
: QAbstractProxyModel(parent)
, m_model(NULL)
, m_viewManager(vm)
{
}

//------------------------------------------------------------------------
ChannelProxy::~ChannelProxy()
{

}

//------------------------------------------------------------------------
void ChannelProxy::setSourceModel(EspinaModelPtr sourceModel)
{
  m_model = sourceModel;
  connect(sourceModel.data(), SIGNAL(rowsInserted(const QModelIndex&, int, int)),
          this, SLOT(sourceRowsInserted(const QModelIndex&, int, int)));
  connect(sourceModel.data(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
          this, SLOT(sourceRowsRemoved(QModelIndex, int, int)));
  connect(sourceModel.data(), SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
          this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex, int, int)));
  connect(sourceModel.data(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
          this, SLOT(sourceDataChanged(const QModelIndex &,const QModelIndex &)));
  QAbstractProxyModel::setSourceModel(sourceModel.data());
}

//------------------------------------------------------------------------
QVariant ChannelProxy::data(const QModelIndex& proxyIndex, int role) const
{
  if (!proxyIndex.isValid())
    return QVariant();

  ModelItemPtr item = indexPtr(proxyIndex);
  switch (item->type())
  {
    case SAMPLE:
    {
      if (Qt::DisplayRole == role)
      {
        SamplePtr sample = qSharedPointerDynamicCast<Sample>(item);
        int numSegs = numChannels(sample);
        QString suffix = (numSegs>0)?QString(" (%1)").arg(numChannels(sample)):QString();
        return item->data(role).toString() + suffix;
      } else if (Qt::DecorationRole == role)
        return QColor(Qt::blue);
      else
        return item->data(role);
    }
    case CHANNEL:
      if (Qt::DecorationRole == role)
      {
        QPixmap channelIcon(3,16);
        channelIcon.fill(proxyIndex.parent().data(role).value<QColor>());
        return channelIcon;
      }else if (Qt::FontRole == role)
      {
        QFont myFont;
        myFont.setBold(m_viewManager->activeChannel() == item);
        return myFont;
      }else
        return item->data(role);
    default:
      Q_ASSERT(false);
  }

  return QAbstractProxyModel::data(proxyIndex, role);
}

//------------------------------------------------------------------------
bool ChannelProxy::hasChildren(const QModelIndex& parent) const
{
  return rowCount(parent) > 0 && columnCount(parent) > 0;
}

//------------------------------------------------------------------------
int ChannelProxy::rowCount(const QModelIndex& parent) const
{
  if (!parent.isValid())
    return m_samples.size();

  // Cast to base type
  ModelItemPtr parentItem = indexPtr(parent);
  int rows = 0;
  if (SAMPLE == parentItem->type())
  {
    SamplePtr sample = qSharedPointerDynamicCast<Sample>(parentItem);
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

  ModelItemPtr parentItem = indexPtr(parent);
  Q_ASSERT(EspINA::SAMPLE == parentItem->type());
  SamplePtr parentSample = samplePtr(parentItem);

  int subSamples = numSubSamples(parentSample);
  if (row < subSamples)
    return mapFromSource(m_model->index(row, column, m_model->sampleIndex(parentSample)));

  int channelRow = row - subSamples;
  Q_ASSERT(channelRow < numChannels(parentSample));
  ModelItemPtr internalPtr = m_channels[parentSample][channelRow];

  return createIndex(row, column, &internalPtr);
}

//------------------------------------------------------------------------
QModelIndex ChannelProxy::parent(const QModelIndex& child) const
{
  if (!child.isValid())
    return QModelIndex();

  ModelItemPtr childItem = indexPtr(child);
  Q_ASSERT(childItem);

  QModelIndex parent;
  // Checks if Sample
  switch (childItem->type())
  {
    case EspINA::SAMPLE:
    {
      //TODO: Support nested samples
      parent = QModelIndex();
      break;
    }
    case EspINA::CHANNEL:
    {
      foreach(SamplePtr sample, m_channels.keys())
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

  ModelItemPtr sourceItem = indexPtr(sourceIndex);
  QModelIndex proxyIndex;
  switch (sourceItem->type())
  {
    case EspINA::SAMPLE:
    {
//       Sample *sample = dynamic_cast<Sample *>(sourceItem);
//       Q_ASSERT(sample);
      //Samples are shown in the same order than in the original model
      proxyIndex = createIndex(sourceIndex.row(),
                               sourceIndex.column(),
                               sourceIndex.internalPointer());
      break;
    }
    case EspINA::CHANNEL:
    {
      ChannelPtr channel = channelPtr(sourceItem);
      Q_ASSERT(channel);
      ModelItemList samples = channel->relatedItems(EspINA::IN, Channel::STAINLINK);
      if (samples.size() > 0)
      {
        SamplePtr sample = samplePtr(samples[0]);
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

  ModelItemPtr proxyItem = indexPtr(proxyIndex);
  Q_ASSERT(!proxyItem.isNull());

  QModelIndex sourceIndex;
  switch (proxyItem->type())
  {
    case SAMPLE:
    {
      SamplePtr proxySample = qSharedPointerDynamicCast<Sample>(proxyItem);
      Q_ASSERT(!proxySample.isNull());
      sourceIndex = m_model->sampleIndex(proxySample);
      break;
    }
    case CHANNEL:
    {
      ChannelPtr proxyChannel = qSharedPointerDynamicCast<Channel>(proxyItem);
      Q_ASSERT(!proxyChannel.isNull());
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
  ModelItemPtr item = indexPtr(sampleIndex);
  if (SAMPLE != item->type())
    return 0;

  SamplePtr sample = qSharedPointerDynamicCast<Sample>(item);
  int total = numChannels(sample);
  return total;
}

//------------------------------------------------------------------------
int ChannelProxy::numSubSamples(QModelIndex sampleIndex) const
{
  ModelItemPtr item = indexPtr(sampleIndex);
  if (EspINA::SAMPLE != item->type())
    return 0;

  SamplePtr sample = samplePtr(item);
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
      ModelItemPtr sourceRow = indexPtr(m_model->index(row, 0, sourceParent));
      Q_ASSERT(EspINA::SAMPLE == sourceRow->type());
      SamplePtr sample = samplePtr(sourceRow);
      m_samples << sample;
    }
    endInsertRows();
    return;
  }

  if (sourceParent == m_model->channelRoot())
  {
    QMap<SamplePtr, ModelItemList> relations;
    for (int child=start; child <= end; child++)
    {
      QModelIndex sourceIndex = m_model->index(child, 0, sourceParent);
      ModelItemPtr sourceItem = indexPtr(sourceIndex);
      Q_ASSERT(EspINA::CHANNEL == sourceItem->type());
      ChannelPtr channel = channelPtr(sourceItem);
      SamplePtr sample = channel->sample();
      if (sample)
        relations[sample] << sourceItem;
    }
    foreach(SamplePtr sample, relations.keys())
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
  ModelItemPtr item = indexPtr(sourceIndex);

  switch (item->type())
  {
    case EspINA::SAMPLE:
    {
      beginRemoveRows(proxyIndex.parent(), start,end);
      SamplePtr sample = samplePtr(item);
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
void ChannelProxy::sourceDataChanged(const QModelIndex& sourceTopLeft,
                                     const QModelIndex& sourceBottomRight)
{
  QModelIndexList sources;
  indices(sourceTopLeft, sourceBottomRight, sources);

  foreach(QModelIndex source, sources)
  {
    QModelIndex proxyIndex = mapFromSource(source);
    if (proxyIndex.isValid())
    {
      ModelItemPtr proxyItem = indexPtr(proxyIndex);
      if (EspINA::SAMPLE == proxyItem->type())
      {
        SamplePtr sample = samplePtr(proxyItem);
        ModelItemList channels = sample->relatedItems(EspINA::OUT,
                                                      Channel::STAINLINK);
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
          foreach(ModelItemPtr seg, remChannels)
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
int ChannelProxy::numChannels(SamplePtr sample) const
{
  return m_channels[sample].size();
}

//------------------------------------------------------------------------
int ChannelProxy::numSubSamples(SamplePtr sample) const
{
  return m_model->rowCount(m_model->sampleIndex(sample));
}