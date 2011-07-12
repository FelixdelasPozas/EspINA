#include <taxonomy.h>
#include <fstream>
#include <QDebug>

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
//       qDebug() << "DIFF ERROR:\n" << name1 << "\n" << name2;
      return 1;
    }
  }
  return 0;  
}

// int writing_reading( int argc, const char* argv[] )
int main(int argc, const char* argv[])
{
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
  
  QString fileContent;//= "test_taxonomy.xml";

  IOTaxonomy::writeXMLTaxonomy( tax, fileContent );

  delete tax;
  
//   qDebug() << " -- Reading file " << fileName << " --";
  tax = IOTaxonomy::loadXMLTaxonomy( fileContent );
//  tax->print();
  
  QString fileName2;// (fileContent);
  //fileName2.append("v2");
  
  IOTaxonomy::writeXMLTaxonomy( tax, fileName2 );
  delete tax;
  
  if( fileContent.compare(fileName2) != 0 )
    return 1;
  
  
  /*
  fileContent = "extra_format_tax.xml";
//   qDebug() << " -- Reading different format file " << fileName << " --";
  if(IOTaxonomy::openXMLTaxonomy( fileContent ) == NULL)
    return 1;
  */
  return 0;
}