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
#include "AddRelationCommand.h"

using namespace ESPINA;

//------------------------------------------------------------------------
AddRelationCommand::AddRelationCommand(ItemAdapterSPtr  ancestor,
                                       ItemAdapterSPtr  successor,
                                       const QString   &relationName,
                                       ModelAdapterSPtr model,
                                       QUndoCommand    *parent)
: QUndoCommand{parent}
, m_ancester  {ancestor}
, m_succesor  {successor}
, m_relation  {relationName}
, m_model     {model}
{
}

//------------------------------------------------------------------------
void AddRelationCommand::redo()
{
  m_model->addRelation(m_ancester, m_succesor, m_relation);
}

//------------------------------------------------------------------------
void AddRelationCommand::undo()
{
  m_model->deleteRelation(m_ancester, m_succesor, m_relation);
}

//------------------------------------------------------------------------
AddRelationsCommand::AddRelationsCommand(RelationList     relations,
                                         ModelAdapterSPtr model,
                                         QUndoCommand    *parent)
: QUndoCommand{parent}
, m_relations {relations}
, m_model     {model}
{
}

//------------------------------------------------------------------------
void AddRelationsCommand::redo()
{
  m_model->addRelations(m_relations);
}

//------------------------------------------------------------------------
void AddRelationsCommand::undo()
{
  m_model->deleteRelations(m_relations);
}

