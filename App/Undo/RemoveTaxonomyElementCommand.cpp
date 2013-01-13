/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2013  <copyright holder> <email>

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


#include "RemoveTaxonomyElementCommand.h"
#include <Core/Model/EspinaModel.h>

using namespace EspINA;

//------------------------------------------------------------------------
RemoveTaxonomyElementCommand::RemoveTaxonomyElementCommand(TaxonomyElementPtr taxonomy,
                                                           EspinaModel       *model,
                                                           QUndoCommand      *parent)
: QUndoCommand(parent)
, m_model(model)
, m_taxonomy(m_model->findTaxonomyElement(taxonomy))
, m_parent  (m_model->findTaxonomyElement(m_taxonomy->parent()))
{
}

//------------------------------------------------------------------------
RemoveTaxonomyElementCommand::~RemoveTaxonomyElementCommand()
{

}

//------------------------------------------------------------------------
void RemoveTaxonomyElementCommand::redo()
{
  m_model->removeTaxonomyElement(m_parent, m_taxonomy);
}

//------------------------------------------------------------------------
void RemoveTaxonomyElementCommand::undo()
{
  m_model->addTaxonomyElement(m_parent, m_taxonomy);
}
