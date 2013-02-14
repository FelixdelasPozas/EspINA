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


#include "ChangeTaxonomyCommand.h"
#include <Core/Model/Segmentation.h>
#include <Core/Model/EspinaModel.h>
#include <GUI/ViewManager.h>

using namespace EspINA;

//------------------------------------------------------------------------
ChangeTaxonomyCommand::ChangeTaxonomyCommand(SegmentationList   segmentations,
                                             TaxonomyElementPtr taxonomy,
                                             EspinaModel       *model,
                                             ViewManager       *viewManager,
                                             QUndoCommand      *parent)
: QUndoCommand(parent)
, m_model(model)
, m_viewManager(viewManager)
, m_taxonomy(m_model->findTaxonomyElement(taxonomy))
{
  foreach(SegmentationPtr segmentation, segmentations)
  {
    SegmentationSPtr key = m_model->findSegmentation(segmentation);
    m_oldTaxonomies[key] = segmentation->taxonomy();
  }
}

//------------------------------------------------------------------------
ChangeTaxonomyCommand::~ChangeTaxonomyCommand()
{

}

//------------------------------------------------------------------------
void ChangeTaxonomyCommand::redo()
{
  SegmentationList segmentations;
  foreach(SegmentationSPtr segmentation, m_oldTaxonomies.keys())
  {
    m_model->changeTaxonomy(segmentation, m_taxonomy);
    segmentations << segmentation.data();
  }
  m_viewManager->updateSegmentationRepresentations(segmentations);
}

//------------------------------------------------------------------------
void ChangeTaxonomyCommand::undo()
{
  SegmentationList segmentations;
  foreach(SegmentationSPtr segmentation, m_oldTaxonomies.keys())
  {
    m_model->changeTaxonomy(segmentation, m_oldTaxonomies[segmentation]);
    segmentations << segmentation.data();
  }
  m_viewManager->updateSegmentationRepresentations(segmentations);
}
