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

#include "SegmentationAdapter.h"

// // EspINA
// #include "Core/Model/Channel.h"
// #include "Core/Model/EspinaFactory.h"
// #include "Core/Model/ModelItem.h"
// #include "Core/Model/Sample.h"
// #include "Core/Model/Segmentation.h"
// #include "Core/Model/Taxonomy.h"
// #include "Core/IO/SegFileReader.h"
// #include "Core/IO/IOErrorHandler.h"
// #include <Core/Extensions/SegmentationExtension.h>
// #include <Core/Extensions/ChannelExtension.h>
// #include "Core/Filters/ChannelReader.h"
// 
// // Qt
// #include <QDebug>
// 
// using namespace EspINA;
// 
// //------------------------------------------------------------------------
// ModelAdapter::ModelAdapter(EspinaFactory *factory)
// : m_factory   (factory)
// , m_relations (new RelationshipGraph())
// , m_lastId    (0)
// , m_changed   (false)
// , m_isTraceable(true)
// {
// }
// 
// //------------------------------------------------------------------------
// ModelAdapter::~ModelAdapter()
// {
// //   qDebug() << "########################################################";
// //   qDebug() << "            Destroying EspINA Model";
// //   qDebug() << "########################################################";
// }
// 
// //------------------------------------------------------------------------
// bool ModelAdapter::isTraceable() const
// {
//   int i = 0;
//   bool result = m_isTraceable;
// 
//   while (result && i < m_filters.size())
//   {
//     result &= m_filters[i]->isTraceable();
//     ++i;
//   }
// 
//   return result;
// }
// 
// 
// //------------------------------------------------------------------------
// void ModelAdapter::reset()
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
// QVariant ModelAdapter::data(const QModelIndex& index, int role) const
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
// bool ModelAdapter::setData ( const QModelIndex& index, const QVariant& value, int role )
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
// QMap<int, QVariant> ModelAdapter::itemData(const QModelIndex &index) const
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
// //------------------------------------------------------------------------
// int ModelAdapter::columnCount(const QModelIndex &parent) const
// {
//     return 1;
// }
// 
// //------------------------------------------------------------------------
// int ModelAdapter::rowCount(const QModelIndex &parent) const
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
// QModelIndex ModelAdapter::parent(const QModelIndex& child) const
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
// QModelIndex ModelAdapter::index (int row, int column, const QModelIndex& parent) const
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
// void ModelAdapter::addSample(SampleSPtr sample)
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
// void ModelAdapter::addSample(SampleSList samples)
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
// //------------------------------------------------------------------------
// void ModelAdapter::addChannel(ChannelSPtr channel)
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
// void ModelAdapter::addChannel(ChannelSList channels)
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
// void ModelAdapter::emitChannelAdded(ChannelSList channels)
// {
//   foreach(ChannelSPtr channel, channels)
//     emit channelAdded(channel);
// }
// 
// //------------------------------------------------------------------------
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
// //------------------------------------------------------------------------
// void ModelAdapter::addSegmentation(SegmentationSPtr segmentation)
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
// void ModelAdapter::addSegmentation(SegmentationSList segmentations)
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
// void ModelAdapter::emitSegmentationAdded(SegmentationSList segmentations)
// {
//   foreach(SegmentationSPtr segmentation, segmentations)
//     emit segmentationAdded(segmentation);
// }
// 
// //------------------------------------------------------------------------
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
// //------------------------------------------------------------------------
// void ModelAdapter::removeSegmentation(SegmentationSList segs)
// {
//   foreach(SegmentationSPtr seg, segs)
//   {
//     removeSegmentation(seg);
//     emit segmentationRemoved(seg);
//   }
// }
// 
// //------------------------------------------------------------------------
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
// //------------------------------------------------------------------------
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
// //------------------------------------------------------------------------
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
// //------------------------------------------------------------------------
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
// //------------------------------------------------------------------------
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
// //------------------------------------------------------------------------
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
// //------------------------------------------------------------------------
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
// //------------------------------------------------------------------------
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
// //------------------------------------------------------------------------
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
// //------------------------------------------------------------------------
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
// //------------------------------------------------------------------------
// QModelIndex ModelAdapter::index(ModelItemSPtr item) const
// {
//   return index(item.get());
// }
// 
// 
// //------------------------------------------------------------------------
// QModelIndex ModelAdapter::taxonomyRoot() const
// {
//     return createIndex ( 0, 0, 0 );
// }
// 
// //------------------------------------------------------------------------
// QModelIndex ModelAdapter::taxonomyIndex(TaxonomyElementPtr node) const
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
// QModelIndex ModelAdapter::taxonomyIndex(TaxonomyElementSPtr node) const
// {
//   return taxonomyIndex(node.get());
// }
// 
// 
// //------------------------------------------------------------------------
// void ModelAdapter::setTaxonomy(TaxonomySPtr taxonomy)
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
// void ModelAdapter::addTaxonomy(TaxonomySPtr taxonomy)
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
// void ModelAdapter::itemModified(ModelItemPtr item)
// {
//   QModelIndex itemIndex = index(item);
//   emit dataChanged(itemIndex, itemIndex);
// }
// 
// //------------------------------------------------------------------------
// void ModelAdapter::addTaxonomy(TaxonomyElementSPtr root)
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
// //------------------------------------------------------------------------
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
// //------------------------------------------------------------------------
// void ModelAdapter::addChannelImplementation(ChannelSPtr channel)
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
// //------------------------------------------------------------------------
// void ModelAdapter::addSegmentationImplementation(SegmentationSPtr segmentation)
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
// //------------------------------------------------------------------------
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
// //------------------------------------------------------------------------
// void ModelAdapter::removeFilterImplementation(FilterSPtr filter)
// {
//   m_filters.removeOne(filter);
//   m_relations->removeItem(filter.get());
// 
//   filter->m_model = NULL;
// }
// 
// 
// //------------------------------------------------------------------------
// ModelItemSPtr ModelAdapter::find(ModelItemPtr item)
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
// TaxonomyElementSPtr ModelAdapter::findTaxonomyElement(ModelItemPtr item)
// {
//   return findTaxonomyElement(taxonomyElementPtr(item));
// }
// 
// //------------------------------------------------------------------------
// TaxonomyElementSPtr ModelAdapter::findTaxonomyElement(TaxonomyElementPtr taxonomyElement)
// {
//   if (taxonomyElement == m_tax->root().get())
//     return m_tax->root();
// 
//   TaxonomyElementPtr parent = taxonomyElement->parent();
//   return parent->element(taxonomyElement->name());
// }
// 
// //------------------------------------------------------------------------
// SampleSPtr ModelAdapter::findSample(ModelItemPtr item)
// {
//   return findSample(samplePtr(item));
// }
// 
// //------------------------------------------------------------------------
// SampleSPtr ModelAdapter::findSample(SamplePtr sample)
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
// ChannelSPtr ModelAdapter::findChannel(ModelItemPtr item)
// {
//   return findChannel(channelPtr(item));
// }
// 
// //------------------------------------------------------------------------
// ChannelSPtr ModelAdapter::findChannel(ChannelPtr channel)
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
// FilterSPtr ModelAdapter::findFilter(ModelItemPtr item)
// {
//   return findFilter(filterPtr(item));
// }
// 
// //------------------------------------------------------------------------
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
// //------------------------------------------------------------------------
// SegmentationSPtr ModelAdapter::findSegmentation(ModelItemPtr item)
// {
//   return findSegmentation(segmentationPtr(item));
// }
// 
// //------------------------------------------------------------------------
// SegmentationSPtr ModelAdapter::findSegmentation(SegmentationPtr segmentation)
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
// //------------------------------------------------------------------------
// TaxonomyElementSPtr ModelAdapter::createTaxonomyElement(TaxonomyElementSPtr parent, const QString &name)
// {
//   return createTaxonomyElement(parent.get(), name);
// }
// 
// //------------------------------------------------------------------------
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
// //------------------------------------------------------------------------
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
// //------------------------------------------------------------------------
// QModelIndex ModelAdapter::sampleRoot() const
// {
//   return createIndex (1, 0, 1);
// }
// 
// //------------------------------------------------------------------------
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
// //------------------------------------------------------------------------
// QModelIndex ModelAdapter::sampleIndex(SampleSPtr sample) const
// {
//   return sampleIndex(sample.get());
// }
// 
// 
// //------------------------------------------------------------------------
// QModelIndex ModelAdapter::channelRoot() const
// {
//   return createIndex (2, 0, 2);
// }
// 
// //------------------------------------------------------------------------
// QModelIndex ModelAdapter::channelIndex(ChannelPtr channel) const
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
// QModelIndex ModelAdapter::channelIndex(ChannelSPtr channel) const
// {
//   return channelIndex(channel.get());
// }
// 
// //------------------------------------------------------------------------
// QModelIndex ModelAdapter::segmentationRoot() const
// {
//     return createIndex (3, 0, 3);
// }
// 
// //------------------------------------------------------------------------
// QModelIndex ModelAdapter::segmentationIndex(SegmentationPtr seg) const
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
// QModelIndex ModelAdapter::segmentationIndex(SegmentationSPtr seg) const
// {
//   return segmentationIndex(seg.get());
// }
// 
// //------------------------------------------------------------------------
// QModelIndex ModelAdapter::filterRoot() const
// {
//   return createIndex(4,0,4);
// }
// 
// //------------------------------------------------------------------------
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
// //------------------------------------------------------------------------
// QModelIndex ModelAdapter::filterIndex(FilterSPtr filter) const
// {
//   return filterIndex(filter.get());
// }
// 