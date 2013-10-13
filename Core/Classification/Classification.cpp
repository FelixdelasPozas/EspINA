#include "Core/Classification/Classification.h"

#include <QFile>
#include <QPixmap>

#include <iostream>
#include <QDebug>

using namespace EspINA;

const QString Classification::ROOT = QString();

Classification::Classification()
: m_root(new Category(NULL, ROOT))
{
}

//-----------------------------------------------------------------------------
Classification::~Classification()
{
   //qDebug() << "Destroy taxonomy";
}

//-----------------------------------------------------------------------------
CategorySPtr Classification::createCategory(const QString& classificationName,
                                            CategoryPtr parent)
{
  CategoryPtr parentNode = parent;

  if (!parentNode)
    parentNode = m_root.get();

  CategorySPtr requestedClassification;

  if (!classificationName.isEmpty())
  {
    QStringList path = classificationName.split("/", QString::SkipEmptyParts);
    for (int i = 0; i < path.size(); ++i)
    {
      requestedClassification = parentNode->subCategory(path.at(i));
      if (i == path.size() - 1 && requestedClassification != nullptr) {
        throw Category::AlreadyDefinedCategoryException();        
      }
      
      if (!requestedClassification)
      {
        requestedClassification = parentNode->createSubCategory(path.at(i));
      }
      parentNode = requestedClassification.get();
    }
  }
  else
  {
    requestedClassification = parentNode->subCategory(QString("Unspecified"));

    if (!requestedClassification)
      requestedClassification = parentNode->createSubCategory(QString("Unspecified"));
  }

  return requestedClassification;
}

//-----------------------------------------------------------------------------
CategorySPtr Classification::createCategory(const QString& name,
                                            CategorySPtr   parent)
{
  return createCategory(name, parent.get());
}

//-----------------------------------------------------------------------------
void Classification::removeCategory(CategoryPtr subCategory)
{
  Q_ASSERT(subCategory);

  if (subCategory != m_root.get())
  {
    CategoryPtr parentElement = subCategory->parent();
    parentElement->removeSubCategory(subCategory);
  }
  else
    m_root.reset();
}

//-----------------------------------------------------------------------------
void Classification::removeCategory(CategorySPtr subCategory)
{
  removeCategory(subCategory.get());
}

//-----------------------------------------------------------------------------
CategorySPtr Classification::category(const QString& qualifiedName)
{
  QStringList path = qualifiedName.split("/", QString::SkipEmptyParts);
  CategorySPtr node = m_root;

  int i = 0;
  while (node.get() != NULL && i < path.length())
  {
    node = node->subCategory(path[i]);
    ++i;
  }

  if (node == m_root)
    node.reset();

  return node;
}

//-----------------------------------------------------------------------------
CategorySPtr Classification::parent(const CategorySPtr subCategory) const
{
  QStringList path = subCategory->classificationName().split("/", QString::SkipEmptyParts);

  CategorySPtr parent = m_root;
  for(int i = 0; i < path.length() - 1; i++)
  {
    parent = parent->subCategory(path[i]);
    Q_ASSERT(parent.get() != NULL);
  }

  return parent;
}

//-----------------------------------------------------------------------------
void Classification::print(int indent)
{
  m_root->print(indent);
}
