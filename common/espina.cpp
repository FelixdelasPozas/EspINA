/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>
 * 
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 * 
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 * 
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "espina.h"

// Debug
#include "espina_debug.h"

//Espina
#include "Config.h"
#include <data/taxonomy.h>
#include "sample.h"
#include "segmentation.h"
#include "cache/cachedObjectBuilder.h"

#include <vtkSMStringVectorProperty.h>
#include <vtkSMProxy.h>

#include <pqApplicationCore.h>
#include <pqServerResources.h>
#include <pqLoadDataReaction.h>
#include <pqObjectBuilder.h>
#include <pqActiveObjects.h>
//#include <pqSaveDataReaction.h>
#include <EspinaSaveDataReaction.h>

#include <fstream>

#include "FilePack.h"
#include <QMessageBox>
#include <qfile.h>
#include "espINAFactory.h"
#include <qfileinfo.h>
#include <qdir.h>
#include <vtkStringList.h>
#include "labelMapExtension.h"


class IOTaxonomy;

//! Internal Model Representation
//! TaxonomyRoot (0,0,0)
//!   taxonomyNode (first m_tax child) => (taxonomyParent,row,col) //NOTE: taxonomyParent can be either a TaxonomyNode or TaxonomyRoot
//!   ... 
//!   taxonomyNode (last m_tax child)
//!      ... other taxonomyNodes
//! SampleRoot (1,0,1)
//!   sample (sampleRoot,row,col)
//!   ... other samples
//! SegmentationRoot (1,0,1)
//!   segmentation (SegmentationRoot,row,col)
//!   ... other segmentations
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
EspINA::EspINA(QObject* parent)
: QAbstractItemModel(parent)
, m_activeSample(NULL)
{
  loadTaxonomy();
  m_newSegType = NULL;//->getComponent("Symetric");
  m_analysis = ProcessingTrace::instance();
}

//------------------------------------------------------------------------
EspINA::~EspINA()
{
  
}

//-----------------------------------------------------------------------------
QVariant EspINA::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();
  
  // Taxonomys' Root
    if (index.internalId() == taxonomyRoot().internalId())
    {
      if (role == Qt::DisplayRole)
	return "Taxonomy";
      else
	return QVariant();
    }
    // Samples' Root
    if (index.internalId() == sampleRoot().internalId())
    {
      if (role == Qt::DisplayRole)
	return "Samples";
      else
	return QVariant();
    }
    
    // Segmentations' Root
    if (index.internalId() == segmentationRoot().internalId())
    {
      if (role == Qt::DisplayRole)
	return "Segmentations";
      else
	return QVariant();
    }
    
    IModelItem *indexItem = static_cast<IModelItem *>(index.internalPointer());
    // Only segmentations have multiple info
    if (index.column() > 0)
    {
      if (role != Qt::DisplayRole)
	return QVariant();
      
      Segmentation *seg = dynamic_cast<Segmentation *>(indexItem);
      assert(seg);
      QStringList availableInfo = seg->availableInformations();
      return seg->information(availableInfo[index.column()]);
    }
    
    // Other elements can display their own data
    return indexItem->data(role);
}

//------------------------------------------------------------------------
bool EspINA::setData(const QModelIndex& index, const QVariant& value, int role)
{
  bool result = false;
  if (index.isValid() && index.parent().isValid())// Root indexes cannot be modified
  {
    // Other elements can set their own data
    IModelItem *indexItem = static_cast<IModelItem *>(index.internalPointer());
    result = indexItem->setData(value, role);
    if (result)
    {
      if (role == Qt::CheckStateRole)
      {
	Segmentation *seg = dynamic_cast<Segmentation *>(indexItem);
	if (seg)
	{
	  seg->origin()->representation(LabelMapExtension::SampleRepresentation::ID)->requestUpdate(true);
	  QModelIndex segIndex = segmentationIndex(seg);
	  emit dataChanged(segIndex,segIndex);
	}
	  
      }
      TaxonomyNode *taxItem = dynamic_cast<TaxonomyNode *>(indexItem);
      if (taxItem && role == Qt::DecorationRole)
      {
	foreach(Segmentation *seg, m_taxonomySegs[taxItem])
	{
	  QModelIndex segIndex = segmentationIndex(seg);
	  //seg->setData(value, role);
	  emit dataChanged(segIndex,segIndex);
	}
	if (m_taxonomySegs[taxItem].size())
	{
	  m_taxonomySegs[taxItem].first()->origin()->representation(LabelMapExtension::SampleRepresentation::ID)->requestUpdate(true);
	}
      }
      emit dataChanged(index,index);
    }
  }
  return result;
}

//------------------------------------------------------------------------
int EspINA::columnCount(const QModelIndex& parent) const
{
  if (parent == segmentationRoot())
  {
    int infoSize = EspINAFactory::instance()->segmentationAvailableInformations().size();
    return infoSize;
  }
  else
    return 1;
}

//------------------------------------------------------------------------
int EspINA::rowCount(const QModelIndex& parent) const
{
  // There are 3 root indexes
  if (!parent.isValid())
    return 3;
  
  if (parent.internalId() == taxonomyRoot().internalId())
    return numOfSubTaxonomies(m_tax);
  
  if (parent.internalId() == sampleRoot().internalId())
    return m_samples.size();
  
  if (parent.internalId() == segmentationRoot().internalId())
    return m_segmentations.size();
  
  // Cast to base type 
  IModelItem *parentItem = static_cast<IModelItem *>(parent.internalPointer());
  // Check if Taxonomy Item
  TaxonomyNode *taxItem = dynamic_cast<TaxonomyNode *>(parentItem);
  if (taxItem)
  {
    //qDebug() << taxItem->getName() <<"Number of subtaxonomies" << numOfSubTaxonomies(taxItem);
    return numOfSubTaxonomies(taxItem);// + numOfSegmentations(taxItem);
  }
  // Otherwise Samples and Segmentations have no children
  return 0;
}

//------------------------------------------------------------------------
QModelIndex EspINA::parent(const QModelIndex& child) const
{
  if (!child.isValid())
    return QModelIndex();
  
  if ( child.internalId() == taxonomyRoot().internalId()
    || child.internalId() == sampleRoot().internalId() 
    || child.internalId() == segmentationRoot().internalId())
    return QModelIndex();
  
  IModelItem *childItem = static_cast<IModelItem *>(child.internalPointer());
  assert (childItem);
  
  // Checks if Taxonomy
  TaxonomyNode *childNode = dynamic_cast<TaxonomyNode *>(childItem);
  if (childNode)
  {
    TaxonomyNode *parentNode = m_tax->getParent(childNode->getName());
    return taxonomyIndex(parentNode);//NOTE: It's ok with TaxonomyRoot
  }
  // Checks if Sample
  Sample *parentSample = dynamic_cast<Sample *>(childItem);
  if (parentSample)
    return sampleRoot();
  
  // Otherwise is a segmentation
  Segmentation *childProduct = dynamic_cast<Segmentation *>(childItem);
  if (childProduct)
    return segmentationRoot();
  
  assert(false);
  return QModelIndex();
}

//------------------------------------------------------------------------
//! Returned index is compossed by the row, column and an element).
QModelIndex EspINA::index(int row, int column, const QModelIndex& parent) const
{
  //if (!hasIndex(row,column,parent))
    //return QModelIndex();
  
  if (!parent.isValid())
  {
    assert(row<3);
    if (row == 0)
      return taxonomyRoot();
    if (row == 1)
      return sampleRoot();
    if (row == 2)
      return segmentationRoot();
  }
  
  IModelItem *internalPtr;
  
  // Checks if parent is Sample's root
  if (parent.internalId() == sampleRoot().internalId())
  {
    assert(row < m_samples.size());
    internalPtr = m_samples[row];
    return createIndex(row,column,internalPtr);
  }
  if (parent.internalId() == segmentationRoot().internalId())
  {
    if (row >= m_segmentations.size())//NOTE: Don't know why, but when removing taxonomy node, wrong row number is gotten
      return QModelIndex();
    internalPtr = m_segmentations[row];
    return createIndex(row,column,internalPtr);
  }
  TaxonomyNode *parentTax;
  if(parent.internalId() == taxonomyRoot().internalId())
    parentTax = m_tax;
  else
  {
    // Neither Samples nor Segmentations have children
    IModelItem *parentItem = static_cast<IModelItem *>(parent.internalPointer());
    parentTax = dynamic_cast<TaxonomyNode *>(parentItem);
  }
  assert(parentTax);//WARNING: Now m_tax can be NULL, but even in that situation, it shouldn't report any children
  int subTaxonomies = numOfSubTaxonomies(parentTax);
  if (row >= subTaxonomies)//NOTE: Don't know why, but when removing taxonomy node, wrong row number is gotten
	return QModelIndex();
  //assert(row < subaxonomies);
  internalPtr = parentTax->getSubElements()[row];
  return createIndex(row,column,internalPtr);
}

//------------------------------------------------------------------------
int EspINA::numOfSubTaxonomies(TaxonomyNode* tax) const
{
  if (tax)
    return tax->getSubElements().size();
  return 0;
}

//------------------------------------------------------------------------
int EspINA::numOfSegmentations(TaxonomyNode* tax) const
{
  return segmentations(tax).size();
}


//------------------------------------------------------------------------
QVariant EspINA::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section >= 0)
  {
    return EspINAFactory::instance()->segmentationAvailableInformations()[section];
  }
  return QVariant();
}

//------------------------------------------------------------------------
Qt::ItemFlags EspINA::flags(const QModelIndex& index) const
{
  if (!index.isValid())
    return Qt::ItemIsEnabled;
  
  if (index == taxonomyRoot() || index == sampleRoot() || index == segmentationRoot())
    return Qt::ItemIsEnabled;
  
  if (index.parent() == sampleRoot())
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
  
  return QAbstractItemModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
}

 //------------------------------------------------------------------------

 //------------------------------------------------------------------------
QModelIndex EspINA::taxonomyRoot() const
{
  return createIndex(0,0,0);
}

//------------------------------------------------------------------------
QModelIndex EspINA::sampleRoot() const
{
  return createIndex(1,0,1);
}

//------------------------------------------------------------------------
QModelIndex EspINA::segmentationRoot() const
{
  return createIndex(2,0,2);
}


QModelIndex EspINA::sampleIndex(Sample* sample) const
{
  // We avoid setting the Taxonomy descriptor as parent of an index
  int row = m_samples.indexOf(sample);
  IModelItem *internalPtr = sample;
  return createIndex(row,0,internalPtr);
}

//------------------------------------------------------------------------
bool EspINA::isLeaf(TaxonomyNode* node) const
{
  return node->getSubElements().size() == 0;
}

//------------------------------------------------------------------------
QModelIndex EspINA::taxonomyIndex(TaxonomyNode* node) const
{
  // We avoid setting the Taxonomy descriptor as parent of an index
  if (!m_tax || node->getName() == m_tax->getName())
    return taxonomyRoot();
  
  TaxonomyNode *parentNode = m_tax->getParent(node->getName());
  if (!parentNode)
    qDebug() << "Child" << node->getName() << "without parent";
  assert(parentNode);
  int row = parentNode->getSubElements().indexOf(node);
  IModelItem *internalPtr = node;
  return createIndex(row,0,internalPtr);
}

//------------------------------------------------------------------------
void EspINA::addTaxonomy(QString name, QString parentName)
{
  QModelIndex parentIndex = taxonomyIndex(m_tax->getComponent(parentName));
  int lastRow = rowCount(parentIndex);
  beginInsertRows(parentIndex, lastRow, lastRow);
  m_tax->addElement(name, parentName);
  endInsertRows();
}

//------------------------------------------------------------------------
void EspINA::removeTaxonomy(QString name)
{
  TaxonomyNode *toRemove =  m_tax->getComponent(name);
  if (toRemove)
  {
    if (m_taxonomySegs[toRemove].size() == 0 && toRemove->getSubElements().size() == 0)
    {
      QModelIndex removeIndex = taxonomyIndex(toRemove);
      int row  = removeIndex.row();
       beginRemoveRows(removeIndex.parent(),row,row);
       m_taxonomySegs.remove(toRemove);
       m_tax->removeElement(toRemove->getName());
       endRemoveRows();
    }else{
      QMessageBox box;
      box.setText("Unable to remove other taxonomies/segmentations are using it.");
      box.exec();
    }
  }
}

//------------------------------------------------------------------------
TaxonomyNode* EspINA::taxonomyParent(TaxonomyNode* node)
{
  if( node )
    return m_tax->getParent(node->getName());
  else
    return m_tax;
}


//------------------------------------------------------------------------
QModelIndex EspINA::segmentationIndex(Segmentation* seg) const
{
  IModelItem *internalPtr = seg;
  return createIndex(m_segmentations.indexOf(seg),0,internalPtr);
}

//------------------------------------------------------------------------
void EspINA::changeTaxonomy(Segmentation* seg, QString& taxName)
{
  // lcate the real segmentation pointer registered
  const TaxonomyNode* oldTax = seg->taxonomy();
  assert(m_taxonomySegs[oldTax].contains(seg));
  TaxonomyNode* newTax = m_tax->getComponent(taxName);
  assert(newTax);

  m_taxonomySegs[oldTax].removeOne(seg);
  seg->setTaxonomy(newTax);
  m_taxonomySegs[newTax].push_back(seg);
  emit dataChanged(segmentationIndex(seg),segmentationIndex(seg));
}

//-----------------------------------------------------------------------------
Segmentation* EspINA::segmentation(QString& segId)
{
  foreach(Segmentation* realSeg, m_segmentations)
  {
    if( realSeg->id() == segId )
      return realSeg;
  }
  assert(false);     
}


//------------------------------------------------------------------------
QList<Segmentation * > EspINA::segmentations(const TaxonomyNode* taxonomy, bool recursive) const
{
  // Get all segmentations that belong to taxonomy
  QList<Segmentation *> segs;
  
  segs.append(m_taxonomySegs[taxonomy]);
  
  if (recursive)
  {
    // Get all segmentations that belong to taxonomy's children
    TaxonomyNode *child;
    foreach(child,taxonomy->getSubElements())
    {
      segs.append(segmentations(child,recursive));
    }
  }
  return segs;
}

//! Returns all the segmentations of a given sample //TODO: depreacte?
QList< Segmentation* > EspINA::segmentations(const Sample* sample) const
{
  return sample->segmentations();
}

//-----------------------------------------------------------------------------
void EspINA::loadFile(QString filePath, QString method)
{
  // GUI -> Remote opens
  /*QString TraceContent, TaxonomyContent;
  QTextStream TraceStream(&TraceContent), TaxonomyStream(&TaxonomyContent);*/
  if( method == "open")
    this->clear();
    // Remote files are loaded through paraview loadSource class

  pqPipelineSource* remoteFile = pqLoadDataReaction::loadData(QStringList(filePath));
  loadSource(remoteFile);
}

//-----------------------------------------------------------------------------
// PRE: m_tax and m_analysis must be pointers to correct data
void EspINA::saveFile(QString& filePath, pqServer* server)
{
  if( !m_tax or !m_analysis )
  {
    qDebug() << "EspINA: Error taxonomy or analysis are NULL. Save aborted";
    return;
  }

  // Retrive ProcessingTrace
  std::ostringstream trace_data;
  m_analysis->print( trace_data );
  // Retrive Taxonomy
  QString tax_data;
  IOTaxonomy::writeXMLTaxonomy(m_tax, tax_data);
  
  if( server ) 
  {
    // Method to store remote files
    pqPipelineSource* remoteWriter =
      pqApplicationCore::instance()->getObjectBuilder()->
      createFilter("filters", "segFileWriter",
                   QMap<QString, QList< pqOutputPort*> >(),
                   pqApplicationCore::instance()->getActiveServer() );
    // Set the file name
    vtkSMStringVectorProperty* fileNameProp =
          vtkSMStringVectorProperty::SafeDownCast(remoteWriter->getProxy()->GetProperty("FileName"));
    fileNameProp->SetElement(0, filePath.toStdString().c_str());
    // Set Trace
    vtkSMStringVectorProperty* traceProp =
          vtkSMStringVectorProperty::SafeDownCast(remoteWriter->getProxy()->GetProperty("Trace"));
    traceProp->SetElement(0, trace_data.str().c_str());
    // Set Taxonomy
    vtkSMStringVectorProperty* taxProp =
          vtkSMStringVectorProperty::SafeDownCast(remoteWriter->getProxy()->GetProperty("Taxonomy"));
    taxProp->SetElement(0, tax_data.toStdString().c_str());
    
     // Save the segmentations in different files
    filePath.remove(QRegExp("\\..*$"));
    foreach(Segmentation* seg, m_segmentations)
      this->saveSegmentation(seg, QDir(filePath)); // salva el fichero en el servidor
    
    //Update the pipeline to obtain the content of the file
    remoteWriter->getProxy()->UpdateVTKObjects();
    remoteWriter->updatePipeline();
    // Destroy de segFileWriter object
    pqApplicationCore::instance()->getObjectBuilder()->destroy(remoteWriter);
  }
  else
  {
    assert(false);
    /*
    std::ofstream file( filePath.toStdString().c_str(), std::_S_trunc );
    m_analysis->print(file);
    */
    QString auxTraceData(trace_data.str().c_str());
    QStringList emptyList; //TODO include the segmentations
    //IOEspinaFile::saveFile( filePath, auxTraceData, tax_data, emptyList);
  }
}


//------------------------------------------------------------------------
void EspINA::addSample(Sample *sample)
{
  sample->setVisible(false);
  /* If this is used to load samples when using .trace files. The next line must be uncommented*/
  // Tracing graph
  m_analysis->addNode(sample);
  
  int lastRow = rowCount(sampleRoot());
  beginInsertRows(sampleRoot(),lastRow,lastRow);
  m_activeSample = sample;
  m_samples.push_back(sample);
  sample->initialize();
  endInsertRows();
  
  emit focusSampleChanged(sample);
}

//------------------------------------------------------------------------
void EspINA::removeSample(Sample* sample)
{
  if( m_samples.contains(sample) )
  {
    // Remove the Segmentations associated
    dynamic_cast<LabelMapExtension::SampleRepresentation *>(sample->representation(LabelMapExtension::SampleRepresentation::ID))->setEnable(false);
    foreach(Segmentation* seg, sample->segmentations())
    {
      removeSegmentation(seg);
    }
    dynamic_cast<LabelMapExtension::SampleRepresentation *>(sample->representation(LabelMapExtension::SampleRepresentation::ID))->setEnable(true);
//     assert(m_sampleSegs[sample].size() == 0);
//     m_sampleSegs.remove(sample);
//     assert(!m_sampleSegs.contains(sample));
    // Remove it from analysis
    QModelIndex index = sampleIndex(sample);
    beginRemoveRows(index.parent(), index.row(), index.row());
    m_samples.removeOne(sample);
    assert(!m_samples.contains(sample));
    m_analysis->removeNode(sample);
    endRemoveRows();
  }
}

//------------------------------------------------------------------------
void EspINA::removeSamples()
{
  foreach(Sample* sample, m_samples)
  {
    this->removeSample(sample);
  }
  assert(m_samples.size() == 0);
//   assert(m_sampleSegs.keys().size() == 0);
  m_activeSample = NULL;
}

//------------------------------------------------------------------------
void EspINA::addSegmentation(Segmentation *seg)
{
  TaxonomyNode *node = m_newSegType;
  
  // We need to notify other components that the model has changed
  int lastRow = rowCount(segmentationRoot());
  
  //beginResetModel();
  beginInsertRows(segmentationRoot(),lastRow,lastRow);
  seg->setTaxonomy(node);
  seg->setOrigin(m_activeSample);
  seg->initialize();
  m_taxonomySegs[m_newSegType].push_back(seg);
  seg->origin()->addSegmentation(seg);
  connect(seg, SIGNAL(updated(Segmentation*)), this, SLOT(internalSegmentationUpdate(Segmentation*)));
  //m_sampleSegs[seg->origin()].push_back(seg);
  m_segmentations.push_back(seg);
  endInsertRows();
}

//------------------------------------------------------------------------
void EspINA::removeSegmentation(Segmentation* seg)
{
  // Update model
  QModelIndex segIndex = segmentationIndex(seg);
  beginRemoveRows(segmentationRoot(),segIndex.row(),segIndex.row());
  m_segmentations.removeOne(seg);
  m_taxonomySegs[seg->taxonomy()].removeOne(seg);
  seg->origin()->removeSegmentation(seg);
  //m_sampleSegs[seg->origin()].removeOne(seg);
  // Free internal memory
  m_analysis->removeNode(seg);
  endRemoveRows();
}


//------------------------------------------------------------------------
void EspINA::setUserDefindedTaxonomy(const QString& taxName)
{
  if (!m_tax)//WARNING:setUserDefindedTaxonomy with no m_tax
    return;
  m_newSegType = m_tax->getComponent(taxName);
  assert(m_newSegType);
}


void EspINA::onProxyCreated(pqProxy* p)
{
//   qDebug() << "EspINA: Proxy" << p->getSMGroup() << "::" << p->getSMName() << " created!";
}

void EspINA::destroyingProxy(pqProxy* p)
{
//   qDebug() << "EspINA: Proxy" << p->getSMGroup() << "::" << p->getSMName() << " is being destroyed!";
}
//------------------------------------------------------------------------
void EspINA::loadSource(pqPipelineSource* proxy)
{
  //TODO Check the type of file .mha, .trace, or .seg
  // .mha at the moment
  pqApplicationCore* core = pqApplicationCore::instance();
  QString filePath = core->serverResources().list().first().path();
  //QString filePath = proxy->getSMName();

  qDebug() << "EspINA: Loading file in server side: " << filePath << "  " << proxy->getSMName();

  if( filePath.endsWith(".pvd") || 
      filePath.endsWith(".mha") || 
      filePath.endsWith(".mhd") ||
      filePath.endsWith(".tif") ||
      filePath.endsWith(".tiff") )
    
  {
    // TODO not supported for multiple Smaples
    //this->removeSamples();
    
    vtkFilter *sampleReader = CachedObjectBuilder::instance()->registerProductCreator(filePath, proxy);
    Sample *sample = EspINAFactory::instance()->CreateSample(sampleReader,0);
    this->addSample(sample); 

    if( !m_tax )
    {
      beginInsertRows(taxonomyRoot(), 0, 0);
      loadTaxonomy();
      endInsertRows();
    }
  }
  else if( filePath.endsWith(".trace") ){ // DEPRECATED

    proxy->updatePipeline(); //Update the pipeline to obtain the content of the file
    proxy->getProxy()->UpdatePropertyInformation();

    vtkSMStringVectorProperty* filePathProp =
          vtkSMStringVectorProperty::SafeDownCast(proxy->getProxy()->GetProperty("Content"));
    qDebug() << "Content:\n" << filePathProp->GetElement(0);
    //std::istringstream trace(std::string(filePathProp->GetElement(0)));
    QString content(filePathProp->GetElement(0));
    QTextStream trace;
    trace.setString(&content);
    m_analysis->readTrace(trace);

  }
  else if( filePath.endsWith(".seg") )
  {
    //this->clear();
    QFileInfo path(filePath.remove(QRegExp("\\..*$")));
    Cache::instance()->setWorkingDirectory(path);
    proxy->updatePipeline(); //Update the pipeline to obtain the content of the file
    proxy->getProxy()->UpdatePropertyInformation();
    // Taxonomy
    vtkSMStringVectorProperty* TaxProp =
          vtkSMStringVectorProperty::SafeDownCast(proxy->getProxy()->GetProperty("Taxonomy"));
    //qDebug() << "Taxonomy:\n" << TaxProp->GetElement(0);
    QString TaxContent(TaxProp->GetElement(0));
    QTextStream tax;
    tax.setString(&TaxContent);
    // TODO Load Tax (try catch)
    // Trace
    vtkSMStringVectorProperty* TraceProp =
          vtkSMStringVectorProperty::SafeDownCast(proxy->getProxy()->GetProperty("Trace"));
    //qDebug() << "Trace:\n" << TraceProp->GetElement(0);
    QString TraceContent(TraceProp->GetElement(0));
    QTextStream trace;
    trace.setString(&TraceContent);

    try{
      if (!m_tax)//TODO: Decide wether to mix, override or check compability
      {
        qDebug("EspINA: Reading taxonomy ...");
	beginInsertRows(taxonomyRoot(), 0, 0);
	m_tax = IOTaxonomy::loadXMLTaxonomy(TaxContent);
	endInsertRows();
	setUserDefindedTaxonomy(m_tax->getSubElements()[0]->getName());
	emit resetTaxonomy();
      }
      qDebug("EspINA: Reading trace ...");
      m_analysis->readTrace(trace);
      // Remove the proxy of the .seg file
      pqObjectBuilder* ob = pqApplicationCore::instance()->getObjectBuilder();
      ob->destroy(proxy);
      
    } catch (...) {
      qDebug() << "Espina: Unable to load File. " << __FILE__ << __LINE__;
    }
  }
  else{
    qDebug() << QString("Error: %1 file not supported yet").arg(filePath.remove(0, filePath.lastIndexOf('.')));
  }
}

//-----------------------------------------------------------------------------
void EspINA::clear()
{
  SelectionManager::instance()->setVOI(NULL);
  SelectionManager::instance()->setSelectionHandler(NULL,Qt::ArrowCursor);
  // Delete Samples (and their segmentations)
  this->removeSamples();
  
  
  // Delete taxonomy
  beginRemoveRows(taxonomyRoot(), 0, rowCount(taxonomyRoot())-1);
  delete m_tax;
  m_tax = NULL;
  endRemoveRows();
}


//------------------------------------------------------------------------
void EspINA::internalSegmentationUpdate(Segmentation* seg)
{
  QModelIndex segIndex = segmentationIndex(seg);
  emit dataChanged(segIndex,segIndex);
}


//------------------------------------------------------------------------
void EspINA::loadTaxonomy()
{
  if( QFile::exists(DEFAULT_TAXONOMY_PATH) )
  {
    m_tax = IOTaxonomy::openXMLTaxonomy(DEFAULT_TAXONOMY_PATH);
  }
  else
  {
    qDebug() << "EspINA: Default taxonomy file not founded at" << DEFAULT_TAXONOMY_PATH;
    m_tax = new TaxonomyNode("Unclassified");
    TaxonomyNode *node = m_tax->addElement("Unknown", "Unclassified");
    node->setColor(QColor(Qt::black));
  }
  setUserDefindedTaxonomy(m_tax->getSubElements()[0]->getName());
  
  emit resetTaxonomy();
}

//-----------------------------------------------------------------------------
bool EspINA::saveSegmentation ( Segmentation* seg, QDir prefixFilePath )
{
  QString tmpfilePath(seg->creator()->id() + ".pvd");
  tmpfilePath = prefixFilePath.filePath(tmpfilePath);
  pqActiveObjects::instance().setActivePort(seg->outputPort());
  
  qDebug() << "EspINA::saveSegementation" << tmpfilePath;
  return EspinaSaveDataReaction::saveActiveData(tmpfilePath);
}
