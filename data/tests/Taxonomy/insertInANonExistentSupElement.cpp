#include <taxonomy.h>

// int insertInANonExistentSupElement(int argc, const char* argv[])
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
  
  if( tax->addElement("Cangrejo", "Crustáceo") != NULL )
    return 1;
  return 0;
}