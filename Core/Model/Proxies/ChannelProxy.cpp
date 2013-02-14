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
void ChannelProxy::setSourceModel(EspinaModel *sourceModel)
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

  sourceRowsInserted(m_model->sampleRoot() , 0, m_model->rowCount(m_model->sampleRoot())  - 1);
  sourceRowsInserted(m_model->channelRoot(), 0, m_model->rowCount(m_model->channelRoot()) - 1);
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
        SamplePtr sample = samplePtr(item);
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
      break;
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
    SamplePtr sample = samplePtr(parentItem);
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

  return createIndex(row, column, internalPtr);
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
      break;
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
      ModelItemSList samples = channel->relatedItems(EspINA::IN, Channel::STAIN_LINK);
      if (samples.size() > 0)
      {
        SamplePtr sample = samplePtr(samples[0].data());
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
      break;
  }

  return proxyIndex;
}

//------------------------------------------------------------------------
QModelIndex ChannelProxy::mapToSource(const QModelIndex& proxyIndex) const
{
  if (!proxyIndex.isValid())
    return QModelIndex();

  ModelItemPtr proxyItem = indexPtr(proxyIndex);

  QModelIndex sourceIndex;
  switch (proxyItem->type())
  {
    case SAMPLE:
    {
      SamplePtr proxySample = samplePtr(proxyItem);
      sourceIndex = m_model->sampleIndex(proxySample);
      break;
    }
    case CHANNEL:
    {
      ChannelPtr proxyChannel = channelPtr(proxyItem);
      sourceIndex = m_model->channelIndex(proxyChannel);
      break;
    }
    default:
      Q_ASSERT(false);
      break;
  }

  return sourceIndex;
}

//------------------------------------------------------------------------
int ChannelProxy::numChannels(QModelIndex sampleIndex, bool recursive) const
{
  ModelItemPtr item = indexPtr(sampleIndex);
  if (SAMPLE != item->type())
    return 0;

  SamplePtr sample = samplePtr(item);
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

// In order to have relations between elements it is necessary
// to insert then first, thus we don't consider related items here
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
    {
      for (int row = start; row <= end; row++)
      {
        QModelIndex sampleIndex = m_model->index(row, 0, sourceParent);
        ModelItemPtr sourceRow = indexPtr(sampleIndex);
        Q_ASSERT(EspINA::SAMPLE == sourceRow->type());
        SamplePtr sample = samplePtr(sourceRow);
        m_samples << sample;
        ModelItemSList channels = sample->relatedItems(EspINA::OUT,
                                                       Channel::STAIN_LINK);
        if (!channels.isEmpty())
        {
          int start = 0;
          int end   = start + channels.size() - 1;
          beginInsertRows(sampleIndex, start, end);
          {
            foreach(ModelItemSPtr item, channels)
            {
              m_channels[sample] << item.data();
            }
          }
          endInsertRows();
        }
      }
    }
    endInsertRows();

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

  if (EspINA::SAMPLE == item->type())
  {
    beginRemoveRows(proxyIndex.parent(), start,end);
    SamplePtr sample = samplePtr(item);
    m_samples.removeOne(sample);
    m_channels.remove(sample);
    endRemoveRows();
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
void debugChannelSets(QString name, QSet<ModelItem *> set)
{
  qDebug() << name;
  foreach(ModelItem *item, set)
    qDebug() << item->data(Qt::DisplayRole).toString();
}

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
        ModelItemSList channels = sample->relatedItems(EspINA::OUT,
                                                       Channel::STAIN_LINK);
        ChannelSet prevChannels = m_channels[sample].toSet();
        // debugChannelSets("Previous Channels", prevChannels);
        ChannelSet currentChannels;
        foreach(ModelItemSPtr channel, channels)
        {
          currentChannels << channel.data();
        }
        // debugChannelSets("Current Channels", currentChannels);

        // We need to copy currentSegs to avoid emptying it
        ChannelSet newChannels = ChannelSet(currentChannels).subtract(prevChannels);
        // debugChannelSets("Channels to be added", newChannels);
        ChannelSet remChannels = ChannelSet(prevChannels).subtract(currentChannels);
        // debugChannelSets("Channels to be removed", remChannels);

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
          int end   = start + newChannels.size() - 1;
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
