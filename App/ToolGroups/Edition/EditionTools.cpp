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

// ESPINA
#include "EditionTools.h"
#include <App/Undo/BrushUndoCommand.h>
#include <Core/Analysis/Filter.h>
#include <Core/Analysis/Output.h>
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.hxx>
#include <Core/IO/DataFactory/MarchingCubesFromFetchedVolumetricData.h>
#include <Filters/SourceFilter.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <Support/Utils/SelectionUtils.h>
#include <Undo/AddSegmentations.h>
#include <Undo/ModifyDataCommand.h>
#include <Undo/RemoveSegmentations.h>
#include <Undo/ContourUndoCommand.h>

// Qt
#include <QApplication>

using ESPINA::Filter;

const Filter::Type SOURCE_FILTER    = "FreeFormSource";
const Filter::Type SOURCE_FILTER_V4 = "EditorToolBar::FreeFormSource";

using namespace ESPINA;
using namespace ESPINA::GUI;

//-----------------------------------------------------------------------------
FilterTypeList EditionTools::ManualFilterFactory::providedFilters() const
{
  FilterTypeList filters;

  filters << SOURCE_FILTER << SOURCE_FILTER_V4;

  return filters;
}

//-----------------------------------------------------------------------------
FilterSPtr EditionTools::ManualFilterFactory::createFilter(InputSList          inputs,
                                                           const Filter::Type& filter,
                                                           SchedulerSPtr       scheduler) const
throw(Unknown_Filter_Exception)
{
  if (!providedFilters().contains(filter)) throw Unknown_Filter_Exception();

  auto ffsFilter = std::make_shared<SourceFilter>(inputs, SOURCE_FILTER, scheduler);

  if (!m_dataFactory)
  {
    m_dataFactory = std::make_shared<MarchingCubesFromFetchedVolumetricData>();
  }

  ffsFilter->setDataFactory(m_dataFactory);

  return ffsFilter;
}

//-----------------------------------------------------------------------------
EditionTools::EditionTools(ModelAdapterSPtr          model,
                           ModelFactorySPtr          factory,
                           FilterDelegateFactorySPtr filterDelegateFactory,
                           ViewManagerSPtr           viewManager,
                           QUndoStack               *undoStack,
                           QWidget                  *parent)
: ToolGroup      {viewManager, QIcon(":/espina/pencil.png"), tr("Edition Tools"), parent}
, m_factory      {factory}
, m_undoStack    {undoStack}
, m_model        {model}
, m_filterFactory{new ManualFilterFactory()}
{
  m_factory->registerFilterFactory(m_filterFactory);

  m_manualEdition = std::make_shared<ManualEditionTool>(model, viewManager);

  connect(m_manualEdition.get(), SIGNAL(stroke(CategoryAdapterSPtr, BinaryMaskSPtr<unsigned char>)),
          this,                  SLOT(drawStroke(CategoryAdapterSPtr, BinaryMaskSPtr<unsigned char>)), Qt::DirectConnection);
  connect(m_manualEdition.get(), SIGNAL(stopDrawing(ViewItemAdapterPtr, bool)),
          this,                  SLOT(onEditionFinished(ViewItemAdapterPtr,bool)));
  connect(m_manualEdition.get(), SIGNAL(drawContours(ContourWidget::ContourList)),
          this,                  SLOT(drawContours(ContourWidget::ContourList)));
  connect(m_manualEdition.get(), SIGNAL(contourEnded(SegmentationAdapterPtr, CategoryAdapterSPtr)),
          this,                  SLOT(endContour(SegmentationAdapterPtr, CategoryAdapterSPtr)));

  m_split = std::make_shared<SplitTool>(model, factory, viewManager, undoStack);
  m_morphological = std::make_shared<MorphologicalEditionTool>(model, factory, filterDelegateFactory, viewManager, undoStack);

  connect(m_viewManager->selection().get(), SIGNAL(selectionChanged()),
          this,                             SLOT(selectionChanged()));

  connect(parent, SIGNAL(abortOperation()),
          this,   SLOT(abortOperation()));
  connect(parent, SIGNAL(analysisClosed()),
          this,   SLOT(abortOperation()));
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
  auto selection     = m_viewManager->selection()->segmentations();
  auto selectionSize = selection.size();

  SegmentationAdapterSPtr selectedSeg;
  auto noSegmentation      = (selectionSize == 0);
  auto onlyOneSegmentation = (selectionSize == 1);
  auto hasRequiredData     = false;

  if(onlyOneSegmentation)
  {
    auto selectedSegmentation = selection.first();
    hasRequiredData = hasVolumetricData(selectedSegmentation->output());
  }

  m_manualEdition->setEnabled(noSegmentation || onlyOneSegmentation);
  m_split        ->setEnabled(onlyOneSegmentation && hasRequiredData);
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
  auto tool      = qobject_cast<ManualEditionToolPtr>(sender());
  auto selection = m_viewManager->selection();

  if(selection->items().empty())
  {
    ChannelAdapterList primaryChannel;
    primaryChannel << m_viewManager->activeChannel();
    selection->set(primaryChannel);
  }

  SegmentationAdapterSPtr segmentation;
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
    auto item    = selection->channels().first();
    auto channel = static_cast<ChannelAdapterPtr>(item);
    auto output  = channel->output();

    auto filter = m_factory->createFilter<SourceFilter>(InputSList(), SOURCE_FILTER);

    auto strokeBounds  = mask->bounds().bounds();
    auto strokeSpacing = output->spacing();
    auto strokeOrigin  = channel->position();

    auto volume = std::make_shared<SparseVolume<itkVolumeType>>(strokeBounds, strokeSpacing, strokeOrigin);
    volume->draw(mask);

    auto mesh = std::make_shared<MarchingCubesMesh<itkVolumeType>>(volume);

    filter->addOutputData(0, volume);
    filter->addOutputData(0, mesh);

    segmentation = m_factory->createSegmentation(filter, 0);
    segmentation->setCategory(category);


    SampleAdapterSList samples;
    samples << QueryAdapter::sample(channel);
    Q_ASSERT(channel && (samples.size() == 1));

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
      m_undoStack->blockSignals(true);
      do
      {
        m_undoStack->undo();
      }
      while(volume->isEmpty());
      m_undoStack->blockSignals(false);

      if(segmentation->output()->numberOfDatas() == 1)
      {
        auto name = segmentation->data(Qt::DisplayRole).toString();
        DefaultDialogs::InformationMessage(tr("Deleting segmentation"),
                                           tr("%1 will be deleted because all its voxels were erased.").arg(name));

        m_undoStack->beginMacro("Remove Segmentation");
        m_undoStack->push(new RemoveSegmentations(segmentation, m_model));
      }
      else
      {
        auto output = segmentation->output();
        m_undoStack->beginMacro("Remove Segmentation's volume");
        m_undoStack->push(new RemoveDataCommand(output, VolumetricData<itkVolumeType>::TYPE));
      }
      m_undoStack->endMacro();
    }
    else
    {
      fitToContents(volume, SEG_BG_VALUE);
    }
  }
}

//-----------------------------------------------------------------------------
void EditionTools::drawContours(ContourWidget::ContourList contours)
{
  bool reduction = false;

  ContourWidget::ContourData contour;
  SegmentationAdapterSPtr segmentation;

  for(auto contourData: contours)
  {
    if(contourData.polyData)
    {
      contour = contourData;
      if(contour.mode == BrushSelector::BrushMode::ERASER)
      {
        reduction = true;
      }
    }

    break;
  }

  if (!contour.polyData || contour.polyData->GetNumberOfPoints() == 0)
    return;

  Q_ASSERT(m_undoStack->index() >= 1);
  const QUndoCommand *command = m_undoStack->command(m_undoStack->index()-1);

  QApplication::setOverrideCursor(Qt::WaitCursor);
  if (command->text() == QString("Draw segmentation using contours"))
  {
    const ContourAddSegmentation *addCommand = dynamic_cast<const ContourAddSegmentation *>(command->child(0));
    addCommand->rasterizeContour(contour);

    segmentation = addCommand->getCreatedSegmentation();
  }
  else
    if (command->text() == QString("Modify segmentation using contours"))
    {
      const ContourUndoCommand *undoCommand = dynamic_cast<const ContourUndoCommand *>(command->child(0));
      undoCommand->rasterizeContour(contour);
    }
    else
      Q_ASSERT(false);

  if (reduction)
  {
    auto volume = volumetricData(segmentation->output());

    if(!volume->isEmpty())
    {
      fitToContents(volume, SEG_BG_VALUE);
    }
    else
    {
      m_undoStack->push(new RemoveSegmentations(segmentation.get(), m_model));
      m_manualEdition->resetContourTool();
    }
  }

  QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
void EditionTools::endContour(SegmentationAdapterPtr segmentation, CategoryAdapterSPtr category)
{
  qDebug() << "editionTools::endContour";
  if(!segmentation)
  {
    m_undoStack->beginMacro("Draw segmentation using contours");
    m_undoStack->push(new ContourAddSegmentation(m_viewManager->activeChannel(),
                                                 category,
                                                 m_model,
                                                 m_factory,
                                                 m_viewManager,
                                                 m_manualEdition));
  }
  else
  {
    m_undoStack->beginMacro("Modify segmentation using contours");
    m_undoStack->push(new ContourUndoCommand(segmentation, m_viewManager, m_manualEdition));
  }

  m_undoStack->endMacro();
}
