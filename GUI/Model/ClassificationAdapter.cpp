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

}

//------------------------------------------------------------------------
QVariant ClassificationAdapter::data(int role) const
{

}

//------------------------------------------------------------------------
PersistentSPtr ClassificationAdapter::item() const
{
  return PersistentSPtr(); //TODO WARNING 2013-10-06
}

//------------------------------------------------------------------------
CategoryAdapterSPtr ClassificationAdapter::root()
{
}

//------------------------------------------------------------------------
CategoryAdapterSList ClassificationAdapter::categories()
{

}

//------------------------------------------------------------------------
CategoryAdapterSPtr ClassificationAdapter::category(const QString& classificationName)
{

}

//------------------------------------------------------------------------
ClassificationAdapter::ClassificationAdapter(ClassificationSPtr classification)
{

}

//------------------------------------------------------------------------
CategoryAdapterSPtr ClassificationAdapter::createCategory(const QString& relativeName, CategoryAdapterSPtr parent)
{
  
}

//------------------------------------------------------------------------
QString ClassificationAdapter::name() const
{
  return m_classification->name();
}

//------------------------------------------------------------------------
CategoryAdapterSPtr ClassificationAdapter::parent(const CategoryAdapterSPtr categor) const
{

}


//------------------------------------------------------------------------
void ClassificationAdapter::removeCategory(CategorySPtr element)
{

}

//------------------------------------------------------------------------
void ClassificationAdapter::setName(const QString& name)
{

}

//------------------------------------------------------------------------
QString EspINA::print(ClassificationSPtr classification, int indent)
{

}
