#include <Core/Model/Taxonomy.h>
#include <Core/IO/SegFileReader.h>
#include <QDebug>
#include <QFile>

using namespace EspINA;

int TaxonomyProperties(int argc, char** argv)
{
  // Create empty Taxonmy
  Taxonomy *tax = new Taxonomy();
  TaxonomySPtr taxSPtr(tax);

  // Add subnode to root node
  TaxonomyElementSPtr node(NULL);
  node = tax->createElement("Level 1");
  Q_ASSERT(node);

  node->addProperty("Color","#FF0000");
  node->addProperty("Region","0,1,0,1");
  node->addProperty("Description", "First Level Node");
  node->addProperty("Error", "ERROR");

  tax->print();
  qDebug() << node->property("Region");
  node->removeProperty("Error");
  tax->print();

  QString stream;
  IOTaxonomy::writeXMLTaxonomy(taxSPtr, stream);
  qDebug() << stream;

//  QFile file("tax.xml");
//  file.open(QIODevice::ReadWrite);
//  file.write(stream.toStdString().c_str());
//  file.close();

  return 0;
}
