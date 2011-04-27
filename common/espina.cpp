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
//Espina
#include <data/taxonomy.h>
#include "products.h"
#include "cache/cachedObjectBuilder.h"

#include <vtkSMStringVectorProperty.h>
#include <vtkSMProxy.h>

#include <pqApplicationCore.h>
#include <pqServerResources.h>
#include <pqLoadDataReaction.h>
#include <pqObjectBuilder.h>

#include <QDebug>
#include <iostream>
#include <fstream>

#include <zip.h>
#include "FilePack.h"

class IOTaxonomy;


//------------------------------------------------------------------------
EspINA *EspINA::m_singleton(NULL);

//------------------------------------------------------------------------
EspINA* EspINA::instance()
{
  if (!m_singleton)
    m_singleton = new EspINA();
  
  return m_singleton;
}

void EspINA::onProxyCreated(pqProxy* p)
{
  qDebug() << "EspINA: Proxy" << p->getSMGroup() << "::" << p->getSMName() << " created!";
}

//------------------------------------------------------------------------
EspINA::~EspINA()
{
  
}


QVariant EspINA::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();
  
  if (index.internalId() == 1)
  {
    if (role == Qt::DisplayRole)
      return "Samples";
    else
      return QVariant();
  }

  if (index.internalId() == 2)
  {
    if (role == Qt::DisplayRole)
      return "Segmentations";
    else
      return QVariant();
  }
  
  IModelItem *indexItem = static_cast<IModelItem *>(index.internalPointer());
  return indexItem->data(role);
}

//------------------------------------------------------------------------
bool EspINA::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (index.isValid())
  {
    if (role == Qt::EditRole)
    {
      IModelItem *item = static_cast<IModelItem *>(index.internalPointer());
      Segmentation *seg = dynamic_cast<Segmentation *>(item);
      if (seg)
      {
	seg->name = value.toString();
      }
      emit dataChanged(index, index);
      return true;
    }
    if (role == Qt::DecorationRole)
      qDebug() << "Cambiando valor";
    if (role == Qt::CheckStateRole)
    {
      IModelItem *item = static_cast<IModelItem *>(index.internalPointer());
      Segmentation *seg = dynamic_cast<Segmentation *>(item);
      if (seg)
      {
	seg->setVisible(value.toBool());
      }
      emit dataChanged(index, index);
      return true;
    }
  }
  return false;
}

//------------------------------------------------------------------------
int EspINA::columnCount(const QModelIndex& parent) const
{
  return 1;
}

//------------------------------------------------------------------------
int EspINA::rowCount(const QModelIndex& parent) const
{
  if (!parent.isValid())
    return 3;
  
  if (parent.internalPointer() == taxonomyRoot().internalPointer())
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
    //std::cout << "Getting rows in source of " << taxItem->getName().toStdString() << std::endl;
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
  
  if ( child.internalPointer() == taxonomyRoot().internalPointer() 
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
    return taxonomyIndex(parentNode);
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
  if (parent.internalPointer() == sampleRoot().internalPointer())
  {
    assert(row < m_samples.size());
    internalPtr = m_samples[row];
  }
  else
  {
    // Checks if parent is Segmentation's root
    if (parent.internalPointer() == segmentationRoot().internalPointer())
    {
      assert(row < m_segmentations.size());
      internalPtr = m_segmentations[row];
    }
    else
    {
      // Neither Samples nor Segmentations have children
      IModelItem *parentItem = static_cast<IModelItem *>(parent.internalPointer());
      TaxonomyNode *taxItem = dynamic_cast<TaxonomyNode *>(parentItem);
      assert(taxItem);
      int subTaxonomies = numOfSubTaxonomies(taxItem);
      assert(row < subTaxonomies);
      internalPtr = taxItem->getSubElements()[row];
    }
  }
  return createIndex(row,column,internalPtr);
}

//------------------------------------------------------------------------
int EspINA::numOfSubTaxonomies(TaxonomyNode* tax) const
{
  return tax->getSubElements().size();
}

//------------------------------------------------------------------------
int EspINA::numOfSegmentations(TaxonomyNode* tax) const
{
  return segmentations(tax).size();
}


//------------------------------------------------------------------------
QVariant EspINA::headerData(int section, Qt::Orientation orientation, int role) const
{
  return QVariant();
}

//------------------------------------------------------------------------
Qt::ItemFlags EspINA::flags(const QModelIndex& index) const
{
  if (!index.isValid())
    return Qt::ItemIsEnabled;
  
  if (index == taxonomyRoot() || index == sampleRoot() || index == segmentationRoot())
    return Qt::ItemIsEnabled;
  
  // Segmentation are read-only (TODO: Allow editing extent/spacing)
  if (index.parent() == sampleRoot())
    return Qt::ItemIsEnabled;
  
  return QAbstractItemModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
}

 //------------------------------------------------------------------------

 //------------------------------------------------------------------------
QModelIndex EspINA::taxonomyRoot() const
{

  return createIndex(0,0,m_tax);
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
  if (node->getName() == m_tax->getName())
    return taxonomyRoot();
  
  TaxonomyNode *parentNode = m_tax->getParent(node->getName());
  assert(parentNode);
  int row = parentNode->getSubElements().indexOf(node);
  IModelItem *internalPtr = node;
  return createIndex(row,0,internalPtr);
}

//------------------------------------------------------------------------
QModelIndex EspINA::segmentationIndex(Segmentation* seg) const
{
  IModelItem *internalPtr = seg;
  return createIndex(m_segmentations.indexOf(seg),0,internalPtr);
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

//! Returns all the segmentations of a given sample
QList< Segmentation* > EspINA::segmentations(const Sample* sample) const
{
  return m_sampleSegs[sample];
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

  if( filePath.endsWith(".pvd") || filePath.endsWith(".mha"))
  {
    //qDebug() << "MHA FILE: " << filePath;
    //EspinaProxy* source = CachedObjectBuilder::instance()->createStack( filePath);
    CachedObjectBuilder::instance()->registerLoadedStack(filePath, proxy);
    this->addSample(proxy, 0, filePath);
  }
  else if( filePath.endsWith(".trace") ){

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
    qDebug() << "Error: .seg files not supported yet";
  else{
    qDebug() << QString("Error: %1 file not supported yet").arg(filePath.remove(0, filePath.lastIndexOf('.')));
  }
}


//-----------------------------------------------------------------------------
void EspINA::loadFile(QString& filePath, pqServer* server)
{
  //TODO cambiar el stream que usa porcessingTrace por QTextStream para homogeneizar 
  QTextStream stream;
  if( !m_samples.isEmpty() )
  {
    //m_activeSample;
//     foreach(Sample* sample, m_samples)
//     {
//       delete sample;//->sourceData();
//     }
      qDebug() << "Delete the other samples"; //TODO
  }
  if( server ) // Not used. Remote files are loaded through paraview loadSource class
  {
     pqPipelineSource* remoteFile = pqLoadDataReaction::loadData(QStringList(filePath));
     remoteFile->updatePipeline(); //Update the pipeline to obtain the content of the file
     remoteFile->getProxy()->UpdatePropertyInformation();

    vtkSMStringVectorProperty* contentProp =
          vtkSMStringVectorProperty::SafeDownCast(remoteFile->getProxy()->GetProperty("Content"));
    //qDebug() << "Content:\n" << StringProp2->GetElement(0);
    
    QString path(contentProp->GetElement(0));
    stream.setString(&path);
    m_analysis->readTrace(stream);
  }
  else // Read local file
  {
    /*
    QFile file(filePath);
    stream.setDevice(&file);
    */
    std::fstream traceContent(filePath.toStdString().c_str());
    if( traceContent.is_open() )
    {
      std::cout << "TraceContent: " << filePath.toStdString().c_str() << " Count" << traceContent.gcount() << std::endl;
      m_analysis->readTrace(traceContent);
    }
  }

}

//-----------------------------------------------------------------------------
void EspINA::saveFile(QString& filePath, pqServer* server)
{
  if( server ) 
  {
    // Method to store remote files
    pqPipelineSource* remoteWriter =
      pqApplicationCore::instance()->getObjectBuilder()->
      createFilter("filters", "remoteFileWriter",
                   QMap<QString, QList< pqOutputPort*> >(),
                   pqApplicationCore::instance()->getActiveServer() );

    vtkSMStringVectorProperty* fileNameProp =
          vtkSMStringVectorProperty::SafeDownCast(remoteWriter->getProxy()->GetProperty("FileName"));
    fileNameProp->SetElement(0, filePath.toStdString().c_str());
    vtkSMStringVectorProperty* contentProp =
          vtkSMStringVectorProperty::SafeDownCast(remoteWriter->getProxy()->GetProperty("Content"));

    std::stringstream content;
    m_analysis->print(content);
    contentProp->SetElement(0, content.str().c_str());
    //Update the pipeline to obtain the content of the file
    remoteWriter->getProxy()->UpdateVTKObjects();
    remoteWriter->updatePipeline();
  }
  else
  {
    /*
    std::ofstream file( filePath.toStdString().c_str(), std::_S_trunc );
    m_analysis->print(file);
    */
    FilePack pack( filePath );
    // Retrive ProcessingTrace
    std::ostringstream trace_data;
    m_analysis->print( trace_data );

    QString s(trace_data.str().c_str());
    pack.addTextSource("trace.dot", s);
    // Retrive Taxonomy
    if( m_tax )
    {
      QString tax_data;
      IOTaxonomy::writeXMLTaxonomy(m_tax, tax_data);
      pack.addTextSource("taxonomy.xml", tax_data);
    }
    pack.close();
    
  }
}


//-----------------------------------------------------------------------------
// void EspINA::saveTrace(QString& filePath)
// {
//   std::ofstream file( filePath.toStdString().c_str(), std::_S_trunc );
//   m_analysis->print(file);
// }


//------------------------------------------------------------------------
void EspINA::addSample(EspinaProxy* source, int portNumber, QString& filePath)
{
  Sample *sample = new Sample(source, portNumber, filePath);
  sample->setVisible(false);
  /* If this is used to load samples when using .trace files. The next line must be uncommented*/
  // Tracing graph
  m_analysis->addNode(sample);
  
  int lastRow = rowCount(sampleRoot());
  beginInsertRows(sampleRoot(),lastRow,lastRow);
  m_activeSample = sample;
  m_samples.push_back(sample);
  endInsertRows();
  
  emit focusSampleChanged(sample);
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
  m_sampleSegs[seg->origin()].push_back(seg);
  qDebug() << "ORIGEN: " << seg->origin();
  m_segmentations.push_back(seg);
  endInsertRows();
}

//------------------------------------------------------------------------
void EspINA::removeSegmentation(Segmentation* seg)
{
  QModelIndex segIndex = segmentationIndex(seg);
  beginRemoveRows(segmentationRoot(),segIndex.row(),segIndex.row());
  m_segmentations.removeOne(seg);
  m_taxonomySegs[seg->taxonomy()].removeOne(seg);
  m_sampleSegs[seg->origin()].removeOne(seg);
  endRemoveRows();
}


//------------------------------------------------------------------------
void EspINA::setUserDefindedTaxonomy(const QString& taxName)
{
  m_newSegType = m_tax->getComponent(taxName);
  assert(m_newSegType);
}


//------------------------------------------------------------------------
EspINA::EspINA(QObject* parent)
: QAbstractItemModel(parent)
, m_activeSample(NULL)
{
  loadTaxonomy();
  m_newSegType = m_tax->getComponent("Symetric");
  m_analysis = ProcessingTrace::instance();
}

//------------------------------------------------------------------------
void EspINA::loadTaxonomy()
{
  m_tax = new TaxonomyNode("FEM");
  TaxonomyNode *newNode;
  newNode = m_tax->addElement("Synapse","FEM");
  newNode->setColor(QColor(255,0,0));
  m_tax->addElement("Vesicles","FEM");
  m_tax->addElement("Symetric","Synapse");
  newNode = m_tax->addElement("Asymetric","Synapse");
  newNode->setColor(QColor(Qt::yellow));
  /* // DEBUG
  m_tax->addElement("A","Vesicles");
  m_tax->addElement("B","Vesicles");
  m_tax->addElement("B1","B");
  m_tax->addElement("B2","B");
  m_tax->addElement("B21","B2");
  m_tax->addElement("B22","B2");
  m_tax->addElement("B23","B2");
  m_tax->addElement("B24","B2");
  m_tax->addElement("B3","B");
  m_tax->addElement("C","Vesicles");
  m_tax->print();
  */
}

