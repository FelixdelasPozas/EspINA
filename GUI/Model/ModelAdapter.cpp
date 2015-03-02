/*

    Copyright (C) 2014  Jorge Peña Pastor<jpena@cesvima.upm.es>

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
#include "ModelAdapter.h"
#include <Core/Analysis/Analysis.h>
#include <Core/Analysis/Sample.h>
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Segmentation.h>
#include "GUI/Model/SegmentationAdapter.h"

using namespace ESPINA;



//------------------------------------------------------------------------
ModelAdapter::ModelAdapter(TimerSPtr timer)
: m_timer      {timer}
, m_analysis   {new Analysis()}
, m_isBatchMode{false}
{
}

//------------------------------------------------------------------------
ModelAdapter::~ModelAdapter()
{
//   qDebug() << "Destroying Model Adapter";
}

//------------------------------------------------------------------------
TimerSPtr ModelAdapter::timer() const
{
  return m_timer;
}

//------------------------------------------------------------------------
void ModelAdapter::setAnalysis(AnalysisSPtr analysis, ModelFactorySPtr factory)
{
  reset(); //It is needed in order to keep the views coherent

  m_analysis = analysis;

  m_timer->increment();

  // Adapt classification
  if (analysis->classification())
  {
    auto classification = std::make_shared<ClassificationAdapter>(analysis->classification());
    beginInsertRows(classificationRoot(), 0, classification->root()->subCategories().size() - 1);
    m_classification = classification;
    endInsertRows();
  }

  if (!analysis->samples().isEmpty())
  {
    // Adapt Samples
    beginInsertRows(sampleRoot(), 0, analysis->samples().size() - 1);
    for(auto sample : analysis->samples())
    {
      auto adapted = factory->adaptSample(sample);
      m_samples << adapted;
      adapted->setModel(this);
    }
    endInsertRows();
  }

  if (!analysis->channels().isEmpty())
  {
    // Adapt channels --> adapt non adapted filters
    beginInsertRows(channelRoot(), 0, analysis->channels().size() - 1);
    for(auto channel : analysis->channels())
    {
      auto adapted = factory->adaptChannel(channel);
      m_channels << adapted;
      adapted->setModel(this);
    }
    endInsertRows();
    emit channelsAdded(toViewItemList(m_channels), m_timer->timeStamp());
    emit channelsAdded(m_channels, m_timer->timeStamp());
  }

  if (!analysis->segmentations().isEmpty())
  {
    // Adapt segmentation --> adapt non adapted filters
    beginInsertRows(segmentationRoot(), 0, analysis->segmentations().size() - 1);
    for(auto segmentation : analysis->segmentations())
    {
      auto adapted = factory->adaptSegmentation(segmentation);

      auto categoy = segmentation->category();

      if (categoy)
      {
        adapted->setCategory(m_classification->category(categoy->classificationName()));
      }

      m_segmentations << adapted;
      adapted->setModel(this);
    }
    endInsertRows();
  }
}

//------------------------------------------------------------------------
void ModelAdapter::setStorage(TemporalStorageSPtr storage)
{
  m_analysis->setStorage(storage);
}

//------------------------------------------------------------------------
TemporalStorageSPtr ModelAdapter::storage() const
{
  return m_analysis->storage();
}


//------------------------------------------------------------------------
void ModelAdapter::add(SampleAdapterSPtr sample)
{
  queueAddCommand(sample, addSampleCommand(sample));

  executeCommandsIfNoBatchMode();
}

//------------------------------------------------------------------------
void ModelAdapter::add(SampleAdapterSList samples)
{
    for(auto sample : samples)
    {
      queueAddCommand(sample, addSampleCommand(sample));
    }
    executeCommandsIfNoBatchMode();
}

//------------------------------------------------------------------------
ModelAdapter::BatchCommandSPtr ModelAdapter::addChannelCommand(ChannelAdapterSPtr channel) throw(Existing_Item_Exception)
{
  auto command = [this, channel]() {
    if (m_channels.contains(channel)) throw Existing_Item_Exception();

    m_analysis->add(channel->m_channel);
    m_channels << channel;

    channel->setModel(this);
  };

  return std::make_shared<Command<decltype(command)>>(command);
}

//------------------------------------------------------------------------
void ModelAdapter::add(ChannelAdapterSPtr channel)
{
  queueAddCommand(channel, addChannelCommand(channel));

  executeCommandsIfNoBatchMode();
}

//------------------------------------------------------------------------
void ModelAdapter::add(ChannelAdapterSList channels)
{
  for(auto channel : channels)
  {
    queueAddCommand(channel, addChannelCommand(channel));
  }

  executeCommandsIfNoBatchMode();
}

//------------------------------------------------------------------------
void ModelAdapter::add(SegmentationAdapterSPtr segmentation)
{
  queueAddCommand(segmentation, addSegmentationCommand(segmentation));

  executeCommandsIfNoBatchMode();
}

//------------------------------------------------------------------------
void ModelAdapter::add(SegmentationAdapterSList segmentations)
{
  for(auto segmentation : segmentations)
  {
    queueAddCommand(segmentation, addSegmentationCommand(segmentation));
  }

  executeCommandsIfNoBatchMode();
}

//------------------------------------------------------------------------
void ModelAdapter::addRelation(ItemAdapterSPtr ancestor, ItemAdapterSPtr successor, const RelationName& relation)
{
  queueAddRelationCommand(ancestor, successor, relation);

  executeCommandsIfNoBatchMode();
}

//------------------------------------------------------------------------
void ModelAdapter::addRelation(const Relation& relation)
{
  addRelation(relation.ancestor, relation.successor, relation.relation);
}

//------------------------------------------------------------------------
void ModelAdapter::addRelations(const RelationList &relations)
{
  for(auto relation: relations)
  {
    queueAddRelationCommand(relation.ancestor, relation.successor, relation.relation);
  }

  executeCommandsIfNoBatchMode();
}

//------------------------------------------------------------------------
void ModelAdapter::remove(SampleAdapterSPtr sample)
{
  queueRemoveCommand(sample, removeSampleCommand(sample));

  executeCommandsIfNoBatchMode();
}

//------------------------------------------------------------------------
void ModelAdapter::remove(SampleAdapterSList samples)
{
  for(auto sample : samples)
  {
    queueRemoveCommand(sample, removeSampleCommand(sample));
  }

  executeCommandsIfNoBatchMode();
}

//------------------------------------------------------------------------
void ModelAdapter::remove(ChannelAdapterSPtr channel)
{
  queueRemoveCommand(channel, removeChannelCommand(channel));

  executeCommandsIfNoBatchMode();
}

//------------------------------------------------------------------------
void ModelAdapter::remove(ChannelAdapterSList channels)
{
  for(auto channel : channels)
  {
    queueRemoveCommand(channel, removeChannelCommand(channel));
  }

  executeCommandsIfNoBatchMode();
}

//------------------------------------------------------------------------
void ModelAdapter::remove(SegmentationAdapterSPtr segmentation)
{
  queueRemoveCommand(segmentation, removeSegmentationCommand(segmentation));

  executeCommandsIfNoBatchMode();
}

//------------------------------------------------------------------------
void ModelAdapter::remove(SegmentationAdapterSList segmentations)
{
  for(auto segmentation : segmentations)
  {
    queueRemoveCommand(segmentation, removeSegmentationCommand(segmentation));
  }

  executeCommandsIfNoBatchMode();
}

//------------------------------------------------------------------------
QModelIndex ModelAdapter::categoryIndex(CategoryAdapterPtr category) const
{
  // We avoid setting the classification root as the parent of an index
  if ( !m_classification || m_classification->root().get() == category)
    return classificationRoot();

  CategoryAdapterPtr parentCategory = category->parent();
  Q_ASSERT(parentCategory);

  CategoryAdapterSPtr subNode = parentCategory->subCategory(category->name());

  int row = parentCategory->subCategories().indexOf(subNode);

  ItemAdapterPtr internalPtr = category;
  return createIndex(row, 0, internalPtr);
}

//------------------------------------------------------------------------
QModelIndex ModelAdapter::categoryIndex(CategoryAdapterSPtr category) const
{
  return categoryIndex(category.get());
}

//------------------------------------------------------------------------
QModelIndex ModelAdapter::channelIndex(ChannelAdapterPtr channel) const
{
  QModelIndex index;

  int row = 0;
  for(auto ptr : m_channels)
  {
    if (ptr.get() == channel)
    {
      ItemAdapterPtr internalPtr = channel;
      index = createIndex(row, 0, internalPtr);
    }
    row++;
  }

  return index;
}

//------------------------------------------------------------------------
QModelIndex ModelAdapter::channelIndex(ChannelAdapterSPtr channel) const
{
  return channelIndex(channel.get());
}

//------------------------------------------------------------------------
QModelIndex ModelAdapter::channelRoot() const
{
  return createIndex(2, 0, 2);
}

//------------------------------------------------------------------------
const ClassificationAdapterSPtr ModelAdapter::classification() const
{
  return m_classification;
}

//------------------------------------------------------------------------
QModelIndex ModelAdapter::classificationRoot() const
{
  return createIndex(0, 0, 0);
}

//------------------------------------------------------------------------
int ModelAdapter::columnCount(const QModelIndex& parent) const
{
  return 1;
}

//------------------------------------------------------------------------
CategoryAdapterSPtr ModelAdapter::createRootCategory(const QString& name)
{
  return createCategory(name, CategoryAdapterSPtr());
}

//------------------------------------------------------------------------
CategoryAdapterSPtr ModelAdapter::createCategory(const QString& name, CategoryAdapterPtr parent)
{
  CategoryAdapterSPtr parentCategory;

  if (parent)
  {
    parentCategory = smartPointer(parent);
  }

  return createCategory(name, parentCategory);
}

//------------------------------------------------------------------------
CategoryAdapterSPtr ModelAdapter::createCategory(const QString& name, CategoryAdapterSPtr parent)
{
  auto parentCategory = m_classification->root();

  if (parent)
  {
    parentCategory = parent;
  }

  Q_ASSERT(!parentCategory->subCategory(name));

  CategoryAdapterSPtr requestedCategory;

  QModelIndex parentItem = categoryIndex(parentCategory);

  int newTaxRow = rowCount(parentItem);
  beginInsertRows(parentItem, newTaxRow, newTaxRow);
  {
    requestedCategory = m_classification->createCategory(name, parentCategory);
  }
  endInsertRows();

  return requestedCategory;
}

//------------------------------------------------------------------------
QVariant ModelAdapter::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();

  if (index == classificationRoot())
  {
    if (role == Qt::DisplayRole)
      return tr("Classification");
    return QVariant();
  }

  if (index == sampleRoot())
  {
    if (role == Qt::DisplayRole)
      return tr("Samples");
    return QVariant();
  }

  if (index == channelRoot())
  {
    if (role == Qt::DisplayRole)
      return tr("Channels");
    return QVariant();
  }

  if (index == segmentationRoot())
  {
    if (role == Qt::DisplayRole)
      return "Segmentations";
    return QVariant();
  }

  return itemAdapter(index)->data(role);
}

//------------------------------------------------------------------------
void ModelAdapter::deleteRelation(ItemAdapterSPtr ancestor, ItemAdapterSPtr successor, const RelationName& relation)
{
  try
  {
    m_analysis->deleteRelation(ancestor->m_analysisItem, successor->m_analysisItem, relation);
  } catch (const Analysis::Relation_Not_Found_Exception &e)
  {
    throw Relation_Not_Found_Exception();
  }
}

//------------------------------------------------------------------------
void ModelAdapter::deleteRelation(const Relation& relation)
{
  deleteRelation(relation.ancestor, relation.successor, relation.relation);
}

//------------------------------------------------------------------------
void ModelAdapter::deleteRelations(const RelationList& relations)
{
  for(auto relation: relations)
  {
    try
    {
      m_analysis->deleteRelation(relation.ancestor->m_analysisItem, relation.successor->m_analysisItem, relation.relation);
    } catch (const Analysis::Relation_Not_Found_Exception &e)
    {
      throw Relation_Not_Found_Exception();
    }
  }
}

//------------------------------------------------------------------------
Qt::ItemFlags ModelAdapter::flags(const QModelIndex& index) const
{
  auto flags = QAbstractItemModel::flags(index);

  if (index.isValid())
  {
    QModelIndex parent = index.parent();
    if (parent != channelRoot() || parent != segmentationRoot())
    {
      flags = flags | Qt::ItemIsUserCheckable;
    }
  }

  return flags;
}

//------------------------------------------------------------------------
QModelIndex ModelAdapter::index(int row, int column, const QModelIndex& parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  if (!parent.isValid())
  {
    Q_ASSERT (row < 4);
    if (row == 0)
      return classificationRoot();
    if (row == 1)
      return sampleRoot();
    if (row == 2)
      return channelRoot();
    if (row == 3)
      return segmentationRoot();
  }

  ItemAdapterPtr internalPtr;

  if (parent == sampleRoot())
  {
    Q_ASSERT(row < m_samples.size());
    internalPtr = m_samples[row].get();
  }
  else if (parent == channelRoot())
  {
    Q_ASSERT(row < m_channels.size());
    internalPtr = m_channels[row].get();
  }
  else if (parent == segmentationRoot())
  {
    Q_ASSERT(row < m_segmentations.size());
    internalPtr = m_segmentations[row].get();
  }
  else
  {
    CategoryAdapterPtr parentCategory;
    if (parent == classificationRoot())
    {
      parentCategory = m_classification->root().get();
    }
    else
    {
      // Neither Samples nor Segmentations have children
      parentCategory = categoryPtr(parent);
    }
    //WARNING: Now m_classification can be NULL, but even in that situation,
    //         it shouldn't report any children
    Q_ASSERT(parentCategory);
    Q_ASSERT(row < parentCategory->subCategories().size());
    internalPtr = parentCategory->subCategories()[row].get();
  }

  return createIndex(row, column, internalPtr);
}

//------------------------------------------------------------------------
QModelIndex ModelAdapter::index(ItemAdapterPtr item) const
{
  QModelIndex res;
  switch (item->type())
  {
    case ItemAdapter::Type::CATEGORY:
      res = categoryIndex(categoryPtr(item));
      break;
    case ItemAdapter::Type::SAMPLE:
      res = sampleIndex(samplePtr(item));
      break;
    case ItemAdapter::Type::CHANNEL:
      res = channelIndex(channelPtr(item));
      break;
    case ItemAdapter::Type::SEGMENTATION:
      res = segmentationIndex(segmentationPtr(item));
      break;
    default:
      throw Item_Not_Found_Exception();
  }
  return res;
}

//------------------------------------------------------------------------
QModelIndex ModelAdapter::index(ItemAdapterSPtr item) const
{
  return index(item.get());
}

//------------------------------------------------------------------------
QMap< int, QVariant > ModelAdapter::itemData(const QModelIndex& index) const
{
  QMap<int, QVariant> data = QAbstractItemModel::itemData(index);

  if (index.isValid() && index.parent().isValid())
  {
    data[RawPointerRole] = QVariant::fromValue(reinterpret_cast<quintptr>(itemAdapter(index)));
  }

  data[TypeRole] = index.data(TypeRole);

  return data;
}

//------------------------------------------------------------------------
QModelIndex ModelAdapter::parent(const QModelIndex& child) const
{
  if (!child.isValid())
    return QModelIndex();

  if ( child == classificationRoot()
    || child == sampleRoot()
    || child == channelRoot()
    || child == segmentationRoot())
    return QModelIndex();

  ItemAdapterPtr childItem = itemAdapter(child);

  switch (childItem->type())
  {
    case ItemAdapter::Type::CATEGORY:
    {
      CategoryAdapterPtr category = categoryPtr(childItem);
      return categoryIndex(category->parent());
    }
    case ItemAdapter::Type::SAMPLE:
      return sampleRoot();
    case ItemAdapter::Type::CHANNEL:
      return channelRoot();
    case ItemAdapter::Type::SEGMENTATION:
      return segmentationRoot();
    default:
      throw -1;
  };

  return QModelIndex();
}

//------------------------------------------------------------------------
ItemAdapterSList ModelAdapter::relatedItems(ItemAdapterPtr item, RelationType type, const RelationName& filter)
{
  ItemAdapterSList items;
  if (type == RELATION_IN || type == RELATION_INOUT) {
    for(auto ancestor : m_analysis->relationships()->ancestors(item->m_analysisItem, filter)) {
      items << find(ancestor);
    }
  }

  if (type == RELATION_OUT || type == RELATION_INOUT) {
    for(auto successor : m_analysis->relationships()->successors(item->m_analysisItem, filter)) {
      items << find(successor);
    }
  }

  return items;
}

//------------------------------------------------------------------------
RelationList ModelAdapter::relations(ItemAdapterPtr item, RelationType type, const RelationName& filter)
{
  RelationList relations;

  if (ESPINA::RELATION_IN == type || ESPINA::RELATION_INOUT == type)
  {
    for(auto edge : m_analysis->relationships()->inEdges(item->m_analysisItem, filter))
    {
      Relation relation;
      relation.ancestor = find(edge.source);
      relation.successor = find(edge.target);
      relation.relation = edge.relationship.c_str();
      relations << relation;
    }
  }

  if (ESPINA::RELATION_OUT == type || ESPINA::RELATION_INOUT == type)
  {
    for(auto edge : m_analysis->relationships()->outEdges(item->m_analysisItem, filter))
    {
      Relation relation;
      relation.ancestor = find(edge.source);
      relation.successor = find(edge.target);
      relation.relation = edge.relationship.c_str();
      relations << relation;
    }
  }

  return relations;
}


//------------------------------------------------------------------------
void ModelAdapter::addCategory(CategoryAdapterSPtr category, CategoryAdapterSPtr parent)
{
  QModelIndex index = categoryIndex(parent);
  auto row = rowCount(index);

  beginInsertRows(index, row, row);
  {
    parent->addSubCategory(category);
  }
  endInsertRows();
}

//------------------------------------------------------------------------
void ModelAdapter::removeCategory(CategoryAdapterSPtr category, CategoryAdapterSPtr parent)
{
  QModelIndex index = categoryIndex(category);

  beginRemoveRows(index.parent(), index.row(), index.row());
  {
    parent->removeSubCategory(category);
  }
  endRemoveRows();
}

//------------------------------------------------------------------------
void ModelAdapter::removeRootCategory(CategoryAdapterSPtr category)
{
  removeCategory(category, m_classification->root());
}

//------------------------------------------------------------------------
void ModelAdapter::reparentCategory(CategoryAdapterSPtr category, CategoryAdapterSPtr parent)
{
  auto previousParent = category->parent();

  if (previousParent == parent.get())
    return;

  QModelIndex oldIndex = index(previousParent);
  QModelIndex newIndex = index(parent);

  int oldRow = previousParent->subCategories().indexOf(category);
  int newRow = parent->subCategories().size();

  beginMoveRows(oldIndex, oldRow, oldRow, newIndex, newRow);
  {
    previousParent->removeSubCategory(category.get());
    parent->addSubCategory(category);
  }
  endMoveRows();

  for(auto segmentation : m_segmentations)
  {
    if (segmentation->category() == category)
    {
      QModelIndex segIndex = segmentationIndex(segmentation);
      emit dataChanged(segIndex, segIndex);
    }
  }
}

//------------------------------------------------------------------------
void ModelAdapter::clear()
{
  Q_ASSERT(!m_isBatchMode);

  beginResetModel();
  {
    m_analysis->clear();

    resetInternalData();
  }
  endResetModel();
}

//------------------------------------------------------------------------
int ModelAdapter::rowCount(const QModelIndex& parent) const
{
  int count = 0;

  // There are 4 root indexes
  if ( !parent.isValid() )
  {
    count = 4;
  }
  else if ( parent == classificationRoot() )
  {
    count = m_classification?m_classification->categories().size():0;
  }
  else if ( parent == sampleRoot() )
  {
    count = m_samples.size();
  }
  else if ( parent == channelRoot() )
  {
    count = m_channels.size();
  }
  else if ( parent == segmentationRoot() )
  {
    count = m_segmentations.size();
  }
  else
  {
    // Cast to base type
    auto parentItem = itemAdapter(parent);
    if (isCategory(parentItem))
    {
      auto parentCategory = categoryPtr(parentItem);
      count = parentCategory->subCategories().size();
    }
  }

  // Otherwise Samples and Segmentations have no children
  return count;
}

//------------------------------------------------------------------------
QModelIndex ModelAdapter::sampleIndex(SampleAdapterPtr sample) const
{
  QModelIndex index;

  int row = 0;
  for(auto ptr : m_samples)
  {
    if (ptr.get() == sample)
    {
      ItemAdapterPtr internalPtr = sample;
      index = createIndex(row, 0, internalPtr);
    }
    row++;
  }

  return index;
}

//------------------------------------------------------------------------
QModelIndex ModelAdapter::sampleIndex(SampleAdapterSPtr sample) const
{
  return sampleIndex(sample.get());
}

QModelIndex ModelAdapter::sampleRoot() const
{
  return createIndex(1, 0, 1);
}

//------------------------------------------------------------------------
QModelIndex ModelAdapter::segmentationIndex(SegmentationAdapterPtr segmentation) const
{
  QModelIndex index;

  int row = 0;
  for(auto ptr : m_segmentations)
  {
    if (ptr.get() == segmentation)
    {
      ItemAdapterPtr internalPtr = segmentation;
      index = createIndex(row, 0, internalPtr);
    }
    ++row;
  }

  return index;
}

//------------------------------------------------------------------------
QModelIndex ModelAdapter::segmentationIndex(SegmentationAdapterSPtr segmentation) const
{
  return segmentationIndex(segmentation.get());
}

//------------------------------------------------------------------------
QModelIndex ModelAdapter::segmentationRoot() const
{
  return createIndex(3, 0, 3);
}

//------------------------------------------------------------------------
void ModelAdapter::beginBatchMode()
{
  m_isBatchMode = true;
}

//------------------------------------------------------------------------
void ModelAdapter::endBatchMode()
{
  m_isBatchMode = false;

  m_timer->increment();

  executeAddCommands();

  executeUpdateCommands();

  executeRemoveCommands();
}

//------------------------------------------------------------------------
void ModelAdapter::setClassification(ClassificationAdapterSPtr classification)
{
  if (m_classification)
  {
    ClassificationAdapterSPtr oldClassification = classification;
    beginRemoveRows(classificationRoot(), 0, rowCount(classificationRoot()) - 1);
    m_classification.reset();
    endRemoveRows();

    emit classificationRemoved(oldClassification);
  }

  if (classification)
  {
    beginInsertRows(classificationRoot(), 0, classification->root()->subCategories().size() - 1);
    m_classification = classification;
    m_analysis->setClassification(classification->m_classification);
    endInsertRows();

    emit classificationAdded(m_classification);
  }
}

//------------------------------------------------------------------------
bool ModelAdapter::setData(const QModelIndex& index, const QVariant& value, int role)
{
  bool result = false;
  if (index.isValid() && index.parent().isValid())// Root indexes cannot be modified
  {
    // Other elements can set their own data
    ItemAdapterPtr indexItem = itemAdapter(index);
    result = indexItem->setData(value, role);
    if (result)
    {
      emit dataChanged(index,index);
      if (isCategory(indexItem))
      {
        CategoryAdapterPtr category = categoryPtr(indexItem);
        for(auto segmentation: m_segmentations)
        {
          if (segmentation->category().get() == category)
          {
            QModelIndex index = segmentationIndex(segmentation);
            emit dataChanged(index, index);
          }
        }
      }
    }
  }
  return result;
}

//------------------------------------------------------------------------
void ModelAdapter::setSegmentationCategory(SegmentationAdapterSPtr segmentation, CategoryAdapterSPtr category)
{
  auto command = [this, segmentation, category]() {
    segmentation->setCategory(category);
  };

  queueUpdateCommand(segmentation, std::make_shared<Command<decltype(command)>>(command));

  executeCommandsIfNoBatchMode();
}

//------------------------------------------------------------------------
void ModelAdapter::notifyRepresentationsModified(ViewItemAdapterSList items)
{
  m_timer->increment();

  emit representationsModified(items, m_timer->timeStamp());
}

//------------------------------------------------------------------------
void ModelAdapter::notifyDependentRepresentationsModified(ViewItemAdapterSList items)
{
  m_timer->increment();

  auto notificationItems = items;
  // TODO: add dependent items

  emit representationsModified(notificationItems, m_timer->timeStamp());
}

//------------------------------------------------------------------------
ItemAdapterSPtr ModelAdapter::find(PersistentSPtr item)
{
  for(auto sample : m_samples)
  {
    PersistentSPtr base = sample->m_sample;
    if (base == item) return sample;
  }

  for(auto channel : m_channels)
  {
    PersistentSPtr base = channel->m_channel;
    if (base == item) return channel;
  }

  for(auto segmentation : m_segmentations)
  {
    PersistentSPtr base = segmentation->m_segmentation;
    if (base == item) return segmentation;
  }

  return ItemAdapterSPtr();
}

//------------------------------------------------------------------------
CategoryAdapterSPtr ModelAdapter::smartPointer(CategoryAdapterPtr category)
{
  if (category == m_classification->root().get())
    return m_classification->root();

  auto parent = category->parent();

  return parent->subCategory(category->name());
}

//------------------------------------------------------------------------
SampleAdapterSPtr ModelAdapter::smartPointer(SampleAdapterPtr sample)
{
  SampleAdapterSPtr pointer;

  int i=0;
  while (!pointer && i < m_samples.size())
  {
    if (m_samples[i].get() == sample)
      pointer = m_samples[i];
    i++;
  }

  return pointer;
}

//------------------------------------------------------------------------
ChannelAdapterSPtr ModelAdapter::smartPointer(ChannelAdapterPtr channel)
{
  ChannelAdapterSPtr pointer;

  int i=0;
  while (!pointer && i < m_channels.size())
  {
    if (m_channels[i].get() == channel)
      pointer = m_channels[i];
    i++;
  }

  return pointer;
}

//------------------------------------------------------------------------
SegmentationAdapterSPtr ModelAdapter::smartPointer(SegmentationAdapterPtr segmentation)
{
  SegmentationAdapterSPtr pointer;

  int i=0;
  while (!pointer && i < m_segmentations.size())
  {
    if (m_segmentations[i].get() == segmentation)
      pointer = m_segmentations[i];
    i++;
  }

  return pointer;
}

//------------------------------------------------------------------------
void ModelAdapter::resetInternalData()
{
  m_segmentations.clear();
  m_channels.clear();
  m_samples.clear();
  m_classification.reset();
}

//------------------------------------------------------------------------
bool ModelAdapter::contains(ItemAdapterSPtr &item, const ItemCommandsList &list) const
{
  for (auto itemCommands : list) {
    if (itemCommands.Item == item) return true;
  }

  return false;
}

//------------------------------------------------------------------------
int ModelAdapter::find(ItemAdapterSPtr &item, const ItemCommandsList &list) const
{
  for (int i = 0; i < list.size(); ++i)
  {
    if (list[i].Item == item)
    {
      return i;
    }
  }

  return -1;
}

//------------------------------------------------------------------------
void ModelAdapter::remove(ItemAdapterSPtr &item, ItemCommandsList &list)
{
  int i = find(item, list);
  if (i >= 0 && i < list.size())
  {
    list.removeAt(i);
  }
}

//------------------------------------------------------------------------
void ModelAdapter::classifyQueues(const ItemCommandsList &queues,
                                  ItemCommandsList       &samplesQueues,
                                  ItemCommandsList       &channelQueues,
                                  ItemCommandsList       &segmentationQueues)
{
  samplesQueues.clear();
  channelQueues.clear();
  segmentationQueues.clear();

  for (auto itemCommands : queues)
  {
    switch (itemCommands.Item->type())
    {
      case ItemAdapter::Type::SAMPLE:
        samplesQueues << itemCommands;
        break;
      case ItemAdapter::Type::CHANNEL:
        channelQueues << itemCommands;
        break;
      case ItemAdapter::Type::SEGMENTATION:
        segmentationQueues << itemCommands;
        break;
      default:
        qWarning() << "Unexpected item commands";
    };
  }
}

//------------------------------------------------------------------------
SampleAdapterSList ModelAdapter::queuedSamples(const ModelAdapter::ItemCommandsList& queue) const
{
  SampleAdapterSList samples;

  for (auto command : queue)
  {
    samples << std::dynamic_pointer_cast<SampleAdapter>(command.Item);
  }

  return samples;
}

//------------------------------------------------------------------------
ChannelAdapterSList ModelAdapter::queuedChannels(const ModelAdapter::ItemCommandsList& queue) const
{
  ChannelAdapterSList channels;

  for (auto command : queue)
  {
    channels << std::dynamic_pointer_cast<ChannelAdapter>(command.Item);
  }

  return channels;
}

//------------------------------------------------------------------------
SegmentationAdapterSList ModelAdapter::queuedSegmentations(const ModelAdapter::ItemCommandsList& queue) const
{
  SegmentationAdapterSList segmentations;

  for (auto command : queue)
  {
    segmentations << std::dynamic_pointer_cast<SegmentationAdapter>(command.Item);
  }

  return segmentations;
}

//------------------------------------------------------------------------
void ModelAdapter::queueAddRelationCommand(ItemAdapterSPtr ancestor,
                                           ItemAdapterSPtr successor,
                                           const QString  &relation)
{
  // In case no item needs to be added, it doesn't matter which is used to keep the refernce
  ItemAdapterSPtr commandQueueItem = ancestor;
  ItemAdapterSPtr emptyQueueItem   = successor;

  // Both reference items must be added in order to add relations
  // As additions are queued the last one of them which is found
  // will keep the reference to the update command
  for (auto itemCommand : m_addCommands)
  {
    if (itemCommand.Item == ancestor)
    {
      commandQueueItem = ancestor;
      emptyQueueItem   = successor;
    }
    else if (itemCommand.Item == successor)
    {
      commandQueueItem = successor;
      emptyQueueItem   = ancestor;
    }
  }

  auto noOpCommand = [](){};

  queueUpdateCommand(commandQueueItem, addRelationCommand(ancestor, successor, relation));
  queueUpdateCommand(emptyQueueItem, std::make_shared<Command<decltype(noOpCommand)>>(noOpCommand));
}

//------------------------------------------------------------------------
void ModelAdapter::queueAddCommand(ItemAdapterSPtr item,
                                   BatchCommandSPtr command)
{
  int i = find(item, m_removeCommands);

  if (0 <= i && i < m_removeCommands.size())
  {
    m_removeCommands.at(i);
  }
  else if (contains(item, m_addCommands) || contains(item, m_updateCommands))
  {
    throw Existing_Item_Exception();
  }
  else
  {
    ItemCommands itemCommands;

    itemCommands.Item = item;
    itemCommands.Commands.push_back(command);

    m_addCommands.push_back(itemCommands);
  }
}

//------------------------------------------------------------------------
void ModelAdapter::queueUpdateCommand(ItemAdapterSPtr  item,
                                      BatchCommandSPtr command)
{
  if (contains(item, m_removeCommands))
  {
    throw Item_Not_Found_Exception();
  }

  int i = find(item, m_addCommands);

  if (i >= 0)
  {
    m_addCommands[i].Commands.push_back(command);
  }
  else
  {
    i = find(item, m_updateCommands);

    if (i == -1)
    {
      i = m_updateCommands.size();

      ItemCommands itemCommands;
      itemCommands.Item = item;
      m_updateCommands.push_back(itemCommands);
    }

    m_updateCommands[i].Commands.push_back(command);
  }
}

//------------------------------------------------------------------------
void ModelAdapter::queueRemoveCommand(ItemAdapterSPtr  item,
                                      BatchCommandSPtr command)
{
  int i = find(item, m_addCommands);

  if (0 <= i && i < m_addCommands.size())
  {
    m_addCommands.removeAt(i);
  }
  else
  {
    i = find(item, m_updateCommands);

    if (0 <= i && i < m_updateCommands.size())
    {
      m_updateCommands.removeAt(i);
    }

    if (contains(item, m_removeCommands)) throw Item_Not_Found_Exception();

    ItemCommands itemCommands;
    itemCommands.Item     =  item;
    itemCommands.Commands << command;

    m_removeCommands.push_back(itemCommands);
  }
}


//------------------------------------------------------------------------
void ModelAdapter::executeCommandsIfNoBatchMode()
{
  if (!m_isBatchMode) endBatchMode();
}

//------------------------------------------------------------------------
void ModelAdapter::executeAddCommands()
{
  ItemCommandsList sampleQueues, channelQueues, segmentationQueues;

  classifyQueues(m_addCommands, sampleQueues, channelQueues, segmentationQueues);

  // Insert Row Signals are grouped by parent node
  executeAddQueues(sampleRoot(),       sampleQueues);
  executeAddQueues(channelRoot(),      channelQueues);
  executeAddQueues(segmentationRoot(), segmentationQueues);

  auto samples = queuedSamples(sampleQueues);
  if (!samples.isEmpty())
  {
    emit samplesAdded(samples);
  }

  auto channels = queuedChannels(channelQueues);
  if (!channels.isEmpty())
  {
    emit channelsAdded(toViewItemList(channels), m_timer->timeStamp());
    emit channelsAdded(channels, m_timer->timeStamp());
  }

  auto segmentations = queuedSegmentations(segmentationQueues);
  if (!segmentations.isEmpty())
  {
    emit segmentationsAdded(segmentations, m_timer->timeStamp());
  }

  m_addCommands.clear();
}

//------------------------------------------------------------------------
void ModelAdapter::executeUpdateCommands()
{
  ItemCommandsList sampleQueues, channelQueues, segmentationQueues;

  // Data Changed Signals are grouped by consecutive indices
  classifyQueues(m_updateCommands, sampleQueues, channelQueues, segmentationQueues);

  executeUpdateQueues(sampleQueues);
  executeUpdateQueues(channelQueues);
  executeUpdateQueues(segmentationQueues);

  m_updateCommands.clear();
}

//------------------------------------------------------------------------
void ModelAdapter::executeRemoveCommands()
{
  ItemCommandsList sampleQueues, channelQueues, segmentationQueues;

  classifyQueues(m_removeCommands, sampleQueues, channelQueues, segmentationQueues);

  auto samples = queuedSamples(sampleQueues);
  if (!samples.isEmpty())
  {
    emit samplesAboutToBeRemoved(samples);
  }

  auto channels = queuedChannels(channelQueues);
  if (!channels.isEmpty())
  {
    emit channelsAboutToBeRemoved(channels, m_timer->timeStamp());
  }

  auto segmentations = queuedSegmentations(segmentationQueues);
  if (!segmentations.isEmpty())
  {
    emit segmentationsAboutToBeRemoved(segmentations, m_timer->timeStamp());
  }
  // Remove Row Signals are grouped by parent node
  executeRemoveQueues(sampleRoot(),       sampleQueues);
  executeRemoveQueues(channelRoot(),      channelQueues);
  executeRemoveQueues(segmentationRoot(), segmentationQueues);

  if (!samples.isEmpty())
  {
    emit samplesRemoved(samples);
  }

  if (!channels.isEmpty())
  {
    emit channelsRemoved(channels, m_timer->timeStamp());
  }

  if (!segmentations.isEmpty())
  {
    emit segmentationsRemoved(segmentations, m_timer->timeStamp());
  }

  m_removeCommands.clear();
}

//------------------------------------------------------------------------
void ModelAdapter::executeAddQueues(QModelIndex parent,
                                    ItemCommandsList &queueList)
{
  if (!queueList.isEmpty())
  {
    int start = rowCount(parent);
    int end   = start + queueList.size() - 1;

    beginInsertRows(parent, start, end);
    // Execute first commands which should add items to the model
    for(auto &itemCommands : queueList)
    {
      auto addCommand = itemCommands.Commands.takeFirst();
      addCommand->execute();
    }

    // Once all items are added, then we can execute the remaining commands
    for(auto &itemCommands : queueList)
    {
      while (!itemCommands.Commands.isEmpty())
      {
        auto command = itemCommands.Commands.takeFirst();
        command->execute();
      }
    }
    endInsertRows();
  }
}

//------------------------------------------------------------------------
void ModelAdapter::executeUpdateQueues(ItemCommandsList &queueList)
{
  for (auto consecutiveQueues : groupConsecutiveQueues(queueList))
  {
    for (auto &command : consecutiveQueues.Commands)
    {
      command->execute();
    }
    emit dataChanged(consecutiveQueues.StartIndex, consecutiveQueues.EndIndex);
  }
}

//------------------------------------------------------------------------
void ModelAdapter::executeRemoveQueues(QModelIndex parent, ItemCommandsList &queueList)
{
  unsigned shift = 0; // we need to correct indices after removal

  for (auto commandQueues : groupConsecutiveQueues(queueList))
  {
    if (!commandQueues.StartIndex.isValid()
      ||!commandQueues.EndIndex  .isValid())
    {
      throw Item_Not_Found_Exception();
    }

    Q_ASSERT(commandQueues.StartIndex.parent() == parent);
    Q_ASSERT(commandQueues.EndIndex  .parent() == parent);

    int start = commandQueues.StartIndex.row() - shift;
    int end   = commandQueues.EndIndex  .row() - shift;

    shift += end - start + 1;

    beginRemoveRows(parent, start, end);
    for (auto command : commandQueues.Commands)
    {
      command->execute();
    }
    endRemoveRows();
  }
}


//------------------------------------------------------------------------
ModelAdapter::ConsecutiveQueuesList ModelAdapter::groupConsecutiveQueues(ItemCommandsList &queueList)
{
  using IndexedCommand = QPair<QModelIndex, CommandQueue>;

  ConsecutiveQueuesList result;

  if (!queueList.isEmpty())
  {
    QList<IndexedCommand> indexedCommands;

    for (auto itemCommands : queueList)
    {
      auto itemIndex = index(itemCommands.Item);

      int  i     = 0;
      bool found = false;
      while (i < indexedCommands.size() && !found)
      {
        found = indexedCommands[i].first.row() > itemIndex.row();

        if (!found) ++i;
      }

      indexedCommands.insert(i, IndexedCommand(itemIndex, itemCommands.Commands));
    }

    CommandQueue batch;

    QModelIndex batchStartIndex;
    QModelIndex batchEndIndex;
    QModelIndex batchNextIndex;

    for (int i = 0; i <= indexedCommands.size(); ++i)
    {
      if (i < indexedCommands.size())
      {
        IndexedCommand &indexedCommand = indexedCommands[i];

        batchNextIndex = indexedCommand.first;

        if (batch.isEmpty())
        {
          batchStartIndex = batchNextIndex;
          batchEndIndex   = batchNextIndex;
        }
      }

      Q_ASSERT(batchNextIndex.row() >= batchStartIndex.row());

      // Not consecutive indices
      if ( batchNextIndex.row() - batchEndIndex.row() > 1
        || i == indexedCommands.size())
      {
        if (!batch.isEmpty())
        {
          ConsecutiveQueues consecutiveQueues;
          consecutiveQueues.StartIndex = batchStartIndex;
          consecutiveQueues.EndIndex   = batchEndIndex;
          consecutiveQueues.Commands  << batch;

          result << consecutiveQueues;

          batch.clear();
          batchStartIndex = batchNextIndex;
        }
      }

      if (i < indexedCommands.size())
      {
        batch << indexedCommands[i].second;
        batchEndIndex = batchNextIndex;
      }
    }
  }

  return result;
}

//------------------------------------------------------------------------
ModelAdapter::BatchCommandSPtr ModelAdapter::addSampleCommand(SampleAdapterSPtr sample) throw(Existing_Item_Exception)
{
  auto command = [this, sample]() {
    if (m_samples.contains(sample)) throw Existing_Item_Exception();

    m_analysis->add(sample->m_sample);
    m_samples << sample;

    sample->setModel(this);
  };

  return std::make_shared<Command<decltype(command)>>(command);
}


//------------------------------------------------------------------------
ModelAdapter::BatchCommandSPtr ModelAdapter::addSegmentationCommand(SegmentationAdapterSPtr segmentation) throw(Existing_Item_Exception)
{
  auto command = [this, segmentation]() {
    if (m_segmentations.contains(segmentation)) throw Existing_Item_Exception();

    m_analysis->add(segmentation->m_segmentation);
    m_segmentations << segmentation;

    segmentation->setModel(this);
  };

  return std::make_shared<Command<decltype(command)>>(command);
}

//------------------------------------------------------------------------
ModelAdapter::BatchCommandSPtr ModelAdapter::addRelationCommand(ItemAdapterSPtr    ancestor,
                                                                ItemAdapterSPtr    successor,
                                                                const RelationName &relation)
{
  auto command = [this, ancestor, successor, relation]() {
    try
    {
      m_analysis->addRelation(ancestor->m_analysisItem, successor->m_analysisItem, relation);
    }
    catch (const Analysis::Existing_Relation_Exception &e)
    {
      throw Existing_Relation_Exception();
    }
  };

  return std::make_shared<Command<decltype(command)>>(command);
}

//------------------------------------------------------------------------
ModelAdapter::BatchCommandSPtr ModelAdapter::removeSampleCommand(SampleAdapterSPtr sample)
{
  auto command = [this, sample]() {
    if (!m_samples.contains(sample)) throw Item_Not_Found_Exception();

    m_analysis->remove(sample->m_sample);
    m_samples.removeOne(sample);

    sample->setModel(nullptr);

    Q_ASSERT (!m_samples.contains(sample));
  };

  return std::make_shared<Command<decltype(command)>>(command);
}

//------------------------------------------------------------------------
ModelAdapter::BatchCommandSPtr ModelAdapter::removeChannelCommand(ChannelAdapterSPtr channel)
{
  auto command = [this, channel]() {
    if (!m_channels.contains(channel)) throw Item_Not_Found_Exception();

    m_analysis->remove(channel->m_channel);
    m_channels.removeOne(channel);

    channel->setModel(nullptr);

    Q_ASSERT (!m_channels.contains(channel));
  };

  return std::make_shared<Command<decltype(command)>>(command);
}

//------------------------------------------------------------------------
ModelAdapter::BatchCommandSPtr ModelAdapter::removeSegmentationCommand(SegmentationAdapterSPtr segmentation)
{
  auto command = [this, segmentation]() {
    if (!m_segmentations.contains(segmentation)) throw Item_Not_Found_Exception();

    m_analysis->remove(segmentation->m_segmentation);
    m_segmentations.removeOne(segmentation);

    segmentation->setModel(nullptr);

    Q_ASSERT (!m_segmentations.contains(segmentation));
  };

  return std::make_shared<Command<decltype(command)>>(command);
}


//------------------------------------------------------------------------
ItemAdapterPtr ESPINA::itemAdapter(const QModelIndex& index)
{
  return static_cast<ItemAdapterPtr>(index.internalPointer());
}

//------------------------------------------------------------------------
bool ESPINA::isClassification(ItemAdapterPtr item)
{
  return ItemAdapter::Type::CLASSIFICATION == item->type();
}

//------------------------------------------------------------------------
bool ESPINA::isCategory(ItemAdapterPtr item)
{
  return ItemAdapter::Type::CATEGORY == item->type();
}

//------------------------------------------------------------------------
// Qt::ItemFlags ModelAdapter::flags(const QModelIndex& index) const
// {
//   if (!index.isValid())
//     return QAbstractItemModel::flags(index);
//
//   if (index == taxonomyRoot() ||
//       index == sampleRoot()   ||
//       index == channelRoot()  ||
//       index == filterRoot()   ||
//       index == segmentationRoot()
//      )
//     return QAbstractItemModel::flags(index);
//
//   ModelItemPtr item = indexPtr(index);
//   if (SEGMENTATION == item->type() || CHANNEL == item->type())
//     return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
//   else
//     return QAbstractItemModel::flags(index);
// }
//
//
//------------------------------------------------------------------------
// void ModelAdapter::removeSample(SampleSPtr sample)
// {
//   Q_ASSERT(m_samples.contains(sample));
//
//   QModelIndex index = sampleIndex(sample.get());
//   beginRemoveRows(index.parent(), index.row(), index.row());
//   {
//     removeSampleImplementation(sample);
//   }
//   endRemoveRows();
//
//   emit sampleRemoved(sample);
//   markAsChanged();
//   Q_ASSERT (!m_samples.contains(sample));
// }
//
//
//------------------------------------------------------------------------
// void ModelAdapter::emitChannelAdded(ChannelSList channels)
// {
//   foreach(ChannelSPtr channel, channels)
//     emit channelAdded(channel);
// }
//
//------------------------------------------------------------------------
// void ModelAdapter::removeChannel(ChannelSPtr channel)
// {
//   Q_ASSERT(m_channels.contains(channel));
//
//   QModelIndex index = channelIndex(channel.get());
//   beginRemoveRows(index.parent(), index.row(), index.row());
//   {
//     removeChannelImplementation(channel);
//   }
//   endRemoveRows();
//
//   emit channelRemoved(channel);
//   markAsChanged();
//
//   Q_ASSERT (!m_channels.contains(channel));
// }
//
//
//------------------------------------------------------------------------
// void ModelAdapter::emitSegmentationAdded(SegmentationSList segmentations)
// {
//   foreach(SegmentationSPtr segmentation, segmentations)
//     emit segmentationAdded(segmentation);
// }
//
//------------------------------------------------------------------------
// void ModelAdapter::removeSegmentation(SegmentationSPtr segmentation)
// {
//   Q_ASSERT(m_segmentations.contains(segmentation));
//
//   QModelIndex index = segmentationIndex(segmentation.get());
//   beginRemoveRows(index.parent(), index.row(), index.row());
//   {
//     removeSegmentationImplementation(segmentation);
//   }
//   endRemoveRows();
//
//   emit segmentationRemoved(segmentation);
//   markAsChanged();
//
//   Q_ASSERT (!m_segmentations.contains(segmentation));
// }
//
//------------------------------------------------------------------------
// void ModelAdapter::removeSegmentation(SegmentationSList segs)
// {
//   foreach(SegmentationSPtr seg, segs)
//   {
//     removeSegmentation(seg);
//     emit segmentationRemoved(seg);
//   }
// }
//
//------------------------------------------------------------------------
// void ModelAdapter::addFilter(FilterSPtr filter)
// {
//   int row = m_filters.size();
//
//   beginInsertRows(filterRoot(), row, row);
//   {
//     addFilterImplementation(filter);
//   }
//   endInsertRows();
//
//   emit filterAdded(filter);
//   markAsChanged();
// }
//
//------------------------------------------------------------------------
// void ModelAdapter::addFilter(FilterSList filters)
// {
//   int start = m_filters.size();
//   int end   = start + filters.size() - 1;
//
//   beginInsertRows(filterRoot(), start, end);
//   {
//     foreach(FilterSPtr filter, filters)
//       addFilterImplementation(filter);
//   }
//   endInsertRows();
//
//   foreach(FilterSPtr filter, filters)
//     emit filterAdded(filter);
//   markAsChanged();
// }
//
//------------------------------------------------------------------------
// void ModelAdapter::removeFilter(FilterSPtr filter)
// {
//   Q_ASSERT(m_filters.contains(filter));
//
//   QModelIndex index = filterIndex(filter.get());
//   beginRemoveRows(index.parent(), index.row(), index.row());
//   {
//     removeFilterImplementation(filter);
//   }
//   endRemoveRows();
//
//   emit filterRemoved(filter);
//   markAsChanged();
// }
//
//------------------------------------------------------------------------
// void ModelAdapter::changeTaxonomy(SegmentationSPtr    segmentation,
//                                  TaxonomyElementSPtr taxonomy)
// {
//   segmentation->setTaxonomy(taxonomy);
//
//   QModelIndex segIndex = segmentationIndex(segmentation.get());
//   emit dataChanged(segIndex, segIndex);
//
//   markAsChanged();
// }
//
//
//------------------------------------------------------------------------
// void ModelAdapter::changeTaxonomyParent(TaxonomyElementSPtr subTaxonomy,
//                                        TaxonomyElementSPtr parent)
// {
//   TaxonomyElementPtr oldParent = subTaxonomy->parent();
//
//   if (oldParent == parent.get())
//     return;
//
//   QModelIndex oldIndex = index(subTaxonomy).parent();
//   QModelIndex newIndex = index(parent);
//
//   int oldRow = oldParent->subElements().indexOf(subTaxonomy);
//   int newRow = parent->subElements().size();
//
//   beginMoveRows(oldIndex, oldRow, oldRow, newIndex, newRow);
//   {
//     oldParent->deleteElement(subTaxonomy.get());
//     parent->addElement(subTaxonomy);
//   }
//   endMoveRows();
//
//   foreach(SegmentationSPtr segmentation, m_segmentations)
//   {
//     if (segmentation->taxonomy() == subTaxonomy)
//     {
//       QModelIndex segIndex = segmentationIndex(segmentation);
//       emit dataChanged(segIndex, segIndex);
//     }
//   }
//
// }
//
//
//------------------------------------------------------------------------
// void ModelAdapter::addRelation(ModelItemSPtr  ancestor,
//                               ModelItemSPtr  successor,
//                               const QString &relation)
// {
//   m_relations->addRelation(ancestor.get(), successor.get(), relation);
//
//   QModelIndex ancestorIndex  = index(ancestor);
//   QModelIndex successorIndex = index(successor);
//
//   emit dataChanged(ancestorIndex,  ancestorIndex);
//   emit dataChanged(successorIndex, successorIndex);
//   markAsChanged();
// }
//
//------------------------------------------------------------------------
// void ModelAdapter::removeRelation(ModelItemSPtr  ancestor,
//                                  ModelItemSPtr  successor,
//                                  const QString &relation)
// {
//   m_relations->removeRelation(ancestor.get(), successor.get(), relation);
//
//   QModelIndex ancestorIndex = index(ancestor);
//   QModelIndex succesorIndex = index(successor);
//
//   emit dataChanged(ancestorIndex, ancestorIndex);
//   emit dataChanged(succesorIndex, succesorIndex);
//   markAsChanged();
// }
//
//------------------------------------------------------------------------
// ModelItemSList ModelAdapter::relatedItems(ModelItemPtr   item,
//                                          RelationType   relType,
//                                          const QString &relName)
// {
//   ModelItemSList res;
//
//   RelationshipGraph::Vertex vertex = m_relations->vertex(item);
//
//   if (relType == RELATION_IN || relType == RELATION_INOUT)
//     foreach(RelationshipGraph::Vertex v, m_relations->ancestors(vertex, relName))
//       res << find(v.item);
//
//   if (relType == RELATION_OUT || relType == RELATION_INOUT)
//     foreach(RelationshipGraph::Vertex v, m_relations->succesors(vertex, relName))
//       res << find(v.item);
//
//   return res;
// }
//
//------------------------------------------------------------------------
// RelationList ModelAdapter::relations(ModelItemPtr   item,
//                                     const QString &relName)
// {
//   RelationList res;
//
//   RelationshipGraph::Vertex vertex = m_relations->vertex(item);
//
//   foreach(RelationshipGraph::Edge edge, m_relations->edges(vertex, relName))
//   {
//     Relation rel;
//     rel.ancestor = find(edge.source.item);
//     rel.succesor = find(edge.target.item);
//     rel.relation = edge.relationship.c_str();
//     res << rel;
//   }
//
//   return res;
// }
//
//
//
//------------------------------------------------------------------------
// QModelIndex ModelAdapter::index(ModelItemPtr item) const
// {
//   QModelIndex res;
//   switch (item->type())
//   {
//     case TAXONOMY:
//       res = taxonomyIndex(taxonomyElementPtr(item));
//       break;
//     case SAMPLE:
//       res = sampleIndex(samplePtr(item));
//       break;
//     case CHANNEL:
//       res = channelIndex(channelPtr(item));
//       break;
//     case FILTER:
//       res = filterIndex(filterPtr(item));
//       break;
//     case SEGMENTATION:
//       res = segmentationIndex(segmentationPtr(item));
//       break;
//     default:
//       Q_ASSERT(false);
//       break;
//   }
//   return res;
// }
//
//------------------------------------------------------------------------
// QModelIndex ModelAdapter::index(ModelItemSPtr item) const
// {
//   return index(item.get());
// }
//
//
//
//------------------------------------------------------------------------
// void ModelAdapter::itemModified(ModelItemPtr item)
// {
//   QModelIndex itemIndex = index(item);
//   emit dataChanged(itemIndex, itemIndex);
// }
//
//------------------------------------------------------------------------
// void ModelAdapter::addTaxonomy(TaxonomyElementSPtr root)
// {
//   Q_ASSERT(false);//DEPRECATED?
//   foreach (TaxonomyElementSPtr node, root->subElements())
//   {
//     addTaxonomyElement(node->qualifiedName(), m_tax->root());
//     addTaxonomy(node);
//   }
//
//   markAsChanged();
// }
//
//------------------------------------------------------------------------
// void ModelAdapter::addSampleImplementation(SampleSPtr sample)
// {
//   Q_ASSERT(sample);
//   Q_ASSERT(!m_samples.contains(sample));
//
//   sample->m_model = this;
//   m_samples << sample;
//   m_relations->addItem(sample.get());
//
//   connect(sample.get(), SIGNAL(modified(ModelItemPtr)),
//           this, SLOT(itemModified(ModelItemPtr)));
// }
//
//------------------------------------------------------------------------
// void ModelAdapter::removeSampleImplementation(SampleSPtr sample)
// {
//   Q_ASSERT(sample);
//   Q_ASSERT(relations(sample.get()).isEmpty());
//
//   m_samples.removeOne(sample);
//   m_relations->removeItem(sample.get());
//
//   sample->m_model = NULL;
// }
//
//------------------------------------------------------------------------
// void ModelAdapter::removeChannelImplementation(ChannelSPtr channel)
// {
//   Q_ASSERT(channel != NULL);
//
//   channel->invalidateExtensions();
//
//   m_channels.removeOne(channel);
//   m_relations->removeItem(channel.get());
//
//   channel->m_model = NULL;
// }
//
//------------------------------------------------------------------------
// void ModelAdapter::removeSegmentationImplementation(SegmentationSPtr segmentation)
// {
//   Q_ASSERT(segmentation);
//
//   segmentation->invalidateExtensions();
//
//   m_segmentations.removeOne(segmentation);
//   m_relations->removeItem(segmentation.get());
//
//   segmentation->m_model = NULL;
// }
//
//------------------------------------------------------------------------
// void ModelAdapter::addFilterImplementation(FilterSPtr filter)
// {
//   Q_ASSERT(!m_filters.contains(filter));
//
//   filter->m_model = this;
//   filter->setTraceable(m_isTraceable);
//   m_filters << filter;
//   m_relations->addItem(filter.get());
// }
//
//------------------------------------------------------------------------
// void ModelAdapter::removeFilterImplementation(FilterSPtr filter)
// {
//   m_filters.removeOne(filter);
//   m_relations->removeItem(filter.get());
//
//   filter->m_model = NULL;
// }
//
//
//------------------------------------------------------------------------
// ModelItemSPtr ModelAdapter::find(ModelItemPtr item)
// {
//   ModelItemSPtr res;
//   switch (item->type())
//   {
//     case ESPINA::TAXONOMY:
//       res = findTaxonomyElement(item);
//       break;
//     case ESPINA::SAMPLE:
//       res = findSample(item);
//       break;
//     case ESPINA::CHANNEL:
//       res = findChannel(item);
//       break;
//     case ESPINA::FILTER:
//       res = findFilter(item);
//       break;
//     case ESPINA::SEGMENTATION:
//       res = findSegmentation(item);
//       break;
//   };
//
//   return res;
// }
//
//------------------------------------------------------------------------
// FilterSPtr ModelAdapter::findFilter(ModelItemPtr item)
// {
//   return findFilter(filterPtr(item));
// }
//
//------------------------------------------------------------------------
// FilterSPtr ModelAdapter::findFilter(FilterPtr filter)
// {
//   FilterSPtr res;
//
//   int i=0;
//   while (!res && i < m_filters.size())
//   {
//     if (m_filters[i].get() == filter)
//       res = m_filters[i];
//     i++;
//   }
//
//   return res;
// }
//
//------------------------------------------------------------------------
// SegmentationSPtr ModelAdapter::findSegmentation(ModelItemPtr item)
// {
//   return findSegmentation(segmentationPtr(item));
// }
//
//------------------------------------------------------------------------
// TaxonomyElementSPtr ModelAdapter::createTaxonomyElement(TaxonomyElementPtr parent, const QString &name)
// {
//   TaxonomyElementPtr parentNode = m_tax->root().get();
//   if (parent)
//     parentNode = parent;
//
//   Q_ASSERT(!parentNode->element(name));
//
//   TaxonomyElementSPtr requestedNode;
//   QModelIndex parentItem = taxonomyIndex(parentNode);
//   int newTaxRow = rowCount(parentItem);
//   beginInsertRows(parentItem, newTaxRow, newTaxRow);
//   {
//     requestedNode = m_tax->createElement(name, parentNode);
//   }
//   endInsertRows();
//
//   markAsChanged();
//
//   return requestedNode;
// }
//
//------------------------------------------------------------------------
// TaxonomyElementSPtr ModelAdapter::createTaxonomyElement(TaxonomyElementSPtr parent, const QString &name)
// {
//   return createTaxonomyElement(parent.get(), name);
// }
//
//------------------------------------------------------------------------
// void ModelAdapter::addTaxonomyElement(TaxonomyElementSPtr parent, TaxonomyElementSPtr element)
// {
//   TaxonomyElementPtr parentNode = m_tax->root().get();
//   if (parent)
//     parentNode = parent.get();
//
//   foreach(TaxonomyElementSPtr elem, parentNode->subElements())
//   //qDebug() << elem->name();
//
//   Q_ASSERT(!parentNode->subElements().contains(element));
//
//   TaxonomyElementSPtr requestedNode;
//   QModelIndex parentItem = taxonomyIndex(parentNode);
//   int newTaxRow = rowCount(parentItem);
//   beginInsertRows(parentItem, newTaxRow, newTaxRow);
//   {
//     parentNode->addElement(element);
//
//     connect(element.get(), SIGNAL(modified(ModelItemPtr)),
//             this, SLOT(itemModified(ModelItemPtr)));
//   }
//   endInsertRows();
//
//   markAsChanged();
// }
//
//------------------------------------------------------------------------
// void ModelAdapter::removeTaxonomyElement(TaxonomyElementSPtr parent, TaxonomyElementSPtr element)
// {
//   QModelIndex elementIndex = index(element);
//
//   beginRemoveRows(elementIndex.parent(), elementIndex.row(), elementIndex.row());
//   {
//     parent->deleteElement(element.get());
//   }
//   endRemoveRows();
//
//   markAsChanged();
// }
//
//------------------------------------------------------------------------
// QModelIndex ModelAdapter::sampleIndex(SamplePtr sample) const
// {
//   QModelIndex index;
//
//   int row = 0;
//   foreach(SampleSPtr ptr, m_samples)
//   {
//     if (ptr.get() == sample)
//     {
//       ModelItemPtr internalPtr = sample;
//       index = createIndex(row, 0, internalPtr);
//     }
//     row++;
//   }
//
//   return index;
// }
//
// }
//
//
//------------------------------------------------------------------------
// QModelIndex ModelAdapter::filterIndex(FilterPtr filter) const
// {
//   QModelIndex index;
//
//   int row = 0;
//   foreach(FilterSPtr ptr, m_filters)
//   {
//     if (ptr.get() == filter)
//     {
//       ModelItemPtr internalPtr = filter;
//       index = createIndex(row, 0, internalPtr);
//     }
//     row++;
//   }
//
//   return index;
// }
//
//------------------------------------------------------------------------
// QModelIndex ModelAdapter::filterIndex(FilterSPtr filter) const
// {
//   return filterIndex(filter.get());
// }
//
