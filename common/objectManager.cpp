#include "objectManager.h"

#include "traceNodes.h"

//ParaQ Includes
#include "pqPipelineSource.h"

ObjectManager *ObjectManager::m_singleton(NULL);

ObjectManager::ObjectManager()
{
}


ObjectManager* ObjectManager::instance()
{
  if (!m_singleton)
    m_singleton = new ObjectManager();
  
  return m_singleton;
}

void ObjectManager::registerProduct(Product* product)
{
  m_products.push_back(product);
  emit render(product);
  if (m_products.size() > 1)
    emit sliceRender(product);
}

