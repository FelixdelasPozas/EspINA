#include "Core/Analysis/Category.h"

// ESPINA
#include <Core/Utils/EspinaException.h>

// Qt
#include <QString>
#include <QColor>

// C++
#include <iostream>

// VTK
#include <vtkMath.h>

using namespace ESPINA;

const QString Category::X_DIM = "Dim_X";
const QString Category::Y_DIM = "Dim_Y";
const QString Category::Z_DIM = "Dim_Z";

#include <QDebug>
//------------------------------------------------------------------------
Category::Category(CategoryPtr parent,
                   const QString &name,
                   const Hue color)
: m_parent(parent)
, m_name(name)
, m_color(color)
{
}

//------------------------------------------------------------------------
Category::~Category()
{
//  qDebug() << "Destroy category " << m_name;
}

//------------------------------------------------------------------------
void Category::setName(const QString &name)
{
  if (m_parent != nullptr && m_parent->subCategory(name).get() != nullptr)
  {
    auto what = QObject::tr("Category already defined, category: %1").arg(name);
    auto details = QObject::tr("Category::setName() -> Category already defined, category: %1").arg(name);

    throw Core::Utils::EspinaException(what, details);
  }

  m_name = name;
}

//------------------------------------------------------------------------
const QString Category::name() const
{
  return m_name;
}

//------------------------------------------------------------------------
const QString Category::classificationName() const
{
  if (m_parent && !m_parent->name().isEmpty())
  {
    return m_parent->classificationName() + "/" + m_name;
  }

  return m_name;
}

//------------------------------------------------------------------------
void Category::setColor(const Hue color)
{
  if (m_color != color)
  {
    m_color = color;
  }
}

//------------------------------------------------------------------------
CategorySPtr Category::createSubCategory(const QString& name)
{
  CategorySPtr subCategory(new Category(this, name));
  subCategory->setColor(m_color);

  m_subCategories << subCategory;

  return subCategory;
}

//-----------------------------------------------------------------------------
void Category::addSubCategory(CategorySPtr subCategory)
{
  // check if already present
  for(auto category: m_subCategories)
  {
    if(category == subCategory) return;
  }

  if (subCategory->m_parent)
  {
    subCategory->m_parent->removeSubCategory(subCategory);
  }
  subCategory->m_parent = this;

  m_subCategories << subCategory;
}

//------------------------------------------------------------------------
void Category::removeSubCategory(CategoryPtr subCategory)
{
  CategorySPtr subNode;

  int index = 0;
  while (!subNode && index < m_subCategories.size())
  {
    if (m_subCategories[index].get() == subCategory)
    {
      subNode = m_subCategories[index];
    }
    else
    {
      index++;
    }
  }

  if (subNode)
  {
    subNode->m_parent = nullptr;
    m_subCategories.removeAt(index);
  }
}

//------------------------------------------------------------------------
const CategorySPtr Category::subCategory(const QString& name) const
{
  CategorySPtr res;

  int i = 0;
  while (!res && i < m_subCategories.size())
  {
    if (m_subCategories[i]->name() == name)
    {
      res = m_subCategories[i];
    }

    i++;
  }

  return res;
}

//------------------------------------------------------------------------
void Category::addProperty(const QString& prop, const QVariant& value)
{
  m_properties[prop] = value;
}

//------------------------------------------------------------------------
void Category::deleteProperty(const QString& prop)
{
  if (m_properties.contains(prop))
  {
    m_properties.remove(prop);
  }
}

//------------------------------------------------------------------------
const QVariant Category::property(const QString& prop) const
{
  return m_properties.value(prop,QVariant());
}

//------------------------------------------------------------------------
QString ESPINA::print(CategorySPtr category, int level)
{
  QString out = QString("%1%2\n").arg(QString(level*2, ' ')).arg(category->name());

  for(auto prop: category->properties())
  {
    out += QString("%1Property: %2 = %3\n").arg(QString(level*2, ' ')).arg(prop).arg(category->property(prop).toString());
  }

  for(auto subCategory: category->subCategories())
  {
    out += QString("%1").arg(print(subCategory, level+1));
  }

  return out;
}
