/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include "AddCategoryCommand.h"

// Qt
#include <QVariant>

using namespace ESPINA;

//------------------------------------------------------------------------
AddCategoryCommand::AddCategoryCommand(CategoryAdapterSPtr parentCategory,
                                       CategoryAdapterSPtr category,
                                       ModelAdapterSPtr    model,
                                       QUndoCommand*       parent)
: QUndoCommand    {parent}
, m_model         {model}
, m_name          {category->name()}
, m_color         {category->color()}
, m_category      {category}
, m_parentCategory{parentCategory}
, m_roiSize       {Vector3<long long>{500,500,500}}
{
}

//------------------------------------------------------------------------
AddCategoryCommand::AddCategoryCommand(CategoryAdapterSPtr parentCategory,
                                       const QString&      name,
                                       ModelAdapterSPtr    model,
                                       QColor              color,
                                       QUndoCommand*       parent)
: QUndoCommand    {parent}
, m_model         {model}
, m_name          {name}
, m_color         {color}
, m_category      {nullptr}
, m_parentCategory{parentCategory}
, m_roiSize       {Vector3<long long>{500,500,500}}
{
}

//------------------------------------------------------------------------
AddCategoryCommand::AddCategoryCommand(CategoryAdapterSPtr parentCategory,
                                       const QString&      name,
                                       ModelAdapterSPtr    model,
                                       QColor              color,
                                       Vector3<long long>  roi,
                                       QUndoCommand*       parent)
: QUndoCommand    {parent}
, m_model         {model}
, m_name          {name}
, m_color         {color}
, m_category      {nullptr}
, m_parentCategory{parentCategory}
, m_roiSize       {roi}
{
  if(roi[0] <= 0 || roi[1] <= 0 || roi[2] <= 0)
  {
    m_roiSize = Vector3<long long>{500,500,500};
  }
}

//------------------------------------------------------------------------
AddCategoryCommand::~AddCategoryCommand()
{
}

//------------------------------------------------------------------------
void AddCategoryCommand::redo()
{
  if (m_category == nullptr)
  {
    m_category = m_model->createCategory(m_name, m_parentCategory);
    m_category->setColor(m_color);
    m_category->addProperty(Category::DIM_X(), m_roiSize[0]);
    m_category->addProperty(Category::DIM_Y(), m_roiSize[1]);
    m_category->addProperty(Category::DIM_Z(), m_roiSize[2]);
  }
  else
  {
    m_model->addCategory(m_category, m_parentCategory);
  }
}

//------------------------------------------------------------------------
void AddCategoryCommand::undo()
{
  m_model->removeCategory(m_category, m_parentCategory);
}
