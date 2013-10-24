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

#include "Analysis.h"

#include "Core/Analysis/Sample.h"
#include "Core/Analysis/Filter.h"
#include "Core/Analysis/Channel.h"
#include "Core/Analysis/Segmentation.h"

using namespace EspINA;

//------------------------------------------------------------------------
Analysis::Analysis()
: m_classification{nullptr}
, m_relations{new DirectedGraph()}
, m_content{new DirectedGraph()}
{

}

//------------------------------------------------------------------------
void Analysis::reset()
{

}

//------------------------------------------------------------------------
void Analysis::setClassification(ClassificationSPtr classification)
{

}

//------------------------------------------------------------------------
void Analysis::add(SampleSPtr sample)
{
  if (m_samples.contains(sample)) throw (Existing_Item_Exception());

  m_samples << sample;
  m_content->addItem(sample);
}

//------------------------------------------------------------------------
void Analysis::add(SampleSList samples)
{
  foreach(SampleSPtr sample, samples)
  {
    add(sample);
  }
}

//------------------------------------------------------------------------
void Analysis::add(ChannelSPtr channel)
{
  if (m_channels.contains(channel)) throw (Existing_Item_Exception());

  m_channels << channel;

  FilterSPtr filter = channel->filter(); // TODO: What happens when a channel change its output?!

  addIfNotExists(filter);

  m_content->addItem(channel);

  RelationName relation = QString("%1").arg(channel->output()->id());

  m_content->addRelation(filter, channel, relation);
}

//------------------------------------------------------------------------
void Analysis::add(ChannelSList channels)
{
  foreach(ChannelSPtr channel, channels)
  {
    add(channel);
  }
}

//------------------------------------------------------------------------
void Analysis::add(SegmentationSPtr segmentation)
{
  if (m_segmentations.contains(segmentation)) throw (Existing_Item_Exception());

  m_segmentations << segmentation;

  FilterSPtr filter = segmentation->filter(); // TODO: What happens when a segmentation change its output?!

  addIfNotExists(filter);

  m_content->addItem(segmentation);

  RelationName relation = QString("%1").arg(segmentation->output()->id());

  m_content->addRelation(filter, segmentation, relation);
}

//------------------------------------------------------------------------
void Analysis::add(SegmentationSList segmentations)
{
  foreach(SegmentationSPtr segmentation, segmentations)
  {
    add(segmentation);
  }
}


//------------------------------------------------------------------------
void Analysis::remove(SampleSPtr sample)
{
  if (!m_samples.contains(sample)) throw (Item_Not_Found_Exception());

  m_samples.removeOne(sample);

  if (m_content->contains(sample))
  {
    m_content->removeItem(sample);
  }

  if (m_relations->contains(sample))
  {
    m_relations->removeItem(sample);
  }
}

//------------------------------------------------------------------------
void Analysis::remove(SampleSList samples)
{
  foreach(SampleSPtr sample, samples)
  {
    remove(sample);
  }
}


//------------------------------------------------------------------------
void Analysis::remove(ChannelSPtr channel)
{
  if (!m_channels.contains(channel)) throw (Item_Not_Found_Exception());

  m_channels.removeOne(channel);

  if (m_content->contains(channel))
  {
    m_content->removeItem(channel);
  }

  if (m_relations->contains(channel))
  {
    m_relations->removeItem(channel);
  }

  removeIfIsolated(channel->filter());
}

//------------------------------------------------------------------------
void Analysis::remove(ChannelSList channels)
{
  foreach(ChannelSPtr channel, channels)
  {
    remove(channel);
  }
}

//------------------------------------------------------------------------
void Analysis::remove(SegmentationSPtr segmentation)
{
  if (!m_segmentations.contains(segmentation)) throw (Item_Not_Found_Exception());

  m_segmentations.removeOne(segmentation);

  if (m_content->contains(segmentation))
  {
    m_content->removeItem(segmentation);
  }

  if (m_relations->contains(segmentation))
  {
    m_relations->removeItem(segmentation);
  }

  removeIfIsolated(segmentation->filter());
}

//------------------------------------------------------------------------
void Analysis::remove(SegmentationSList segmentations)
{
  foreach(SegmentationSPtr segmentation, segmentations)
  {
    remove(segmentation);
  }
}

//------------------------------------------------------------------------
void Analysis::addRelation(PersistentSPtr    ancestor,
                           PersistentSPtr    succesor,
                           const RelationName& relation)
{
  if (!m_relations->contains(ancestor)) m_relations->addItem(ancestor);

  if (!m_relations->contains(succesor)) m_relations->addItem(succesor);

  if (findRelation(ancestor, succesor, relation))  throw (Existing_Relation_Exception());

  m_relations->addRelation(ancestor, succesor, relation);
}

//------------------------------------------------------------------------
void Analysis::deleteRelation(PersistentSPtr    ancestor,
                              PersistentSPtr    succesor,
                              const RelationName& relation)
{
  if (!findRelation(ancestor, succesor, relation)) throw (Relation_Not_Found_Exception());

  m_relations->removeRelation(ancestor, succesor, relation);

  removeIfIsolated(m_relations, ancestor);
  removeIfIsolated(m_relations, succesor);
}

//------------------------------------------------------------------------
bool Analysis::removeIfIsolated(DirectedGraphSPtr graph, PersistentSPtr item)
{
  bool removed = false;

  DirectedGraph::Vertex v = graph->vertex(item);

  if (graph->contains(item) && graph->edges(v).isEmpty())
  {
    graph->removeItem(item);
    removed = true;
  }

  return removed;
}

//------------------------------------------------------------------------
void Analysis::addIfNotExists(FilterSPtr filter)
{
  // NOTE: We could use m_filters instead to check if there is a copy in the content
  if (!m_content->contains(filter)) 
  {
    m_filters << filter;
    m_content->addItem(filter);
  }
}


//------------------------------------------------------------------------
void Analysis::removeIfIsolated(FilterSPtr filter)
{
  if (removeIfIsolated(m_content, filter))
  {
    m_filters.removeOne(filter);
  }
}

//------------------------------------------------------------------------
bool Analysis::findRelation(PersistentSPtr    ancestor,
                            PersistentSPtr    succesor,
                            const RelationName& relation)
{
  DirectedGraph::Vertex v = m_relations->vertex(ancestor);
  foreach(DirectedGraph::Edge edge, m_relations->outEdges(v, relation))
  {
   if (edge.relationship == relation.toStdString() && edge.target.item == succesor) return true;
  }

  return false;
}


// //------------------------------------------------------------------------
// void EspinaModel::reset()
// {
//   beginResetModel();
//   {
//     if (!m_segmentations.isEmpty())
//     {
//       foreach(SegmentationSPtr segmentation, m_segmentations)
//       {
//         foreach(Segmentation::InformationExtension extension, m_factory->segmentationExtensions())
//         {
//           extension->invalidate(segmentation.get());
//         }
//       }
//       m_segmentations.clear();
//     }
//     if (!m_filters.isEmpty())
//     {
//       m_filters.clear();
//     }
//     if (!m_channels.isEmpty())
//     {
//       foreach(ChannelSPtr channel, m_channels)
//       {
//         foreach(Channel::ExtensionPtr extension, m_factory->channelExtensions())
//         {
//           extension->invalidate(channel.get());
//         }
//       }
//       m_channels.clear();
//     }
//     if (!m_samples.isEmpty())
//     {
//       m_samples.clear();
//     }
//
//     m_tax.reset();
//
//     m_relations->clear();
//   }
//   endResetModel();
//
//   m_changed = false;
//   m_lastId  = 0;
//   m_isTraceable = true;
// }
//
// //-----------------------------------------------------------------------------
// QVariant EspinaModel::data(const QModelIndex& index, int role) const
// {
//   if (!index.isValid())
//     return QVariant();
//
//   if (index == taxonomyRoot())
//   {
//     if (role == Qt::DisplayRole)
//       return tr("Categories");
//     return QVariant();
//   }
//
//   if (index == sampleRoot())
//   {
//     if (role == Qt::DisplayRole)
//       return tr("Samples");
//     return QVariant();
//   }
//
//   if (index == channelRoot())
//   {
//     if (role == Qt::DisplayRole)
//       return tr("Channels");
//     return QVariant();
//   }
//
//   if (index == segmentationRoot())
//   {
//     if (role == Qt::DisplayRole)
//       return "Segmentations";
//     return QVariant();
//   }
//
//   if (index == filterRoot())
//   {
//     if (role == Qt::DisplayRole)
//       return "Filters";
//     return QVariant();
//   }
//
//   ModelItemPtr indexItem = indexPtr(index);
//   return indexItem->data(role);
// }
//
// //------------------------------------------------------------------------
// bool EspinaModel::setData ( const QModelIndex& index, const QVariant& value, int role )
// {
//   bool result = false;
//   if (index.isValid() && index.parent().isValid())// Root indexes cannot be modified
//   {
//     // Other elements can set their own data
//     ModelItemPtr indexItem = indexPtr(index);
//     result = indexItem->setData(value, role);
//     if (result)
//     {
//       emit dataChanged(index,index);
//       if (EspINA::TAXONOMY == indexItem->type())
//       {
//         TaxonomyElementPtr taxonomy = taxonomyElementPtr(indexItem);
//         foreach(SegmentationSPtr segmentation, m_segmentations)
//         {
//           if (segmentation->taxonomy().get() == taxonomy)
//           {
//             QModelIndex index = segmentationIndex(segmentation);
//             emit dataChanged(index, index);
//           }
//         }
//       }
//     }
//   }
//   return result;
// }
//
// //------------------------------------------------------------------------
// QMap<int, QVariant> EspinaModel::itemData(const QModelIndex &index) const
// {
//   QMap<int, QVariant> data = QAbstractItemModel::itemData(index);
//
//   if (index.isValid() && index.parent().isValid())
//     data[RawPointerRole] = QVariant::fromValue(reinterpret_cast<quintptr>(indexPtr(index)));
//   data[TypeRole]       = index.data(TypeRole);
//
//   return data;
// }
//
// //------------------------------------------------------------------------
// Qt::ItemFlags EspinaModel::flags(const QModelIndex& index) const
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
// //------------------------------------------------------------------------
// int EspinaModel::columnCount(const QModelIndex &parent) const
// {
//     return 1;
// }
//
// //------------------------------------------------------------------------
// int EspinaModel::rowCount(const QModelIndex &parent) const
// {
//   int count = 0;
//     // There are 4 root indexes
//     if ( !parent.isValid() )
//     {
//         count = 5;
//     }
//     else if ( parent == taxonomyRoot() )
//     {
//         count = m_tax?m_tax->elements().size() :0;
//     }
//     else if ( parent == sampleRoot() )
//     {
//       count = m_samples.size();
//     }
//     else if ( parent == channelRoot() )
//     {
//       count = m_channels.size();
//     }
//     else if ( parent == segmentationRoot() )
//     {
//       count = m_segmentations.size();
//     }
//     else if ( parent == filterRoot() )
//     {
//       count = m_filters.size();
//     }
//     else
//     {
//       // Cast to base type
//       ModelItemPtr parentItem = indexPtr(parent);
//       if (TAXONOMY == parentItem->type())
//       {
//         TaxonomyElementPtr taxItem = taxonomyElementPtr(parentItem);
//         count = taxItem->subElements().size();
//       }
//     }
//
//     // Otherwise Samples and Segmentations have no children
//     return count;
// }
//
// //------------------------------------------------------------------------
// QModelIndex EspinaModel::parent(const QModelIndex& child) const
// {
//   if (!child.isValid())
//     return QModelIndex();
//
//   if ( child == taxonomyRoot()
//     || child == sampleRoot()
//     || child == channelRoot()
//     || child == segmentationRoot()
//     || child == filterRoot() )
//     return QModelIndex();
//
//   ModelItemPtr childItem = indexPtr(child);
//
//   switch (childItem->type())
//   {
//     case TAXONOMY:
//     {
//       TaxonomyElementPtr taxElement = taxonomyElementPtr(childItem);
//       return taxonomyIndex(taxElement->parent());
//     }
//     case SAMPLE:
//       return sampleRoot();
//     case CHANNEL:
//       return channelRoot();
//     case SEGMENTATION:
//       return segmentationRoot();
//     case FILTER:
//       return filterRoot();
//     default:
//       Q_ASSERT(false);
//       return QModelIndex();
//   };
//   return QModelIndex();
// }
//
// //------------------------------------------------------------------------
// // Returned index is compossed by the row, column and an element.
// QModelIndex EspinaModel::index (int row, int column, const QModelIndex& parent) const
// {
//   if (!hasIndex(row, column, parent))
//     return QModelIndex();
//
//   if (!parent.isValid())
//   {
//     Q_ASSERT (row < 5);
//     if (row == 0)
//       return taxonomyRoot();
//     if (row == 1)
//       return sampleRoot();
//     if (row == 2)
//       return channelRoot();
//     if (row == 3)
//       return segmentationRoot();
//     if (row == 4)
//       return filterRoot();
//   }
//
//   ModelItemPtr internalPtr;
//
//   if (parent == sampleRoot())
//   {
//     Q_ASSERT(row < m_samples.size());
//     internalPtr = m_samples[row].get();
//   }
//   else if (parent == channelRoot())
//   {
//     Q_ASSERT(row < m_channels.size());
//     internalPtr = m_channels[row].get();
//   }
//   else if (parent == segmentationRoot())
//   {
//     Q_ASSERT(row < m_segmentations.size());
//     internalPtr = m_segmentations[row].get();
//   }
//   else if (parent == filterRoot())
//   {
//     Q_ASSERT(row < m_filters.size());
//     internalPtr = m_filters[row].get();
//   } else
//   {
//     TaxonomyElementPtr parentTax;
//     if (parent == taxonomyRoot())
//     {
//       parentTax = m_tax->root().get();
//     }
//     else
//     {
//       // Neither Samples nor Segmentations have children
//       ModelItemPtr parentItem = indexPtr(parent);
//       parentTax = taxonomyElementPtr(parentItem);
//     }
//     //WARNING: Now m_tax can be NULL, but even in that situation,
//     //         it shouldn't report any children
//     Q_ASSERT(parentTax);
//     Q_ASSERT(row < parentTax->subElements().size());
//     internalPtr = parentTax->subElements()[row].get();
//   }
//
//   return createIndex(row, column, internalPtr);
// }
//
// //------------------------------------------------------------------------
// void EspinaModel::addSample(SampleSPtr sample)
// {
//
//   int row = m_samples.size();
//
//   beginInsertRows(sampleRoot(), row, row);
//   {
//     addSampleImplementation(sample);
//   }
//   endInsertRows();
//
//   emit sampleAdded(sample);
//   markAsChanged();
// }
//
// //------------------------------------------------------------------------
// void EspinaModel::addSample(SampleSList samples)
// {
//   int start = m_samples.size();
//   int end   = start + samples.size() - 1;
//
//   beginInsertRows(sampleRoot(), start, end);
//   {
//     foreach(SampleSPtr sample, samples)
//       addSampleImplementation(sample);
//   }
//   endInsertRows();
//
//   foreach(SampleSPtr sample, samples)
//     emit sampleAdded(sample);
//   markAsChanged();
// }
//
//
// //------------------------------------------------------------------------
// void EspinaModel::removeSample(SampleSPtr sample)
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
// //------------------------------------------------------------------------
// void EspinaModel::addChannel(ChannelSPtr channel)
// {
//   int row = m_channels.size();
//
//   beginInsertRows(channelRoot(), row, row);
//   {
//     addChannelImplementation(channel);
//   }
//   endInsertRows();
//
//   markAsChanged();
// }
//
// //------------------------------------------------------------------------
// void EspinaModel::addChannel(ChannelSList channels)
// {
//   int start = m_channels.size();
//   int end   = start + channels.size() - 1;
//
//   beginInsertRows(channelRoot(), start, end);
//   {
//     foreach(ChannelSPtr channel, channels)
//       addChannelImplementation(channel);
//   }
//   endInsertRows();
//
//   markAsChanged();
// }
//
// //------------------------------------------------------------------------
// void EspinaModel::emitChannelAdded(ChannelSList channels)
// {
//   foreach(ChannelSPtr channel, channels)
//     emit channelAdded(channel);
// }
//
// //------------------------------------------------------------------------
// void EspinaModel::removeChannel(ChannelSPtr channel)
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
// //------------------------------------------------------------------------
// void EspinaModel::addSegmentation(SegmentationSPtr segmentation)
// {
//   int row = m_segmentations.size();
//
//   beginInsertRows(segmentationRoot(), row, row);
//   {
//     addSegmentationImplementation(segmentation);
//   }
//   endInsertRows();
//
//   markAsChanged();
// }
//
// //------------------------------------------------------------------------
// void EspinaModel::addSegmentation(SegmentationSList segmentations)
// {
//   int start = m_segmentations.size();
//   int end   = start + segmentations.size() - 1;
//
//   beginInsertRows(segmentationRoot(), start, end);
//   {
//     foreach(SegmentationSPtr seg, segmentations)
//       addSegmentationImplementation(seg);
//   }
//   endInsertRows();
//
//   markAsChanged();
// }
//
// //------------------------------------------------------------------------
// void EspinaModel::emitSegmentationAdded(SegmentationSList segmentations)
// {
//   foreach(SegmentationSPtr segmentation, segmentations)
//     emit segmentationAdded(segmentation);
// }
//
// //------------------------------------------------------------------------
// void EspinaModel::removeSegmentation(SegmentationSPtr segmentation)
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
// //------------------------------------------------------------------------
// void EspinaModel::removeSegmentation(SegmentationSList segs)
// {
//   foreach(SegmentationSPtr seg, segs)
//   {
//     removeSegmentation(seg);
//     emit segmentationRemoved(seg);
//   }
// }
//
// //------------------------------------------------------------------------
// void EspinaModel::addFilter(FilterSPtr filter)
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
// //------------------------------------------------------------------------
// void EspinaModel::addFilter(FilterSList filters)
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
// //------------------------------------------------------------------------
// void EspinaModel::removeFilter(FilterSPtr filter)
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
// //------------------------------------------------------------------------
// void EspinaModel::changeTaxonomy(SegmentationSPtr    segmentation,
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
// //------------------------------------------------------------------------
// void EspinaModel::changeTaxonomyParent(TaxonomyElementSPtr subTaxonomy,
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
//
// //------------------------------------------------------------------------
// void EspinaModel::removeRelation(ModelItemSPtr  ancestor,
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
// //------------------------------------------------------------------------
// ModelItemSList EspinaModel::relatedItems(ModelItemPtr   item,
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
// //------------------------------------------------------------------------
// RelationList EspinaModel::relations(ModelItemPtr   item,
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
// //------------------------------------------------------------------------
// QModelIndex EspinaModel::index(ModelItemPtr item) const
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
// //------------------------------------------------------------------------
// QModelIndex EspinaModel::index(ModelItemSPtr item) const
// {
//   return index(item.get());
// }
//
//
// //------------------------------------------------------------------------
// QModelIndex EspinaModel::taxonomyRoot() const
// {
//     return createIndex ( 0, 0, 0 );
// }
//
// //------------------------------------------------------------------------
// QModelIndex EspinaModel::taxonomyIndex(TaxonomyElementPtr node) const
// {
//   // We avoid setting the Taxonomy descriptor as parent of an index
//   if ( !m_tax || m_tax->root().get() == node)
//     return taxonomyRoot();
//
//   TaxonomyElementPtr parentNode = node->parent();
//   Q_ASSERT(parentNode);
//
//   TaxonomyElementSPtr subNode = parentNode->element(node->name());
//   int row = parentNode->subElements().indexOf(subNode);
//   ModelItemPtr internalPtr = node;
//   return createIndex(row, 0, internalPtr);
// }
//
// //------------------------------------------------------------------------
// QModelIndex EspinaModel::taxonomyIndex(TaxonomyElementSPtr node) const
// {
//   return taxonomyIndex(node.get());
// }
//
//
// //------------------------------------------------------------------------
// void EspinaModel::setTaxonomy(TaxonomySPtr taxonomy)
// {
//   if (m_tax)
//   {
//     TaxonomySPtr oldTax = taxonomy;
//     beginRemoveRows(taxonomyRoot(), 0, rowCount(taxonomyRoot()) - 1);
//     m_tax.reset();
//     endRemoveRows();
//
//     emit taxonomyRemoved(oldTax);
//   }
//
//   if (taxonomy)
//   {
//     beginInsertRows(taxonomyRoot(), 0, taxonomy->elements().size() - 1);
//     m_tax = taxonomy;
//     endInsertRows();
//
//     emit taxonomyAdded(m_tax);
//   }
//   markAsChanged();
// }
//
// //------------------------------------------------------------------------
// void EspinaModel::addTaxonomy(TaxonomySPtr taxonomy)
// {
//   if (m_tax)
//   {
//     Q_ASSERT(false); // TODO 1.4: Mix .seg is not finished
//     addTaxonomy(taxonomy->root());
//   }
//   else
//   {
//     setTaxonomy(taxonomy);
//   }
//
//   markAsChanged();
// }
//
// //------------------------------------------------------------------------
// void EspinaModel::itemModified(ModelItemPtr item)
// {
//   QModelIndex itemIndex = index(item);
//   emit dataChanged(itemIndex, itemIndex);
// }
//
// //------------------------------------------------------------------------
// void EspinaModel::addTaxonomy(TaxonomyElementSPtr root)
// {
//   Q_ASSERT(false);//DEPRECATED?
// //   foreach (TaxonomyElementSPtr node, root->subElements())
// //   {
// //     addTaxonomyElement(node->qualifiedName(), m_tax->root());
// //     addTaxonomy(node);
// //   }
// //
// //   markAsChanged();
// }
//
// //------------------------------------------------------------------------
// void EspinaModel::addSampleImplementation(SampleSPtr sample)
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
// //------------------------------------------------------------------------
// void EspinaModel::removeSampleImplementation(SampleSPtr sample)
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
// //------------------------------------------------------------------------
// void EspinaModel::addChannelImplementation(ChannelSPtr channel)
// {
//   Q_ASSERT(channel);
//   Q_ASSERT(!m_channels.contains(channel));
//
//   channel->m_model = this;
//   m_channels << channel;
//   m_relations->addItem(channel.get());
//
//   connect(channel.get(), SIGNAL(modified(ModelItemPtr)),
//           this, SLOT(itemModified(ModelItemPtr)));
// }
//
// //------------------------------------------------------------------------
// void EspinaModel::removeChannelImplementation(ChannelSPtr channel)
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
// //------------------------------------------------------------------------
// void EspinaModel::addSegmentationImplementation(SegmentationSPtr segmentation)
// {
//   Q_ASSERT(segmentation.get() != NULL);
//   Q_ASSERT(m_segmentations.contains(segmentation) == false);
//
//   if (segmentation->number() == 0)
//     segmentation->setNumber(++m_lastId);
//   else
//     m_lastId = qMax(m_lastId, segmentation->number());
//
//   segmentation->m_model = this;
//   segmentation->initializeExtensions();
//
//   m_segmentations << segmentation;
//   m_relations->addItem(segmentation.get());
//
//   connect(segmentation.get(), SIGNAL(modified(ModelItemPtr)),
//           this, SLOT(itemModified(ModelItemPtr)));
// }
//
// //------------------------------------------------------------------------
// void EspinaModel::removeSegmentationImplementation(SegmentationSPtr segmentation)
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
// //------------------------------------------------------------------------
// void EspinaModel::addFilterImplementation(FilterSPtr filter)
// {
//   Q_ASSERT(!m_filters.contains(filter));
//
//   filter->m_model = this;
//   filter->setTraceable(m_isTraceable);
//   m_filters << filter;
//   m_relations->addItem(filter.get());
// }
//
// //------------------------------------------------------------------------
// void EspinaModel::removeFilterImplementation(FilterSPtr filter)
// {
//   m_filters.removeOne(filter);
//   m_relations->removeItem(filter.get());
//
//   filter->m_model = NULL;
// }
//
//
// //------------------------------------------------------------------------
// ModelItemSPtr EspinaModel::find(ModelItemPtr item)
// {
//   ModelItemSPtr res;
//   switch (item->type())
//   {
//     case EspINA::TAXONOMY:
//       res = findTaxonomyElement(item);
//       break;
//     case EspINA::SAMPLE:
//       res = findSample(item);
//       break;
//     case EspINA::CHANNEL:
//       res = findChannel(item);
//       break;
//     case EspINA::FILTER:
//       res = findFilter(item);
//       break;
//     case EspINA::SEGMENTATION:
//       res = findSegmentation(item);
//       break;
//   };
//
//   return res;
// }
//
// //------------------------------------------------------------------------
// TaxonomyElementSPtr EspinaModel::findTaxonomyElement(ModelItemPtr item)
// {
//   return findTaxonomyElement(taxonomyElementPtr(item));
// }
//
// //------------------------------------------------------------------------
// TaxonomyElementSPtr EspinaModel::findTaxonomyElement(TaxonomyElementPtr taxonomyElement)
// {
//   if (taxonomyElement == m_tax->root().get())
//     return m_tax->root();
//
//   TaxonomyElementPtr parent = taxonomyElement->parent();
//   return parent->element(taxonomyElement->name());
// }
//
// //------------------------------------------------------------------------
// SampleSPtr EspinaModel::findSample(ModelItemPtr item)
// {
//   return findSample(samplePtr(item));
// }
//
// //------------------------------------------------------------------------
// SampleSPtr EspinaModel::findSample(SamplePtr sample)
// {
//   SampleSPtr res;
//
//   int i=0;
//   while (!res && i < m_samples.size())
//   {
//     if (m_samples[i].get() == sample)
//       res = m_samples[i];
//     i++;
//   }
//
//   return res;
// }
//
// //------------------------------------------------------------------------
// ChannelSPtr EspinaModel::findChannel(ModelItemPtr item)
// {
//   return findChannel(channelPtr(item));
// }
//
// //------------------------------------------------------------------------
// ChannelSPtr EspinaModel::findChannel(ChannelPtr channel)
// {
//   ChannelSPtr res;
//
//   int i=0;
//   while (!res && i < m_channels.size())
//   {
//     if (m_channels[i].get() == channel)
//       res = m_channels[i];
//     i++;
//   }
//
//   return res;
// }
//
// //------------------------------------------------------------------------
// FilterSPtr EspinaModel::findFilter(ModelItemPtr item)
// {
//   return findFilter(filterPtr(item));
// }
//
// //------------------------------------------------------------------------
// FilterSPtr EspinaModel::findFilter(FilterPtr filter)
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
// //------------------------------------------------------------------------
// SegmentationSPtr EspinaModel::findSegmentation(ModelItemPtr item)
// {
//   return findSegmentation(segmentationPtr(item));
// }
//
// //------------------------------------------------------------------------
// SegmentationSPtr EspinaModel::findSegmentation(SegmentationPtr segmentation)
// {
//   SegmentationSPtr res;
//
//   int i=0;
//   while (!res && i < m_segmentations.size())
//   {
//     if (m_segmentations[i].get() == segmentation)
//       res = m_segmentations[i];
//     i++;
//   }
//
//   return res;
// }
//
// //------------------------------------------------------------------------
// TaxonomyElementSPtr EspinaModel::createTaxonomyElement(TaxonomyElementPtr parent, const QString &name)
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
// //------------------------------------------------------------------------
// TaxonomyElementSPtr EspinaModel::createTaxonomyElement(TaxonomyElementSPtr parent, const QString &name)
// {
//   return createTaxonomyElement(parent.get(), name);
// }
//
// //------------------------------------------------------------------------
// void EspinaModel::addTaxonomyElement(TaxonomyElementSPtr parent, TaxonomyElementSPtr element)
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
// //------------------------------------------------------------------------
// void EspinaModel::removeTaxonomyElement(TaxonomyElementSPtr parent, TaxonomyElementSPtr element)
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
// //------------------------------------------------------------------------
// QModelIndex EspinaModel::sampleRoot() const
// {
//   return createIndex (1, 0, 1);
// }
//
// //------------------------------------------------------------------------
// QModelIndex EspinaModel::sampleIndex(SamplePtr sample) const
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
// //------------------------------------------------------------------------
// QModelIndex EspinaModel::sampleIndex(SampleSPtr sample) const
// {
//   return sampleIndex(sample.get());
// }
//
//
// //------------------------------------------------------------------------
// QModelIndex EspinaModel::channelRoot() const
// {
//   return createIndex (2, 0, 2);
// }
//
// //------------------------------------------------------------------------
// QModelIndex EspinaModel::channelIndex(ChannelPtr channel) const
// {
//   QModelIndex index;
//
//   int row = 0;
//   foreach(ChannelSPtr ptr, m_channels)
//   {
//     if (ptr.get() == channel)
//     {
//       ModelItemPtr internalPtr = channel;
//       index = createIndex(row, 0, internalPtr);
//     }
//     row++;
//   }
//
//   return index;
// }
//
// //------------------------------------------------------------------------
// QModelIndex EspinaModel::channelIndex(ChannelSPtr channel) const
// {
//   return channelIndex(channel.get());
// }
//
// //------------------------------------------------------------------------
// QModelIndex EspinaModel::segmentationRoot() const
// {
//     return createIndex (3, 0, 3);
// }
//
// //------------------------------------------------------------------------
// QModelIndex EspinaModel::segmentationIndex(SegmentationPtr seg) const
// {
//   QModelIndex index;
//
//   int row = 0;
//   foreach(SegmentationSPtr ptr, m_segmentations)
//   {
//     if (ptr.get() == seg)
//     {
//       ModelItemPtr internalPtr = seg;
//       index = createIndex(row, 0, internalPtr);
//     }
//     row++;
//   }
//
//   return index;
// }
//
// //------------------------------------------------------------------------
// QModelIndex EspinaModel::segmentationIndex(SegmentationSPtr seg) const
// {
//   return segmentationIndex(seg.get());
// }
//
// //------------------------------------------------------------------------
// QModelIndex EspinaModel::filterRoot() const
// {
//   return createIndex(4,0,4);
// }
//
// //------------------------------------------------------------------------
// QModelIndex EspinaModel::filterIndex(FilterPtr filter) const
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
// //------------------------------------------------------------------------
// QModelIndex EspinaModel::filterIndex(FilterSPtr filter) const
// {
//   return filterIndex(filter.get());
// }
//