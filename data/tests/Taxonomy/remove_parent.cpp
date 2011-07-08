#include "taxonomy.h"

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
  
  // Remove an element
  tax->removeElement("Pera");
  // Remove a non existent element
  tax->removeElement("Pera");
  if( tax->getParent("Pera") || tax->getComponent("Pera") )
    return 1;
  
  
  tax->print();
  delete tax;
  return 0; // ERROR !!!
}
