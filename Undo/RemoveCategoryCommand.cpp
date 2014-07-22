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


#include "RemoveCategoryCommand.h"

using namespace ESPINA;

//------------------------------------------------------------------------
RemoveCategoryCommand::RemoveCategoryCommand(CategoryAdapterPtr category,
                                             ModelAdapterSPtr   model,
                                             QUndoCommand*      parent)
: QUndoCommand(parent)
, m_model   {model}
, m_category{m_model->smartPointer(category)}
, m_parent  {m_model->smartPointer(category->parent())}
{
}

//------------------------------------------------------------------------
RemoveCategoryCommand::~RemoveCategoryCommand()
{
}

//------------------------------------------------------------------------
void RemoveCategoryCommand::redo()
{
  m_model->removeCategory(m_category, m_parent);
}

//------------------------------------------------------------------------
void RemoveCategoryCommand::undo()
{
  m_model->addCategory(m_category, m_parent);
}
