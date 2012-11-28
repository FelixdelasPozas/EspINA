#include "common/model/Taxonomy.h"

#include <QDebug>
#include <QFile>

#include <iostream>
#include <assert.h>

int TaxonomyProperties(int argc, char** argv)
{
  // Create empty Taxonmy
  Taxonomy tax;

  // Add subnode to root node

  TaxonomyElement *node;
  node = tax.addElement("Level 1");
  assert(node);

  node->addProperty("Color","#FF0000");
  node->addProperty("Region","0,1,0,1");
  node->addProperty("Description", "First Level Node");
  node->addProperty("Error", "ERROR");

  qDebug() << node->property("Region");

  node->removeProperty("Error");

  tax.print();

  QString stream;
  IOTaxonomy::writeXMLTaxonomy(&tax, stream);

  QFile file("tax.xml");
  file.open(QIODevice::ReadWrite);
  file.write(stream.toStdString().c_str());
  file.close();

  return 0;
}