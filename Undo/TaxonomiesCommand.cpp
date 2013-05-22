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


#include "TaxonomiesCommand.h"
#include <Core/Model/EspinaModel.h>

using namespace EspINA;

//------------------------------------------------------------------------
AddTaxonomyCommand::AddTaxonomyCommand(TaxonomySPtr taxonomy,
                                       EspinaModel  *model,
                                       QUndoCommand *parent)
: QUndoCommand(parent)
, m_model(model)
, m_prevTaxonomy(taxonomy)
{

}

//------------------------------------------------------------------------
void AddTaxonomyCommand::redo()
{
  swapTaxonomy();
}

//------------------------------------------------------------------------
void AddTaxonomyCommand::undo()
{
  swapTaxonomy();
}

//------------------------------------------------------------------------
void AddTaxonomyCommand::swapTaxonomy()
{
  TaxonomySPtr tmp = m_model->taxonomy();
  m_model->setTaxonomy(m_prevTaxonomy);
  m_prevTaxonomy = tmp;
}


//------------------------------------------------------------------------
AddTaxonomyElement::AddTaxonomyElement(TaxonomyElementSPtr parentTaxonomy,
                                       TaxonomyElementSPtr element,
                                       EspinaModel        *model,
                                       QUndoCommand       *parent)
: QUndoCommand(parent)
, m_model(model)
, m_name (element->name())
, m_color(element->color())
, m_taxonomy(element)
, m_parentTaxonomy(parentTaxonomy)
{

}

//------------------------------------------------------------------------
AddTaxonomyElement::AddTaxonomyElement(TaxonomyElementPtr parentTaxonomy,
                                       const QString     &name,
                                       EspinaModel       *model,
                                       QColor             color,
                                       QUndoCommand      *parent)
: QUndoCommand(parent)
, m_model(model)
, m_name (name)
, m_color(color)
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
  if (!m_taxonomy)
  {
    m_taxonomy = m_model->createTaxonomyElement(m_parentTaxonomy, m_name);
    m_taxonomy->setColor(m_color);
  }
  else
  {
    m_model->addTaxonomyElement(m_parentTaxonomy, m_taxonomy);
  }
}

//------------------------------------------------------------------------
void AddTaxonomyElement::undo()
{
  m_model->removeTaxonomyElement(m_parentTaxonomy, m_taxonomy);
}

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