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

//------------------------------------------------------------------------
CategorySelector::CategorySelector(ModelAdapterSPtr model, QObject* parent)
: QWidgetAction     {parent}
, m_model           {model}
, m_selectedCategory{nullptr}
{
  connect(m_model.get(), SIGNAL(modelAboutToBeReset()),
          this,          SLOT(invalidateState()));
  connect(m_model.get(), SIGNAL(modelReset()),
          this,          SLOT(resetRootItem()));
}

//------------------------------------------------------------------------
QWidget* CategorySelector::createWidget(QWidget* parent)
{
  QComboTreeView *categorySelector = new QComboTreeView(parent);

  categorySelector->setModel(m_model.get());
  categorySelector->setRootModelIndex(m_model->classificationRoot());
  categorySelector->setMinimumHeight(28);

  connect(categorySelector, SIGNAL(activated(QModelIndex)),
          this,             SLOT(categorySelected(QModelIndex)));

  connect(categorySelector, SIGNAL(destroyed(QObject*)),
          this, SLOT(onWidgetDestroyed(QObject *)));

  if (m_selectedCategory)
  {
    categorySelector->setCurrentModelIndex(m_model->categoryIndex(m_selectedCategory));
  }

  m_pool << categorySelector;

  emit widgetCreated();

  return categorySelector;
}

//------------------------------------------------------------------------
void CategorySelector::onWidgetDestroyed(QObject *object)
{
  m_pool.removeOne(object);
}

//------------------------------------------------------------------------
void CategorySelector::categorySelected(const QModelIndex& index)
{
  if (!index.isValid())
    return;

  auto item = itemAdapter(index);
  Q_ASSERT(ItemAdapter::Type::CATEGORY == item->type());

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
  for (auto object : m_pool)
  {
    auto categorySelector = dynamic_cast<QComboTreeView *>(object);
    categorySelector->setRootModelIndex(QModelIndex());
  }
}

//------------------------------------------------------------------------
void CategorySelector::resetRootItem()
{
  for (auto object : m_pool)
  {
    auto categorySelector = dynamic_cast<QComboTreeView *>(object);
    categorySelector->setRootModelIndex(m_model->classificationRoot());
  }
}

//------------------------------------------------------------------------
void CategorySelector::selectCategory(CategoryAdapterSPtr category)
{
  m_selectedCategory = category;

  for (auto object : m_pool)
  {
    auto categorySelector = dynamic_cast<QComboTreeView *>(object);
    categorySelector->setCurrentModelIndex(m_model->categoryIndex(category));
  }
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
