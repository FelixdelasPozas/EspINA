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


#include "MoveTaxonomiesCommand.h"
#include <Core/Model/EspinaModel.h>

using namespace EspINA;

//------------------------------------------------------------------------
MoveTaxonomiesCommand::MoveTaxonomiesCommand(TaxonomyElementList taxonomies,
                                             TaxonomyElementPtr  parentTaxonomy,
                                             EspinaModel         *model,
                                             QUndoCommand        *parent)
: QUndoCommand    (parent)
, m_model         (model)
, m_parentTaxonomy(m_model->findTaxonomyElement(parentTaxonomy))
{
  foreach(TaxonomyElementPtr subTaxonomy, taxonomies)
  {
    TaxonomyElementSPtr key = m_model->findTaxonomyElement(subTaxonomy);
    m_oldTaxonomyParents[key]    = m_model->findTaxonomyElement(subTaxonomy->parent());
  }
}

//------------------------------------------------------------------------
MoveTaxonomiesCommand::~MoveTaxonomiesCommand()
{

}


//------------------------------------------------------------------------
void MoveTaxonomiesCommand::redo()
{
  foreach(TaxonomyElementSPtr subTaxonomy, m_oldTaxonomyParents.keys())
  {
    m_model->changeTaxonomyParent(subTaxonomy, m_parentTaxonomy);
  }
}

//------------------------------------------------------------------------
void MoveTaxonomiesCommand::undo()
{
  foreach(TaxonomyElementSPtr subTaxonomy, m_oldTaxonomyParents.keys())
  {
    m_model->changeTaxonomyParent(subTaxonomy, m_oldTaxonomyParents[subTaxonomy]);
  }
}
