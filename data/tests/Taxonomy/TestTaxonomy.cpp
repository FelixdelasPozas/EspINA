#include "taxonomy.h"
#include <QDebug>

#include <assert.h>
#include <stdio.h>
#include <fstream>

int diff( std::string file1, std::string file2)
{
  char name1[256], name2[256];
  std::ifstream f1, f2;
  
  f1.open(file1.c_str());
  f2.open(file2.c_str());
  
  while( f1.good() )
  {
    f1.getline(name1, 256);
    f2.getline(name2, 256);
    if( strncmp(name1, name2, 256) ){
      qDebug() << "DIFF ERROR:\n" << name1 << "\n" << name2;
      return 1;
    }
  }
  return 0;  
}

int main(int argc, const char* argv[]){

  qDebug(" -- Testing TaxnomyNode class --");
  TaxonomyNode* tax;
  QString sup = "Seres vivos";
  tax = new TaxonomyNode(sup);
  tax->addElement("Animales", "Seres vivos");
  tax->addElement("Vegetales", "Seres vivos");
  
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
  IOTaxonomy::writeXMLTaxonomy( tax, fileName );
  
  delete tax;
  
  qDebug() << " -- Reading file " << fileName << " --";
  tax = IOTaxonomy::openXMLTaxonomy( fileName );
  tax->print();
  
  QString fileName2 (fileName);
  fileName2.append("v2");
  
  IOTaxonomy::writeXMLTaxonomy( tax, fileName2 );
  
  qDebug() << "diff " << fileName << " " << fileName2;
  assert( diff( fileName.toStdString(), fileName2.toStdString()) == 0 );
  
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
