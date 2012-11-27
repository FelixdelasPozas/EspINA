#include "Core/Model/Taxonomy.h"

#include <stack>
#include <QFile>
#include <QPixmap>
#include <QXmlStreamReader>

#include <iostream>
#include <QDebug>
#include <assert.h>
#include <qpaintengine.h>

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

const QString TaxonomyElement::X_DIM = "Dim_X";
const QString TaxonomyElement::Y_DIM = "Dim_Y";
const QString TaxonomyElement::Z_DIM = "Dim_Z";

//------------------------------------------------------------------------
TaxonomyElement::TaxonomyElement(const QString name, const QString RGBColor)
: m_parent(NULL)
, m_name(name)//, m_elements(NULL)
, m_color(RGBColor)
{
}

//------------------------------------------------------------------------
TaxonomyElement::~TaxonomyElement()
{
//  qDebug() << "Destroy node " << m_name;
  TaxonomyElement *node;
  foreach(node, m_elements)
  {
    delete node;
  }
}

//------------------------------------------------------------------------
TaxonomyElement* TaxonomyElement::addElement(const QString qualifiedName)
{
  TaxonomyElement *parent = this;
  QString name;
  foreach(name, qualifiedName.split("/",QString::SkipEmptyParts))
  {
    TaxonomyElement *child = parent->element(name);
    if (!child)
      child = parent->insertNode(name);
    parent = child;
  }
  
  return parent;
}

//------------------------------------------------------------------------
TaxonomyElement* TaxonomyElement::element(const QString qualifiedName)
{
  QString::SectionFlag flag = QString::SectionSkipEmpty;
  QString node = qualifiedName.section("/",0,0,flag);
  QString subNodes = qualifiedName.section("/",1,-1,flag);

  TaxonomyElement *child;
  foreach(child, m_elements)
  {
    if (node == child->m_name)
    {
      if (subNodes.isEmpty())
	return child;
      else
	return child->element(qualifiedName.section("/",1));
    }
  }

  return NULL;
}

//------------------------------------------------------------------------
TaxonomyElement* TaxonomyElement::parentNode() const
{
  return m_parent;
}

//------------------------------------------------------------------------
void TaxonomyElement::setName(QString name)
{
  bool exits = false;
  foreach(TaxonomyElement *sibling, m_parent->m_elements)
  {
    exits = exits || sibling->name() == name;
  }
  if (!exits)
    m_name = name;
}

//------------------------------------------------------------------------
const QString TaxonomyElement::name() const
{
  return m_name;
}

//------------------------------------------------------------------------
const QString TaxonomyElement::qualifiedName() const
{
  if (m_parent && !m_parent->name().isEmpty())
    return m_parent->qualifiedName() + "/" + m_name;
  else
    return m_name;
}

//------------------------------------------------------------------------
void TaxonomyElement::addProperty(const QString& prop, const QVariant& value)
{
  m_properties[prop] = value;
}

//------------------------------------------------------------------------
void TaxonomyElement::removeProperty(const QString& prop)
{
  if (m_properties.contains(prop))
    m_properties.remove(prop);
}

//------------------------------------------------------------------------
QVariant TaxonomyElement::property(const QString& prop) const
{
  return m_properties.value(prop,QVariant());
}


//------------------------------------------------------------------------
void TaxonomyElement::print(int level)
{
  if (!m_name.isEmpty())
  {
    std::cout <<
    std::string(level*2, ' ') << m_name.toStdString() << std::endl;
    if (m_properties.size() > 0)
    {
      std::cout << std::string(level*2, ' ') << "<" << std::endl;
      foreach(QString key, m_properties.keys())
      {
	std::cout << std::string((level+1)*2, ' ')
	<< key.toStdString() << ": " << m_properties[key].toString().toStdString()
	<< std::endl;
      }
      std::cout << std::string(level*2, ' ') << ">" << std::endl;
    }
  }

  TaxonomyElement *node;
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



//------------------------------------------------------------------------

//------------------------------------------------------------------------
TaxonomyElement* TaxonomyElement::addElement(QString subElement, QString supElement)//, QString RGBColor)
{
  //ASSERT( this->getComponent(subElement)==NULL);
  if( this->element(subElement) )
    throw "RepeatedElementException"; //TODO change exception
  TaxonomyElement* supNode = this->element(supElement);
  TaxonomyElement *newElement = NULL;
  if( supNode )
  {
//     QString newColor = ((RGBColor == "") ? supNode->color().name() : RGBColor);
    newElement = supNode->insertNode( subElement );
    //newElement->setColor(supNode->getColor());
  }
  else{
    std::cerr << "Error: " << supElement.toStdString() << " does not exist in the taxonomy" << std::endl;
    //exit(1);
  }
  return newElement;
}

//------------------------------------------------------------------------
void TaxonomyElement::removeChild(QString name)
{
  TaxonomyElement *node;
  if( (node = element(name)) )
  {
//     TaxonomyNode *parent = node->parentNode();
    int index = m_elements.indexOf(node);
    m_elements.remove(index);
  }
  else{
    std::cerr << "Error: " << name.toStdString() << " does not exist in the taxonomy" << std::endl;
  }
}


//------------------------------------------------------------------------
// TaxonomyNode* TaxonomyNode::getParent(QString name)
// {
//   TaxonomyNode *parent = NULL;
//   
//   for (int i = 0; i < getSubElements().size(); i++)
//   {
//     if (getSubElements()[i]->name() == name)
// 	parent = this;
//     else
//       parent = getSubElements()[i]->getParent(name);
//     if (parent)
//      return parent;
//   }
//   
//   return parent;
// }



// //------------------------------------------------------------------------
// TaxonomyNode* TaxonomyNode::getComponent(QString name)
// {
//   TaxonomyNode* taxNode = NULL;
//   if (m_name == name)
//     return this;
//   
//   TaxonomyNode *node;
//   foreach(node,m_elements)
//   {
//     taxNode = node->getComponent(name);
//     if (taxNode)
//       break;
//   }
//     
// //   if( m_name.compare(name) != 0 ){
// //     if (m_elements){ 
// //       for( std::vector< TaxonomyNode* >::iterator it = m_elements->begin(); it < m_elements->end(); it++){
// // 	taxNode = (*it)->getComponent( name );
// // 	if( taxNode )
// // 	  break;
// //       }
// //     }
// //   }
// //   else
// //     return this;
//   return taxNode;
// }



//------------------------------------------------------------------------

//------------------------------------------------------------------------
//! Returns a vector with the subelements of a node
// QVector<TaxonomyNode *> TaxonomyNode::getSubElements() const
// {
//   return m_elements;
// }

// //------------------------------------------------------------------------
// TaxonomyNode* TaxonomyNode::insertElement(QString subElement, QString RGBColor)
// {
//   //if( !m_elements )
//   //  m_elements = new std::vector<TaxonomyNode*>();
//   TaxonomyNode *newElement = new TaxonomyNode(subElement, RGBColor);
//   
//   m_elements.push_back(newElement);
//   return newElement;
// }

//------------------------------------------------------------------------
TaxonomyElement* TaxonomyElement::insertNode(const QString& name)
{
  TaxonomyElement *node = new TaxonomyElement(name);
  m_elements.append(node);
  node->m_parent = this;
  node->setColor(this->color());

  return node;
}


//------------------------------------------------------------------------
QVariant TaxonomyElement::data(int role) const
{
  switch (role)
  {
    case Qt::DisplayRole:
    case Qt::EditRole:
	return name();
    case Qt::DecorationRole:
    {
      QPixmap icon(16,16);
      icon.fill(color());
      return icon;
    }
    default:
      return QVariant();
  }
}

//------------------------------------------------------------------------
bool TaxonomyElement::setData(const QVariant& value, int role)
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

//-----------------------------------------------------------------------------
// TAXONOMY
//-----------------------------------------------------------------------------
Taxonomy::Taxonomy()
: m_root(new TaxonomyElement(QString()))
{
}

//-----------------------------------------------------------------------------
Taxonomy::~Taxonomy()
{
//  qDebug() << "Destroy taxonomy";
  if (m_root)
    delete m_root;
}

//-----------------------------------------------------------------------------
TaxonomyElement* Taxonomy::addElement(const QString name, const QString parent)
{
  TaxonomyElement *node = NULL;

  if (parent.isEmpty())
    node = m_root->addElement(name);
  else
  {
    TaxonomyElement *parentNode = element(parent);
    if (parentNode)
      node = parentNode->addElement(name);
  }

  return node;
}

//-----------------------------------------------------------------------------
void Taxonomy::removeElement(const QString qualifiedName)
{
  TaxonomyElement *node = element(qualifiedName);
  assert(node);
  TaxonomyElement *parent = node->parentNode();
  assert(parent);
  
  parent->removeChild(node->name());
}

//-----------------------------------------------------------------------------
TaxonomyElement* Taxonomy::element(const QString qualifiedName)
{
  QString::SectionFlag flag = QString::SectionSkipEmpty;
  return m_root->element(qualifiedName);

  QString rootName = qualifiedName.section("/",0,0,flag);

  if (rootName != m_root->name())
    return NULL;

  QString subName = qualifiedName.section("/",1,-1,flag);
  if (subName.isEmpty())
    return m_root;
  else
    return m_root->element(subName);
}

//-----------------------------------------------------------------------------
void Taxonomy::print(int indent)
{
  m_root->print(indent);
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

QString concatenate(std::stack<QString> hierarchy)
{
  QString res;
  while (!hierarchy.empty())
  {
    res = QString(hierarchy.top()) + "/" + res;
    hierarchy.pop();
  }
  return res;
}

Taxonomy* IOTaxonomy::readXML(QXmlStreamReader& xmlStream)
{
  // Read the XML
//   QXmlStreamReader xmlStream(&file);
  QStringRef nodeName, color;
  Taxonomy *tax = new Taxonomy();
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
//         if( taxHierarchy.empty() )
//         {
//           tax = new Taxonomy();
//         }
//         else
//         {
	  QString qualified = concatenate(taxHierarchy);
          TaxonomyElement *node = tax->addElement( nodeName.toString(), qualified);
	  node->setColor(color.toString());
	  QXmlStreamAttribute attrib;
	  foreach(attrib, xmlStream.attributes())
	  {
	    if (attrib.name() == "name" || attrib.name() == "color")
	      continue;
	    node->addProperty(attrib.name().toString(), attrib.value().toString());
	  }
//         }
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


Taxonomy *IOTaxonomy::openXMLTaxonomy(QString fileName)
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
  Taxonomy *tax = readXML(xmlStream);
  file.close();
  return tax;
}

Taxonomy *IOTaxonomy::loadXMLTaxonomy(QString content)
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

void IOTaxonomy::writeTaxonomy(Taxonomy *tax, QXmlStreamWriter& stream)
{
  if( tax )
    foreach(TaxonomyElement *node, tax->elements())
      IOTaxonomy::writeTaxonomyElement(node, stream);
}

void IOTaxonomy::writeTaxonomyElement(TaxonomyElement* node, QXmlStreamWriter& stream)
{
  if( node )
  {
    stream.writeStartElement( "node" );
    stream.writeAttribute("name", node->name());
    stream.writeAttribute("color", node->color().name());
    foreach(QString prop, node->properties())
    {
      stream.writeAttribute(prop, node->property(prop).toString());
    }
    TaxonomyElement* subnode;
    foreach(subnode, node->subElements())
    {
      IOTaxonomy::writeTaxonomyElement(subnode, stream );
    }
    stream.writeEndElement();
  }
}

//-----------------------------------------------------------------------------
void IOTaxonomy::writeXMLTaxonomy(Taxonomy *tax, QString& destination)
{
  QXmlStreamWriter stream(&destination);

  stream.setAutoFormatting(true);
  stream.writeStartDocument();
  stream.writeStartElement("Taxonomy");

  IOTaxonomy::writeTaxonomy(tax, stream);

  stream.writeEndElement();
  stream.writeEndDocument();
}