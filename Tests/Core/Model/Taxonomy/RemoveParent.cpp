#include <Core/Model/Taxonomy.h>
#include <Core/IO/SegFileReader.h>
#include <QDebug>

using namespace EspINA;

int RemoveParent(int argc, char* argv[])
{
  TaxonomySPtr tax(new Taxonomy());
  TaxonomyElementSPtr rootNode(tax->createElement("Root"));
  
  tax->createElement("Leaf 1", rootNode);
  TaxonomyElementSPtr subnode(tax->createElement("Leaf 2", rootNode));

  subnode->createElement("Leaf 2-1");
  subnode->createElement("Leaf 2-2");

  QString stream, original;
  original.append("<?xml version=\"1.0\"?>\n");
  original.append("<Taxonomy>\n");
  original.append("    <node name=\"Root\" color=\"#00ff00\">\n");
  original.append("        <node name=\"Leaf 1\" color=\"#00ff00\"/>\n");
  original.append("        <node name=\"Leaf 2\" color=\"#00ff00\">\n");
  original.append("            <node name=\"Leaf 2-1\" color=\"#00ff00\"/>\n");
  original.append("            <node name=\"Leaf 2-2\" color=\"#00ff00\"/>\n");
  original.append("        </node>\n");
  original.append("    </node>\n");
  original.append("</Taxonomy>\n");
  IOTaxonomy::writeXMLTaxonomy(tax, stream);

  if (stream.compare(original) != 0)
    return 1;

  tax->deleteElement(subnode);
  stream.clear();
  IOTaxonomy::writeXMLTaxonomy(tax, stream);
  original.clear();
  original.append("<?xml version=\"1.0\"?>\n");
  original.append("<Taxonomy>\n");
  original.append("    <node name=\"Root\" color=\"#00ff00\">\n");
  original.append("        <node name=\"Leaf 1\" color=\"#00ff00\"/>\n");
  original.append("    </node>\n");
  original.append("</Taxonomy>\n");

  if (stream.compare(original) != 0)
    return 1;

  tax->deleteElement(rootNode);
  stream.clear();
  IOTaxonomy::writeXMLTaxonomy(tax, stream);
  original.clear();
  original.append("<?xml version=\"1.0\"?>\n");
  original.append("<Taxonomy/>\n");

  if (stream.compare(original) != 0)
    return 1;

  tax.clear();
  return 0;
}
