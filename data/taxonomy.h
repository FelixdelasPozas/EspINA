#ifndef _TAXONOMY_
#define _TAXONOMY_

#include <vector>

// Qt dependencies
#include <QString>
#include <QXmlStreamWriter>

class TaxonomyNode
{
 std::vector<TaxonomyNode *>* m_elements;
 QString m_name;
 
 void insertElement( QString subElement ); // Without checking 
public:

  TaxonomyNode( QString name );
  ~TaxonomyNode();
  
  void print(int level=0);   

  //void addElement( QString subElement ); // With checking. TO DELTE. No tiene sentido estando el otro addElement
  
  // It introduces the subElement string as a subnode of supElement string. If subElement
  // exists in all the tree that this object has it returns an Error.
  // Note that to check the existence of the subElement name, it is necesarry to insert all 
  // the elements through the TaxonoyNode object at the top of the tree.
  TaxonomyNode* addElement( QString subElement, QString supElement );
 
  TaxonomyNode* getComponent( QString name ); 
  QString getName() {return m_name;}
  std::vector<TaxonomyNode*>* getSubElements();
  
};

class IOTaxonomy
{
private:
  static void writeTaxonomyNode(TaxonomyNode* node, QXmlStreamWriter& stream);
  
public:
  IOTaxonomy();
  ~IOTaxonomy();
  
  static TaxonomyNode* openXMLTaxonomy( QString& fileName);
  static void writeXMLTaxonomy(TaxonomyNode& tax, QString fileName);
};

#endif // _TAXONOMY_
