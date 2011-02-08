#include "taxonomy.h"
#include <stack>

#include <QFile>
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

void TaxonomyNode::insertElement(QString subElement)
{
  if( !m_elements )
    m_elements = new std::vector<TaxonomyNode*>();
  m_elements->push_back( new TaxonomyNode(subElement) );
}

TaxonomyNode::~TaxonomyNode()
{
  if( m_elements )
    for(std::vector< TaxonomyNode* >::iterator it = m_elements->begin(); it < m_elements->end(); it++)
      delete (*it);
    
    delete( m_elements );
}

void TaxonomyNode::print(int level)
{
  std::cout << 
  std::string(level*2, ' ') <<
  m_name.toStdString() <<
  std::endl;

  if(m_elements){
    std::vector<TaxonomyNode *>::iterator it;
    for( it = m_elements->begin(); it < m_elements->end(); it++)
      (*it)->print(level+1);
  }
}

TaxonomyNode::TaxonomyNode(QString name):
  m_name(name), m_elements(NULL)
{
}

/*
void TaxonomyNode::addElement(QString subElement)
{
  assert( this->getComponent(subElement)==NULL);
  this->insertElement( subElement );
}
*/

TaxonomyNode* TaxonomyNode::addElement(QString subElement, QString supElement)
{

  //ASSERT( this->getComponent(subElement)==NULL);
  if( this->getComponent(subElement) )
    throw "RepeatedElementException"; //TODO change exception
  TaxonomyNode* supNode = this->getComponent(supElement);
  if( supNode )
    supNode->insertElement( subElement );
  else{
    std::cerr << "Error: " << supElement.toStdString() << " does not exist in the taxonomy" << std::endl;
    //exit(1);
  }
}


TaxonomyNode* TaxonomyNode::getComponent(QString name)
{
  TaxonomyNode* taxNode = NULL;
  if( m_name.compare(name) != 0 ){
    if (m_elements){ 
      for( std::vector< TaxonomyNode* >::iterator it = m_elements->begin(); it < m_elements->end(); it++){
	taxNode = (*it)->getComponent( name );
	if( taxNode )
	  break;
      }
    }
  }
  else
    return this;
  return taxNode;
}

// Returns a vector with the subelements of a node
std::vector< TaxonomyNode* >* TaxonomyNode::getSubElements()
{
  return m_elements;
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

TaxonomyNode* IOTaxonomy::openXMLTaxonomy(QString& fileName)
{
  QFile file( fileName );
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) 
  {
/*    QMessageBox::critical(this, "IOTaxonomy::openXMLTaxonomy", 
			  "Couldn't open the file", 
			  QMessageBox::Ok);*/
    qDebug() <<"File could not be oppended";
    return NULL;
  }
  QXmlStreamReader stream(&file);
  QStringRef nodeName;
  TaxonomyNode* tax;

  std::stack<QString> taxHierarchy;
  while(!stream.atEnd())
  {
    stream.readNextStartElement();
    if( stream.name() == "node")
    {
      if( stream.isStartElement() )
      {
	nodeName = stream.attributes().value("name");
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
      else if( stream.isEndElement() )
      {
	taxHierarchy.pop();
      }
    }
  }
  file.close();
  return tax;
}


void IOTaxonomy::writeTaxonomyNode(TaxonomyNode* node, QXmlStreamWriter& stream)
{
  if( node )
  {
    std::vector<TaxonomyNode*>* nodes;
    stream.writeStartElement( "node" );
    stream.writeAttribute("name", node->getName());
    nodes = node->getSubElements();
    if( nodes )
    {
	std::vector<TaxonomyNode*>::iterator itNodes;
	for( itNodes=nodes->begin(); itNodes < nodes->end(); itNodes++ )
	{
	  IOTaxonomy::writeTaxonomyNode( (*itNodes), stream );
	}
    }
    stream.writeEndElement();
  }
}


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

