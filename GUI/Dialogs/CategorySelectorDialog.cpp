/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  <copyright holder> <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "CategorySelectorDialog.h"

using namespace EspINA;

//-----------------------------------------------------------------------------
CategorySelectorDialog::CategorySelectorDialog(ModelAdapterSPtr model,
                                               QWidget*         parent,
                                               Qt::WindowFlags  f)
: QDialog(parent, f)
, m_model(model)
{
  setupUi(this);

  m_categories->setModel(model.get());
  m_categories->setRootIndex(model->classificationRoot());
}

//-----------------------------------------------------------------------------
CategoryAdapterSList CategorySelectorDialog::categories()
{
  CategoryAdapterSList selection;

  for(auto index : m_categories->selectionModel()->selectedIndexes())
  {
    auto category = categoryPtr(index);
    selection << m_model->smartPointer(category);
  }

  return selection;
}
