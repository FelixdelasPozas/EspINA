/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#include "CategorySelector.h"

// Qt
#include "QComboTreeView.h"

using namespace ESPINA;
using namespace ESPINA::GUI::Widgets;

//------------------------------------------------------------------------
CategorySelector::CategorySelector(ModelAdapterSPtr model, QWidget* parent)
: QComboTreeView    {parent}
, m_model           {model}
, m_selectedCategory{nullptr}
{
  setModel(m_model.get());
  setRootModelIndex(m_model->classificationRoot());
  setMinimumHeight(28);

  connect(this, SIGNAL(activated(QModelIndex)),
          this, SLOT(categorySelected(QModelIndex)));

  connect(m_model.get(), SIGNAL(modelAboutToBeReset()),
          this,          SLOT(invalidateState()));
  connect(m_model.get(), SIGNAL(modelReset()),
          this,          SLOT(resetRootItem()));
}

//------------------------------------------------------------------------
void CategorySelector::categorySelected(const QModelIndex& index)
{
  if (!index.isValid())
    return;

  auto item = itemAdapter(index);
  Q_ASSERT(isCategory(item));

  auto category = m_model->smartPointer(categoryPtr(item));

  if (m_selectedCategory != category)
  {
    m_selectedCategory = category;

    emit categoryChanged(m_selectedCategory);
  }
}

//------------------------------------------------------------------------
void CategorySelector::invalidateState()
{
  m_selectedCategory.reset();
  setRootModelIndex(QModelIndex());
}

//------------------------------------------------------------------------
void CategorySelector::resetRootItem()
{
  setRootModelIndex(m_model->classificationRoot());
}

//------------------------------------------------------------------------
void CategorySelector::selectCategory(CategoryAdapterSPtr category)
{
  m_selectedCategory = category;

  setCurrentModelIndex(m_model->categoryIndex(category));
}

//------------------------------------------------------------------------
CategoryAdapterSPtr CategorySelector::selectedCategory()
{
  if (!m_selectedCategory)
  {
    m_selectedCategory = m_model->classification()->categories().first();
  }

  return m_selectedCategory;
}
