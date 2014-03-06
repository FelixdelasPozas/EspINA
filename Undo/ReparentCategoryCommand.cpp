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

#include "ReparentCategoryCommand.h"

using namespace EspINA;

//------------------------------------------------------------------------
ReparentCategoryCommand::ReparentCategoryCommand(CategoryAdapterPtr category,
                                                 CategoryAdapterPtr parentCategory,
                                                 ModelAdapterSPtr   model,
                                                 QUndoCommand*      parent)
: QUndoCommand(parent)
, m_model(model)
, m_parent(m_model->smartPointer(parentCategory))
{
  storePreviousParent(category);
}

//------------------------------------------------------------------------
ReparentCategoryCommand::ReparentCategoryCommand(CategoryAdapterList categories,
                                                 CategoryAdapterPtr  parentCategory,
                                                 ModelAdapterSPtr    model,
                                                 QUndoCommand*       parent)
: QUndoCommand    (parent)
, m_model         (model)
, m_parent(m_model->smartPointer(parentCategory))
{
  for(auto category : categories)
  {
    storePreviousParent(category);
  }
}

//------------------------------------------------------------------------
ReparentCategoryCommand::~ReparentCategoryCommand()
{

}


//------------------------------------------------------------------------
void ReparentCategoryCommand::redo()
{
  for(auto category : m_previousParents.keys())
  {
    m_model->reparentCategory(category, m_parent);
  }
}

//------------------------------------------------------------------------
void ReparentCategoryCommand::undo()
{
  for(auto category : m_previousParents.keys())
  {
    m_model->reparentCategory(category, m_previousParents[category]);
  }
}

//------------------------------------------------------------------------
void ReparentCategoryCommand::storePreviousParent(CategoryAdapterPtr category)
{
  auto categorySPtr = m_model->smartPointer(category);
  m_previousParents[categorySPtr] = m_model->smartPointer(category->parent());
}

