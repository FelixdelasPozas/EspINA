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


#include "AddRelation.h"
#include "common/EspinaCore.h"


//------------------------------------------------------------------------
AddRelation::AddRelation(ModelItem* ancestor,
			 ModelItem* successor,
			 const QString description,
			 QUndoCommand* parent)
: QUndoCommand(parent)
, m_ancester(ancestor)
, m_succesor(successor)
, m_description(description)
{
}

//------------------------------------------------------------------------
AddRelation::AddRelation(ModelItemPtr ancestor,
			 ModelItemPtr successor,
			 const QString description,
			 QUndoCommand* parent)
: QUndoCommand(parent)
, m_ancester(ancestor.data())
, m_succesor(successor.data())
, m_description(description)
{

}

//------------------------------------------------------------------------
void AddRelation::redo()
{
  EspinaCore::instance()->model()->addRelation(m_ancester, m_succesor, m_description);
}

//------------------------------------------------------------------------
void AddRelation::undo()
{
  EspinaCore::instance()->model()->removeRelation(m_ancester, m_succesor, m_description);
}
