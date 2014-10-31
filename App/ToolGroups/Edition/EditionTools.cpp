/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#include "EditionTools.h"

#include <App/Undo/BrushUndoCommand.h>
#include <Core/Analysis/Filter.h>
#include <Core/IO/FetchBehaviour/MarchingCubesFromFetchedVolumetricData.h>
#include <Filters/FreeFormSource.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <Undo/AddSegmentations.h>
#include <Undo/RemoveSegmentations.h>

using ESPINA::Filter;

const Filter::Type FREEFORM_FILTER    = "FreeFormSource";
const Filter::Type FREEFORM_FILTER_V4 = "EditorToolBar::FreeFormSource";

using namespace ESPINA;
using namespace ESPINA::GUI;

//-----------------------------------------------------------------------------
FilterTypeList EditionTools::ManualFilterFactory::providedFilters() const
{
  FilterTypeList filters;

  filters << FREEFORM_FILTER << FREEFORM_FILTER_V4;

  return filters;
}

//-----------------------------------------------------------------------------
FilterSPtr EditionTools::ManualFilterFactory::createFilter(InputSList         inputs,
                                                                const Filter::Type& filter,
                                                                SchedulerSPtr       scheduler) const
throw(Unknown_Filter_Exception)
{
  if (FREEFORM_FILTER != filter && FREEFORM_FILTER_V4 != filter) throw Unknown_Filter_Exception();

  auto ffsFilter = FilterSPtr{new FreeFormSource(inputs, FREEFORM_FILTER, scheduler)};
  if (!m_fetchBehaviour)
  {
    m_fetchBehaviour = DataFactorySPtr{new MarchingCubesFromFetchedVolumetricData()};
  }
  ffsFilter->setDataFactory(m_fetchBehaviour);

  return ffsFilter;
}

//-----------------------------------------------------------------------------
EditionTools::EditionTools(ModelAdapterSPtr model,
                           ModelFactorySPtr factory,
                           ViewManagerSPtr  viewManager,
                           QUndoStack      *undoStack,
                           QWidget         *parent)
: ToolGroup      {viewManager, QIcon(":/espina/pencil.png"), tr("Edition Tools"), parent}
, m_factory      {factory}
, m_undoStack    {undoStack}
, m_model        {model}
, m_filterFactory{new ManualFilterFactory()}
{
  m_factory->registerFilterFactory(m_filterFactory);

  m_manualEdition = ManualEditionToolSPtr(new ManualEditionTool(model, viewManager));
  connect(m_manualEdition.get(), SIGNAL(stroke(CategoryAdapterSPtr, BinaryMaskSPtr<unsigned char>)),
          this,                  SLOT(drawStroke(CategoryAdapterSPtr, BinaryMaskSPtr<unsigned char>)), Qt::DirectConnection);
  connect(m_manualEdition.get(), SIGNAL(stopDrawing(ViewItemAdapterPtr, bool)),
          this,                  SLOT(onEditionFinished(ViewItemAdapterPtr,bool)));


  m_split = SplitToolSPtr(new SplitTool(model, factory, viewManager, undoStack));
  m_morphological = MorphologicalEditionToolSPtr(new MorphologicalEditionTool(model, factory, viewManager, undoStack));

  connect(m_viewManager->selection().get(), SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
  connect(parent, SIGNAL(abortOperation()), this, SLOT(abortOperation()));
  connect(parent, SIGNAL(analysisClosed()), this, SLOT(abortOperation()));
}

//-----------------------------------------------------------------------------
EditionTools::~EditionTools()
{
  abortOperation();

  disconnect(m_viewManager.get());
  disconnect(this->parent());
}

//-----------------------------------------------------------------------------
void EditionTools::setEnabled(bool value)
{
  m_enabled = value;

  m_manualEdition->setEnabled(value);
  m_split        ->setEnabled(value);
  m_morphological->setEnabled(value);
}

//-----------------------------------------------------------------------------
bool EditionTools::enabled() const
{
  return m_enabled;
}

//-----------------------------------------------------------------------------
ToolSList EditionTools::tools()
{
  m_manualEdition->setEnabled(true);
  m_split        ->setEnabled(true);
  m_morphological->setEnabled(true);

  selectionChanged();

  ToolSList availableTools;
  availableTools << m_manualEdition;
  availableTools << m_split;
  availableTools << m_morphological;

  return availableTools;
}

//-----------------------------------------------------------------------------
void EditionTools::selectionChanged()
{
  int listSize = m_viewManager->selection()->segmentations().size();

  bool noSegmentation         = listSize == 0;
  bool onlyOneSegmentation    = listSize == 1;
  m_manualEdition->setEnabled(noSegmentation || onlyOneSegmentation);
  m_split        ->setEnabled(onlyOneSegmentation);
  // NOTE: morphological tools manage selection on their own, as it's tools
  // haven't a unique requirement.
}

//-----------------------------------------------------------------------------
void EditionTools::abortOperation()
{
  m_manualEdition->abortOperation();
  m_split        ->abortOperation();
}

//-----------------------------------------------------------------------------
void EditionTools::drawStroke(CategoryAdapterSPtr category, BinaryMaskSPtr<unsigned char> mask)
{
  ManualEditionToolPtr tool = qobject_cast<ManualEditionToolPtr>(sender());

  SegmentationAdapterSPtr segmentation;
  auto selection = m_viewManager->selection();
  if(selection->items().empty())
  {
    ChannelAdapterList primaryChannel;
    primaryChannel << m_viewManager->activeChannel();
    selection->set(primaryChannel);
  }

  if(!selection->segmentations().empty())
  {
    auto item = selection->segmentations().first();
    segmentation = m_model->smartPointer(reinterpret_cast<SegmentationAdapterPtr>(item));
    m_undoStack->beginMacro(tr("Modify Segmentation"));
    m_undoStack->push(new DrawUndoCommand(segmentation, mask));
    m_undoStack->endMacro();
  }
  else if(!selection->channels().empty())
  {
    InputSList inputs;
    inputs << m_viewManager->activeChannel()->asInput();

    auto filter = m_factory->createFilter<FreeFormSource>(inputs, FREEFORM_FILTER);
    filter->setMask(mask);

    segmentation = m_factory->createSegmentation(filter, 0);
    segmentation->setCategory(category);

    auto item = selection->channels().first();
    auto channelItem = static_cast<ChannelAdapterPtr>(item);

    SampleAdapterSList samples;
    samples << QueryAdapter::sample(channelItem);
    Q_ASSERT(channelItem && (samples.size() == 1));

    m_undoStack->beginMacro(tr("Add Segmentation"));
    m_undoStack->push(new AddSegmentations(segmentation, samples, m_model));
    m_undoStack->endMacro();

    SegmentationAdapterList list;
    list << segmentation.get();
    m_viewManager->selection()->clear();
    m_viewManager->selection()->set(list);
    tool->updateReferenceItem();
  }
  else
  {
    Q_ASSERT(false);
  }

  m_viewManager->updateSegmentationRepresentations(segmentation.get());
  m_viewManager->updateViews();
}

//-----------------------------------------------------------------------------
void EditionTools::onEditionFinished(ViewItemAdapterPtr item, bool eraserModeEntered)
{
  if (eraserModeEntered && item && isSegmentation(item))
  {
    auto segmentation = segmentationPtr(item);

    auto volume = volumetricData(segmentation->output());

    if (volume->isEmpty())
    {
      auto name = segmentation->data(Qt::DisplayRole).toString();
      DefaultDialogs::InformationMessage(tr("Deleting segmentation"),
                                         tr("%1 has been deleted because all its voxels where erased.").arg(name));

      m_undoStack->blockSignals(true);
      do
      {
        m_undoStack->undo();
      }
      while (volume->isEmpty());
      m_undoStack->blockSignals(false);

      m_undoStack->beginMacro("Remove Segmentation");
      m_undoStack->push(new RemoveSegmentations(segmentation, m_model));
      m_undoStack->endMacro();
    }
    else
    {
      fitToContents(volume, SEG_BG_VALUE);
    }
  }
}
