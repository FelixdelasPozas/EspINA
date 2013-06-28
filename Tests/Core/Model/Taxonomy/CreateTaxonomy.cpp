#include <Core/Model/Taxonomy.h>
#include <QDebug>

using namespace EspINA;

int CreateTaxonomy(int argc, char** argv)
{
  // Create empty Taxonomy
  Taxonomy tax;
  TaxonomyElementSPtr node(NULL);
  
  // Add sub-node to root node
  Q_ASSERT(tax.createElement("Level 1"));
  Q_ASSERT(tax.element("Level 1"));

  // Add sub-node to given node
  Q_ASSERT(tax.createElement("Level 11", tax.element("Level 1")));
  Q_ASSERT(tax.createElement("Level 12", tax.element("Level 1")));
  
  node = tax.createElement("Level 2");
  Q_ASSERT(node);

  // Use node to add sub-nodes
  node = node->createElement("Level 21");
  Q_ASSERT(node);
  node = node->createElement("Level 211");
  Q_ASSERT(node);

  Q_ASSERT(tax.createElement("Level 3"));

  return 0;
}
