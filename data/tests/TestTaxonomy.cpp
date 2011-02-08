#include <iostream>
#include <map>
#include <vector>
#include <string>
#include "taxonomy.h"

#include <QDebug>

int main(int argc, const char* argv[]){

  qDebug(" -- Testing TaxnomyNode class --");
  TaxonomyNode* tax;
  QString sup = "Seres vivos";
  tax = new TaxonomyNode(sup);
  tax->addElement("Animales", sup);
  tax->addElement("Vegetales", sup);
  
  sup = "Animales";
  tax->addElement("Aves", sup);
  tax->addElement("Mamiferos", sup);
  
  sup = "Vegetales";
  tax->addElement("Verduras", sup);
  tax->addElement("Frutas", sup);
  
  sup = "Verduras";
  tax->addElement("Zanahoria", sup);
  tax->addElement("Puerros", sup);
  
  sup = "Frutas";
  tax->addElement("Naranja", sup);
  tax->addElement("Manzana", sup);
  tax->addElement("Pera", sup);
  
  // insertion of a repetead element
  try{
    tax->addElement("Pera", "Animales");
  }
  catch (const char* e){
    qDebug() << " -- Exception "<< e << " throwed --";
  }
  // insertion with a non-existent supelement
  tax->addElement("Cangrejo", "Crustáceo");  
  
  tax->print();
  
  
  qDebug(" -- Writing to an xml file --");
  QString fileName = "test_taxonomy.xml";
  if( argc > 1 )
    fileName = argv[1];
  IOTaxonomy::writeXMLTaxonomy( *tax, fileName );
  
  delete tax;
  
  qDebug() << " -- Reading file " << fileName << " --";
  tax = IOTaxonomy::openXMLTaxonomy( fileName );
  tax->print();
  
  IOTaxonomy::writeXMLTaxonomy( *tax, fileName.append("v2"));
  
  //TODO diff fileName vs fileName+"v2"
  
  delete tax;
  
  fileName = "extra_format_tax.xml";
  qDebug() << " -- Reading different format file " << fileName << " --";
  tax = IOTaxonomy::openXMLTaxonomy( fileName );
  tax->print();
  
  //std::cout << "Tax deleted ..." << std::endl;
  //IOTaxonomy::writeXMLTaxonomy( *tax, QString(argv[1]) );
  /* elementos repetidos */
  //std::cout << "Introduciendo otra vez 'Naranja'" << std::endl;
  //tax->addElement("Naranja", sup); // --> assert

  return 0;
}
