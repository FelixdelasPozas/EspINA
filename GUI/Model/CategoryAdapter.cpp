/*

    Copyright (C) 2014  Jorge Peña Pastor<jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// ESPINA
#include "CategoryAdapter.h"
#include <GUI/ColorEngines/IntensitySelectionHighlighter.h>

// ESPINA
#include <Core/Analysis/Category.h>
#include <QPixmap>
#include <QDebug>

using namespace ESPINA;
using namespace ESPINA::GUI::ColorEngines;

//------------------------------------------------------------------------
CategoryAdapter::CategoryAdapter(CategorySPtr category)
: ItemAdapter{PersistentSPtr()}
, m_category {category}
, m_parent   {nullptr}
{
  // NOTE: parent need to be set by instance creator.
}

//------------------------------------------------------------------------
CategoryAdapter::CategoryAdapter(CategoryAdapterPtr parent, const QString& name)
: ItemAdapter{PersistentSPtr()}
, m_category {nullptr}
, m_parent   {parent}
{
  // NOTE: m_category needs to be set by instance creator.
}

//------------------------------------------------------------------------
CategoryAdapter::~CategoryAdapter()
{
}

//------------------------------------------------------------------------
QVariant CategoryAdapter::data(int role) const
{
  switch (role)
  {
    case Qt::DisplayRole:
    case Qt::EditRole:
      return name();
    case Qt::DecorationRole:
    {
      QPixmap icon(16,16);
      icon.fill(color());
      return icon;
    }
    case TypeRole:
      return typeId(ItemAdapter::Type::CATEGORY);
    default:
      return QVariant();
  }
}

//------------------------------------------------------------------------
bool CategoryAdapter::setData(const QVariant& value, int role)
{
  bool successful = false;

  if (role == Qt::EditRole)
  {
    setName(value.toString());
    successful = true;
  }
  else
    if (role == Qt::DecorationRole)
    {
      setColor(value.value<QColor>());
      successful = true;
    }

  return successful;
}

//------------------------------------------------------------------------
QString CategoryAdapter::classificationName() const
{
  return m_category->classificationName();
}

//------------------------------------------------------------------------
QColor CategoryAdapter::color() const
{
  return selectedColor(m_category->color());
}

//------------------------------------------------------------------------
void CategoryAdapter::setName(const QString& name)
{
  m_category->setName(name);
}

//------------------------------------------------------------------------
QString CategoryAdapter::name() const
{
  return m_category->name();
}

//------------------------------------------------------------------------
void CategoryAdapter::addProperty(const QString& prop, const QVariant& value)
{
  m_category->addProperty(prop, value);
}

//------------------------------------------------------------------------
void CategoryAdapter::addSubCategory(CategoryAdapterSPtr subCategory)
{
  // do not add if already present
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

  m_category->addSubCategory(subCategory->m_category);
}

//------------------------------------------------------------------------
CategoryAdapterSPtr CategoryAdapter::createSubCategory(const QString& name)
{
  CategoryAdapterSPtr subCategory(new CategoryAdapter(this, name));
  subCategory->m_category = m_category->createSubCategory(name);

  m_subCategories << subCategory;

  return subCategory;
}

//------------------------------------------------------------------------
void CategoryAdapter::deleteProperty(const QString& prop)
{
  m_category->deleteProperty(prop);
}

//------------------------------------------------------------------------
QStringList CategoryAdapter::properties() const
{
  return m_category->properties();
}

//------------------------------------------------------------------------
QVariant CategoryAdapter::property(const QString& prop) const
{
  return m_category->property(prop);
}

//------------------------------------------------------------------------
void CategoryAdapter::removeSubCategory(CategoryAdapterPtr subCategory)
{
  CategoryAdapterSPtr subNode = nullptr;

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
    m_category->removeSubCategory(subNode->m_category);
  }
}

//------------------------------------------------------------------------
void CategoryAdapter::setColor(const QColor& color)
{
  m_category->setColor(color.hue());
}

//------------------------------------------------------------------------
CategoryAdapterSPtr CategoryAdapter::subCategory(const QString& name) const
{
  CategoryAdapterSPtr res = nullptr;

  int i = 0;
  while (!res && i < m_subCategories.size())
  {
    if (m_subCategories[i]->name() == name)
    {
      res = m_subCategories[i];
    }

    ++i;
  }

  return res;
}

//------------------------------------------------------------------------
CategoryAdapterPtr ESPINA::toCategoryAdapterPtr(const QModelIndex& index)
{
  return static_cast<CategoryAdapterPtr>(index.internalPointer());
}

//------------------------------------------------------------------------
CategoryAdapterPtr ESPINA::toCategoryAdapterPtr(ItemAdapterPtr item)
{
  return dynamic_cast<CategoryAdapterPtr>(item);
}

//------------------------------------------------------------------------
QString ESPINA::print(CategoryAdapterSPtr category, int indent)
{
  return print(category->m_category, indent);
}
