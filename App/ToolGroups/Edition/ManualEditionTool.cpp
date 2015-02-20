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
#include "ManualEditionTool.h"
#include <Undo/BrushUndoCommand.h>

#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.hxx>
#include <Core/IO/DataFactory/MarchingCubesFromFetchedVolumetricData.h>
#include <GUI/Model/CategoryAdapter.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/Widgets/SliderAction.h>
#include <GUI/Widgets/DrawingWidget.h>
#include <Support/Settings/EspinaSettings.h>
#include <Filters/SourceFilter.h>
#include <Undo/AddSegmentations.h>

// Qt
#include <QAction>
#include <QUndoStack>

using ESPINA::Filter;

const Filter::Type SOURCE_FILTER    = "FreeFormSource";
const Filter::Type SOURCE_FILTER_V4 = "EditorToolBar::FreeFormSource";

using namespace ESPINA;

//-----------------------------------------------------------------------------
FilterTypeList ManualEditionTool::ManualFilterFactory::providedFilters() const
{
  FilterTypeList filters;

  filters << SOURCE_FILTER << SOURCE_FILTER_V4;

  return filters;
}

//-----------------------------------------------------------------------------
FilterSPtr ManualEditionTool::ManualFilterFactory::createFilter(InputSList          inputs,
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

//------------------------------------------------------------------------
ManualEditionTool::ManualEditionTool(ModelAdapterSPtr model,
                                     ModelFactorySPtr factory,
                                     QUndoStack      *undoStack,
                                     ViewManagerSPtr  viewManager)
: m_model               {model}
, m_factory             {factory}
, m_undoStack           {undoStack}
, m_viewManager         {viewManager}
, m_filterFactory       {new ManualFilterFactory()}
, m_drawingWidget       {model, viewManager}
, m_mode                {Mode::CREATION}
, m_referenceItem       {nullptr}
, m_enabled             {false}
{
  qRegisterMetaType<ViewItemAdapterPtr>("ViewItemAdapterPtr");
  //qRegisterMetaType<CategoryAdapterSPtr>("CategoryAdapterSPtr");
  //qRegisterMetaType<BinaryMaskSPtr<unsigned char>>("BinaryMaskSPtr<unsigned char>");

  m_factory->registerFilterFactory(m_filterFactory);

  connect(m_viewManager->selection().get(), SIGNAL(selectionChanged()),
          this,                             SLOT(updateReferenceItem()));

  connect(&m_drawingWidget, SIGNAL(strokeStarted(BrushSPtr, RenderView*)),
          this,             SLOT(onStrokeStarted(BrushSPtr, RenderView*)));

  connect(&m_drawingWidget, SIGNAL(maskPainted(BinaryMaskSPtr<unsigned char>)),
          this,             SLOT(onMaskCreated(BinaryMaskSPtr<unsigned char>)));
}

//------------------------------------------------------------------------
ManualEditionTool::~ManualEditionTool()
{
}

//------------------------------------------------------------------------
void ManualEditionTool::setEnabled(bool value)
{
  m_enabled = value;
//   m_categorySelector->setEnabled(value);
//   m_drawToolSelector->setEnabled(value);
//   m_radiusWidget->setEnabled(value);
}

//------------------------------------------------------------------------
bool ManualEditionTool::enabled() const
{
  return m_enabled;
}

//------------------------------------------------------------------------
QList<QAction *> ManualEditionTool::actions() const
{
  updateReferenceItem();

  return m_drawingWidget.actions();
}

//------------------------------------------------------------------------
void ManualEditionTool::drawStroke(Selector::Selection selection)
{
//   auto mask = selection.first().first;
//   auto category = m_categorySelector->selectedCategory();
//   emit stroke(category, mask);
}

//------------------------------------------------------------------------
void ManualEditionTool::abortOperation()
{
 // unsetSelector();
}


//------------------------------------------------------------------------
void ManualEditionTool::updateReferenceItem()
{
  ViewItemAdapterPtr currentItem = m_referenceItem;

  m_mode          = Mode::CREATION;
  m_referenceItem = nullptr;

  auto selection     = m_viewManager->selection();
  auto segmentations = selection->segmentations();

  if (segmentations.size() > 1) return;

  if (segmentations.size() == 1)
  {
    auto segmentation = segmentations.first();
    auto category     = segmentation->category();

    m_drawingWidget.setCategory(category);
    m_drawingWidget.clearBrushImage();

    m_mode          = Mode::CREATION;
    m_referenceItem = segmentation;
  }

  auto validVolume = !segmentations.isEmpty() && hasVolumetricData(m_referenceItem->output());

  if (!validVolume)
  {
    if(!m_referenceItem)
    {
      auto channels = selection->channels();

      m_referenceItem = channels.isEmpty()?m_viewManager->activeChannel():channels.first();
    }

    m_drawingWidget.setBrushImage(QImage(":/espina/brush_new.svg"));
  }

  if (currentItem && currentItem != m_referenceItem)
  {
    m_drawingWidget.stopDrawing();
  }

  m_drawingWidget.setCanErase(validVolume);


  auto output  = m_referenceItem->output();
  auto origin  = volumetricData(output)->origin();
  auto spacing = output->spacing();

  m_drawingWidget.setMaskProperties(spacing, origin);
}

//------------------------------------------------------------------------
CategoryAdapterSPtr ManualEditionTool::currentReferenceCategory()
{
  CategoryAdapterSPtr currentCategory;

//   if (m_currentSelector)
//   {
//     auto item = m_currentSelector->referenceItem();
//
//     if (item && isSegmentation(item))
//     {
//       auto segmentation = segmentationPtr(item);
//
//       currentCategory = segmentation->category();
//     }
//   }

  return currentCategory;
}

//------------------------------------------------------------------------
bool ManualEditionTool::isCreationMode() const
{
  return Mode::CREATION == m_mode;
}

// //------------------------------------------------------------------------
// ViewItemAdapterPtr ManualEditionTool::referenceItem()
// {
//   ViewItemAdapterPtr item;
//
//   auto selection     = m_viewManager->selection();
//   auto channels      = selection->channels();
//   auto segmentations = selection->segmentations();
//
//   if(segmentations.isEmpty())
//   {
//     if (channels.isEmpty())
//     {
//       item = m_viewManager->activeChannel();
//     }
//     else
//     {
//       item = selection->channels().first();
//     }
//   }
//   else
//   {
//     auto segmentation = segmentations.first();
//     // TODO
// //     auto category     = segmentation->category();
// //
// //     if (m_categorySelector->selectedCategory() != category)
// //     {
// //       m_categorySelector->blockSignals(true);
// //       m_categorySelector->selectCategory(category);
// //       m_categorySelector->blockSignals(false);
// //     }
//
//     item = segmentation;
//   }
//
//   return item;
// }

//------------------------------------------------------------------------
void ManualEditionTool::createSegmentation(BinaryMaskSPtr<unsigned char> mask)
{
  auto channel = channelPtr(m_referenceItem);
  auto output  = channel->output();

  auto spacing = output->spacing();
  auto origin  = channel->position();

  auto filter = m_factory->createFilter<SourceFilter>(InputSList(), SOURCE_FILTER);

  auto volume = std::make_shared<SparseVolume<itkVolumeType>>(mask->bounds().bounds(), spacing, origin);
  volume->draw(mask);

  auto mesh = std::make_shared<MarchingCubesMesh<itkVolumeType>>(volume);

  filter->addOutputData(0, volume);
  filter->addOutputData(0, mesh);

  auto segmentation = m_factory->createSegmentation(filter, 0);
  segmentation->setCategory(m_drawingWidget.selectedCategory());

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

  m_referenceItem = segmentation.get();
}

//------------------------------------------------------------------------
void ManualEditionTool::modifySegmentation(BinaryMaskSPtr<unsigned char> mask)
{
    auto segmentation = m_model->smartPointer(reinterpret_cast<SegmentationAdapterPtr>(m_referenceItem));
    m_undoStack->beginMacro(tr("Modify Segmentation"));
    m_undoStack->push(new DrawUndoCommand(segmentation, mask));
    m_undoStack->endMacro();
}

//------------------------------------------------------------------------
void ManualEditionTool::onStrokeStarted(BrushSPtr brush, RenderView *view)
{
//   m_strokePainter = std::make_shared<StrokePainter>(m_referenceItem, view, m_selectedBrush);
//
//   auto actor = m_strokePainter->strokeActor();
//
  auto showStroke = isCreationMode();
  brush->setStrokeVisibility(showStroke);

  if (!showStroke)
  {
//     m_temporalPipeline = std::make_shared<ManualEditionPipeline>();
//     m_temporalPipeline->setTemporalActor(actor);
//     m_referenceItem->setTemporalRepresentation(m_temporalPipeline);
//     m_referenceItem->invalidateRepresentations();
  }

}

//------------------------------------------------------------------------
void ManualEditionTool::onMaskCreated(BinaryMaskSPtr<unsigned char> mask)
{
  if (isCreationMode())
  {
    createSegmentation(mask);
  }
  else
  {
    m_referenceItem->setTemporalRepresentation(m_temporalPipeline);
    modifySegmentation(mask);
  }

  m_strokePainter.reset();
}