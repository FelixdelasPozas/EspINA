#include "Core/Analysis/Classification.h"

#include <QFile>
#include <QPixmap>

#include <iostream>
#include <QDebug>

using namespace EspINA;

const QString Classification::ROOT = QString();

Classification::Classification(const QString& name)
: m_name(name)
, m_root(new Category(NULL, ROOT))
{
}

//-----------------------------------------------------------------------------
Classification::~Classification()
{
   //qDebug() << "Destroy taxonomy";
}

//-----------------------------------------------------------------------------
CategorySPtr Classification::createCategory(const QString& relativeName,
                                            CategorySPtr   parent)
{
  CategoryPtr parentNode = parent.get();

  if (!parentNode)
    parentNode = m_root.get();

  CategorySPtr requestedClassification;

  if (!relativeName.isEmpty())
  {
    QStringList path = relativeName.split("/", QString::SkipEmptyParts);
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
void Classification::removeCategory(CategorySPtr subCategory)
{
  Q_ASSERT(subCategory);

  if (subCategory != m_root)
  {
    CategoryPtr parentElement = subCategory->parent();
    parentElement->removeSubCategory(subCategory);
  }
  else
    m_root.reset();
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
bool Classification::operator==(const Classification& rhs)
{
  return m_name == rhs.m_name;
}

//-----------------------------------------------------------------------------
void Classification::print(int indent)
{
  m_root->print(indent);
}