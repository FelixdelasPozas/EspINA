/*

 Copyright (C) 2018 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <Core/Utils/ListUtils.hxx>
#include <GUI/Model/Proxies/LocationProxy.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/View/ViewState.h>

// Qt
#include <QMimeData>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::GUI::Model::Proxy;

enum DragSourceEnum
{
  NoSource           = 0,
  SegmentationSource = 1 << 0,
  StackSource        = 1 << 1,
  InvalidSource      = SegmentationSource|StackSource
};
Q_DECLARE_FLAGS(DragSource, DragSourceEnum);

Q_DECLARE_OPERATORS_FOR_FLAGS(DragSource)

//--------------------------------------------------------------------
LocationProxy::LocationProxy(ModelAdapterSPtr sourceModel, GUI::View::ViewState& viewState, QObject* parent)
: QAbstractProxyModel{parent}
, m_model            {nullptr}
, m_viewState        (viewState)
, m_orphanVisible    {Qt::Checked}
{
  if(sourceModel) setSourceModel(sourceModel);
}

//--------------------------------------------------------------------
void LocationProxy::setSourceModel(ModelAdapterSPtr sourceModel)
{
  if(m_model == sourceModel) return;

  if (m_model)
  {
    disconnect(m_model.get(), SIGNAL(rowsInserted(const QModelIndex&, int, int)),
               this,          SLOT(sourceRowsInserted(const QModelIndex&, int, int)));
    disconnect(m_model.get(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
               this,          SLOT(sourceRowsRemoved(QModelIndex,int,int)));
    disconnect(m_model.get(), SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
               this,          SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
    disconnect(m_model.get(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
               this,          SLOT(sourceDataChanged(QModelIndex,QModelIndex)));
    disconnect(m_model.get(), SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
               this,          SLOT(sourceRowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));
    disconnect(m_model.get(), SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
               this,          SLOT(sourceRowsMoved(QModelIndex,int,int,QModelIndex,int)));
    disconnect(m_model.get(), SIGNAL(modelAboutToBeReset()),
               this,          SLOT(sourceModelReset()));
  }

  m_visible.clear();
  m_segmentations.clear();

  m_model = sourceModel;

  if (m_model)
  {
    connect(m_model.get(), SIGNAL(rowsInserted(const QModelIndex&, int, int)),
            this, SLOT(sourceRowsInserted(const QModelIndex&, int, int)));
    connect(m_model.get(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
            this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));
    connect(m_model.get(), SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
            this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
    connect(m_model.get(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this,SLOT(sourceDataChanged(QModelIndex,QModelIndex)));
    connect(m_model.get(), SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
            this, SLOT(sourceRowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));
    connect(m_model.get(), SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
            this, SLOT(sourceRowsMoved(QModelIndex,int,int,QModelIndex,int)));
    connect(m_model.get(), SIGNAL(modelAboutToBeReset()),
            this, SLOT(sourceModelReset()));

    QAbstractProxyModel::setSourceModel(m_model.get());

    sourceRowsInserted(m_model->channelRoot(), 0, m_model->rowCount(m_model->channelRoot()) - 1);
    sourceRowsInserted(m_model->segmentationRoot(), 0, m_model->rowCount(m_model->segmentationRoot()) - 1);
  }
}

//--------------------------------------------------------------------
QVariant LocationProxy::data(const QModelIndex &proxyIndex, int role) const
{
  if(!proxyIndex.isValid()) return QVariant();

  auto item = itemAdapter(proxyIndex);

  if(item)
  {
    switch(item->type())
    {
      case ItemAdapter::Type::SEGMENTATION:
        return item->data(role);
        break;
      case ItemAdapter::Type::CHANNEL:
        {
          auto stack = channelPtr(item);
          if(!stack) return QVariant();
          switch(role)
          {
            case Qt::DisplayRole:
              return stack->data(role).toString() + tr(" (%1)").arg(m_segmentations[stack].size());
              break;
            case Qt::CheckStateRole:
              return m_visible[stack];
              break;
            default:
              return item->data(role);
          }
        }
        break;
      default:
        break;
    }
  }
  else
  {
    switch(role)
    {
      case Qt::DisplayRole:
        return tr("Segmentations without a related stack");
        break;
      case Qt::CheckStateRole:
        return m_orphanVisible;
        break;
      default:
        return QVariant();
        break;
    }
  }

  return QAbstractProxyModel::data(proxyIndex, role);
}

//--------------------------------------------------------------------
bool LocationProxy::setData(const QModelIndex &index, const QVariant &value, int role)
{
  bool result = false;

  if(Qt::CheckStateRole == role)
  {
    const auto visibility = value.toBool();

    changeIndexVisibility(index, visibility);

    if(index.parent().isValid())
    {
      changeParentCheckStateRole(index, visibility);
    }

    notifyModifiedRepresentations(index);

    result = true;
  }

  return result;
}

//--------------------------------------------------------------------
bool LocationProxy::hasChildren(const QModelIndex &parent) const
{
  return (!m_segmentations.keys().isEmpty() || !m_orphaned.isEmpty()) && (rowCount(parent) > 0) && (columnCount(parent) > 0);
}

//--------------------------------------------------------------------
int LocationProxy::rowCount(const QModelIndex &parent) const
{
  if(parent.isValid())
  {
    auto item = itemAdapter(parent);
    if(item && isChannel(item))
    {
      auto stack = channelPtr(item);
      if(stack)
      {
        return m_segmentations[stack].size();
      }
    }
    else
    {
      return m_orphaned.size();
    }
  }
  else
  {
    if(!m_orphaned.isEmpty()) return m_segmentations.keys().size() + 1;

    return m_segmentations.keys().size();
  }

  return 0;
}

//--------------------------------------------------------------------
QModelIndex LocationProxy::parent(const QModelIndex &child) const
{
  if(child.isValid())
  {
    auto item = itemAdapter(child);
    if(item && item->type() == ItemAdapter::Type::SEGMENTATION)
    {
      auto segmentation = segmentationPtr(item);
      if(segmentation)
      {
        auto stack = stackOf(segmentation);
        if(stack)
        {
          auto pos = m_segmentations.keys().indexOf(stack);
          if(pos != -1)
          {
            return createIndex(pos, 0, stack);
          }
        }
        else
        {
          return orphanIndex();
        }
      }
    }
  }

  return QModelIndex();
}

//--------------------------------------------------------------------
QModelIndex LocationProxy::index(int row, int column, const QModelIndex &parent) const
{
  if (hasIndex(row, column, parent))
  {
    if(parent.isValid())
    {
      // SEGMENTATION
      auto item = itemAdapter(parent);
      if(item && item->type() == ItemAdapter::Type::CHANNEL)
      {
        auto stack = channelPtr(item);
        if(stack)
        {
          return createIndex(row, column, m_segmentations[stack].at(row));
        }
      }
    }
    else
    {
      // STACK
      if(row < m_segmentations.keys().size())
      {
        return createIndex(row, column, m_segmentations.keys().at(row));
      }
      else
      {
        return createIndex(row, column, nullptr);
      }
    }
  }

  return QModelIndex();
}

//--------------------------------------------------------------------
QModelIndex LocationProxy::mapFromSource(const QModelIndex &sourceIndex) const
{
  if (!sourceIndex.isValid()                       ||
      m_segmentations.isEmpty()                    ||
      sourceIndex == m_model->classificationRoot() ||
      sourceIndex == m_model->sampleRoot()         ||
      sourceIndex == m_model->channelRoot()        ||
      sourceIndex == m_model->segmentationRoot())
  {
    return QModelIndex();
  }

  auto item = itemAdapter(sourceIndex);

  if(item)
  {
    switch (item->type())
    {
      case ItemAdapter::Type::CHANNEL:
        {
          auto stack = channelPtr(item);
          auto pos   = m_segmentations.keys().indexOf(stack);
          if(pos != -1)
          {
            return createIndex(pos, 0, sourceIndex.internalPointer());
          }
        }

        break;
      case ItemAdapter::Type::SEGMENTATION:
        {
          auto segmentation = segmentationPtr(item);
          if(segmentation)
          {
            for(auto stack: m_segmentations.keys())
            {
              if(m_segmentations[stack].contains(segmentation))
              {
                return createIndex(m_segmentations[stack].indexOf(segmentation), 0, sourceIndex.internalPointer());
              }
            }
          }
        }
        break;
      default:
        break;
    }
  }
  else
  {
    qWarning() << "LocationProxy::mapFromSource() invalid source index" << sourceIndex;
  }
  return QModelIndex();
}

//--------------------------------------------------------------------
QModelIndex LocationProxy::mapToSource(const QModelIndex &proxyIndex) const
{
  if(proxyIndex.isValid())
  {
    auto proxyItem = itemAdapter(proxyIndex);
    if(proxyItem)
    {
      switch(proxyItem->type())
      {
        case ItemAdapter::Type::CHANNEL:
          {
            auto stack = channelPtr(proxyItem);
            if(stack)
            {
              return m_model->channelIndex(stack);
            }
          }
          break;
        case ItemAdapter::Type::SEGMENTATION:
          {
            auto segmentation = segmentationPtr(proxyItem);
            if(segmentation)
            {
              return m_model->segmentationIndex(segmentation);
            }
          }
          break;
        default:
          break;
      }
    }
  }

  return QModelIndex();
}

//--------------------------------------------------------------------
Qt::ItemFlags LocationProxy::flags(const QModelIndex &index) const
{
  auto indexFlags = Qt::ItemFlags(Qt::ItemIsSelectable|Qt::ItemIsUserCheckable|Qt::ItemIsEnabled);
  auto parent = index.parent();

  switch(parent.isValid())
  {
    case false:
      if(index.internalPointer()) indexFlags = indexFlags | Qt::ItemIsDropEnabled;
      break;
    default:
      indexFlags = indexFlags | Qt::ItemIsDragEnabled;
      break;
  }

  return indexFlags;
}

//--------------------------------------------------------------------
bool LocationProxy::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
  using DraggedItem = QMap<int, QVariant>;

  if(!parent.isValid()) return false;

  auto parentItem = itemAdapter(parent);
  if(!parentItem) return false;

  DragSource source = NoSource;

  // Recover dragged item information
  QByteArray  encoded = data->data("application/x-qabstractitemmodeldatalist");

  QDataStream        stream(&encoded, QIODevice::ReadOnly);
  QList<DraggedItem> draggedItems;

  while (!stream.atEnd())
  {
    int row, col;
    DraggedItem itemData;
    stream >> row >> col >> itemData;

    switch (ItemAdapter::type(itemData[TypeRole].toInt()))
    {
      case ItemAdapter::Type::SEGMENTATION:
        source |= SegmentationSource;
        break;
      case ItemAdapter::Type::CHANNEL:
        source |= StackSource;
        break;
      default:
        source = InvalidSource;
        break;
    }

    draggedItems << itemData;
  }

  if(source == SegmentationSource)
  {
    SegmentationAdapterList sources;
    for (auto draggedItem : draggedItems)
    {
      auto item = reinterpret_cast<ItemAdapterPtr>(draggedItem[RawPointerRole].value<quintptr>());
      sources << segmentationPtr(item);
    }

    ChannelAdapterPtr stack;
    if(parentItem->type() == ItemAdapter::Type::CHANNEL)
    {
      stack = channelPtr(parentItem);
    }
    else
    {
      Q_ASSERT(parentItem->type() == ItemAdapter::Type::SEGMENTATION);
      auto segmentation = segmentationPtr(parentItem);
      auto stacks = QueryAdapter::channels(segmentation);
      Q_ASSERT(!stacks.isEmpty() && stacks.size() == 1);
      stack = stacks.first().get();
    }

    emit segmentationsDropped(sources, stack);
  }

  return false;
}

//--------------------------------------------------------------------
void LocationProxy::sourceRowsInserted(const QModelIndex &sourceParent, int start, int end)
{
  if((end < start)                         ||
     sourceParent == m_model->sampleRoot() ||
     sourceParent == m_model->classificationRoot())
  {
    return;
  }

  if(sourceParent == m_model->channelRoot())
  {
    beginInsertRows(QModelIndex(), start, end);

    for(int row = start; row <= end; ++row)
    {
      auto insertedItem = itemAdapter(m_model->index(row, 0, sourceParent));
      auto stack        = channelPtr(insertedItem);
      Q_ASSERT(stack);
      m_visible[stack]  = stack->data(Qt::CheckStateRole).toBool() ? Qt::Checked : Qt::Unchecked;
      m_segmentations.insert(stack, SegmentationAdapterList());
    }

    endInsertRows();
  }
  else if(sourceParent == m_model->segmentationRoot())
  {
    auto groupedSegmentations = groupSegmentationsByStack(start, end);

    for(auto stack : groupedSegmentations.keys())
    {
      int startRow = m_segmentations[stack.get()].size();
      int endRow   = startRow + groupedSegmentations[stack].size() - 1;

      beginInsertRows(channelIndex(stack.get()), startRow, endRow);
      m_segmentations[stack.get()] << toList<SegmentationAdapter>(groupedSegmentations[stack]);
      endInsertRows();
    }
  }
  else
  {
    Q_ASSERT(false);
  }
}

//--------------------------------------------------------------------
void LocationProxy::sourceRowsAboutToBeRemoved(const QModelIndex &sourceParent, int start, int end)
{
  if((end < start)                         ||
     sourceParent == m_model->sampleRoot() ||
     sourceParent == m_model->classificationRoot())
  {
    return;
  }

  if(sourceParent == m_model->channelRoot())
  {
    beginRemoveRows(QModelIndex(), start, end);

    for(int row = start; row <= end; ++row)
    {
      auto removedItem = itemAdapter(m_model->index(row, 0, QModelIndex()));
      auto stack       = channelPtr(removedItem);
      Q_ASSERT(stack);
      m_visible.remove(stack);
      m_segmentations.remove(stack);
    }

    endRemoveRows();
  }
  else if(sourceParent == m_model->segmentationRoot())
  {
    for(int row = start; row <= end; ++row)
    {
      auto sourceIndex  = m_model->index(row, 0, sourceParent);
      auto sourceItem   = itemAdapter(sourceIndex);
      auto segmentation = segmentationPtr(sourceItem);

      if(sourceItem && segmentation)
      {
        if(m_orphaned.contains(segmentation))
        {
          auto numStart    = m_orphaned.indexOf(segmentation);
          beginRemoveRows(orphanIndex(), numStart, numStart);
          m_orphaned.removeAll(segmentation);
          endRemoveRows();
        }
        else
        {
          for(auto stack: m_segmentations.keys())
          {
            if(m_segmentations[stack].contains(segmentation))
            {
              auto numStart = m_segmentations[stack].indexOf(segmentation);
              beginRemoveRows(channelIndex(stack), numStart, numStart);
              m_segmentations[stack].removeAll(segmentation);
              endRemoveRows();
              break;
            }
          }
        }
      }
    }
  }
}

//--------------------------------------------------------------------
void LocationProxy::sourceRowsRemoved(const QModelIndex &sourceParent, int start, int end)
{
}

//--------------------------------------------------------------------
void LocationProxy::sourceRowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent, int destinationRow)
{
  QModelIndex from = mapFromSource(sourceParent);
  QModelIndex to   = mapFromSource(destinationParent);

  beginMoveRows(from, sourceStart, sourceEnd,
                to, destinationRow);

  auto fromItem = itemAdapter(sourceParent);
  auto toItem = itemAdapter(destinationParent);
  if(!fromItem || fromItem->type() != ItemAdapter::Type::CHANNEL) return;
  if(!toItem || toItem->type() != ItemAdapter::Type::CHANNEL) return;

  auto fromStack = channelPtr(fromItem);
  auto toStack = channelPtr(toItem);
  if(!fromStack || !toStack) return;

  for(int i = sourceStart; i <= sourceEnd; ++i)
  {
    auto item = itemAdapter(sourceParent.child(i, 0));
    if(!item && item->type() != ItemAdapter::Type::SEGMENTATION) continue;
    auto segmentation = segmentationPtr(item);
    if(!segmentation) continue;

    m_segmentations[fromStack].removeAll(segmentation);
    m_segmentations[toStack] << segmentation;
  }
}

//--------------------------------------------------------------------
void LocationProxy::sourceRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent, int destinationRow)
{
  endMoveRows();
}

//--------------------------------------------------------------------
void LocationProxy::sourceDataChanged(const QModelIndex &sourceTopLeft, const QModelIndex &sourceBottomRight)
{
  if(!sourceTopLeft.isValid() || !sourceBottomRight.isValid()) return;

  QModelIndexList sources, stackIndexes, sourceIndexes;

  if(sourceTopLeft.parent() != sourceBottomRight.parent() ||
     sourceTopLeft.parent() == m_model->classificationRoot() ||
     sourceTopLeft.parent() == m_model->sampleRoot())
  {
    return;
  }
  else
  {
    indices(sourceTopLeft, sourceBottomRight, sources);

    for(auto sourceItem: sources)
    {
      auto proxyIndex = mapFromSource(sourceItem);
      if(!proxyIndex.isValid()) continue;

      auto item = itemAdapter(sourceItem);
      if(item)
      {
        auto segmentation = segmentationPtr(item);
        if(segmentation)
        {
          auto stacks = QueryAdapter::channels(segmentation);
          if(stacks.isEmpty())
          {
            m_orphaned << segmentation;
            if(!stackIndexes.contains(orphanIndex())) stackIndexes << orphanIndex();
          }
          else
          {
            auto stack = stacks.first().get();

            for(auto key: m_segmentations.keys())
            {
              if((key != stack) && m_segmentations[key].contains(segmentation))
              {
                m_segmentations[key].removeAll(segmentation);
                auto stackIndex = channelIndex(key);
                if(!stackIndexes.contains(stackIndex)) stackIndexes << stackIndex;
                continue;
              }

              if((key == stack) && !m_segmentations[key].contains(segmentation))
              {
                m_segmentations[key] << segmentation;
                auto stackIndex = channelIndex(key);
                if(!stackIndexes.contains(stackIndex)) stackIndexes << stackIndex;
              }
            }
          }

          sourceIndexes << proxyIndex;
        }
      }
    }

    if(!stackIndexes.isEmpty())
    {
      emit layoutAboutToBeChanged();

      for(auto index: stackIndexes)
      {
        emit dataChanged(index, index);
      }

      emit layoutChanged();
    }

    for(auto index: sourceIndexes)
    {
      emit dataChanged(index, index);
    }
  }
}

//------------------------------------------------------------------------
bool LocationProxy::indices(const QModelIndex &topLeft, const QModelIndex &bottomRight, QModelIndexList& result)
{
  if(!topLeft.isValid() || !bottomRight.isValid()) return false;

  result << topLeft;

  if (topLeft == bottomRight) return true;

  for (int r = 0; r < m_model->rowCount(topLeft); r++)
  {
    if (indices(topLeft.child(r, 0), bottomRight, result)) return true;
  }

  for (int r = topLeft.row() + 1; r < m_model->rowCount(topLeft.parent()); r++)
  {
    if (indices(topLeft.sibling(r,0), bottomRight, result)) return true;
  }

  return false;
}

//--------------------------------------------------------------------
void LocationProxy::sourceModelReset()
{
  beginResetModel();
  {
    m_visible.clear();
    m_segmentations.clear();
    m_orphaned.clear();
    m_orphanVisible = Qt::Checked;
  }
  endResetModel();
}

//--------------------------------------------------------------------
void LocationProxy::changeIndexVisibility(const QModelIndex &index, bool value)
{
  auto item = itemAdapter(index);

  if(item)
  {
    if (item->type() == ItemAdapter::Type::CHANNEL)
    {
      auto stack = channelPtr(item);
      m_visible[stack] = value ? Qt::Checked : Qt::Unchecked;

      for (int r=0; r < rowCount(index); r++)
      {
        changeIndexVisibility(index.child(r,0), value);
      }
      m_model->setData(mapToSource(index), value, Qt::CheckStateRole);
    }
    else if(item->type() == ItemAdapter::Type::SEGMENTATION)
    {
      m_model->setData(mapToSource(index), value, Qt::CheckStateRole);
    }
  }
  else
  {
    m_orphanVisible = value ? Qt::Checked : Qt::Unchecked;

    for (int r=0; r < rowCount(index); r++)
    {
      changeIndexVisibility(index.child(r,0), value);
    }
  }
}

//--------------------------------------------------------------------
void LocationProxy::changeParentCheckStateRole(const QModelIndex &index, bool value)
{
  auto parentIndex = parent(index);

  if(parentIndex.isValid())
  {
    auto parentItem = itemAdapter(parentIndex);
    auto stack      = channelPtr(parentItem);

    int parentRows = rowCount(parentIndex);
    auto checkState = Qt::Unchecked;

    for (int row = 0; row < parentRows; row++)
    {
      auto rowState = parentIndex.child(row, 0).data(Qt::CheckStateRole).toBool() ? Qt::Checked : Qt::Unchecked;
      if (0 == row)
      {
        checkState = rowState;
      }
      else
        if (checkState != rowState)
        {
          checkState = Qt::PartiallyChecked;
          break;
        }
    }

    if(stack)
    {
      m_visible[stack] = checkState;
    }
    else
    {
      m_orphanVisible = checkState;
    }

    emit dataChanged(parentIndex, parentIndex);
  }
}

//--------------------------------------------------------------------
void LocationProxy::notifyModifiedRepresentations(const QModelIndex &index)
{
  auto item = itemAdapter(index);

  ViewItemAdapterList modifiedItems;

  if(item)
  {
    if (item->type() == ItemAdapter::Type::CHANNEL)
    {
      auto stack = channelPtr(item);
      if(stack)
      {
        modifiedItems << stack;
        modifiedItems << toList<ViewItemAdapter>(m_segmentations[stack]);
      }
    }
    else if (item->type() == ItemAdapter::Type::SEGMENTATION)
    {
      auto segmentation = segmentationPtr(item);
      if(segmentation)
      {
        modifiedItems << segmentation;
      }
    }
    else
    {
      Q_ASSERT(false);
    }
  }

  if(!modifiedItems.isEmpty()) m_viewState.invalidateRepresentations(modifiedItems);
}

//--------------------------------------------------------------------
const QMap<ChannelAdapterSPtr, ItemAdapterList> LocationProxy::groupSegmentationsByStack(int start, int end)
{
  QMap<ChannelAdapterSPtr, ItemAdapterList> result;
  SegmentationAdapterList orphaned;

  if(start <= end)
  {
    for (int row = start; row <= end; row++)
    {
      auto sourceIndex    = m_model->index(row, 0, m_model->segmentationRoot());
      auto sourceItem     = itemAdapter(sourceIndex);
      auto segmentation   = segmentationPtr(sourceItem);
      auto stacks         = QueryAdapter::channels(segmentation);
      if(!stacks.isEmpty())
      {
        auto stack = stacks.first();
        if (stack && stack.get())
        {
          result[stack] << sourceItem;
        }
        else
        {
          orphaned << segmentation;
        }
      }
      else
      {
        orphaned << segmentation;
      }
    }
  }

  if(!orphaned.isEmpty())
  {
    auto alreadyIn   = m_orphaned.size();
    beginInsertRows(orphanIndex(), alreadyIn, alreadyIn + orphaned.size());
    m_orphaned << orphaned;
    endInsertRows();
  }

  return result;
}

//--------------------------------------------------------------------
const QModelIndex LocationProxy::channelIndex(const ChannelAdapterPtr stack)
{
  auto pos = m_segmentations.keys().indexOf(stack);
  if(pos != -1)
  {
    return createIndex(pos, 0, stack);
  }

  return QModelIndex();
}

//--------------------------------------------------------------------
const SegmentationAdapterList LocationProxy::segmentationsOf(const ChannelAdapterPtr stack) const
{
  SegmentationAdapterList result;

  if(m_segmentations.keys().contains(stack))
  {
    result << m_segmentations[stack];
    result.detach();
  }

  return result;
}

//--------------------------------------------------------------------
const SegmentationAdapterList LocationProxy::orphanedSegmentations() const
{
  SegmentationAdapterList result;

  if(!m_orphaned.isEmpty())
  {
    result = m_orphaned;
    result.detach();
  }

  return result;
}

//--------------------------------------------------------------------
const ChannelAdapterPtr LocationProxy::stackOf(const SegmentationAdapterPtr segmentation) const
{
  ChannelAdapterPtr result = nullptr;

  for(auto stack: m_segmentations.keys())
  {
    if(m_segmentations[stack].contains(segmentation)) return stack;
  }

  return result;
}

//--------------------------------------------------------------------
const QModelIndex LocationProxy::orphanIndex() const
{
  if(!m_orphaned.isEmpty())
  {
    return createIndex(m_segmentations.keys().size(), 0, nullptr);
  }

  return QModelIndex();
}
