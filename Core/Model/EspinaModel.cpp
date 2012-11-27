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

// Qt
#include <QDebug>

//------------------------------------------------------------------------
EspinaModel::EspinaModel(EspinaFactory *factory, QObject* parent)
: QAbstractItemModel(parent)
, m_factory   (factory)
, m_tax       (NULL)
, m_relations (new RelationshipGraph())
, m_lastId    (0)
, m_changed   (false)
{
}

//------------------------------------------------------------------------
EspinaModel::~EspinaModel()
{
  if (m_tax)
    delete m_tax;
}

//------------------------------------------------------------------------
void EspinaModel::reset()
{
  if (!m_segmentations.isEmpty())
  {
    beginRemoveRows(segmentationRoot(),0,m_segmentations.size()-1);
    m_segmentations.clear();
    endRemoveRows();
  }
  if (!m_filters.isEmpty())
  {
    beginRemoveRows(filterRoot(),0,m_filters.size()-1);
    m_filters.clear();
    endRemoveRows();
  }
  if (!m_channels.isEmpty())
  {
    beginRemoveRows(channelRoot(),0,m_channels.size()-1);
    m_channels.clear();
    endRemoveRows();
  }
  if (!m_samples.isEmpty())
  {
    beginRemoveRows(sampleRoot(),0,m_samples.size()-1);
    m_samples.clear();
    endRemoveRows();
  }
  setTaxonomy(NULL);
  m_relations->clear();//NOTE: Should we remove every item in the previous blocks?

  foreach(QDir tmpDir, m_tmpDirs)
  {
    QDir parentDir = tmpDir;
    parentDir.cdUp();
    foreach(QFileInfo file, tmpDir.entryInfoList())
      QFile::remove(file.absoluteFilePath());

    parentDir.rmdir(tmpDir.absolutePath());
  }
}

//-----------------------------------------------------------------------------
QVariant EspinaModel::data (const QModelIndex& index, int role) const
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

  ModelItem* indexItem = indexPtr(index);
  return indexItem->data(role);
}

//------------------------------------------------------------------------
bool EspinaModel::setData ( const QModelIndex& index, const QVariant& value, int role )
{
  bool result = false;
  if (index.isValid() && index.parent().isValid())// Root indexes cannot be modified
  {
    // Other elements can set their own data
    ModelItem *indexItem = indexPtr(index);
    result = indexItem->setData(value, role);
    if (result) //NOTE: is emit required?
      emit dataChanged(index,index);
  }
  return result;
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

  ModelItem *item = indexPtr(index);
  if (ModelItem::SEGMENTATION == item->type() || ModelItem::CHANNEL == item->type())
    return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
  else
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}


//------------------------------------------------------------------------
int EspinaModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

//------------------------------------------------------------------------
int EspinaModel::rowCount(const QModelIndex &parent) const
{
    // There are 4 root indexes
    if ( !parent.isValid() )
        return 5;

    if ( parent == taxonomyRoot() )
        return m_tax?m_tax->elements().size() :0;

    if ( parent == sampleRoot() )
        return m_samples.size();

    if ( parent == channelRoot() )
        return m_channels.size();

    if ( parent == segmentationRoot() )
        return m_segmentations.size();

    if ( parent == filterRoot() )
        return m_filters.size();

    // Cast to base type
    ModelItem *parentItem = indexPtr(parent);
    if (parentItem->type() == ModelItem::TAXONOMY)
    {
      TaxonomyElement *taxItem = dynamic_cast<TaxonomyElement *>(parentItem);
      Q_ASSERT(taxItem);
      return taxItem->subElements().size();
    }

    // Otherwise Samples and Segmentations have no children
    return 0;
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

  ModelItem *childItem = indexPtr(child);
  Q_ASSERT (childItem);

  switch (childItem->type())
  {
    case ModelItem::TAXONOMY:
    {
      TaxonomyElement *childNode = dynamic_cast<TaxonomyElement *>(childItem);
      Q_ASSERT(childNode);
      return taxonomyIndex(childNode->parentNode());//NOTE: It's ok with TaxonomyRoot
    }
    case ModelItem::SAMPLE:
      return sampleRoot();
    case ModelItem::CHANNEL:
      return channelRoot();
    case ModelItem::SEGMENTATION:
      return segmentationRoot();
    case ModelItem::FILTER:
      return filterRoot();
    default:
      Q_ASSERT(false);
      return QModelIndex();
  };
  return QModelIndex();
}

//------------------------------------------------------------------------
// Returned index is compossed by the row, column and an element).
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

  ModelItem *internalPtr;

  if (parent == sampleRoot())
  {
    Q_ASSERT(row < m_samples.size());
    internalPtr = m_samples[row];
    return createIndex(row, column, internalPtr);
  }
  if (parent == channelRoot())
  {
    Q_ASSERT(row < m_channels.size());
    internalPtr = m_channels[row];
    return createIndex(row, column, internalPtr);
  }
  if (parent == segmentationRoot())
  {
    //     if (row >= m_segmentations.size())//NOTE: Don't know why, but when removing taxonomy node, wrong row number is gotten
    //       return QModelIndex();
    Q_ASSERT(row < m_segmentations.size());
    internalPtr = m_segmentations[row];
    return createIndex(row, column, internalPtr);
  }
  if (parent == filterRoot())
  {
    Q_ASSERT(row < m_filters.size());
    internalPtr = m_filters[row];
    return createIndex(row, column, internalPtr);
  }

  TaxonomyElement *parentTax;
  if (parent == taxonomyRoot())
  {
    parentTax = m_tax->elements()[0]->parentNode();//recover root node...
  }
  else
  {
    // Neither Samples nor Segmentations have children
    ModelItem *parentItem = indexPtr(parent);
    parentTax = dynamic_cast<TaxonomyElement *>(parentItem);
  }
  //WARNING: Now m_tax can be NULL, but even in that situation,
  //         it shouldn't report any children
  Q_ASSERT(parentTax);
  Q_ASSERT(row < parentTax->subElements().size());
  internalPtr = parentTax->subElements()[row];
  return createIndex(row, column, internalPtr);
}

//------------------------------------------------------------------------
void EspinaModel::addSample (Sample *sample)
{
  Q_ASSERT(m_samples.contains(sample) == false);

  int row = m_samples.size();

  beginInsertRows(sampleRoot(), row, row);
  m_samples << sample;
  m_relations->addItem(sample);
  endInsertRows();
  markAsChanged();
}

//------------------------------------------------------------------------
void EspinaModel::addSample(QList<Sample *> samples)
{
//   beginInsertRows(sampleRoot(),);
//   foreach(SamplePtr sample, samples)
//   {
//     Q_ASSERT(m_samples.contains(sample) == false);
//   }
  markAsChanged();
}


//------------------------------------------------------------------------
void EspinaModel::removeSample(Sample *sample)
{
  if (m_samples.contains(sample) == false)
    return;

  //     // Remove the Segmentations associated
    //     dynamic_cast<LabelMapExtension::SampleRepresentation *>(sample->representation(LabelMapExtension::SampleRepresentation::ID))->setEnable(false);
    //     foreach(Segmentation* seg, sample->segmentations())
    //     {
      //       removeSegmentation(seg);
    //     }
    //     dynamic_cast<LabelMapExtension::SampleRepresentation *>(sample->representation(LabelMapExtension::SampleRepresentation::ID))->setEnable(true);
    // //     assert(m_sampleSegs[sample].size() == 0);
    // //     m_sampleSegs.remove(sample);
    // //     assert(!m_sampleSegs.contains(sample));
    // Remove it from analysis
  QModelIndex index = sampleIndex(sample);
  beginRemoveRows(index.parent(), index.row(), index.row());
  m_samples.removeOne(sample);
  Q_ASSERT (m_samples.contains(sample) == false );
  //   m_analysis->removeNode(sample);
  endRemoveRows();
  markAsChanged();
}

//------------------------------------------------------------------------
void EspinaModel::addChannel(Channel *channel)
{
  Q_ASSERT(m_channels.contains(channel) == false);

  int row = m_channels.size();

  beginInsertRows(channelRoot(), row, row);
//   channel->initialize();
  m_channels << channel;
  m_relations->addItem(channel);

  connect(channel, SIGNAL(modified(ModelItem*)),
	  this, SLOT(itemModified(ModelItem*)));
  endInsertRows();
  markAsChanged();
}

//------------------------------------------------------------------------
void EspinaModel::removeChannel(Channel *channel)
{
  if (m_channels.contains(channel) == false)
    return;

  QModelIndex index = channelIndex(channel);

  beginRemoveRows(index.parent(), index.row(), index.row());
  m_channels.removeOne(channel);
  Q_ASSERT(m_channels.contains(channel) == false);
  endRemoveRows();
  markAsChanged();
}

//------------------------------------------------------------------------
void EspinaModel::addSegmentation(Segmentation *seg)
{
  Q_ASSERT(m_segmentations.contains(seg) == false);
  int row = m_segmentations.size();

  beginInsertRows(segmentationRoot(), row, row);
  if (seg->number() == 0)
    seg->setNumber(++m_lastId);
  else
    m_lastId = qMax(m_lastId, seg->number());
  m_segmentations << seg;
  m_relations->addItem(seg);
  connect(seg, SIGNAL(modified(ModelItem*)),
	  this, SLOT(itemModified(ModelItem*)));
  endInsertRows();
  markAsChanged();
}

//------------------------------------------------------------------------
void EspinaModel::addSegmentation(QList<Segmentation *> segs)
{
  int numExistingSegs = m_segmentations.size();
  int numNewSegs = segs.size();

  beginInsertRows(segmentationRoot(), numExistingSegs, numExistingSegs+numNewSegs-1);
  foreach(Segmentation *seg, segs)
  {
    Q_ASSERT(m_segmentations.contains(seg) == false);
    if (seg->number() == 0)
      seg->setNumber(++m_lastId);
    else
      m_lastId = qMax(m_lastId, seg->number());
    m_segmentations << seg;
    m_relations->addItem(seg);
    connect(seg, SIGNAL(modified(ModelItem*)),
	    this, SLOT(itemModified(ModelItem*)));
  }
  endInsertRows();
  markAsChanged();
}


//------------------------------------------------------------------------
void EspinaModel::removeSegmentation(Segmentation *seg)
{
//   // Update model
  Q_ASSERT(m_segmentations.contains(seg) == true);
  QModelIndex index = segmentationIndex(seg);
  beginRemoveRows(index.parent(), index.row(), index.row());
  m_segmentations.removeOne(seg);
  m_relations->removeItem(seg);
  Q_ASSERT(m_segmentations.contains(seg) == false);
  endRemoveRows();
  markAsChanged();
}

//------------------------------------------------------------------------
void EspinaModel::removeSegmentation(QList<Segmentation *> segs)
{
  //TODO: Group removals ==> also in proxies
  foreach(Segmentation *seg, segs)
  {
    removeSegmentation(seg);
  }
  markAsChanged();
}

//------------------------------------------------------------------------
void EspinaModel::changeTaxonomy(Segmentation* seg, TaxonomyElement* taxonomy)
{
  seg->setTaxonomy(taxonomy);

  QModelIndex segIndex = segmentationIndex(seg);
  emit dataChanged(segIndex, segIndex);
  markAsChanged();
}


//------------------------------------------------------------------------
void EspinaModel::addFilter(Filter *filter)
{
  Q_ASSERT(!m_filters.contains(filter));

  int row = m_filters.size();

  beginInsertRows(filterRoot(), row, row);
  m_filters << filter;
  m_relations->addItem(filter);
  endInsertRows();
  markAsChanged();
}

//------------------------------------------------------------------------
void EspinaModel::removeFilter(Filter *filter)
{
  Q_ASSERT(m_filters.contains(filter) == true);
  QModelIndex index = filterIndex(filter);
  beginRemoveRows(index.parent(), index.row(), index.row());
  m_filters.removeOne(filter);
  m_relations->removeItem(filter);
  Q_ASSERT(m_filters.contains(filter) == false);
  endRemoveRows();
  markAsChanged();
}

//------------------------------------------------------------------------
void EspinaModel::addRelation(ModelItem* ancestor, ModelItem* successor, QString relation)
{
  m_relations->addRelation(ancestor, successor, relation);
  QModelIndex ancestorIndex = index(ancestor);
  QModelIndex successorIndex = index(successor);
//   qDebug() << ancestorIndex.data().toString() << "==" << relation << "==>" << successorIndex.data().toString();
  emit dataChanged(ancestorIndex, ancestorIndex);
  emit dataChanged(successorIndex, successorIndex);
  markAsChanged();
}

//------------------------------------------------------------------------
void EspinaModel::removeRelation(ModelItem* ancestor, ModelItem* successor, QString relation)
{
  m_relations->removeRelation(ancestor, successor, relation);
  QModelIndex ancestorIndex = index(ancestor);
  QModelIndex succesorIndex = index(successor);
  emit dataChanged(ancestorIndex, ancestorIndex);
  emit dataChanged(succesorIndex, succesorIndex);
  markAsChanged();
}

//------------------------------------------------------------------------
void EspinaModel::serializeRelations(std::ostream& stream, RelationshipGraph::PrintFormat format)
{
  m_relations->updateVertexInformation();
  m_relations->write(stream, format);
}

//------------------------------------------------------------------------
bool EspinaModel::loadSerialization(istream& stream,
                                    QDir tmpDir,
                                    RelationshipGraph::PrintFormat format)
{
  QSharedPointer<RelationshipGraph> input(new RelationshipGraph());
  m_tmpDirs << tmpDir;

  input->read(stream);
//   qDebug() << "Check";
//   input->write(std::cout, RelationshipGraph::GRAPHVIZ);

  typedef QPair<ModelItem *, ModelItem::Arguments> NonInitilizedItem;
  QList<NonInitilizedItem> nonInitializedItems;
  QList<VertexProperty> segmentationNodes;
  QList<Segmentation *> newSegmentations;

  foreach(VertexProperty v, input->vertices())
  {
    VertexProperty fv;
    if (m_relations->find(v, fv))
    {
      input->setItem(v.vId, fv.item);
      qDebug() << "Updating existing vertex" << fv.item->data(Qt::DisplayRole).toString();
    }else
    {
      switch (RelationshipGraph::type(v))
      {
        case ModelItem::SAMPLE:
        {
          ModelItem::Arguments args(v.args.c_str());
          Sample *sample = m_factory->createSample(v.name.c_str(), v.args.c_str());
          addSample(sample);
          nonInitializedItems << NonInitilizedItem(sample, args);
          input->setItem(v.vId, sample);
          break;
        }
        case ModelItem::CHANNEL:
        {
          ModelItem::Arguments args(v.args.c_str());
          ModelItem::Arguments extArgs(args.value(ModelItem::EXTENSIONS, QString()));
          args.remove(ModelItem::EXTENSIONS);
          // TODO: Move link management code inside Channel's Arguments class
          QStringList link = args[Channel::VOLUME].split("_");
          Q_ASSERT(link.size() == 2);
          Vertices ancestors = input->ancestors(v.vId, link[0]);
          Q_ASSERT(ancestors.size() == 1);
          ModelItem *item = ancestors.first().item;
          Q_ASSERT(ModelItem::FILTER == item->type());
          Filter *filter =  dynamic_cast<Filter *>(item);
          filter->update();
          Channel *channel = m_factory->createChannel(filter, link[1].toInt());
          channel->initialize(args);
          if (channel->itkVolume() == NULL)
            return false;
          addChannel(channel);
          nonInitializedItems << NonInitilizedItem(channel, extArgs);
          input->setItem(v.vId, channel);
          break;
        }
        case ModelItem::FILTER:
        {
          Filter::NamedInputs inputs;
          Filter::Arguments args(v.args.c_str());
          QStringList inputLinks = args[Filter::INPUTS].split(",", QString::SkipEmptyParts);
          // We need to update id values for future filters
          foreach(QString inputLink, inputLinks)
          {
            QStringList link = inputLink.split("_");
            Q_ASSERT(link.size() == 2);
            Vertices ancestors = input->ancestors(v.vId, link[0]);
            Q_ASSERT(ancestors.size() == 1);
            ModelItem *item = ancestors.first().item;
            Q_ASSERT(ModelItem::FILTER == item->type());
            Filter *filter =  dynamic_cast<Filter *>(item);
            inputs[link[0]] = filter;
          }
          Filter *filter = m_factory->createFilter(v.name.c_str(), inputs, args);
          filter->setTmpDir(tmpDir);
          //filter->update();
          addFilter(filter);
          input->setItem(v.vId, filter);
          break;
        }
        case ModelItem::SEGMENTATION:
        {
          segmentationNodes << v;
          break;
        }
        default:
          Q_ASSERT(false);
          break;
      }
    }
  }

  foreach(VertexProperty v, segmentationNodes)
  {
    Vertices ancestors = input->ancestors(v.vId, CREATELINK);
    Q_ASSERT(ancestors.size() == 1);
    ModelItem *item = ancestors.first().item;
    Filter *filter =  dynamic_cast<Filter *>(item);
    filter->update();
    if (filter->outputs().isEmpty())
      return false;

    ModelItem::Arguments args(QString(v.args.c_str()));
    ModelItem::Arguments extArgs(args.value(ModelItem::EXTENSIONS, QString()));
    args.remove(ModelItem::EXTENSIONS);
    Segmentation *seg = m_factory->createSegmentation(filter, args[Segmentation::OUTPUT].toInt());
    seg->setNumber(args[Segmentation::NUMBER].toInt());
    TaxonomyElement *taxonomy = m_tax->element(args[Segmentation::TAXONOMY]);
    if (taxonomy)
      seg->setTaxonomy(taxonomy);
    newSegmentations << seg;
    nonInitializedItems << NonInitilizedItem(seg, extArgs);
    input->setItem(v.vId, seg);
  }

  addSegmentation(newSegmentations);

  foreach(Edge e, input->edges())
  { //Should store just the modelitem?
    Q_ASSERT(e.source.item);
    Q_ASSERT(e.target.item);
    addRelation(e.source.item, e.target.item, e.relationship.c_str());
  }

  foreach(NonInitilizedItem item, nonInitializedItems)
    item.first->initializeExtensions(item.second);

  return true;
}

//------------------------------------------------------------------------
QModelIndex EspinaModel::index(ModelItem* item) const
{
  QModelIndex res;
  switch (item->type())
  {
    case ModelItem::TAXONOMY:
      res = taxonomyIndex(dynamic_cast<TaxonomyElement *>(item));
      break;
    case ModelItem::SAMPLE:
      res = sampleIndex(dynamic_cast<Sample *>(item));
      break;
    case ModelItem::CHANNEL:
      res = channelIndex(dynamic_cast<Channel *>(item));
      break;
    case ModelItem::FILTER:
      res = filterIndex(dynamic_cast<Filter *>(item));
      break;
    case ModelItem::SEGMENTATION:
      res = segmentationIndex(dynamic_cast<Segmentation *>(item));
      break;
    default:
      Q_ASSERT(false);
      break;
  }
  return res;
}

//------------------------------------------------------------------------
QModelIndex EspinaModel::taxonomyRoot() const
{
    return createIndex ( 0, 0, 0 );
}

//------------------------------------------------------------------------
void EspinaModel::setTaxonomy(Taxonomy *tax)
{
  if (m_tax)
  {
    beginRemoveRows ( taxonomyRoot(),0,rowCount ( taxonomyRoot() )-1 );
    delete m_tax;
    m_tax = NULL;
    endRemoveRows();
  }

  if (tax)
  {
    beginInsertRows ( taxonomyRoot(), 0, tax->elements().size()-1 );
    m_tax = tax;
    endInsertRows();
  }

  emit dataChanged ( taxonomyRoot(),taxonomyRoot() );
  //   setUserDefindedTaxonomy(m_tax->elements()[0]->qualifiedName());
  //   emit resetTaxonomy();
}

//------------------------------------------------------------------------
void EspinaModel::addTaxonomy(Taxonomy* tax)
{
  if (m_tax)
    addTaxonomy(tax->root());
  else
    setTaxonomy(tax);
}

//------------------------------------------------------------------------
void EspinaModel::itemModified(ModelItem* item)
{
  QModelIndex itemIndex = index(item);
  emit dataChanged(itemIndex, itemIndex);
}

//------------------------------------------------------------------------
void EspinaModel::addTaxonomy(TaxonomyElement *root)
{
  foreach (TaxonomyElement *node, root->subElements())
  {
    addTaxonomyElement(taxonomyRoot(), node->qualifiedName());
    addTaxonomy(node);
  }
  markAsChanged();
}

//------------------------------------------------------------------------
QModelIndex EspinaModel::addTaxonomyElement(const QModelIndex& parent, QString qualifiedName)
{
  TaxonomyElement *parentNode = m_tax->root();
  if (parent != taxonomyRoot())
  {
    ModelItem *item = indexPtr(parent);
    Q_ASSERT (item->type() == ModelItem::TAXONOMY);
    parentNode = dynamic_cast<TaxonomyElement *>(item);
  }
  Q_ASSERT(parentNode);
  QStringList subTaxonomies = qualifiedName.split("/");
  TaxonomyElement *requestedNode = NULL;
  foreach (QString subTax, subTaxonomies)
  {
    requestedNode = parentNode->element(subTax);
    if (!requestedNode)
    {
      QModelIndex parentItem = taxonomyIndex(parentNode);
      int newTaxRow = rowCount(parentItem);
      beginInsertRows(parentItem, newTaxRow, newTaxRow);
      requestedNode = m_tax->addElement(subTax, parentNode->qualifiedName());
      endInsertRows();
    }
    parentNode = requestedNode;
  }
  markAsChanged();
  return taxonomyIndex(requestedNode);
}

//------------------------------------------------------------------------
void EspinaModel::addTaxonomyElement(QString qualifiedName)
{
  

}

//------------------------------------------------------------------------
void EspinaModel::removeTaxonomyElement(const QModelIndex &index)
{
    ModelItem *item = indexPtr(index);
    Q_ASSERT (item->type() == ModelItem::TAXONOMY);
    TaxonomyElement *node = dynamic_cast<TaxonomyElement *>(item);
    beginRemoveRows(index.parent(), index.row(), index.row());
    node->parentNode()->removeChild(node->name());
    endRemoveRows();
    markAsChanged();
}

//------------------------------------------------------------------------
void EspinaModel::removeTaxonomyElement(QString qualifiedName)
{
  // deprecated
  Q_ASSERT(false);
}

//------------------------------------------------------------------------
QModelIndex EspinaModel::sampleRoot() const
{
  return createIndex (1, 0, 1);
}

//------------------------------------------------------------------------
QModelIndex EspinaModel::sampleIndex(Sample *sample) const
{
  QModelIndex sampleIndex;

  ModelItem *internalPtr = sample;
  int row = m_samples.indexOf(sample);
  if (row >= 0)
    sampleIndex = createIndex(row, 0, internalPtr);

  return sampleIndex;
}

//------------------------------------------------------------------------
QModelIndex EspinaModel::channelRoot() const
{
  return createIndex (2, 0, 2);
}

//------------------------------------------------------------------------
QModelIndex EspinaModel::channelIndex(Channel *channel) const
{
    int row = m_channels.indexOf (channel);
    ModelItem *internalPtr = channel;
    return createIndex (row, 0, internalPtr);
}


//------------------------------------------------------------------------
QModelIndex EspinaModel::segmentationRoot() const
{
    return createIndex (3, 0, 3);
}

//------------------------------------------------------------------------
QModelIndex EspinaModel::segmentationIndex(Segmentation *seg) const
{
  int row = m_segmentations.indexOf(seg);
  ModelItem *internalPtr = seg;
  return createIndex(row, 0, internalPtr);
}

//------------------------------------------------------------------------
QModelIndex EspinaModel::filterRoot() const
{
  return createIndex(4,0,4);
}

//------------------------------------------------------------------------
QModelIndex EspinaModel::filterIndex(Filter *filter) const
{
  int row = m_filters.indexOf(filter);
  ModelItem *internalPtr = filter;
  return createIndex(row, 0, internalPtr);
}

//------------------------------------------------------------------------
QModelIndex EspinaModel::taxonomyIndex(TaxonomyElement *node) const
{
  // We avoid setting the Taxonomy descriptor as parent of an index
  if ( !m_tax || m_tax->root() == node)
    return taxonomyRoot();

  TaxonomyElement *parentNode = node->parentNode();

  if ( !parentNode )
    qDebug() << "Child" << node->qualifiedName() << "without parent";

  Q_ASSERT(parentNode);
  int row = parentNode->subElements().indexOf(node);
  ModelItem *internalPtr = node;
  return createIndex(row, 0, internalPtr);
}
