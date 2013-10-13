#include "Core/Classification/Category.h"

#include <QFile>
#include <QPixmap>

#include <iostream>
#include <QDebug>
#include <assert.h>

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

    emit colorChanged(this);
  }
}

//------------------------------------------------------------------------
CategorySPtr Category::createSubCategory(const QString& name)
{
//   Q_ASSERT(!element(name));

  CategorySPtr subCategory(new Category(this, name));
  subCategory->setColor(m_color);

  m_subCategories << subCategory;

  return subCategory;
}

//-----------------------------------------------------------------------------
void Category::addSubCategory(CategorySPtr subCategory)
{
//   Q_ASSERT(!m_subCategories.contains(taxElement));

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

  if (subNode) {
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
void Category::print(int level)
{
  if (!m_name.isEmpty())
  {
    std::cout <<
    std::string(level*2, ' ') << m_name.toStdString() << std::endl;
    if (m_properties.size() > 0)
    {
      std::cout << std::string(level*2, ' ') << "<" << std::endl;
      foreach(QString key, m_properties.keys())
      {
        std::cout << std::string((level+1)*2, ' ')
        << key.toStdString() << ": " << m_properties[key].toString().toStdString()
        << std::endl;
      }
      std::cout << std::string(level*2, ' ') << ">" << std::endl;
    }
  }

  foreach (CategorySPtr node, m_subCategories) {
    node->print(level+1);
  }
}