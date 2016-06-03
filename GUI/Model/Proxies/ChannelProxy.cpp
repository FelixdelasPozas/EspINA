/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include "ChannelProxy.h"
#include <GUI/Model/SampleAdapter.h>
#include <GUI/Model/ChannelAdapter.h>
#include <Core/Analysis/Channel.h>
#include <Core/Utils/EspinaException.h>
#include <GUI/Model/Utils/QueryAdapter.h>

// Qt
#include <QPixmap>
#include <QSet>
#include <QMimeData>
#include <QFont>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;

using ChannelSet = QSet<ItemAdapterPtr>;

//------------------------------------------------------------------------
ChannelProxy::ChannelProxy(ModelAdapterSPtr sourceModel, QObject* parent)
: QAbstractProxyModel{parent}
, m_activeChannel    {nullptr}
{
  setSourceModel(sourceModel);
}

//------------------------------------------------------------------------
ChannelProxy::~ChannelProxy()
{
}

//------------------------------------------------------------------------
void ChannelProxy::setSourceModel(ModelAdapterSPtr sourceModel)
{
  if (m_model)
  {
    disconnect(m_model.get(), SIGNAL(rowsInserted(const QModelIndex&, int, int)),
               this,          SLOT(sourceRowsInserted(const QModelIndex&, int, int)));
    disconnect(m_model.get(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
               this,          SLOT(sourceRowsRemoved(QModelIndex, int, int)));
    disconnect(m_model.get(), SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
               this,          SLOT(sourceRowsAboutToBeRemoved(QModelIndex, int, int)));
    disconnect(m_model.get(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
               this,          SLOT(sourceDataChanged(const QModelIndex &,const QModelIndex &)));
    disconnect(m_model.get(), SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
               this,          SLOT(sourceRowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));
    disconnect(m_model.get(), SIGNAL(modelAboutToBeReset()),
               this,          SLOT(sourceModelReset()));
  }

  m_model = sourceModel;

  if (m_model)
  {
    connect(m_model.get(), SIGNAL(rowsInserted(const QModelIndex&, int, int)),
            this,          SLOT(sourceRowsInserted(const QModelIndex&, int, int)));
    connect(m_model.get(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
            this,          SLOT(sourceRowsRemoved(QModelIndex, int, int)));
    connect(m_model.get(), SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
            this,          SLOT(sourceRowsAboutToBeRemoved(QModelIndex, int, int)));
    connect(m_model.get(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
            this,          SLOT(sourceDataChanged(const QModelIndex &,const QModelIndex &)));
    connect(m_model.get(), SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
            this,          SLOT(sourceRowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));
    connect(m_model.get(), SIGNAL(modelAboutToBeReset()),
            this,          SLOT(sourceModelReset()));

    QAbstractProxyModel::setSourceModel(sourceModel.get());

    m_activeChannel = nullptr;
    m_channels.clear();
    m_samples.clear();

    sourceRowsInserted(m_model->sampleRoot() , 0, m_model->rowCount(m_model->sampleRoot())  - 1);
    sourceRowsInserted(m_model->channelRoot(), 0, m_model->rowCount(m_model->channelRoot()) - 1);
  }
  else
  {
    reset();
  }
}

//------------------------------------------------------------------------
QVariant ChannelProxy::data(const QModelIndex& proxyIndex, int role) const
{
  if (!proxyIndex.isValid()) return QVariant();

  auto item = itemAdapter(proxyIndex);

  if (isSample(item))
  {
    if (Qt::DisplayRole == role)
    {
      auto sample = samplePtr(item);
      auto numSegs = numChannels(sample);
      auto suffix = (numSegs > 0) ? QString(" (%1)").arg(numChannels(sample)) : QString();

      return item->data(role).toString() + suffix;
    }
    else
    {
      if (Qt::DecorationRole == role)
      {
        return QColor(Qt::blue);
      }
      else
      {
        return item->data(role);
      }
    }
  }

  if (isChannel(item))
  {
    if (Qt::DecorationRole == role)
    {
      return channelPtr(item)->data(role);;
    }
    else
    {
      if (Qt::FontRole == role)
      {
        QFont myFont;
        myFont.setBold(m_activeChannel == item);
        return myFont;
      }
      else
      {
        return item->data(role);
      }
    }
  }

  return QVariant();
}

//------------------------------------------------------------------------
bool ChannelProxy::hasChildren(const QModelIndex& parent) const
{
  return rowCount(parent) > 0 && columnCount(parent) > 0;
}

//------------------------------------------------------------------------
int ChannelProxy::rowCount(const QModelIndex& parent) const
{
  if (!parent.isValid()) return m_samples.size();

  // Cast to base type
  auto parentItem = itemAdapter(parent);
  int rows = 0;
  if (isSample(parentItem))
  {
    auto sample = samplePtr(parentItem);
    rows = numSubSamples(sample) + numChannels(sample);
  }
  return rows;
}

//------------------------------------------------------------------------
QModelIndex ChannelProxy::index(int row, int column, const QModelIndex& parent) const
{
  if (!hasIndex(row, column, parent)) return QModelIndex();

  if (!parent.isValid())
  {
    return mapFromSource(m_model->index(row, column, m_model->sampleRoot()));
  }

  auto parentItem = itemAdapter(parent);

  if(!parentItem) return QModelIndex();

  Q_ASSERT(isSample(parentItem));
  auto parentSample = samplePtr(parentItem);

  int subSamples = numSubSamples(parentSample);
  if (row < subSamples)
  {
    return mapFromSource(m_model->index(row, column, m_model->sampleIndex(parentSample)));
  }

  auto channelRow = row - subSamples;
  Q_ASSERT(channelRow < numChannels(parentSample));
  auto internalPtr = m_channels[parentSample][channelRow];

  return createIndex(row, column, internalPtr);
}

//------------------------------------------------------------------------
QModelIndex ChannelProxy::parent(const QModelIndex& child) const
{
  if (!child.isValid()) return QModelIndex();

  auto childItem = itemAdapter(child);

  QModelIndex parent;
  if (isSample(childItem))
  {
    //TODO: Support nested samples
    parent = QModelIndex();
  }
  else
  {
    if (isChannel(childItem))
    {
      for (auto sample : m_channels.keys())
      {
        if (m_channels[sample].contains(childItem))
        {
          parent = mapFromSource(m_model->sampleIndex(sample));
        }
      }
    }
    else
    {
      auto what = QObject::tr("Unknown or invalid item type for child, item type value: %1").arg(static_cast<int>(childItem->type()));
      auto details = QObject::tr("ChannelProxy::parent() -> Unknown or invalid item type for child, item type value: %1").arg(static_cast<int>(childItem->type()));

      throw EspinaException(what, details);
    }
  }

  return parent;
}

//------------------------------------------------------------------------
QModelIndex ChannelProxy::mapFromSource(const QModelIndex& sourceIndex) const
{
  if (!sourceIndex.isValid()) return QModelIndex();

  if (sourceIndex == m_model->classificationRoot() ||
      sourceIndex == m_model->sampleRoot()         ||
      sourceIndex == m_model->channelRoot()        ||
      sourceIndex == m_model->segmentationRoot())
    return QModelIndex();

  auto sourceItem = itemAdapter(sourceIndex);

  QModelIndex proxyIndex;
  if (isSample(sourceItem))
  {
    //Samples are shown in the same order than in the original model
    proxyIndex = createIndex(sourceIndex.row(), sourceIndex.column(), sourceIndex.internalPointer());
  }
  else
  {
    if (isChannel(sourceItem))
    {
      auto channel = channelPtr(sourceItem);
      Q_ASSERT(channel);
      auto samples = m_model->relatedItems(channel, ESPINA::RELATION_IN, Channel::STAIN_LINK);
      if (samples.size() > 0)
      {
        auto sample = samplePtr(samples[0].get());
        int row = m_channels[sample].indexOf(sourceItem);
        if (row >= 0)
        {
          row += numSubSamples(sample);
          proxyIndex = createIndex(row, 0, sourceIndex.internalPointer());
        }
      }
    }
    else
    {
      proxyIndex = QModelIndex();
    }
  }

  return proxyIndex;
}

//------------------------------------------------------------------------
QModelIndex ChannelProxy::mapToSource(const QModelIndex& proxyIndex) const
{
  if (!proxyIndex.isValid()) return QModelIndex();

  auto proxyItem = itemAdapter(proxyIndex);

  QModelIndex sourceIndex;
  if (isSample(proxyItem))
  {
    auto proxySample = samplePtr(proxyItem);
    sourceIndex = m_model->sampleIndex(proxySample);
  }
  else
  {
    if (isChannel(proxyItem))
    {
      auto proxyChannel = channelPtr(proxyItem);
      sourceIndex = m_model->channelIndex(proxyChannel);
    }
    else
    {
      auto what = QObject::tr("Unknown or invalid item type for proxy index, item type value: %1").arg(static_cast<int>(proxyItem->type()));
      auto details = QObject::tr("ChannelProxy::mapToSource() -> Unknown or invalid item type for proxy index, item type value: %1").arg(static_cast<int>(proxyItem->type()));

      throw EspinaException(what, details);
    }
  }

  return sourceIndex;
}

//------------------------------------------------------------------------
Qt::ItemFlags ChannelProxy::flags(const QModelIndex& index) const
{
//  return QAbstractProxyModel::flags(index);

  // TODO: enable proper drag and drop for channels to change sample.
  Qt::ItemFlags f = QAbstractProxyModel::flags(index) | Qt::ItemIsDropEnabled;

  if (index.isValid())
  {
    auto sourceItem = itemAdapter(index);

    if (isChannel(sourceItem))
    {
      f = f | Qt::ItemIsDragEnabled;
    }
  }

  return f;
}

//------------------------------------------------------------------------
bool ChannelProxy::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
  using DraggedItem = QMap<int, QVariant>;

  if (!parent.isValid()) return false;

  auto parentItem = itemAdapter(parent);

  if(!parentItem) return false;

  // Recover dragged item information
  auto encoded = data->data("application/x-qabstractitemmodeldatalist");
  QDataStream stream(&encoded, QIODevice::ReadOnly);

  ChannelAdapterList draggedChannels;

  while (!stream.atEnd())
  {
    DraggedItem itemData;
    int row, col;
    stream >> row >> col >> itemData;

    auto draggedItem = reinterpret_cast<ItemAdapterPtr>(itemData[RawPointerRole].value<quintptr>());

    if(!draggedItem) return false;
    Q_ASSERT(isChannel(draggedItem));

    draggedChannels << channelPtr(draggedItem);
  }

  if (!isSample(parentItem))
  {
    parentItem = itemAdapter(parent.parent());
  }

  Q_ASSERT(isSample(parentItem));

  emit channelsDragged(draggedChannels, samplePtr(parentItem));

  return true;
}

//------------------------------------------------------------------------
int ChannelProxy::numChannels(QModelIndex sampleIndex, bool recursive) const
{
  auto item = itemAdapter(sampleIndex);
  int total = 0;

  if (isSample(item))
  {
    auto sample = samplePtr(item);
    total = numChannels(sample);
  }

  return total;
}

//------------------------------------------------------------------------
int ChannelProxy::numSubSamples(QModelIndex sampleIndex) const
{
  auto item = itemAdapter(sampleIndex);
  int total = 0;

  if (isSample(item))
  {
    auto sample = samplePtr(item);
    total = numSubSamples(sample);
  }

  return total;
}

//------------------------------------------------------------------------
QModelIndexList ChannelProxy::channels(QModelIndex sampleIndex, bool recursive) const
{
  QModelIndexList res;

  auto start = numSubSamples(sampleIndex);
  auto end = start + numChannels(sampleIndex) - 1;
  if (recursive)
  {
    for (int tax = 0; tax < start; tax++)
    {
      res << channels(index(tax, 0, sampleIndex), true);
    }
  }

  if (start <= end)
  {
    res << proxyIndices(sampleIndex, start, end);
  }

  return res;
}

//------------------------------------------------------------------------
void ChannelProxy::sourceRowsInserted(const QModelIndex& sourceParent, int start, int end)
{
  if (!sourceParent.isValid()) return;

  if (sourceParent == m_model->classificationRoot() ||
      sourceParent == m_model->segmentationRoot())
    return;

  if (sourceParent == m_model->sampleRoot())
  {
    beginInsertRows(QModelIndex(), start, end);
    {
      for (int row = start; row <= end; row++)
      {
        auto sampleIndex = m_model->index(row, 0, sourceParent);
        auto sourceRow = itemAdapter(sampleIndex);

        if(!sourceRow) return;

        Q_ASSERT(isSample(sourceRow));
        auto sample = samplePtr(sourceRow);
        m_samples << sample;
        auto channels = m_model->relatedItems(sample, ESPINA::RELATION_OUT, Channel::STAIN_LINK);
        for (auto channel : channels)
        {
          if (channel)
          {
            m_channels[sample] << channel.get();
          }
        }
      }
    }
    endInsertRows();
  }
  else
  {
    if (sourceParent == m_model->channelRoot())
    {
      for (int row = start; row <= end; row++)
      {
        auto channelIndex = m_model->index(row, 0, sourceParent);
        auto sourceRow = itemAdapter(channelIndex);
        Q_ASSERT(isChannel(sourceRow));
        auto channel = channelPtr(sourceRow);

        auto samples = m_model->relatedItems(channel, ESPINA::RELATION_IN, Channel::STAIN_LINK);

        if (samples.size() == 1)
        {
          auto sample      = samplePtr(samples.first().get());
          auto sampleIndex = m_model->sampleIndex(sample);
          int channelStart = m_model->rowCount(sampleIndex);
          int channelEnd   = channelStart + end - start;
          if (sample && channel && (!m_channels.contains(sample) || !m_channels[sample].contains(channel)))
          {
            beginInsertRows(sampleIndex, channelStart, channelEnd);
            {
              m_channels[sample] << channel;
            }
            endInsertRows();
          }
        }
      }
    }
  }
}

//------------------------------------------------------------------------
void ChannelProxy::sourceRowsAboutToBeRemoved(const QModelIndex& sourceParent, int start, int end)
{
  if (!sourceParent.isValid()) return;

  if (sourceParent == m_model->classificationRoot() ||
      sourceParent == m_model->segmentationRoot())
    return;

  if (sourceParent == m_model->sampleRoot())
  {
    beginRemoveRows(QModelIndex(), start, end);
    for (int row = start; row <= end; ++row)
    {
      auto sourceIndex = m_model->index(row, 0, sourceParent);
      auto item        = itemAdapter(sourceIndex);
      auto sample      = samplePtr(item);

      m_samples.removeOne(sample);
      m_channels.remove(sample);
    }
    endRemoveRows();
  }
  else
  {
    if (sourceParent == m_model->channelRoot())
    {
      for (int row = start; row <= end; row++)
      {
        auto sourceIndex = m_model->index(row, 0, sourceParent);
        auto proxyIndex  = mapFromSource(sourceIndex);
        auto item        = itemAdapter(sourceIndex);

        auto channel = channelPtr(item);
        auto parentSample = sample(channel);

        int sampleRow  = m_samples.indexOf(parentSample);
        int channelRow = m_channels[parentSample].indexOf(channel);

        QModelIndex parent = index(sampleRow, 0);

        if(m_activeChannel == channel)
        {
          m_activeChannel = nullptr;
        }

        beginRemoveRows(parent, channelRow, channelRow);
        m_channels[parentSample].removeOne(channel);
        endRemoveRows();
      }
    }
  }
}

//------------------------------------------------------------------------
void ChannelProxy::sourceRowsRemoved(const QModelIndex& sourceParent, int start, int end)
{
}

//------------------------------------------------------------------------
void ChannelProxy::sourceRowsAboutToBeMoved(const QModelIndex& sourceParent, int sourceStart, int sourceEnd, const QModelIndex& destinationParent, int destinationRow)
{
  Q_ASSERT(sourceStart == sourceEnd);

  auto proxySourceParent       = mapFromSource(sourceParent);
  auto proxyDestionationParent = mapFromSource(destinationParent);

  auto sourceItem   = itemAdapter(sourceParent);
  auto sourceSample = samplePtr(sourceItem);

  if(!sourceItem || !sourceSample) return;

  auto movingChannel = channelPtr(sourceItem);

  if(!movingChannel) return;

  int prevChannelRow = m_channels[sourceSample].indexOf(movingChannel);

  beginMoveRows(proxySourceParent, prevChannelRow, prevChannelRow,
                proxyDestionationParent, destinationRow);
}

//------------------------------------------------------------------------
void ChannelProxy::sourceRowsMoved(const QModelIndex& sourceParent, int sourceStart, int sourceEnd, const QModelIndex& destinationParent, int destinationRow)
{
  endMoveRows();
}

//------------------------------------------------------------------------
bool ChannelProxy::indices(const QModelIndex& topLeft, const QModelIndex& bottomRight, QModelIndexList& result)
{
  result << topLeft;

  if (topLeft == bottomRight) return true;

  for (int r = 0; r < m_model->rowCount(topLeft); r++)
  {
    if (indices(topLeft.child(r, 0), bottomRight, result))
    {
      return true;
    }
  }

  for (int r = topLeft.row(); r < m_model->rowCount(topLeft.parent()); r++)
  {
    if (indices(topLeft.sibling(r + 1, 0), bottomRight, result))
    {
      return true;
    }
  }

  return false;
}

//------------------------------------------------------------------------
QModelIndexList ChannelProxy::proxyIndices(const QModelIndex& parent, int start, int end) const
{
  QModelIndexList res;
  for (int row = start; row <= end; row++)
  {
    auto proxyIndex = index(row, 0, parent);
    res << proxyIndex;

    int numChildren = rowCount(proxyIndex);
    if (numChildren > 0)
    {
      res << proxyIndices(proxyIndex,0,numChildren - 1);
    }
  }

  return res;
}

//------------------------------------------------------------------------
void ChannelProxy::sourceDataChanged(const QModelIndex& sourceTopLeft,
                                     const QModelIndex& sourceBottomRight)
{
  QModelIndexList sources;
  indices(sourceTopLeft, sourceBottomRight, sources);

  for(auto source: sources)
  {
    auto proxyIndex = mapFromSource(source);
    if (proxyIndex.isValid())
    {
      auto proxyItem = itemAdapter(proxyIndex);
      if (isSample(proxyItem))
      {
        for(auto sample: m_channels.keys())
        {
          auto sampleIndex = mapFromSource(m_model->sampleIndex(sample));
          auto channels = m_model->relatedItems(sample, ESPINA::RELATION_OUT, Channel::STAIN_LINK);
          auto prevChannels = m_channels[sample].toSet();

          ChannelSet currentChannels;
          for(auto channel : channels)
          {
            currentChannels << channel.get();
          }
          auto newChannels = ChannelSet(currentChannels).subtract(prevChannels);
          auto remChannels = ChannelSet(prevChannels).subtract(currentChannels);

          if (remChannels.size() > 0)
          {
            for(auto remChannel : remChannels)
            {
              int row = m_channels[sample].indexOf(remChannel);
              beginRemoveRows(sampleIndex, row, row);
              m_channels[sample].removeAt(row);
              endRemoveRows();
            }
          }

          if (newChannels.size() > 0)
          {
            int start = m_channels[sample].size();
            int end   = start + newChannels.size() - 1;
            beginInsertRows(sampleIndex, start, end);
            m_channels[sample] << newChannels.toList();
            endInsertRows();
          }

          if(!remChannels.isEmpty() || !newChannels.isEmpty())
          {
            emit dataChanged(sampleIndex, sampleIndex);
          }
        }
      }
      else
      {
          emit dataChanged(proxyIndex, proxyIndex);
      }
    }
  }
}

//------------------------------------------------------------------------
void ChannelProxy::sourceModelReset()
{
  beginResetModel();
  {
    m_activeChannel = nullptr;
    m_samples.clear();
    m_channels.clear();
  }
  endResetModel();
}

//------------------------------------------------------------------------
int ChannelProxy::numChannels(SampleAdapterPtr sample) const
{
  return m_channels[sample].size();
}

//------------------------------------------------------------------------
int ChannelProxy::numSubSamples(SampleAdapterPtr sample) const
{
  return m_model->rowCount(m_model->sampleIndex(sample));
}

//------------------------------------------------------------------------
void ChannelProxy::setActiveChannel(ChannelAdapterPtr channel)
{
  if (m_activeChannel != channel)
  {
    QList<ChannelAdapterPtr> channels;

    if(m_activeChannel != nullptr)
    {
      channels << m_activeChannel;
    }

    if(channel != nullptr)
    {
      channels << channel;
    }

    m_activeChannel = channel;

    for(auto item: channels)
    {
      emitModified(item);
    }
  }
}

//------------------------------------------------------------------------
void ChannelProxy::updateInternalData()
{
  for(auto sample: m_model->samples())
  {
    if(!m_channels.contains(sample.get()))
    {
      m_samples << sample.get();
      m_channels.insert(sample.get(), ItemAdapterList());
    }

    for(auto channel: QueryAdapter::channels(sample))
    {
      if(!m_channels[sample.get()].contains(channel.get()))
      {
        m_channels[sample.get()] << channel.get();
      }
    }
  }
}

//------------------------------------------------------------------------
void ChannelProxy::emitModified(ItemAdapterPtr item)
{
  if(item && (isSample(item) || isChannel(item)))
  {
    updateInternalData();

    auto index = mapFromSource(m_model->index(item));
    emit dataChanged(index, index);
  }
}

//------------------------------------------------------------------------
SampleAdapterPtr ChannelProxy::sample(ChannelAdapterPtr channel) const
{
  for (auto sample : m_channels.keys())
  {
    if (m_channels.keys().contains(sample) && m_channels[sample].contains(channel)) return sample;
  }

  return nullptr;
}
