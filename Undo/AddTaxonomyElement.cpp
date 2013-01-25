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


#include "AddTaxonomyElement.h"
#include <Core/Model/EspinaModel.h>

using namespace EspINA;

//------------------------------------------------------------------------
AddTaxonomyElement::AddTaxonomyElement(TaxonomyElementPtr parentTaxonomy,
                                       const QString     &name,
                                       EspinaModel       *model,
                                       QUndoCommand      *parent)
: QUndoCommand(parent)
, m_model(model)
, m_name (name)
, m_parentTaxonomy(m_model->findTaxonomyElement(parentTaxonomy))
{
}

//------------------------------------------------------------------------
AddTaxonomyElement::~AddTaxonomyElement()
{

}

//------------------------------------------------------------------------
void AddTaxonomyElement::redo()
{
  // if it has been used before, we should use the same object in case other
  // commands keep its reference
  if (m_taxonomy.isNull())
  {
    m_taxonomy = m_model->createTaxonomyElement(m_parentTaxonomy, m_name);
  } else
  {
    m_model->addTaxonomyElement(m_parentTaxonomy, m_taxonomy);
  }
}

//------------------------------------------------------------------------
void AddTaxonomyElement::undo()
{
  m_model->removeTaxonomyElement(m_parentTaxonomy, m_taxonomy);
}
