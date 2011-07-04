#include "taxonomy.h"
#include <stack>

#include <QFile>
#include <QPixmap>
#include <QXmlStreamReader>

#include <iostream>
#include <QDebug>

//#include <assert.h>
/*
#define ASSERT(x) \
  if (! (x)) \
  { \
    std::cout << "ERROR: Assert " << #x << " failed\n"; \
    std::cout << " in " << __FILE__ << ":" << __LINE__  << "\n"; \
  } \
  assert(x)
*/

//------------------------------------------------------------------------
TaxonomyNode::TaxonomyNode(QString name, QString RGBColor)
: m_name(name)//, m_elements(NULL)
, m_description(name)
, m_color(RGBColor)
{
}

//------------------------------------------------------------------------
TaxonomyNode::~TaxonomyNode()
{
  TaxonomyNode *node;
  foreach(node,m_elements)
  {
    delete node;
  }
//   if( m_elements )
//     for(std::vector< TaxonomyNode* >::iterator it = m_elements->begin(); it < m_elements->end(); it++)
//       delete (*it);
//     
//     delete( m_elements );
}

//------------------------------------------------------------------------
void TaxonomyNode::print(int level)
{
  std::cout << 
  std::string(level*2, ' ') <<
  m_name.toStdString() <<
  std::endl;

  TaxonomyNode *node;
  foreach (node, m_elements)
  {
    node->print(level+1);
  }
//   if(m_elements){
//     std::vector<TaxonomyNode *>::iterator it;
//     for( it = m_elements->begin(); it < m_elements->end(); it++)
//       (*it)->print(level+1);
//   }
}


/*
void TaxonomyNode::addElement(QString subElement)
{
  assert( this->getComponent(subElement)==NULL);
  this->insertElement( subElement );
}
*/

//------------------------------------------------------------------------
TaxonomyNode* TaxonomyNode::addElement(QString subElement, QString supElement, QString RGBColor)
{
  //ASSERT( this->getComponent(subElement)==NULL);
  if( this->getComponent(subElement) )
    throw "RepeatedElementException"; //TODO change exception
  TaxonomyNode* supNode = this->getComponent(supElement);
  TaxonomyNode *newElement = NULL;
  if( supNode )
  {
    QString newColor = ((RGBColor == "") ? supNode->getColor().name() : RGBColor);
    newElement = supNode->insertElement( subElement, newColor );
    //newElement->setColor(supNode->getColor());
  }
  else{
    std::cerr << "Error: " << supElement.toStdString() << " does not exist in the taxonomy" << std::endl;
    //exit(1);
  }
  return newElement;
}

//------------------------------------------------------------------------
void TaxonomyNode::removeElement(QString subElement)
{
  TaxonomyNode *element;
  if( element = this->getComponent(subElement) )
  {
    TaxonomyNode *parent = this->getParent(subElement);
    int index = parent->m_elements.indexOf(element);
    parent->m_elements.remove(index);
  }
  else{
    std::cerr << "Error: " << subElement.toStdString() << " does not exist in the taxonomy" << std::endl;
  }
}


//------------------------------------------------------------------------
TaxonomyNode* TaxonomyNode::getParent(QString name)
{
  TaxonomyNode *parent = NULL;
  
  for (int i = 0; i < getSubElements().size(); i++)
  {
    if (getSubElements()[i]->getName() == name)
	parent = this;
    else
      parent = getSubElements()[i]->getParent(name);
    if (parent)
     return parent;
  }
  
  return parent;
}



//------------------------------------------------------------------------
TaxonomyNode* TaxonomyNode::getComponent(QString name)
{
  TaxonomyNode* taxNode = NULL;
  if (m_name == name)
    return this;
  
  TaxonomyNode *node;
  foreach(node,m_elements)
  {
    taxNode = node->getComponent(name);
    if (taxNode)
      break;
  }
    
//   if( m_name.compare(name) != 0 ){
//     if (m_elements){ 
//       for( std::vector< TaxonomyNode* >::iterator it = m_elements->begin(); it < m_elements->end(); it++){
// 	taxNode = (*it)->getComponent( name );
// 	if( taxNode )
// 	  break;
//       }
//     }
//   }
//   else
//     return this;
  return taxNode;
}

//------------------------------------------------------------------------

//------------------------------------------------------------------------
//! Returns a vector with the subelements of a node
QVector<TaxonomyNode *> TaxonomyNode::getSubElements() const
{
  return m_elements;
}

//------------------------------------------------------------------------
TaxonomyNode* TaxonomyNode::insertElement(QString subElement, QString RGBColor)
{
  //if( !m_elements )
  //  m_elements = new std::vector<TaxonomyNode*>();
  TaxonomyNode *newElement = new TaxonomyNode(subElement, RGBColor);
  
  m_elements.push_back(newElement);
  return newElement;
}

//------------------------------------------------------------------------
QVariant TaxonomyNode::data(int role) const
{
  switch (role)
  {
    case Qt::DisplayRole:
    case Qt::EditRole:
	return getName();
    case Qt::DecorationRole:
    {
      QPixmap icon(16,16);
      icon.fill(getColor());
      return icon;
    }
    default:
      return QVariant();
  }
}

//------------------------------------------------------------------------
bool TaxonomyNode::setData(const QVariant& value, int role)
{
  if (role == Qt::EditRole)
  {
    setName(value.toString());
    return true;
  }
  if (role == Qt::DecorationRole)
  {
    setColor(value.value<QColor>());
    return true;
  }
  return false;
}




/****************
 ** IOTaxonomy **
 ****************/

IOTaxonomy::IOTaxonomy()
{

}

IOTaxonomy::~IOTaxonomy()
{

}


TaxonomyNode* IOTaxonomy::readXML(QXmlStreamReader& xmlStream)
{
  // Read the XML
//   QXmlStreamReader xmlStream(&file);
  QStringRef nodeName, color;
  TaxonomyNode* tax;
  std::stack<QString> taxHierarchy;
  while(!xmlStream.atEnd())
  {
    xmlStream.readNextStartElement();
    if( xmlStream.name() == "node")
    {
      if( xmlStream.isStartElement() )
      {
        nodeName = xmlStream.attributes().value("name");
        color = xmlStream.attributes().value("color");
        if( taxHierarchy.empty() )
        {
          tax = new TaxonomyNode( nodeName.toString(), color.toString() );
        }
        else
        {
          tax->addElement( nodeName.toString(), taxHierarchy.top(), color.toString() );
        }
        taxHierarchy.push( nodeName.toString() );
      }
      else if( xmlStream.isEndElement() )
      {
        taxHierarchy.pop();
      }
    }
  }
  return tax;
}


TaxonomyNode* IOTaxonomy::openXMLTaxonomy(QString fileName)
{
  QFile file( fileName );
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
//    QMessageBox::critical(this, "IOTaxonomy::openXMLTaxonomy",
//               "Couldn't open the file",
//               QMessageBox::Ok);
    qDebug() <<"File could not be oppended";
    return NULL;
  }

  // Read the XML
  QXmlStreamReader xmlStream(&file);
  /*
  QStringRef nodeName;
  TaxonomyNode* tax;
  std::stack<QString> taxHierarchy;
  while(!xmlStream.atEnd())
  {
    xmlStream.readNextStartElement();
    if( xmlStream.name() == "node")
    {
      if( xmlStream.isStartElement() )
      {
        nodeName = xmlStream.attributes().value("name");
        if( taxHierarchy.empty() )
        {
          tax = new TaxonomyNode( nodeName.toString() );
        }
        else
        {
          tax->addElement( nodeName.toString(), taxHierarchy.top() );
        }
        taxHierarchy.push( nodeName.toString() );
      }
      else if( xmlStream.isEndElement() )
      {
        taxHierarchy.pop();
      }
    }
  }
  */
  TaxonomyNode* tax = readXML(xmlStream);
  file.close();
  return tax;
}

TaxonomyNode* IOTaxonomy::loadXMLTaxonomy(QString& content)
{

//   if( content.device() )
//     xmlStream.setDevice( content.device() );
//   else if( content.string() )
//     xmlStream = QXmlStreamReader(*content.string());
    
  // Read the XML
  QXmlStreamReader xmlStream(content);
  /*
  QStringRef nodeName;
  TaxonomyNode* tax;
  std::stack<QString> taxHierarchy;
  while(!xmlStream.atEnd())
  {
    xmlStream.readNextStartElement();
    if( xmlStream.name() == "node")
    {
      if( xmlStream.isStartElement() )
      {
        nodeName = xmlStream.attributes().value("name");
        if( taxHierarchy.empty() )
        {
          tax = new TaxonomyNode( nodeName.toString() );
        }
        else
        {
          tax->addElement( nodeName.toString(), taxHierarchy.top() );
        }
        taxHierarchy.push( nodeName.toString() );
      }
      else if( xmlStream.isEndElement() )
      {
        taxHierarchy.pop();
      }
    }
  }
  return tax;
  */
  return readXML(xmlStream);
}


void IOTaxonomy::writeTaxonomyNode(TaxonomyNode* node, QXmlStreamWriter& stream)
{
  if( node )
  {
    //QVector<TaxonomyNode*> nodes;
    stream.writeStartElement( "node" );
    stream.writeAttribute("name", node->getName());
    stream.writeAttribute("color", node->getColor().name());
    TaxonomyNode* subnode;
    foreach(subnode, node->getSubElements())
    {
      IOTaxonomy::writeTaxonomyNode(subnode, stream );
    }
//     nodes = node->getSubElements();
//     if( nodes )
//     {
// 	std::vector<TaxonomyNode*>::iterator itNodes;
// 	for( itNodes=nodes->begin(); itNodes < nodes->end(); itNodes++ )
// 	{
// 	  IOTaxonomy::writeTaxonomyNode( (*itNodes), stream );
// 	}
//     }
    stream.writeEndElement();
  }
}

/*
void IOTaxonomy::writeXMLTaxonomy(TaxonomyNode& tax, QString fileName)
{
  QFile fd (fileName);
  fd.open( QIODevice::WriteOnly | QIODevice::Truncate );
  QXmlStreamWriter stream(&fd);
  
  stream.setAutoFormatting(true);
  stream.writeStartDocument();
  stream.writeStartElement("Taxonomy");
  
  IOTaxonomy::writeTaxonomyNode( &tax, stream );
  
  stream.writeEndElement();
  stream.writeEndDocument();
  fd.close();
}
*/

//-----------------------------------------------------------------------------
void IOTaxonomy::writeXMLTaxonomy(TaxonomyNode* tax, QString& destination)
{
//   QFile fd (fileName);
//   fd.open( QIODevice::WriteOnly | QIODevice::Truncate );
  QXmlStreamWriter stream(&destination);

  stream.setAutoFormatting(true);
  stream.writeStartDocument();
  stream.writeStartElement("Taxonomy");

  IOTaxonomy::writeTaxonomyNode( tax, stream );

  stream.writeEndElement();
  stream.writeEndDocument();
  
  //fd.close();
}
