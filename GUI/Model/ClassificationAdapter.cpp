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

#include "ClassificationAdapter.h"

// EspINA

using namespace EspINA;

//------------------------------------------------------------------------
ClassificationAdapter::ClassificationAdapter(const QString& name)
: m_classification{new Tree<Category>(name)}
, m_adaptedClassification(name)
{
  m_adaptedClassification.root()->m_category = m_classification->root();
}

//------------------------------------------------------------------------
ClassificationAdapter::~ClassificationAdapter()
{

}

//------------------------------------------------------------------------
bool ClassificationAdapter::setData(const QVariant& value, int role)
{
  if (role == Qt::EditRole)
  {
    setName(value.toString());
    return true;
  }
  if (role == Qt::DecorationRole)
  {
    m_adaptedClassification.root()->setData(value, role);
    return true;
  }
  return false;
}

//------------------------------------------------------------------------
QVariant ClassificationAdapter::data(int role) const
{
  switch (role)
  {
    case Qt::DisplayRole:
    case Qt::EditRole:
      return name();
    case Qt::DecorationRole:
    {
      return m_adaptedClassification.root()->data(role);
    }
    case TypeRole:
      return typeId(ItemAdapter::Type::CLASSIFICATION);
    default:
      return QVariant();
  }
}

//------------------------------------------------------------------------
CategoryAdapterSPtr ClassificationAdapter::root()
{
  return m_adaptedClassification.root();
}

//------------------------------------------------------------------------
CategoryAdapterSList ClassificationAdapter::categories()
{
  return m_adaptedClassification.root()->subCategories();
}

//------------------------------------------------------------------------
CategoryAdapterSPtr ClassificationAdapter::category(const QString& classificationName)
{
  return m_adaptedClassification.node(classificationName);
}

//------------------------------------------------------------------------
ClassificationAdapter::ClassificationAdapter(ClassificationSPtr classification)
{
}

//------------------------------------------------------------------------
CategoryAdapterSPtr ClassificationAdapter::createCategory(const QString& relativeName, CategoryAdapterSPtr parent)
{
  CategoryAdapterPtr parentCategoryAdapter = parent.get();

  if (!parentCategoryAdapter)
  {
    parentCategoryAdapter = m_adaptedClassification.root().get();
  }

  CategoryPtr parentCategory = parentCategoryAdapter->m_category.get();

  CategorySPtr        requestedCategory;
  CategoryAdapterSPtr requestedCategoryAdapter;

  if (!relativeName.isEmpty())
  {
    QStringList path = relativeName.split("/", QString::SkipEmptyParts);
    for (int i = 0; i < path.size(); ++i)
    {
      requestedCategory        = parentCategory->subCategory(path.at(i));
      requestedCategoryAdapter = parentCategoryAdapter->subCategory(path.at(i));

      if (i == path.size() - 1 && requestedCategory != nullptr) {
        throw Already_Defined_Node_Exception();
      }

      if (!requestedCategory)
      {
        requestedCategory        = parentCategory->createSubCategory(path.at(i));
        requestedCategoryAdapter = CategoryAdapterSPtr{new CategoryAdapter(requestedCategory)};
        parentCategoryAdapter->addSubCategory(requestedCategoryAdapter);
      }

      parentCategory        = requestedCategory.get();
      parentCategoryAdapter = requestedCategoryAdapter.get();
    }
  } else {
    QString defaultName{"Undefined"};

    requestedCategory = parentCategory->subCategory(defaultName);

    if (!requestedCategory)
    {
      requestedCategory        = parentCategory->createSubCategory(defaultName);
      requestedCategoryAdapter = CategoryAdapterSPtr{new CategoryAdapter(requestedCategory)};
      parentCategoryAdapter->addSubCategory(requestedCategoryAdapter);
    }
  }

  return requestedCategoryAdapter;
}

//------------------------------------------------------------------------
QString ClassificationAdapter::name() const
{
  return m_classification->name();
}

//------------------------------------------------------------------------
CategoryAdapterSPtr ClassificationAdapter::parent(const CategoryAdapterSPtr category) const
{
  return m_adaptedClassification.parent(category);
}


//------------------------------------------------------------------------
void ClassificationAdapter::removeCategory(CategoryAdapterSPtr element)
{
  m_classification->removeNode(element->m_category);
  m_adaptedClassification.removeNode(element);
}

//------------------------------------------------------------------------
void ClassificationAdapter::setName(const QString& name)
{
  m_classification->setName(name);
}

//------------------------------------------------------------------------
QString EspINA::print(ClassificationAdapterSPtr classification, int indent)
{
  QString out = QString("Classification: %1\n").arg(classification->name());

  for(auto category : classification->categories())
  {
    out += QString("%1").arg(print(category, indent));
  }

  return out;
}
