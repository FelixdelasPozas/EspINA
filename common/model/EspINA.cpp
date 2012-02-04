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


#include "model/EspINA.h"

// Debug
#include <QDebug>
// #include "espina_debug.h"

//Espina
#include "model/Channel.h"
#include "model/Sample.h"
#include "model/Segmentation.h"
#include "model/Taxonomy.h"

//------------------------------------------------------------------------
EspINA *EspINA::m_singleton(NULL);

//------------------------------------------------------------------------
EspINA* EspINA::instance()
{
  if (!m_singleton)
    m_singleton = new EspINA();

  return m_singleton;
}

//------------------------------------------------------------------------
EspINA::EspINA ( QObject* parent )
        : QAbstractItemModel ( parent )
        , m_tax ( NULL )
// , m_activeSample(NULL)
// , m_nextValidSegId(1)
{
//   loadTaxonomy();
//   m_newSegType = NULL;//->getComponent("Symetric");
//   m_analysis = ProcessingTrace::instance();
}

//------------------------------------------------------------------------
EspINA::~EspINA()
{
  if (m_tax)
    delete m_tax;
}

//------------------------------------------------------------------------
void EspINA::clear()
{
  setTaxonomy ( NULL );
  if (!m_samples.isEmpty())
  {
    beginRemoveRows(sampleRoot(),0,0);
    m_samples.clear();
    endRemoveRows();
  }
  if (!m_channels.isEmpty())
  {
    beginRemoveRows(channelRoot(),0,0);
    m_channels.clear();
    endRemoveRows();
  }
}

//-----------------------------------------------------------------------------
QVariant EspINA::data (const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();

  // Taxonomys' Root
  if (index == taxonomyRoot())
  {
    if (role == Qt::DisplayRole)
      if (m_tax)
	return m_tax->name();
      else
	return tr("Taxonomies");
      return QVariant();
  }
  // Samples' Root
  if (index == sampleRoot())
  {
    if (role == Qt::DisplayRole)
      return tr("Samples");
    return QVariant();
  }
  // Channels' Root
  if (index == channelRoot())
  {
    if (role == Qt::DisplayRole)
      return tr("Channels");
    return QVariant();
  }
  // Segmentations' Root
  if (index == segmentationRoot())
  {
    if (role == Qt::DisplayRole)
      return "Segmentations";
    return QVariant();
  }

  IModelItem *indexItem = static_cast<IModelItem *>(index.internalPointer());
  return indexItem->data(role);
}

//------------------------------------------------------------------------
bool EspINA::setData ( const QModelIndex& index, const QVariant& value, int role )
{
//   bool result = false;
//   if (index.isValid() && index.parent().isValid())// Root indexes cannot be modified
//   {
//     // Other elements can set their own data
//     IModelItem *indexItem = static_cast<IModelItem *>(index.internalPointer());
//     result = indexItem->setData(value, role);
//     if (result)
//     {
//       if (role == Qt::CheckStateRole)
//       {
// 	Segmentation *seg = dynamic_cast<Segmentation *>(indexItem);
// 	if (seg)
// 	{
// 	  QElapsedTimer timer;
// 	  timer.start();
// 	  seg->origin()->representation(LabelMapExtension::SampleRepresentation::ID)->requestUpdate(true);
// 	  qDebug() << "Updating Check took: " << timer.elapsed();
// 	  QModelIndex segIndex = segmentationIndex(seg);
// 	  emit dataChanged(segIndex,segIndex);
// 	}
//       }
//       TaxonomyNode *taxItem = dynamic_cast<TaxonomyNode *>(indexItem);
//       if (taxItem && role == Qt::DecorationRole)
//       {
// 	qDebug() << "Change color to" << taxItem->qualifiedName();
// 	foreach(Segmentation *seg, m_taxonomySegs[taxItem])
// 	{
// 	  QModelIndex segIndex = segmentationIndex(seg);
// 	  //seg->setData(value, role);
// 	  emit dataChanged(segIndex,segIndex);
// 	}
// 	if (m_taxonomySegs[taxItem].size())
// 	{
// 	  qDebug() << "Request segmentation update";
// 	  m_taxonomySegs[taxItem].first()->origin()->representation(LabelMapExtension::SampleRepresentation::ID)->requestUpdate(true);
// 	}
//       }
//       emit dataChanged(index,index);
//     }
//   }
//   return result;
    return false;
}

//------------------------------------------------------------------------
int EspINA::columnCount ( const QModelIndex& parent ) const
{
//   if (parent == segmentationRoot())
//   {
//     int infoSize = EspINAFactory::instance()->segmentationAvailableInformations().size();
//     return infoSize;
//   }
//   else
    return 1;
}

//------------------------------------------------------------------------
int EspINA::rowCount ( const QModelIndex& parent ) const
{
    // There are 4 root indexes
    if ( !parent.isValid() )
        return 4;

    if ( parent == taxonomyRoot() )
        return m_tax?m_tax->elements().size() :0;

    if ( parent == sampleRoot() )
        return m_samples.size();

    if ( parent == channelRoot() )
        return m_channels.size();

    if ( parent == segmentationRoot() )
        return m_segmentations.size();

    // Cast to base type
    IModelItem *parentItem = static_cast<IModelItem *> ( parent.internalPointer() );
    if (parentItem->type() == IModelItem::TAXONOMY)
    {
      TaxonomyNode *taxItem = dynamic_cast<TaxonomyNode *> ( parentItem );
      Q_ASSERT(taxItem);
      return taxItem->subElements().size();
    }

    // Otherwise Samples and Segmentations have no children
    return 0;
}

//------------------------------------------------------------------------
QModelIndex EspINA::parent (const QModelIndex& child) const
{
  if (!child.isValid())
    return QModelIndex();

  if ( child == taxonomyRoot()
    || child == sampleRoot()
    || child == channelRoot()
    || child == segmentationRoot() )
    return QModelIndex();

  IModelItem *childItem = static_cast<IModelItem *>(child.internalPointer());
  Q_ASSERT (childItem);

  switch (childItem->type())
  {
    case IModelItem::TAXONOMY:
    {
      TaxonomyNode *childNode = dynamic_cast<TaxonomyNode *>(childItem);
      Q_ASSERT(childNode);
      return taxonomyIndex(childNode->parentNode());//NOTE: It's ok with TaxonomyRoot
    }
    case IModelItem::SAMPLE:
      return sampleRoot();
    case IModelItem::CHANNEL:
      return channelRoot();
    case IModelItem::SEGMENTATION:
      return segmentationRoot();
    default:
      Q_ASSERT(false);
      return QModelIndex();
  };
  return QModelIndex();
}

//------------------------------------------------------------------------
// Returned index is compossed by the row, column and an element).
QModelIndex EspINA::index (int row, int column, const QModelIndex& parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  if (!parent.isValid())
  {
    Q_ASSERT (row < 4);
    if (row == 0)
      return taxonomyRoot();
    if (row == 1)
      return sampleRoot();
    if (row == 2)
      return channelRoot();
    if (row == 3)
      return segmentationRoot();
  }

  IModelItem *internalPtr;

  // Checks if parent is Sample's root
  if (parent == sampleRoot())
  {
    Q_ASSERT(row < m_samples.size());
    internalPtr = m_samples[row];
    return createIndex(row, column, internalPtr);
  }
  // Checks if parent is Channel's root
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

  TaxonomyNode *parentTax;
  if (parent == taxonomyRoot())
  {
    parentTax = m_tax->elements()[0]->parentNode();//recover root node...
  }
  else
  {
    // Neither Samples nor Segmentations have children
    IModelItem *parentItem = static_cast<IModelItem *>(parent.internalPointer());
    parentTax = dynamic_cast<TaxonomyNode *>(parentItem);
  }
  //WARNING: Now m_tax can be NULL, but even in that situation,
  //         it shouldn't report any children
  Q_ASSERT(parentTax);
  Q_ASSERT(row < parentTax->subElements().size());
  internalPtr = parentTax->subElements()[row];
  return createIndex(row, column, internalPtr);
//   int subTaxonomies = parentTax->subElements().size();
//   //NOTE: Don't know why, but when removing taxonomy node, wrong row number is gotten
//   if ( row >= subTaxonomies ) 
//     return QModelIndex();
}

//------------------------------------------------------------------------
void EspINA::addSample ( Sample* sample )
{
    int row = m_samples.size();
    beginInsertRows ( sampleRoot(), row, row );
    m_samples.push_back ( sample );
    endInsertRows();
}

//------------------------------------------------------------------------
void EspINA::removeSample ( Sample* sample )
{
    if ( m_samples.contains ( sample ) == false )
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
    QModelIndex index = sampleIndex ( sample );
    beginRemoveRows ( index.parent(), index.row(), index.row() );
    m_samples.removeOne ( sample );
    Q_ASSERT ( m_samples.contains ( sample ) == false );
//   m_analysis->removeNode(sample);
    endRemoveRows();
}

//------------------------------------------------------------------------
void EspINA::addChannel ( Sample* sample, Channel* channel )
{
    int row = m_channels.size();
    beginInsertRows ( channelRoot(), row, row );
    m_channels.push_back ( channel );
    endInsertRows();
}

//------------------------------------------------------------------------
void EspINA::removeChannel ( Sample* sample, Channel* channel )
{
    if ( m_samples.contains ( sample ) == false )
        return;
    if ( m_channels.contains ( channel ) == false )
        return;

    // Remove it from analysis
    QModelIndex index = channelIndex ( channel );
    beginRemoveRows ( index.parent(), index.row(), index.row() );
    m_channels.removeOne ( channel );
    Q_ASSERT ( m_channels.contains ( channel ) == false );
    endRemoveRows();

}

//------------------------------------------------------------------------
void EspINA::addSegmentation(Segmentation *seg)
{
//   TaxonomyNode *node = m_newSegType;
//
//   // We need to notify other components that the model has changed
  int row = m_segmentations.size();

  beginInsertRows(segmentationRoot(), row, row);
//   if (!seg->taxonomy())
//     seg->setTaxonomy(node);
//   seg->setOrigin(m_activeSample);
//   if (!seg->validId())
//     seg->setId(nextSegmentationId());
//   seg->initialize();
//   m_taxonomySegs[m_newSegType].push_back(seg);
//   seg->origin()->addSegmentation(seg);
//   connect(seg, SIGNAL(updated(Segmentation*)), this, SLOT(internalSegmentationUpdate(Segmentation*)));
//   //m_sampleSegs[seg->origin()].push_back(seg);
  m_segmentations.push_back(seg);
  endInsertRows();
}

//------------------------------------------------------------------------
void EspINA::removeSegmentation(Segmentation* seg)
{
//   // Update model
  Q_ASSERT(m_segmentations.contains(seg) == true);
  QModelIndex index = segmentationIndex(seg);
  beginRemoveRows(index.parent(), index.row(), index.row());
  m_segmentations.removeOne(seg);
  Q_ASSERT(m_segmentations.contains(seg) == false);
//   m_taxonomySegs[seg->taxonomy()].removeOne(seg);
//   seg->origin()->removeSegmentation(seg);
//   //m_sampleSegs[seg->origin()].removeOne(seg);
//   // Free internal memory
//   m_analysis->removeNode(seg);
  endRemoveRows();
}

// //------------------------------------------------------------------------
// int EspINA::numOfSubTaxonomies(TaxonomyNode* tax) const
// {
//   if (tax)
//     return tax->subElements().size();
//   return 0;
// }
//
// //------------------------------------------------------------------------
// int EspINA::numOfSegmentations(TaxonomyNode* tax) const
// {
//   return segmentations(tax).size();
// }
//
//
// //------------------------------------------------------------------------
// QVariant EspINA::headerData(int section, Qt::Orientation orientation, int role) const
// {
//   if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section >= 0)
//   {
//     return EspINAFactory::instance()->segmentationAvailableInformations()[section];
//   }
//   return QVariant();
// }
//
// //------------------------------------------------------------------------
// Qt::ItemFlags EspINA::flags(const QModelIndex& index) const
// {
//   if (!index.isValid())
//     return Qt::ItemIsEnabled;
//
//   if (index == taxonomyRoot() || index == sampleRoot() || index == segmentationRoot())
//     return Qt::ItemIsEnabled;
//
//   if (index.parent() == sampleRoot())
//     return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
//
//   return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable | Qt::ItemIsEditable ;
// }
//
// //  //------------------------------------------------------------------------
// // bool EspINA::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
// // {
// //   qDebug("Dropping Data");
// //   return QAbstractItemModel::dropMimeData(data, action, row, column, parent);
// // }

//------------------------------------------------------------------------
QModelIndex EspINA::taxonomyRoot() const
{
    return createIndex ( 0, 0, 0 );
}

//------------------------------------------------------------------------
void EspINA::setTaxonomy ( Taxonomy* tax )
{
    if ( m_tax )
    {
        beginRemoveRows ( taxonomyRoot(),0,rowCount ( taxonomyRoot() )-1 );
        m_tax = NULL;
        endRemoveRows();
    }

    if ( tax )
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
QModelIndex EspINA::addTaxonomyElement(const QModelIndex& parent, QString qualifiedName)
{
  int newTaxRow = rowCount(parent);
  QString parentName = m_tax->name();
  if (parent != taxonomyRoot())
  {
    IModelItem *item = static_cast<IModelItem *> (parent.internalPointer());
    Q_ASSERT (item->type() == IModelItem::TAXONOMY);
    TaxonomyNode *node = dynamic_cast<TaxonomyNode*> (item);
    parentName = node->qualifiedName();
  }
  TaxonomyNode *parentNode = m_tax->element(parentName);
  Q_ASSERT(parentNode);
  TaxonomyNode *requestedNode = parentNode->element(qualifiedName);
  if (!requestedNode)
  {
    beginInsertRows(parent, newTaxRow, newTaxRow);
    requestedNode = m_tax->addElement(qualifiedName, parentName);
    endInsertRows();
  }
  return taxonomyIndex(requestedNode);
}

//------------------------------------------------------------------------
void EspINA::addTaxonomyElement(QString qualifiedName)
{

}

//------------------------------------------------------------------------
void EspINA::removeTaxonomyElement ( const QModelIndex& index )
{
    IModelItem *item = static_cast<IModelItem *> ( index.internalPointer() );
    Q_ASSERT ( item->type() == IModelItem::TAXONOMY );
    TaxonomyNode *node = dynamic_cast<TaxonomyNode*> ( item );
    beginRemoveRows ( index.parent(),index.row(),index.row() );
    node->parentNode()->removeChild ( node->name() );
    endRemoveRows();
}

//------------------------------------------------------------------------
void EspINA::removeTaxonomyElement(QString qualifiedName)
{
  bool deprecated = false;
  Q_ASSERT(deprecated);
}

//------------------------------------------------------------------------
QModelIndex EspINA::sampleRoot() const
{
    return createIndex (1, 0, 1);
}

//------------------------------------------------------------------------
QModelIndex EspINA::sampleIndex (Sample* sample) const
{
  int row = m_samples.indexOf (sample);
  IModelItem *internalPtr = sample;
  return createIndex (row, 0, internalPtr);
}

//------------------------------------------------------------------------
QModelIndex EspINA::channelRoot() const
{
  return createIndex (2, 0, 2);
}

//------------------------------------------------------------------------
QModelIndex EspINA::channelIndex(Channel* channel) const
{
    int row = m_channels.indexOf (channel);
    IModelItem *internalPtr = channel;
    return createIndex (row, 0, internalPtr);
}


//------------------------------------------------------------------------
QModelIndex EspINA::segmentationRoot() const
{
    return createIndex (3, 0, 3);
}

//------------------------------------------------------------------------
QModelIndex EspINA::segmentationIndex(Segmentation* seg) const
{
  int row = m_segmentations.indexOf(seg);
  IModelItem *internalPtr = seg;
  return createIndex(row, 0, internalPtr);
}

// //------------------------------------------------------------------------
// bool EspINA::isLeaf(TaxonomyNode* node) const
// {
//   return node->subElements().size() == 0;
// }
//
//------------------------------------------------------------------------
QModelIndex EspINA::taxonomyIndex ( TaxonomyNode* node ) const
{
    // We avoid setting the Taxonomy descriptor as parent of an index
    if ( !m_tax || node->name() == m_tax->name() )
        return taxonomyRoot();

    TaxonomyNode *parentNode = node->parentNode();

    if ( !parentNode )
        qDebug() << "Child" << node->qualifiedName() << "without parent";

    Q_ASSERT ( parentNode );
    int row = parentNode->subElements().indexOf ( node );
    IModelItem *internalPtr = node;
    return createIndex ( row,0,internalPtr );
}

// //------------------------------------------------------------------------
// void EspINA::addTaxonomy(QString name, QString parentName)
// {
//   QModelIndex parentIndex = taxonomyIndex(m_tax->element(parentName));
//   int lastRow = rowCount(parentIndex);
//   beginInsertRows(parentIndex, lastRow, lastRow);
//   m_tax->addElement(name, parentName);
//   endInsertRows();
// }
//
// //------------------------------------------------------------------------
// void EspINA::removeTaxonomy(QString name)
// {
//   TaxonomyNode *toRemove =  m_tax->element(name);
//   if (toRemove)
//   {
//     if (m_taxonomySegs[toRemove].size() == 0 && toRemove->subElements().size() == 0)
//     {
//       QModelIndex removeIndex = taxonomyIndex(toRemove);
//       int row  = removeIndex.row();
//        beginRemoveRows(removeIndex.parent(),row,row);
//        m_taxonomySegs.remove(toRemove);
//        m_tax->removeElement(toRemove->qualifiedName());
//        endRemoveRows();
//     }else{
//       QMessageBox box;
//       box.setText("Unable to remove other taxonomies/segmentations are using it.");
//       box.exec();
//     }
//   }
// }
//
// //------------------------------------------------------------------------
// TaxonomyNode* EspINA::taxonomyParent(TaxonomyNode* node)
// {
//   if( node )
//     return node->parentNode();
//   else
//     assert(false);//return m_tax;
// }
//
//
// //------------------------------------------------------------------------
// int EspINA::requestId(int suggestedId)
// {
// //   m_nextValidSegId--;
//   if (m_nextValidSegId <= suggestedId)
//     m_nextValidSegId = suggestedId+1;
//
//   return suggestedId;
// }
//
//
// //------------------------------------------------------------------------
// void EspINA::changeId(Segmentation* seg, int id)
// {
//   seg->setId(id);
//   emit dataChanged(segmentationIndex(seg),segmentationIndex(seg));
// }
//
//
// //-----------------------------------------------------------------------------
// Segmentation* EspINA::segmentation(QString& segId)
// {
//   foreach(Segmentation* realSeg, m_segmentations)
//   {
//     if( realSeg->id() == segId )
//       return realSeg;
//   }
//   assert(false);
// }
//
//
// //------------------------------------------------------------------------
// QList<Segmentation * > EspINA::segmentations(const TaxonomyNode* taxonomy, bool recursive) const
// {
//   // Get all segmentations that belong to taxonomy
//   QList<Segmentation *> segs;
//
//   segs.append(m_taxonomySegs[taxonomy]);
//
//   if (recursive)
//   {
//     // Get all segmentations that belong to taxonomy's children
//     TaxonomyNode *child;
//     foreach(child,taxonomy->subElements())
//     {
//       segs.append(segmentations(child,recursive));
//     }
//   }
//   return segs;
// }
//
// //! Returns all the segmentations of a given sample //TODO: depreacte?
// QList< Segmentation* > EspINA::segmentations(const Sample* sample) const
// {
//   return sample->segmentations();
// }
//
// //-----------------------------------------------------------------------------
// void EspINA::changeTaxonomy(Segmentation* seg, TaxonomyNode* newTaxonomy)
// {
//   if (seg->taxonomy() == newTaxonomy)
//     return;
//
//   assert(m_taxonomySegs[seg->taxonomy()].contains(seg));
//   m_taxonomySegs[seg->taxonomy()].removeOne(seg);
//   seg->setTaxonomy(newTaxonomy);
//   m_taxonomySegs[newTaxonomy].push_back(seg);
//
//   QModelIndex segIndex = segmentationIndex(seg);
//   emit dataChanged(segIndex,segIndex);
//   seg->origin()->representation(LabelMapExtension::SampleRepresentation::ID)->requestUpdate(true);
// }
//
// //------------------------------------------------------------------------
// void EspINA::changeTaxonomy(Segmentation* seg, const QString& taxName)
// {
//   TaxonomyNode* newTax = m_tax->element(taxName);
//
//   changeTaxonomy(seg, newTax);
// }
//
//
// //-----------------------------------------------------------------------------
// void EspINA::loadFile(QString filePath, QString method)
// {
//   // GUI -> Remote opens
//   /*QString TraceContent, TaxonomyContent;
//   QTextStream TraceStream(&TraceContent), TaxonomyStream(&TaxonomyContent);*/
//   if( method == "open")
//     this->clear();
//     // Remote files are loaded through paraview loadSource class
//
//   pqPipelineSource* remoteFile = pqLoadDataReaction::loadData(QStringList(filePath));
//   loadSource(remoteFile);
// }
//
// //-----------------------------------------------------------------------------
// // PRE: m_tax and m_analysis must be pointers to correct data
// void EspINA::saveFile(QString& filePath, pqServer* server)
// {
//   if( !m_tax or !m_analysis )
//   {
//     qDebug() << "EspINA: Error taxonomy or analysis are NULL. Save aborted";
//     return;
//   }
//
//   // Retrive ProcessingTrace
//   std::ostringstream trace_data;
//   m_analysis->print( trace_data );
//   // Retrive Taxonomy
//   QString tax_data;
//   IOTaxonomy::writeXMLTaxonomy(m_tax, tax_data);
//
//   if( server )
//   {
//     // Method to store remote files
//     pqPipelineSource* remoteWriter =
//       pqApplicationCore::instance()->getObjectBuilder()->
//       createFilter("filters", "segFileWriter",
//                    QMap<QString, QList< pqOutputPort*> >(),
//                    pqApplicationCore::instance()->getActiveServer() );
//     // Set the file name
//     vtkSMStringVectorProperty* fileNameProp =
//           vtkSMStringVectorProperty::SafeDownCast(remoteWriter->getProxy()->GetProperty("FileName"));
//     fileNameProp->SetElement(0, filePath.toStdString().c_str());
//     // Set Trace
//     vtkSMStringVectorProperty* traceProp =
//           vtkSMStringVectorProperty::SafeDownCast(remoteWriter->getProxy()->GetProperty("Trace"));
//     traceProp->SetElement(0, trace_data.str().c_str());
//     // Set Taxonomy
//     vtkSMStringVectorProperty* taxProp =
//           vtkSMStringVectorProperty::SafeDownCast(remoteWriter->getProxy()->GetProperty("Taxonomy"));
//     taxProp->SetElement(0, tax_data.toStdString().c_str());
//
//      // Save the segmentations in different files
//     filePath.remove(QRegExp("\\..*$"));
//     foreach(Segmentation* seg, m_segmentations)
//       this->saveSegmentation(seg, QDir(filePath)); // salva el fichero en el servidor
//
//     //Update the pipeline to obtain the content of the file
//     remoteWriter->getProxy()->UpdateVTKObjects();
//     remoteWriter->updatePipeline();
//     // Destroy de segFileWriter object
//     pqApplicationCore::instance()->getObjectBuilder()->destroy(remoteWriter);
//   }
//   else
//   {
//     assert(false);
//     /*
//     std::ofstream file( filePath.toStdString().c_str(), std::_S_trunc );
//     m_analysis->print(file);
//     */
//     QString auxTraceData(trace_data.str().c_str());
//     QStringList emptyList; //TODO include the segmentations
//     //IOEspinaFile::saveFile( filePath, auxTraceData, tax_data, emptyList);
//   }
// }
//
//
// //------------------------------------------------------------------------
// void EspINA::addSample(Sample *sample)
// {
//   sample->setVisible(false);
//   /* If this is used to load samples when using .trace files. The next line must be uncommented*/
//   // Tracing graph
//   m_analysis->addNode(sample);
//
//   int lastRow = rowCount(sampleRoot());
//   beginInsertRows(sampleRoot(),lastRow,lastRow);
//   m_activeSample = sample;
//   m_samples.push_back(sample);
//   sample->initialize();
//   endInsertRows();
//
//   emit focusSampleChanged(sample);
// }

// //------------------------------------------------------------------------
// void EspINA::removeSamples()
// {
//   foreach(Sample* sample, m_samples)
//   {
//     this->removeSample(sample);
//   }
//   assert(m_samples.size() == 0);
// //   assert(m_sampleSegs.keys().size() == 0);
//   m_activeSample = NULL;
// }
//
// //------------------------------------------------------------------------
// void EspINA::setUserDefindedTaxonomy(const QString& taxName)
// {
//   if (!m_tax)//WARNING:setUserDefindedTaxonomy with no m_tax
//     return;
//   m_newSegType = m_tax->element(taxName);
//   assert(m_newSegType);
// }
//
//
// void EspINA::onProxyCreated(pqProxy* p)
// {
// //   qDebug() << "EspINA: Proxy" << p->getSMGroup() << "::" << p->getSMName() << " created!";
// }
//
// void EspINA::destroyingProxy(pqProxy* p)
// {
// //   qDebug() << "EspINA: Proxy" << p->getSMGroup() << "::" << p->getSMName() << " is being destroyed!";
// }
// //------------------------------------------------------------------------
// void EspINA::loadSource(pqPipelineSource* proxy)
// {
//   //TODO Check the type of file .mha, .trace, or .seg
//   // .mha at the moment
//   pqApplicationCore* core = pqApplicationCore::instance();
//   QString filePath = core->serverResources().list().first().path();
//   //QString filePath = proxy->getSMName();
//
//   qDebug() << "EspINA: Loading file in server side: " << filePath << "  " << proxy->getSMName();
//
//   const QString extension = filePath.section('.',-1);
//
//   if( extension == "pvd" ||
//       extension == "mha" || extension == "mhd" ||
//       extension == "tif" || extension == "tiff" )
//   {
//     // TODO not supported for multiple Samples
//     //this->removeSamples();
//     QString sampleId = filePath.section('/', -1);
//     vtkFilter *sampleReader = CachedObjectBuilder::instance()->registerProductCreator(sampleId, proxy);
//     Sample *sample = EspINAFactory::instance()->CreateSample(sampleReader,0, filePath.section('/',0,-2));
//     this->addSample(sample);
//
//     if( !m_tax )
//     {
//       beginInsertRows(taxonomyRoot(), 0, 0);
//       loadTaxonomy();
//       endInsertRows();
//     }
//   }
//   else if( extension == "seg" )
//   {
//     //this->clear();
//     QFileInfo path(filePath.remove(QRegExp("\\..*$")));
//     Cache::instance()->setWorkingDirectory(path);
//     proxy->updatePipeline(); //Update the pipeline to obtain the content of the file
//     proxy->getProxy()->UpdatePropertyInformation();
//     // Taxonomy
//     vtkSMStringVectorProperty* TaxProp =
//           vtkSMStringVectorProperty::SafeDownCast(proxy->getProxy()->GetProperty("Taxonomy"));
//     //qDebug() << "Taxonomy:\n" << TaxProp->GetElement(0);
//     QString TaxContent(TaxProp->GetElement(0));
//     QTextStream tax;
//     tax.setString(&TaxContent);
//     // TODO Load Tax (try catch)
//     // Trace
//     vtkSMStringVectorProperty* TraceProp =
//           vtkSMStringVectorProperty::SafeDownCast(proxy->getProxy()->GetProperty("Trace"));
//     //qDebug() << "Trace:\n" << TraceProp->GetElement(0);
//     QString TraceContent(TraceProp->GetElement(0));
//     QTextStream trace;
//     trace.setString(&TraceContent);
//
//     try{
//       if (!m_tax)//TODO: Decide wether to mix, override or check compability
//       {
//         qDebug("EspINA: Reading taxonomy ...");
// // 	beginInsertRows(taxonomyRoot(), 0, 0);
// // 	m_tax = IOTaxonomy::loadXMLTaxonomy(TaxContent);
// // 	endInsertRows();
// // 	setUserDefindedTaxonomy(m_tax->subElements()[0]->getName());
// // 	emit resetTaxonomy();
// 	loadTaxonomy(IOTaxonomy::loadXMLTaxonomy(TaxContent));
//       }
//       qDebug("EspINA: Reading trace ...");
//       m_analysis->readTrace(trace);
//       // Remove the proxy of the .seg file
//       pqObjectBuilder* ob = pqApplicationCore::instance()->getObjectBuilder();
//       ob->destroy(proxy);
//
//     } catch (...) {
//       qDebug() << "Espina: Unable to load File. " << __FILE__ << __LINE__;
//     }
//   }
//   else
//   {
//     EspinaPluginManager::instance()->readFile(proxy,filePath);
// //     if (filePath.endsWith(".segmha"))
// //   {
// //     proxy->updatePipeline();
// //     SegmhaImporterFilter *segmhaImporter = new SegmhaImporterFilter(proxy,filePath.section('/', -1));
// //   else{
// //     qDebug() << QString("Error: %1 file not supported yet").arg(filePath.remove(0, filePath.lastIndexOf('.')));
//   }
// }
//
// //-----------------------------------------------------------------------------
// void EspINA::clear()
// {
//   SelectionManager::instance()->setVOI(NULL);
//   SelectionManager::instance()->setSelectionHandler(NULL,Qt::ArrowCursor);
//   // Delete Samples (and their segmentations)
//   this->removeSamples();
//
//   this->removeTaxonomy();
//
//   m_nextValidSegId = 1;
// }
//
//
// //------------------------------------------------------------------------
// void EspINA::internalSegmentationUpdate(Segmentation* seg)
// {
//   QModelIndex segIndex = segmentationIndex(seg);
//   emit dataChanged(segIndex,segIndex);
// }
//
//
// //------------------------------------------------------------------------
// void EspINA::removeTaxonomy()
// {
//   // Delete taxonomy
//   beginRemoveRows(taxonomyRoot(), 0, rowCount(taxonomyRoot())-1);
//   delete m_tax;
//   m_tax = NULL;
//   endRemoveRows();
// }
//
// //------------------------------------------------------------------------
// void EspINA::loadTaxonomy()
// {
//   if( QFile::exists(DEFAULT_TAXONOMY_PATH) )
//   {
//     m_tax = IOTaxonomy::openXMLTaxonomy(DEFAULT_TAXONOMY_PATH);
//   }
//   else
//   {
//     qDebug() << "EspINA: Default taxonomy file not founded at" << DEFAULT_TAXONOMY_PATH;
//     m_tax = new Taxonomy("Unclassified");
//     TaxonomyNode *node = m_tax->addElement("Unknown");
//     node->setColor(QColor(Qt::black));
//   }
//   setUserDefindedTaxonomy(m_tax->elements()[0]->qualifiedName());
//
//   emit resetTaxonomy();
// }
//
// //-----------------------------------------------------------------------------
// bool EspINA::saveSegmentation ( Segmentation* seg, QDir prefixFilePath )
// {
//   QString tmpfilePath(seg->creator()->id() + ".pvd");
//   tmpfilePath = prefixFilePath.filePath(tmpfilePath);
//   pqActiveObjects::instance().setActivePort(seg->outputPort());
//
//   qDebug() << "EspINA::saveSegementation" << tmpfilePath;
//   return EspinaSaveDataReaction::saveActiveData(tmpfilePath);
// }
