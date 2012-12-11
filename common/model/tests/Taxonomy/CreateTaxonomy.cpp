#include "common/model/Taxonomy.h"

#include <assert.h>
#include <QDebug>

int CreateTaxonomy(int argc, char** argv)
{
  // Create empty Taxonmy
  Taxonomy tax;
  
  // Add subnode to root node
  assert(tax.addElement("Level 1"));
  assert(tax.element("Level 1"));
  // Add subnode to given node
  assert(tax.addElement("Level 11", "Level 1"));
  assert(tax.addElement("Level 12", "Level 1"));
  
  TaxonomyElement *node;
  node = tax.addElement("Level 2");
  assert(node);
  // Use node to add subnodes
  node = node->addElement("Level 21");
  assert(node);
  node->addElement("Level 211");
  assert(node);

  assert(tax.addElement("Level 3"));

  // Add qualified node
  node = tax.addElement("Level 2/Level 22");
  assert(node);
  TaxonomyElement * a =  tax.addElement("Level 2/Level 21/Level 212");
  // Repeated nodes are not duplicated, and the previous node is returned
  TaxonomyElement * b = tax.addElement("Level 2/Level 21/Level 212"); 
  assert(a == b);
  assert(tax.addElement("Level 2/Level 22/Level 221"));
  
  // Add node to qualified node
  assert(tax.addElement("Level 222","Level 2/Level 22"));
  // Add qualified node to qualified node
  assert(tax.addElement("Level 222/Level 2221",node->qualifiedName()));//"Root/Level 2/Level 22"));
  node = tax.addElement("Level 222/Level 2222", "Level 2/Level 22");
  assert(node);
  assert(tax.addElement("Level 2221/Level 22211",node->parentNode()->qualifiedName()));//"Level 2/Level 22/Level 222"));
  assert(node->parentNode()->element(node->name()) == node);
  
  assert(tax.element("Level 2/Level 22/Level 221"));
  assert(tax.element(node->qualifiedName()));
  assert(tax.element("Level 2/Level 22/Level 223") == NULL);
  assert(tax.element("Level 2/Level 21/Level 221") == NULL);
  assert(tax.element("Level 1/Level 22/Level 221") == NULL);

  QString taxonomy("Level 1/Level 13/Level 134");
  node = tax.addElement(taxonomy);
  assert(node);

  assert(node->qualifiedName() == taxonomy);

  tax.print();
  
  return 0;
}