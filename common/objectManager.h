#ifndef OBJECT_MANAGER_H
#define OBJECT_MANAGER_H

#include <QObject>
#include <QList>

//Forward declarations
class Product;
class IRenderable;

/// This class manages the segmentation process of EM image stacks
class ObjectManager : public QObject
{
  Q_OBJECT
public:
  static ObjectManager *instance();
  
  Product *activeStack(){return m_products.first();}
  
public slots:
  //! A new Product has been added
  void registerProduct(Product *product);
  
signals:
  void render(IRenderable *product);
  void sliceRender(IRenderable *product);
  
protected:
  ObjectManager();
  
private:
  //TODO: This single list could be decomposed into severals
  // to optimize class operations
  QList<Product *> m_products;
  static ObjectManager *m_singleton;
};

#endif// OBJECT_MANAGER_H
