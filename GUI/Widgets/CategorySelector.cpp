/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#include "CategorySelector.h"

#include "QComboTreeView.h"

using namespace EspINA;

//------------------------------------------------------------------------
CategorySelector::CategorySelector(ModelAdapterSPtr model, QObject* parent)
: QWidgetAction{parent}
, m_model{model}
, m_selectedCategory{nullptr}
{
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
//   connect(m_model.get(),    SIGNAL(taxonomyAdded(TaxonomySPtr)),
//           this,  SLOT(updateTaxonomy(TaxonomySPtr)));
  connect(m_model.get(), SIGNAL(modelReset()),
          this,          SLOT(resetRootItem()));

  connect(categorySelector, SIGNAL(destroyed(QObject*)),
          this, SLOT(onWidgetDestroyed(QObject *)));

  categorySelected(categorySelector->currentModelIndex());

  m_pool << categorySelector;

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

  auto category = categoryPtr(item);

  m_selectedCategory = m_model->smartPointer(category);
  emit categoryChanged(m_selectedCategory);
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
