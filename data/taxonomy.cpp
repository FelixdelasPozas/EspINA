#include "taxonomy.h"
#include <iomanip>
#include <assert.h>

#define ASSERT(x) \
  if (! (x)) \
  { \
    std::cout << "ERROR: Assert " << #x << " failed\n"; \
    std::cout << " in " << __FILE__ << ":" << __LINE__  << "\n"; \
  } \
  assert(x)

void TaxonomyNode::insertElement(std::string subElement)
{
  if( !m_elements )
    m_elements = new std::vector<TaxonomyNode*>();
  m_elements->push_back( new TaxonomyNode(subElement) );
}

TaxonomyNode::~TaxonomyNode()
{
  std::vector<TaxonomyNode *>::iterator it;
  if( m_elements )
    //std::vector<TaxonomyNode *>::iterator it;
    for(it = m_elements->begin(); it < m_elements->end(); it++)
      delete (*it);
    
    delete( m_elements );
}

void TaxonomyNode::print(int level)
{
  std::cout << 
  std::string(level*2, ' ') <<
  //std::string("-") <<
  m_name <<
  std::endl;
  
  if(m_elements){
    std::vector<TaxonomyNode *>::iterator it;
    for( it = m_elements->begin(); it < m_elements->end(); it++)
      (*it)->print(level+1);
  }
}


TaxonomyNode::TaxonomyNode(std::string name):
  m_name(name), m_elements(NULL)
{
}

/*
void TaxonomyNode::addElement(std::string subElement)
{
  assert( this->getComponent(subElement)==NULL);
  this->insertElement( subElement );
}
*/

TaxonomyNode* TaxonomyNode::addElement(std::string subElement, std::string supElement)
{

  ASSERT( this->getComponent(subElement)==NULL);
  TaxonomyNode* supNode = this->getComponent(supElement);
  if( supNode )
    supNode->insertElement( subElement );
}


TaxonomyNode* TaxonomyNode::getComponent(std::string name)
{
  TaxonomyNode* taxNode = NULL;
  if( m_name.compare(name) != 0 ){
    if (m_elements){ 
      std::vector<TaxonomyNode *>::iterator it;
      
      for( it = m_elements->begin(); it < m_elements->end(); it++){
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
