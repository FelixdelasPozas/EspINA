#ifndef _TAXONOMY_
#define _TAXONOMY_

#include <vector>
#include <string>
#include <iostream>

class TaxonomyNode
{
 std::vector<TaxonomyNode *>* m_elements;
 std::string m_name;
 
 void insertElement( std::string subElement ); // Without checking 
public:

  TaxonomyNode( std::string name );
  ~TaxonomyNode();
  
  void print(int level=0);   

  //void addElement( std::string subElement ); // With checking. TO DELTE. No tiene sentido estando el otro addElement
  TaxonomyNode* addElement( std::string subElement, std::string supElement );
 
  TaxonomyNode* getComponent( std::string name ); 
  std::string getName() {return m_name;}

  
};

#endif // _TAXONOMY_
