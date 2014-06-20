#include "Core/Analysis/Category.h"

// Qt
#include <QString>

// C++
#include <iostream>

//#include <assert.h>
/*
#define ASSERT(x) \
  if (! (x)) \
  { \
    std::cout << "ERROR: Assert " << #x << " failed\n"; \
    std::cout << " in " << __FILE__ << ":" << __LINE__  << "\n"; \
  } \
  assert(x)
*/

using namespace EspINA;

// const QString Category::X_DIM = "Dim_X";
// const QString Category::Y_DIM = "Dim_Y";
// const QString Category::Z_DIM = "Dim_Z";

//------------------------------------------------------------------------
Category::Category(CategoryPtr parent,
                   const QString &name,
                   const QString &RGBColor)
: m_parent(parent)
, m_name(name)
, m_color(RGBColor)
{
}

//------------------------------------------------------------------------
Category::~Category()
{
   //qDebug() << "Destroy node " << m_name;
}

//------------------------------------------------------------------------
void Category::setName(const QString &name)
{
  if (m_parent != nullptr && m_parent->subCategory(name).get() != nullptr) {
    throw AlreadyDefinedCategoryException();
  }
  m_name = name;
}

//------------------------------------------------------------------------
QString Category::name() const
{
  return m_name;
}

//------------------------------------------------------------------------
QString Category::classificationName() const
{
  if (m_parent && !m_parent->name().isEmpty())
    return m_parent->classificationName() + "/" + m_name;
  else
    return m_name;
}

//------------------------------------------------------------------------
void Category::setColor(const QColor &color)
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
    if(category == subCategory)
      return;

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
      subNode = m_subCategories[index];
    else
      index++;
  }

  if (subNode)
  {
    subNode->m_parent = nullptr;
    m_subCategories.removeAt(index);
  }
}

//------------------------------------------------------------------------
CategorySPtr Category::subCategory(const QString& name) const
{
  CategorySPtr res;

  int i = 0;
  while (!res && i < m_subCategories.size())
  {
    if (m_subCategories[i]->name() == name)
      res = m_subCategories[i];

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
    m_properties.remove(prop);
}

//------------------------------------------------------------------------
QVariant Category::property(const QString& prop) const
{
  return m_properties.value(prop,QVariant());
}

//------------------------------------------------------------------------
QString EspINA::print(CategorySPtr category, int level)
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
