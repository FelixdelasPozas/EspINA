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

#include "AddCategoryCommand.h"

using namespace EspINA;

//------------------------------------------------------------------------
AddCategoryCommand::AddCategoryCommand(CategoryAdapterPtr parentCategory,
                                       CategoryAdapterPtr category,
                                       ModelAdapterSPtr   model,
                                       QUndoCommand*      parent)
: QUndoCommand(parent)
, m_model(model)
, m_name (category->name())
, m_color(category->color())
// , m_category(category)
// , m_parentCategory(parentCategory)
{

}

//------------------------------------------------------------------------
AddCategoryCommand::AddCategoryCommand(CategoryAdapterPtr parentClassification,
                                       const QString&     name,
                                       ModelAdapterSPtr   model,
                                       QColor             color,
                                       QUndoCommand*      parent)
: QUndoCommand(parent)
, m_model(model)
, m_name (name)
, m_color(color)
, m_parentCategory(m_model->smartPointer(parentClassification))
{
}

//------------------------------------------------------------------------
AddCategoryCommand::~AddCategoryCommand()
{

}

//------------------------------------------------------------------------
void AddCategoryCommand::redo()
{
  // if it has been used before, we should use the same object in case other
  // commands keep its reference
  if (!m_category)
  {
    m_category = m_model->createCategory(m_name, m_parentCategory);
    m_category->setColor(m_color);
  }
  else
  {
    m_model->addCategory(m_parentCategory, m_category);
  }
}

//------------------------------------------------------------------------
void AddCategoryCommand::undo()
{
  m_model->removeCategory(m_parentCategory, m_category);
}