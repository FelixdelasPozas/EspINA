/*
    Copyright (c) 2013, Jorge Peña Pastor <jpena@cesvima.upm.es>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the <organization> nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "UndoableEspinaModel.h"

#include "App/Undo/AtomicModelOperations.h"
#include <Undo/TaxonomiesCommand.h>

using namespace ESPINA;

//---------------------------------------------------------------------------
void UndoableEspinaModel::setTaxonomy(TaxonomySPtr tax)
{
  m_model->setTaxonomy(tax);
}

//---------------------------------------------------------------------------
void UndoableEspinaModel::addTaxonomy(TaxonomySPtr taxonomy)
{
  m_undoStack->push(new AddTaxonomyCommand(taxonomy, m_model));
}

//---------------------------------------------------------------------------
TaxonomyElementSPtr UndoableEspinaModel::createTaxonomyElement(TaxonomyElementPtr parent, const QString &name)
{
  m_undoStack->push(new AddTaxonomyElement(parent, name, m_model, parent->color()));
  return parent->element(name);
}

//---------------------------------------------------------------------------
TaxonomyElementSPtr UndoableEspinaModel::createTaxonomyElement(TaxonomyElementSPtr parent, const QString &name)
{
  m_undoStack->push(new AddTaxonomyElement(parent.get(), name, m_model, parent->color()));
  return parent->element(name);
}

//---------------------------------------------------------------------------
void UndoableEspinaModel::addTaxonomyElement(TaxonomyElementSPtr parent, TaxonomyElementSPtr element)
{
  m_undoStack->push(new AddTaxonomyElement(parent, element, m_model));
}

//---------------------------------------------------------------------------
void UndoableEspinaModel::removeTaxonomyElement(TaxonomyElementSPtr parent, TaxonomyElementSPtr element)
{
  m_undoStack->push(new RemoveTaxonomyElementCommand(element.get(), m_model));
}

//---------------------------------------------------------------------------
void UndoableEspinaModel::addSample(SampleSPtr sample)
{
  m_undoStack->push(new AddSampleCommand(sample, m_model));
}

//---------------------------------------------------------------------------
void UndoableEspinaModel::addSample(SampleSList samples)
{
  m_undoStack->push(new AddSampleCommand(samples, m_model));
}

//---------------------------------------------------------------------------
void UndoableEspinaModel::removeSample(SampleSPtr sample)
{
  m_undoStack->push(new RemoveSampleCommand(sample, m_model));
}

//---------------------------------------------------------------------------
void UndoableEspinaModel::addChannel(ChannelSPtr channel)
{
  m_undoStack->push(new AddChannelCommand(channel, m_model));
}

//---------------------------------------------------------------------------
void UndoableEspinaModel::addChannel(ChannelSList channels)
{
  m_undoStack->push(new AddChannelCommand(channels, m_model));
}

//---------------------------------------------------------------------------
void UndoableEspinaModel::removeChannel(ChannelSPtr channel)
{
  m_undoStack->push(new RemoveChannelCommand(channel, m_model));
}

//---------------------------------------------------------------------------
void UndoableEspinaModel::addSegmentation(SegmentationSPtr segmentation)
{
  m_undoStack->push(new AddSegmentationCommand(segmentation, m_model));
}

//---------------------------------------------------------------------------
void UndoableEspinaModel::addSegmentation(SegmentationSList segmentations)
{
  m_undoStack->push(new AddSegmentationCommand(segmentations, m_model));
}

//---------------------------------------------------------------------------
void UndoableEspinaModel::removeSegmentation(SegmentationSPtr segmentation)
{
  m_undoStack->push(new RemoveSegmentationCommand(segmentation, m_model));
}

//---------------------------------------------------------------------------
void UndoableEspinaModel::removeSegmentation(SegmentationSList segmentations)
{
  m_undoStack->push(new RemoveSegmentationCommand(segmentations, m_model));
}

//---------------------------------------------------------------------------
void UndoableEspinaModel::addFilter(FilterSPtr filter)
{
  m_undoStack->push(new AddFilterCommand(filter, m_model));
}

//---------------------------------------------------------------------------
void UndoableEspinaModel::addFilter(FilterSList filters)
{
  m_undoStack->push(new AddFilterCommand(filters, m_model));
}

//---------------------------------------------------------------------------
void UndoableEspinaModel::removeFilter(FilterSPtr filter)
{
  m_undoStack->push(new RemoveFilterCommand(filter, m_model));
}

//---------------------------------------------------------------------------
void UndoableEspinaModel::addRelation(ModelItemSPtr ancestor, ModelItemSPtr succesor, const QString &relation)
{
  m_undoStack->push(new AddRelationCommand(ancestor, succesor, relation, m_model));
}

//---------------------------------------------------------------------------
void UndoableEspinaModel::removeRelation(ModelItemSPtr ancestor, ModelItemSPtr succesor, const QString &relation)
{
  m_undoStack->push(new RemoveRelationCommand(ancestor, succesor, relation, m_model));
}
