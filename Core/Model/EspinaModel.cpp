/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor<jpena@cesvima.upm.es>

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

#include "EspinaModel.h"

// EspINA
#include "Core/Model/Channel.h"
#include "Core/Model/EspinaFactory.h"
#include "Core/Model/ModelItem.h"
#include "Core/Model/Sample.h"
#include "Core/Model/Segmentation.h"
#include "Core/Model/Taxonomy.h"
#include "Core/IO/EspinaIO.h"
#include "Core/IO/ErrorHandler.h"
#include <Core/Extensions/SegmentationExtension.h>
#include <Core/Extensions/ChannelExtension.h>
#include "Filters/ChannelReader.h"

// Qt
#include <QDebug>

using namespace EspINA;

//------------------------------------------------------------------------
EspinaModel::EspinaModel(EspinaFactory *factory)
: m_factory   (factory)
, m_tax       (NULL)
, m_relations (new RelationshipGraph())
, m_lastId    (0)
, m_changed   (false)
{
}

//------------------------------------------------------------------------
EspinaModel::~EspinaModel()
{
  qDebug() << "########################################################";
  qDebug() << "            Destroying EspINA Model";
  qDebug() << "########################################################";
}

//------------------------------------------------------------------------
void EspinaModel::reset()
{
  beginResetModel();
  {
    if (!m_segmentations.isEmpty())
    {
      foreach(SegmentationSPtr segmentation, m_segmentations)
      {
        foreach(Segmentation::InformationExtension extension, m_factory->segmentationExtensions())
        {
          extension->invalidate(segmentation.data());
        }
      }
      m_segmentations.clear();
    }
    if (!m_filters.isEmpty())
    {
      m_filters.clear();
    }
    if (!m_channels.isEmpty())
    {
      foreach(ChannelSPtr channel, m_channels)
      {
        foreach(Channel::ExtensionPtr extension, m_factory->channelExtensions())
        {
          extension->invalidate(channel.data());
        }
      }
      m_channels.clear();
    }
    if (!m_samples.isEmpty())
    {
      m_samples.clear();
    }

    m_tax.clear();

    m_relations->clear();
  }
  endResetModel();

  m_changed = false;
  m_lastId  = 0;
}

//-----------------------------------------------------------------------------
QVariant EspinaModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();

  if (index == taxonomyRoot())
  {
    if (role == Qt::DisplayRole)
      return tr("Taxonomies");
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

  if (index == filterRoot())
  {
    if (role == Qt::DisplayRole)
      return "Filters";
    return QVariant();
  }

  ModelItemPtr indexItem = indexPtr(index);
  return indexItem->data(role);
}

//------------------------------------------------------------------------
bool EspinaModel::setData ( const QModelIndex& index, const QVariant& value, int role )
{
  bool result = false;
  if (index.isValid() && index.parent().isValid())// Root indexes cannot be modified
  {
    // Other elements can set their own data
    ModelItemPtr indexItem = indexPtr(index);
    result = indexItem->setData(value, role);
    if (result)
    {
      emit dataChanged(index,index);
      if (EspINA::TAXONOMY == indexItem->type())
      {
        TaxonomyElementPtr taxonomy = taxonomyElementPtr(indexItem);
        foreach(SegmentationSPtr segmentation, m_segmentations)
        {
          if (segmentation->taxonomy().data() == taxonomy)
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
QMap<int, QVariant> EspinaModel::itemData(const QModelIndex &index) const
{
  QMap<int, QVariant> data = QAbstractItemModel::itemData(index);

  if (index.isValid() && index.parent().isValid())
    data[RawPointerRole] = QVariant::fromValue(reinterpret_cast<quintptr>(indexPtr(index)));
  data[TypeRole]       = index.data(TypeRole);

  return data;
}

//------------------------------------------------------------------------
Qt::ItemFlags EspinaModel::flags(const QModelIndex& index) const
{
  if (!index.isValid())
    return QAbstractItemModel::flags(index);

  if (index == taxonomyRoot() ||
      index == sampleRoot()   ||
      index == channelRoot()  ||
      index == filterRoot()   ||
      index == segmentationRoot() 
     )
    return QAbstractItemModel::flags(index);

  ModelItemPtr item = indexPtr(index);
  if (SEGMENTATION == item->type() || CHANNEL == item->type())
    return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
  else
    return QAbstractItemModel::flags(index);
}


//------------------------------------------------------------------------
int EspinaModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

//------------------------------------------------------------------------
int EspinaModel::rowCount(const QModelIndex &parent) const
{
  int count = 0;
    // There are 4 root indexes
    if ( !parent.isValid() )
    {
        count = 5;
    }
    else if ( parent == taxonomyRoot() )
    {
        count = m_tax?m_tax->elements().size() :0;
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
    else if ( parent == filterRoot() )
    {
      count = m_filters.size();
    }
    else
    {
      // Cast to base type
      ModelItemPtr parentItem = indexPtr(parent);
      if (TAXONOMY == parentItem->type())
      {
        TaxonomyElementPtr taxItem = taxonomyElementPtr(parentItem);
        count = taxItem->subElements().size();
      }
    }

    // Otherwise Samples and Segmentations have no children
    return count;
}

//------------------------------------------------------------------------
QModelIndex EspinaModel::parent(const QModelIndex& child) const
{
  if (!child.isValid())
    return QModelIndex();

  if ( child == taxonomyRoot()
    || child == sampleRoot()
    || child == channelRoot()
    || child == segmentationRoot()
    || child == filterRoot() )
    return QModelIndex();

  ModelItemPtr childItem = indexPtr(child);

  switch (childItem->type())
  {
    case TAXONOMY:
    {
      TaxonomyElementPtr taxElement = taxonomyElementPtr(childItem);
      return taxonomyIndex(taxElement->parent());
    }
    case SAMPLE:
      return sampleRoot();
    case CHANNEL:
      return channelRoot();
    case SEGMENTATION:
      return segmentationRoot();
    case FILTER:
      return filterRoot();
    default:
      Q_ASSERT(false);
      return QModelIndex();
  };
  return QModelIndex();
}

//------------------------------------------------------------------------
// Returned index is compossed by the row, column and an element.
QModelIndex EspinaModel::index (int row, int column, const QModelIndex& parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  if (!parent.isValid())
  {
    Q_ASSERT (row < 5);
    if (row == 0)
      return taxonomyRoot();
    if (row == 1)
      return sampleRoot();
    if (row == 2)
      return channelRoot();
    if (row == 3)
      return segmentationRoot();
    if (row == 4)
      return filterRoot();
  }

  ModelItemPtr internalPtr;

  if (parent == sampleRoot())
  {
    Q_ASSERT(row < m_samples.size());
    internalPtr = m_samples[row].data();
  }
  else if (parent == channelRoot())
  {
    Q_ASSERT(row < m_channels.size());
    internalPtr = m_channels[row].data();
  }
  else if (parent == segmentationRoot())
  {
    Q_ASSERT(row < m_segmentations.size());
    internalPtr = m_segmentations[row].data();
  }
  else if (parent == filterRoot())
  {
    Q_ASSERT(row < m_filters.size());
    internalPtr = m_filters[row].data();
  } else
  {
    TaxonomyElementPtr parentTax;
    if (parent == taxonomyRoot())
    {
      parentTax = m_tax->root().data();
    }
    else
    {
      // Neither Samples nor Segmentations have children
      ModelItemPtr parentItem = indexPtr(parent);
      parentTax = taxonomyElementPtr(parentItem);
    }
    //WARNING: Now m_tax can be NULL, but even in that situation,
    //         it shouldn't report any children
    Q_ASSERT(parentTax);
    Q_ASSERT(row < parentTax->subElements().size());
    internalPtr = parentTax->subElements()[row].data();
  }

  return createIndex(row, column, internalPtr);
}

//------------------------------------------------------------------------
void EspinaModel::addSample(SampleSPtr sample)
{

  int row = m_samples.size();

  beginInsertRows(sampleRoot(), row, row);
  {
    addSampleImplementation(sample);
  }
  endInsertRows();

  emit sampleAdded(sample);
  markAsChanged();
}

//------------------------------------------------------------------------
void EspinaModel::addSample(SampleSList samples)
{
  int start = m_samples.size();
  int end   = start + samples.size() - 1;

  beginInsertRows(sampleRoot(), start, end);
  {
    foreach(SampleSPtr sample, samples)
      addSampleImplementation(sample);
  }
  endInsertRows();

  foreach(SampleSPtr sample, samples)
    emit sampleAdded(sample);
  markAsChanged();
}


//------------------------------------------------------------------------
void EspinaModel::removeSample(SampleSPtr sample)
{
  Q_ASSERT(m_samples.contains(sample));

  QModelIndex index = sampleIndex(sample.data());
  beginRemoveRows(index.parent(), index.row(), index.row());
  {
    removeSampleImplementation(sample);
  }
  endRemoveRows();

  emit sampleRemoved(sample);
  markAsChanged();
  Q_ASSERT (!m_samples.contains(sample));
}

//------------------------------------------------------------------------
void EspinaModel::addChannel(ChannelSPtr channel)
{
  int row = m_channels.size();

  beginInsertRows(channelRoot(), row, row);
  {
    addChannelImplementation(channel);
  }
  endInsertRows();

  markAsChanged();
}

//------------------------------------------------------------------------
void EspinaModel::addChannel(ChannelSList channels)
{
  int start = m_channels.size();
  int end   = start + channels.size() - 1;

  beginInsertRows(channelRoot(), start, end);
  {
    foreach(ChannelSPtr channel, channels)
      addChannelImplementation(channel);
  }
  endInsertRows();

  markAsChanged();
}

//------------------------------------------------------------------------
void EspinaModel::emitChannelAdded(ChannelSList channels)
{
  foreach(ChannelSPtr channel, channels)
    emit channelAdded(channel);
}

//------------------------------------------------------------------------
void EspinaModel::removeChannel(ChannelSPtr channel)
{
  Q_ASSERT(m_channels.contains(channel));

  QModelIndex index = channelIndex(channel.data());
  beginRemoveRows(index.parent(), index.row(), index.row());
  {
    removeChannelImplementation(channel);
  }
  endRemoveRows();

  emit channelRemoved(channel);
  markAsChanged();

  Q_ASSERT (!m_channels.contains(channel));
}

//------------------------------------------------------------------------
void EspinaModel::addSegmentation(SegmentationSPtr segmentation)
{
  int row = m_segmentations.size();

  beginInsertRows(segmentationRoot(), row, row);
  {
    addSegmentationImplementation(segmentation);
  }
  endInsertRows();

  markAsChanged();
}

//------------------------------------------------------------------------
void EspinaModel::addSegmentation(SegmentationSList segmentations)
{
  int start = m_segmentations.size();
  int end   = start + segmentations.size() - 1;

  beginInsertRows(segmentationRoot(), start, end);
  {
    foreach(SegmentationSPtr seg, segmentations)
      addSegmentationImplementation(seg);
  }
  endInsertRows();

  markAsChanged();
}

//------------------------------------------------------------------------
void EspinaModel::emitSegmentationAdded(SegmentationSList segmentations)
{
  foreach(SegmentationSPtr segmentation, segmentations)
    emit segmentationAdded(segmentation);
}

//------------------------------------------------------------------------
void EspinaModel::removeSegmentation(SegmentationSPtr segmentation)
{
  Q_ASSERT(m_segmentations.contains(segmentation));

  QModelIndex index = segmentationIndex(segmentation.data());
  beginRemoveRows(index.parent(), index.row(), index.row());
  {
    removeSegmentationImplementation(segmentation);
  }
  endRemoveRows();

  emit segmentationRemoved(segmentation);
  markAsChanged();

  Q_ASSERT (!m_segmentations.contains(segmentation));
}

//------------------------------------------------------------------------
void EspinaModel::removeSegmentation(SegmentationSList segs)
{
  foreach(SegmentationSPtr seg, segs)
  {
    removeSegmentation(seg);
    emit segmentationRemoved(seg);
  }
}

//------------------------------------------------------------------------
void EspinaModel::addFilter(FilterSPtr filter)
{
  int row = m_filters.size();

  beginInsertRows(filterRoot(), row, row);
  {
    addFilterImplementation(filter);
  }
  endInsertRows();

  emit filterAdded(filter);
  markAsChanged();
}

//------------------------------------------------------------------------
void EspinaModel::addFilter(FilterSList filters)
{
  int start = m_filters.size();
  int end   = start + filters.size() - 1;

  beginInsertRows(filterRoot(), start, end);
  {
    foreach(FilterSPtr filter, filters)
      addFilterImplementation(filter);
  }
  endInsertRows();

  foreach(FilterSPtr filter, filters)
    emit filterAdded(filter);
  markAsChanged();
}

//------------------------------------------------------------------------
void EspinaModel::removeFilter(FilterSPtr filter)
{
  Q_ASSERT(m_filters.contains(filter));

  QModelIndex index = filterIndex(filter.data());
  beginRemoveRows(index.parent(), index.row(), index.row());
  {
    removeFilterImplementation(filter);
  }
  endRemoveRows();

  emit filterRemoved(filter);
  markAsChanged();
}

//------------------------------------------------------------------------
void EspinaModel::changeTaxonomy(SegmentationSPtr    segmentation,
                                 TaxonomyElementSPtr taxonomy)
{
  segmentation->setTaxonomy(taxonomy);

  QModelIndex segIndex = segmentationIndex(segmentation.data());
  emit dataChanged(segIndex, segIndex);

  markAsChanged();
}


//------------------------------------------------------------------------
void EspinaModel::changeTaxonomyParent(TaxonomyElementSPtr subTaxonomy,
                                       TaxonomyElementSPtr parent)
{
  TaxonomyElementPtr oldParent = subTaxonomy->parent();

  if (oldParent == parent)
    return;

  QModelIndex oldIndex = index(subTaxonomy).parent();
  QModelIndex newIndex = index(parent);

  int oldRow = oldParent->subElements().indexOf(subTaxonomy);
  int newRow = parent->subElements().size();

  beginMoveRows(oldIndex, oldRow, oldRow, newIndex, newRow);
  {
    oldParent->deleteElement(subTaxonomy.data());
    parent->addElement(subTaxonomy);
  }
  endMoveRows();

  foreach(SegmentationSPtr segmentation, m_segmentations)
  {
    if (segmentation->taxonomy() == subTaxonomy)
    {
      QModelIndex segIndex = segmentationIndex(segmentation);
      emit dataChanged(segIndex, segIndex);
    }
  }
  
}


//------------------------------------------------------------------------
void EspinaModel::addRelation(ModelItemSPtr  ancestor,
                              ModelItemSPtr  successor,
                              const QString &relation)
{
  m_relations->addRelation(ancestor.data(), successor.data(), relation);

  QModelIndex ancestorIndex  = index(ancestor);
  QModelIndex successorIndex = index(successor);

  emit dataChanged(ancestorIndex,  ancestorIndex);
  emit dataChanged(successorIndex, successorIndex);
  markAsChanged();
}

//------------------------------------------------------------------------
void EspinaModel::removeRelation(ModelItemSPtr  ancestor,
                                 ModelItemSPtr  successor,
                                 const QString &relation)
{
  m_relations->removeRelation(ancestor.data(), successor.data(), relation);

  QModelIndex ancestorIndex = index(ancestor);
  QModelIndex succesorIndex = index(successor);

  emit dataChanged(ancestorIndex, ancestorIndex);
  emit dataChanged(succesorIndex, succesorIndex);
  markAsChanged();
}

//------------------------------------------------------------------------
ModelItemSList EspinaModel::relatedItems(ModelItemPtr   item,
                                         RelationType   relType,
                                         const QString &relName)
{
  ModelItemSList res;

  RelationshipGraph::VertexId vertex = m_relations->vertex(item);

  if (relType == IN || relType == INOUT)
    foreach(VertexProperty v, m_relations->ancestors(vertex, relName))
      res << find(v.item);

  if (relType == OUT || relType == INOUT)
    foreach(VertexProperty v, m_relations->succesors(vertex, relName))
      res << find(v.item);

  return res;
}

//------------------------------------------------------------------------
RelationList EspinaModel::relations(ModelItemPtr   item,
                                    const QString &relName)
{
   RelationList res;

   RelationshipGraph::VertexId vertex = m_relations->vertex(item);
  foreach(Edge edge, m_relations->edges(vertex, relName))
  {
    Relation rel;
    rel.ancestor = find(edge.source.item);
    rel.succesor = find(edge.target.item);
    rel.relation = edge.relationship.c_str();
    res << rel;
  }

  return res; 
}



//------------------------------------------------------------------------
QModelIndex EspinaModel::index(ModelItemPtr item) const
{
  QModelIndex res;
  switch (item->type())
  {
    case TAXONOMY:
      res = taxonomyIndex(taxonomyElementPtr(item));
      break;
    case SAMPLE:
      res = sampleIndex(samplePtr(item));
      break;
    case CHANNEL:
      res = channelIndex(channelPtr(item));
      break;
    case FILTER:
      res = filterIndex(filterPtr(item));
      break;
    case SEGMENTATION:
      res = segmentationIndex(segmentationPtr(item));
      break;
    default:
      Q_ASSERT(false);
      break;
  }
  return res;
}

//------------------------------------------------------------------------
QModelIndex EspinaModel::index(ModelItemSPtr item) const
{
  return index(item.data());
}


//------------------------------------------------------------------------
QModelIndex EspinaModel::taxonomyRoot() const
{
    return createIndex ( 0, 0, 0 );
}

//------------------------------------------------------------------------
QModelIndex EspinaModel::taxonomyIndex(TaxonomyElementPtr node) const
{
  // We avoid setting the Taxonomy descriptor as parent of an index
  if ( !m_tax || m_tax->root() == node)
    return taxonomyRoot();

  TaxonomyElementPtr parentNode = node->parent();
  Q_ASSERT(parentNode);

  TaxonomyElementSPtr subNode = parentNode->element(node->name());
  int row = parentNode->subElements().indexOf(subNode);
  ModelItemPtr internalPtr = node;
  return createIndex(row, 0, internalPtr);
}

//------------------------------------------------------------------------
QModelIndex EspinaModel::taxonomyIndex(TaxonomyElementSPtr node) const
{
  return taxonomyIndex(node.data());
}


//------------------------------------------------------------------------
void EspinaModel::setTaxonomy(TaxonomySPtr tax)
{
  if (m_tax)
  {
    TaxonomySPtr oldTax = tax;
    beginRemoveRows(taxonomyRoot(), 0, rowCount(taxonomyRoot()) - 1);
    m_tax.clear();
    endRemoveRows();

    emit taxonomyRemoved(oldTax);
  }

  if (tax)
  {
    beginInsertRows(taxonomyRoot(), 0, tax->elements().size() - 1);
    m_tax = tax;
    endInsertRows();

    emit taxonomyAdded(m_tax);
  }
  markAsChanged();
}

//------------------------------------------------------------------------
void EspinaModel::addTaxonomy(TaxonomySPtr tax)
{
  if (m_tax)
    addTaxonomy(tax->root());
  else
    setTaxonomy(tax);

  markAsChanged();
}

//------------------------------------------------------------------------
void EspinaModel::itemModified(ModelItemPtr item)
{
  QModelIndex itemIndex = index(item);
  emit dataChanged(itemIndex, itemIndex);
}

//------------------------------------------------------------------------
void EspinaModel::addTaxonomy(TaxonomyElementSPtr root)
{
  Q_ASSERT(false);//DEPRECATED?
//   foreach (TaxonomyElementSPtr node, root->subElements())
//   {
//     addTaxonomyElement(node->qualifiedName(), m_tax->root());
//     addTaxonomy(node);
//   }
// 
//   markAsChanged();
}

//------------------------------------------------------------------------
void EspinaModel::addSampleImplementation(SampleSPtr sample)
{
  Q_ASSERT(!sample.isNull());
  Q_ASSERT(!m_samples.contains(sample));

  sample->m_model = this;
  m_samples << sample;
  m_relations->addItem(sample.data());

  connect(sample.data(), SIGNAL(modified(ModelItemPtr)),
          this, SLOT(itemModified(ModelItemPtr)));
}

//------------------------------------------------------------------------
void EspinaModel::removeSampleImplementation(SampleSPtr sample)
{
  Q_ASSERT(!sample.isNull());
  Q_ASSERT(relations(sample.data()).isEmpty());

  m_samples.removeOne(sample);
  m_relations->removeItem(sample.data());

  sample->m_model = NULL;
}

//------------------------------------------------------------------------
void EspinaModel::addChannelImplementation(ChannelSPtr channel)
{
  Q_ASSERT(!channel.isNull());
  Q_ASSERT(!m_channels.contains(channel));

  channel->m_model = this;
  m_channels << channel;
  m_relations->addItem(channel.data());

  connect(channel.data(), SIGNAL(modified(ModelItemPtr)),
          this, SLOT(itemModified(ModelItemPtr)));
}

//------------------------------------------------------------------------
void EspinaModel::removeChannelImplementation(ChannelSPtr channel)
{
  Q_ASSERT(!channel.isNull());

  channel->invalidateExtensions();

  m_channels.removeOne(channel);
  m_relations->removeItem(channel.data());

  channel->m_model = NULL;
}

//------------------------------------------------------------------------
void EspinaModel::addSegmentationImplementation(SegmentationSPtr segmentation)
{
  Q_ASSERT(!segmentation.isNull());
  Q_ASSERT(m_segmentations.contains(segmentation) == false);

  if (segmentation->number() == 0)
    segmentation->setNumber(++m_lastId);
  else
    m_lastId = qMax(m_lastId, segmentation->number());

  segmentation->m_model = this;
  segmentation->initializeExtensions();

  m_segmentations << segmentation;
  m_relations->addItem(segmentation.data());

  connect(segmentation.data(), SIGNAL(modified(ModelItemPtr)),
          this, SLOT(itemModified(ModelItemPtr)));
}

//------------------------------------------------------------------------
void EspinaModel::removeSegmentationImplementation(SegmentationSPtr segmentation)
{
  Q_ASSERT(!segmentation.isNull());

  segmentation->invalidateExtensions();

  m_segmentations.removeOne(segmentation);
  m_relations->removeItem(segmentation.data());

  segmentation->m_model = NULL;
}

//------------------------------------------------------------------------
void EspinaModel::addFilterImplementation(FilterSPtr filter)
{
  Q_ASSERT(!m_filters.contains(filter));

  filter->m_model = this;
  m_filters << filter;
  m_relations->addItem(filter.data());
}

//------------------------------------------------------------------------
void EspinaModel::removeFilterImplementation(FilterSPtr filter)
{
  m_filters.removeOne(filter);
  m_relations->removeItem(filter.data());

  filter->m_model = NULL;
}


//------------------------------------------------------------------------
ModelItemSPtr EspinaModel::find(ModelItemPtr item)
{
  ModelItemSPtr res;
  switch (item->type())
  {
    case EspINA::TAXONOMY:
      res = findTaxonomyElement(item);
      break;
    case EspINA::SAMPLE:
      res = findSample(item);
      break;
    case EspINA::CHANNEL:
      res = findChannel(item);
      break;
    case EspINA::FILTER:
      res = findFilter(item);
      break;
    case EspINA::SEGMENTATION:
      res = findSegmentation(item);
      break;
  };

  return res;
}

//------------------------------------------------------------------------
TaxonomyElementSPtr EspinaModel::findTaxonomyElement(ModelItemPtr item)
{
  return findTaxonomyElement(taxonomyElementPtr(item));
}

//------------------------------------------------------------------------
TaxonomyElementSPtr EspinaModel::findTaxonomyElement(TaxonomyElementPtr taxonomyElement)
{
  if (taxonomyElement == m_tax->root())
    return m_tax->root();

  TaxonomyElementPtr parent = taxonomyElement->parent();
  return parent->element(taxonomyElement->name());
}

//------------------------------------------------------------------------
SampleSPtr EspinaModel::findSample(ModelItemPtr item)
{
  return findSample(samplePtr(item));
}

//------------------------------------------------------------------------
SampleSPtr EspinaModel::findSample(SamplePtr sample)
{
  SampleSPtr res;

  int i=0;
  while (res.isNull() && i < m_samples.size())
  {
    if (m_samples[i].data() == sample)
      res = m_samples[i];
    i++;
  }

  return res;
}

//------------------------------------------------------------------------
ChannelSPtr EspinaModel::findChannel(ModelItemPtr item)
{
  return findChannel(channelPtr(item));
}

//------------------------------------------------------------------------
ChannelSPtr EspinaModel::findChannel(ChannelPtr channel)
{
  ChannelSPtr res;

  int i=0;
  while (res.isNull() && i < m_channels.size())
  {
    if (m_channels[i].data() == channel)
      res = m_channels[i];
    i++;
  }

  return res;
}

//------------------------------------------------------------------------
FilterSPtr EspinaModel::findFilter(ModelItemPtr item)
{
  return findFilter(filterPtr(item));
}

//------------------------------------------------------------------------
FilterSPtr EspinaModel::findFilter(FilterPtr filter)
{
  FilterSPtr res;

  int i=0;
  while (res.isNull() && i < m_filters.size())
  {
    if (m_filters[i].data() == filter)
      res = m_filters[i];
    i++;
  }

  return res;
}

//------------------------------------------------------------------------
SegmentationSPtr EspinaModel::findSegmentation(ModelItemPtr item)
{
  return findSegmentation(segmentationPtr(item));
}

//------------------------------------------------------------------------
SegmentationSPtr EspinaModel::findSegmentation(SegmentationPtr segmentation)
{
  SegmentationSPtr res;

  int i=0;
  while (res.isNull() && i < m_segmentations.size())
  {
    if (m_segmentations[i].data() == segmentation)
      res = m_segmentations[i];
    i++;
  }

  return res;
}

//------------------------------------------------------------------------
TaxonomyElementSPtr EspinaModel::createTaxonomyElement(TaxonomyElementPtr parent, const QString &name)
{
  TaxonomyElementPtr parentNode = m_tax->root().data();
  if (parent)
    parentNode = parent;

  Q_ASSERT(parentNode->element(name).isNull());

  TaxonomyElementSPtr requestedNode;
  QModelIndex parentItem = taxonomyIndex(parentNode);
  int newTaxRow = rowCount(parentItem);
  beginInsertRows(parentItem, newTaxRow, newTaxRow);
  {
    requestedNode = m_tax->createElement(name, parentNode);
  }
  endInsertRows();

  markAsChanged();

  return requestedNode;
}

//------------------------------------------------------------------------
TaxonomyElementSPtr EspinaModel::createTaxonomyElement(TaxonomyElementSPtr parent, const QString &name)
{
  return createTaxonomyElement(parent.data(), name);
}

//------------------------------------------------------------------------
void EspinaModel::addTaxonomyElement(TaxonomyElementSPtr parent, TaxonomyElementSPtr element)
{
  TaxonomyElementPtr parentNode = m_tax->root().data();
  if (parent)
    parentNode = parent.data();

  foreach(TaxonomyElementSPtr elem, parentNode->subElements())
    qDebug() << elem->name();

  Q_ASSERT(!parentNode->subElements().contains(element));

  TaxonomyElementSPtr requestedNode;
  QModelIndex parentItem = taxonomyIndex(parentNode);
  int newTaxRow = rowCount(parentItem);
  beginInsertRows(parentItem, newTaxRow, newTaxRow);
  {
    parentNode->addElement(element);

    connect(element.data(), SIGNAL(modified(ModelItemPtr)),
            this, SLOT(itemModified(ModelItemPtr)));
  }
  endInsertRows();

  markAsChanged();
}

//------------------------------------------------------------------------
void EspinaModel::removeTaxonomyElement(TaxonomyElementSPtr parent, TaxonomyElementSPtr element)
{
  QModelIndex elementIndex = index(element);

  beginRemoveRows(elementIndex.parent(), elementIndex.row(), elementIndex.row());
  {
    parent->deleteElement(element.data());
  }
  endRemoveRows();

  markAsChanged();
}

//------------------------------------------------------------------------
QModelIndex EspinaModel::sampleRoot() const
{
  return createIndex (1, 0, 1);
}

//------------------------------------------------------------------------
QModelIndex EspinaModel::sampleIndex(SamplePtr sample) const
{
  QModelIndex index;

  int row = 0;
  foreach(SampleSPtr ptr, m_samples)
  {
    if (ptr.data() == sample)
    {
      ModelItemPtr internalPtr = sample;
      index = createIndex(row, 0, internalPtr);
    }
    row++;
  }

  return index;
}

//------------------------------------------------------------------------
QModelIndex EspinaModel::sampleIndex(SampleSPtr sample) const
{
  return sampleIndex(sample.data());
}


//------------------------------------------------------------------------
QModelIndex EspinaModel::channelRoot() const
{
  return createIndex (2, 0, 2);
}

//------------------------------------------------------------------------
QModelIndex EspinaModel::channelIndex(ChannelPtr channel) const
{
  QModelIndex index;

  int row = 0;
  foreach(ChannelSPtr ptr, m_channels)
  {
    if (ptr.data() == channel)
    {
      ModelItemPtr internalPtr = channel;
      index = createIndex(row, 0, internalPtr);
    }
    row++;
  }

  return index;
}

//------------------------------------------------------------------------
QModelIndex EspinaModel::channelIndex(ChannelSPtr channel) const
{
  return channelIndex(channel.data());
}

//------------------------------------------------------------------------
QModelIndex EspinaModel::segmentationRoot() const
{
    return createIndex (3, 0, 3);
}

//------------------------------------------------------------------------
QModelIndex EspinaModel::segmentationIndex(SegmentationPtr seg) const
{
  QModelIndex index;

  int row = 0;
  foreach(SegmentationSPtr ptr, m_segmentations)
  {
    if (ptr.data() == seg)
    {
      ModelItemPtr internalPtr = seg;
      index = createIndex(row, 0, internalPtr);
    }
    row++;
  }

  return index;
}

//------------------------------------------------------------------------
QModelIndex EspinaModel::segmentationIndex(SegmentationSPtr seg) const
{
  return segmentationIndex(seg.data());
}

//------------------------------------------------------------------------
QModelIndex EspinaModel::filterRoot() const
{
  return createIndex(4,0,4);
}

//------------------------------------------------------------------------
QModelIndex EspinaModel::filterIndex(FilterPtr filter) const
{
  QModelIndex index;

  int row = 0;
  foreach(FilterSPtr ptr, m_filters)
  {
    if (ptr.data() == filter)
    {
      ModelItemPtr internalPtr = filter;
      index = createIndex(row, 0, internalPtr);
    }
    row++;
  }

  return index;
}

//------------------------------------------------------------------------
QModelIndex EspinaModel::filterIndex(FilterSPtr filter) const
{
  return filterIndex(filter.data());
}
