/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor<jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
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

#include "CategoryAdapter.h"

// EspINA
#include <Core/Analysis/Category.h>

using namespace EspINA;

//------------------------------------------------------------------------
QVariant CategoryAdapter::data(int role) const
{

}

//------------------------------------------------------------------------
bool CategoryAdapter::setData(const QVariant& value, int role)
{

}

//------------------------------------------------------------------------
QString CategoryAdapter::classificationName() const
{
  return m_category->classificationName();
}

//------------------------------------------------------------------------
QColor CategoryAdapter::color() const
{
  return m_category->color();
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

}

//------------------------------------------------------------------------
void CategoryAdapter::addSubCategory(CategoryAdapterSPtr subCategory)
{
  m_subCategories << subCategory;
}

//------------------------------------------------------------------------
CategoryAdapter::CategoryAdapter(CategorySPtr category)
{

}

//------------------------------------------------------------------------
CategoryAdapter::CategoryAdapter(CategoryAdapterPtr parent, const QString& name)
{

}


//------------------------------------------------------------------------
CategoryAdapterSPtr CategoryAdapter::createSubCategory(const QString& name)
{

}

//------------------------------------------------------------------------
void CategoryAdapter::deleteProperty(const QString& prop)
{

}

//------------------------------------------------------------------------
PersistentSPtr CategoryAdapter::item() const
{

}

//------------------------------------------------------------------------
QStringList CategoryAdapter::properties() const
{

}

//------------------------------------------------------------------------
QVariant CategoryAdapter::property(const QString& prop) const
{

}

//------------------------------------------------------------------------
void CategoryAdapter::removeSubCategory(CategoryAdapterPtr subCategory)
{

}

//------------------------------------------------------------------------
void CategoryAdapter::setColor(const QColor& color)
{

}

//------------------------------------------------------------------------
CategoryAdapterSPtr CategoryAdapter::subCategory(const QString& name) const
{
  CategoryAdapterSPtr res;

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
CategoryAdapter::~CategoryAdapter()
{

}

//------------------------------------------------------------------------
CategoryAdapterPtr EspINA::categoryAdapterPtr(const QModelIndex& index)
{
  return static_cast<CategoryAdapterPtr>(index.internalPointer());
}

//------------------------------------------------------------------------
CategoryAdapterPtr EspINA::categoryAdapterPtr(ItemAdapterPtr item)
{
  return static_cast<CategoryAdapterPtr>(item);
}


QString EspINA::print(CategoryAdapterSPtr category, int level)
{

}