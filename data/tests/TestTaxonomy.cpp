#include <iostream>
#include <map>
#include <vector>
#include <string>
#include "taxonomy.h"

int main(int argc, const char* argv[]){
  TaxonomyNode* tax;
  
  std::string sup = "Seres vivos";
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
  
  tax->print();
  
  /* elementos repetidos */
  //std::cout << "Introduciendo otra vez 'Naranja'" << std::endl;
  //tax->addElement("Naranja", sup); // --> assert

  return 0;
}
