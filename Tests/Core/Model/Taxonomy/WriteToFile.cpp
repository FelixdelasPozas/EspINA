#include <Core/Model/Taxonomy.h>
#include <Core/IO/EspinaIO.h>
#include <QDebug>
#include <QFile>

using namespace EspINA;

// int writing_reading( int argc, const char* argv[] )
int WriteToFile(int argc, char* argv[])
{
  TaxonomySPtr tax(new Taxonomy());
  TaxonomyElementSPtr rootNode(tax->createElement("Root"));
  
  tax->createElement("Leaf 1", rootNode);
  TaxonomyElementSPtr subnode(tax->createElement("Leaf 2", rootNode));

  subnode->createElement("Leaf 2-1");
  subnode->createElement("Leaf 2-2");

  QString fileContent1;
  IOTaxonomy::writeXMLTaxonomy(tax, fileContent1);
  
  TaxonomySPtr otherTax(IOTaxonomy::loadXMLTaxonomy(fileContent1));
  
  QString fileContent2;
  IOTaxonomy::writeXMLTaxonomy(otherTax, fileContent2);

  if(fileContent1.compare(fileContent2) != 0)
    return 1;
  
  tax.clear();
  otherTax.clear();

  QFile f("test.xml");
  if( f.open(QIODevice::WriteOnly | QIODevice::Truncate) )
  {
    f.write(fileContent1.toUtf8());
    f.close();
  }
  else
    return 1;

  if((tax = IOTaxonomy::openXMLTaxonomy("test.xml")) == NULL)
    return 1;

  fileContent2.clear();
  IOTaxonomy::writeXMLTaxonomy(tax, fileContent2);
  if( fileContent1.compare(fileContent2) != 0 )
    return 1;

  tax.clear();
  std::system("rm -f test.xml");

  return 0;
}
