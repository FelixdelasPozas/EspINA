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
#include "ManualSegmentTool.h"
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.hxx>
#include <Core/IO/DataFactory/MarchingCubesFromFetchedVolumetricData.h>
#include <GUI/Model/CategoryAdapter.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/Widgets/DrawingWidget.h>
#include <GUI/Widgets/ProgressAction.h>
#include <GUI/Widgets/Styles.h>
#include <GUI/View/RenderView.h>
#include <Support/Settings/EspinaSettings.h>
#include <Filters/SourceFilter.h>
#include <GUI/Model/SegmentationAdapter.h>
#include <Undo/AddSegmentations.h>
#include <Undo/DrawUndoCommand.h>

// Qt
#include <QAction>
#include <QUndoStack>
#include <QPushButton>

using ESPINA::Filter;

const Filter::Type SOURCE_FILTER    = "FreeFormSource";
const Filter::Type SOURCE_FILTER_V4 = "::FreeFormSource";

using namespace ESPINA;
using namespace ESPINA::GUI::Widgets;

//-----------------------------------------------------------------------------
FilterTypeList ManualSegmentTool::ManualFilterFactory::providedFilters() const
{
  FilterTypeList filters;

  filters << SOURCE_FILTER << SOURCE_FILTER_V4;

  return filters;
}

//-----------------------------------------------------------------------------
FilterSPtr ManualSegmentTool::ManualFilterFactory::createFilter(InputSList          inputs,
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
ManualSegmentTool::ManualSegmentTool(Support::Context &context)
: ProgressTool(":espina/manual_segmentation.svg", tr("Create segmentations manually"), context)
, m_model        {context.model()}
, m_factory      {context.factory()}
, m_colorEngine  {context.colorEngine()}
, m_filterFactory{new ManualFilterFactory()}
, m_drawingWidget(context)
, m_mode         {Mode::SINGLE_STROKE}
, m_createSegmentation{true}
, m_referenceItem{nullptr}
, m_validStroke  {true}
{
  qRegisterMetaType<ViewItemAdapterPtr>("ViewItemAdapterPtr");

  m_factory->registerFilterFactory(m_filterFactory);

  setCheckable(true);
  setExclusive(true);

  addSettingsWidget(&m_drawingWidget);

  initMultiStrokeWidgets();

  connect(getSelection().get(), SIGNAL(selectionChanged()),
          this,                 SLOT(onSelectionChanged()));

  connect(this, SIGNAL(toggled(bool)),
          this, SLOT(onToolToggled(bool)));

  connect(&m_drawingWidget, SIGNAL(painterChanged(MaskPainterSPtr)),
          this,             SLOT(onPainterChanged(MaskPainterSPtr)));

  connect(&m_drawingWidget, SIGNAL(strokeStarted(BrushPainter*,RenderView*)),
          this,             SLOT(onStrokeStarted(BrushPainter*,RenderView*)));

  connect(&m_drawingWidget, SIGNAL(maskPainted(BinaryMaskSPtr<unsigned char>)),
          this,             SLOT(onMaskCreated(BinaryMaskSPtr<unsigned char>)));

  connect(&m_drawingWidget, SIGNAL(categoryChanged(CategoryAdapterSPtr)),
          this,             SLOT(onCategoryChange(CategoryAdapterSPtr)));

  setEventHandler(m_drawingWidget.painter());
}

//------------------------------------------------------------------------
ManualSegmentTool::~ManualSegmentTool()
{
}

//------------------------------------------------------------------------
void ManualSegmentTool::abortOperation()
{
  this->m_drawingWidget.abortOperation();
}

//------------------------------------------------------------------------
void ManualSegmentTool::onSelectionChanged()
{
  setInitialStroke();
}

//------------------------------------------------------------------------
bool ManualSegmentTool::isCreationMode() const
{
  return m_createSegmentation;
}

//------------------------------------------------------------------------
SegmentationAdapterSPtr ManualSegmentTool::referenceSegmentation() const
{
  Q_ASSERT(m_referenceItem);

  auto segmentation = reinterpret_cast<SegmentationAdapterPtr>(m_referenceItem);
  return m_model->smartPointer(segmentation);
}

//------------------------------------------------------------------------
void ManualSegmentTool::initMultiStrokeWidgets()
{
  auto multiStroke      = Styles::createToolButton(":espina/single_stroke.svg", tr("Toggle single/multi stroke segmentations"));
  auto nextSegmentation = Styles::createToolButton(":espina/next_segmentation.svg", tr("Start a new multi-stroke segmentation"));

  multiStroke->setCheckable(true);

  connect(multiStroke, SIGNAL(toggled(bool)),
          this,        SLOT(onStrokeModeToggled(bool)));

  connect(multiStroke,      SIGNAL(toggled(bool)),
          nextSegmentation, SLOT(setVisible(bool)));

  nextSegmentation->setVisible(false);

  connect(nextSegmentation, SIGNAL(clicked(bool)),
          this,             SLOT(startNextSegmentation()));


  addSettingsWidget(multiStroke);
  addSettingsWidget(nextSegmentation);
}

//------------------------------------------------------------------------
void ManualSegmentTool::setInitialStroke()
{
  m_createSegmentation = true;

  if (m_referenceItem)
  {
    m_drawingWidget.stopDrawing();
  }

  auto channels = getSelection()->channels();

  m_referenceItem = channels.isEmpty()?getActiveChannel():channels.first();

  auto brushColor = m_drawingWidget.selectedCategory()->color();

  m_drawingWidget.setBrushImage(QImage(":/espina/brush_new.svg"));
  m_drawingWidget.setDrawingColor(brushColor);
  m_drawingWidget.setCanErase(false);

  auto output  = m_referenceItem->output();
  auto origin  = readLockVolume(output)->origin();
  auto spacing = output->spacing();

  m_drawingWidget.setMaskProperties(spacing, origin);
}

//------------------------------------------------------------------------
void ManualSegmentTool::setMultiStroke()
{
  m_createSegmentation = false;

  m_drawingWidget.clearBrushImage();
  m_drawingWidget.setCanErase(true);
}

//------------------------------------------------------------------------
void ManualSegmentTool::createSegmentation(BinaryMaskSPtr<unsigned char> mask)
{
  auto channel = channelPtr(m_referenceItem);
  auto output  = channel->output();

  auto spacing = output->spacing();
  auto origin  = channel->position();

  auto filter = m_factory->createFilter<SourceFilter>(InputSList(), SOURCE_FILTER);

  auto volume = std::make_shared<SparseVolume<itkVolumeType>>(mask->bounds().bounds(), spacing, origin);
  volume->draw(mask);

  filter->addOutputData(0, volume);

  auto mesh = std::make_shared<MarchingCubesMesh<itkVolumeType>>(filter->output(0).get());

  filter->addOutputData(0, mesh);

  auto segmentation = m_factory->createSegmentation(filter, 0);
  segmentation->setCategory(m_drawingWidget.selectedCategory());

  SampleAdapterSList samples;
  samples << QueryAdapter::sample(channel);
  Q_ASSERT(channel && (samples.size() == 1));

  auto undoStack = getUndoStack();

  undoStack->beginMacro(tr("Add Segmentation"));
  undoStack->push(new AddSegmentations(segmentation, samples, m_model));
  undoStack->endMacro();

  SegmentationAdapterList list;
  list << segmentation.get();

  // TODO: should set clear previous selection?
  getSelection()->clear();
  getSelection()->set(list);

  m_referenceItem = segmentation.get();
}

//------------------------------------------------------------------------
void ManualSegmentTool::modifySegmentation(BinaryMaskSPtr<unsigned char> mask)
{
  m_referenceItem->clearTemporalRepresentation();

  auto undoStack = getUndoStack();
  undoStack->beginMacro(tr("Modify Segmentation"));
  undoStack->push(new DrawUndoCommand(referenceSegmentation(), mask));
  undoStack->endMacro();

  if(mask->foregroundValue() == SEG_BG_VALUE)
  {
    emit voxelsDeleted(m_referenceItem);
  }
}

//------------------------------------------------------------------------
void ManualSegmentTool::onStrokeStarted(BrushPainter *painter, RenderView *view)
{
  auto showStroke = isCreationMode();

  painter->setStrokeVisibility(showStroke);

  if (!showStroke)
  {
    auto volume = readLockVolume(m_referenceItem->output());
    auto strokePainter = painter->strokePainter();

    auto canvas = strokePainter->strokeCanvas();
    auto actor  = strokePainter->strokeActor();

    int extent[6];
    canvas->GetExtent(extent);
    auto isValid = [&extent](int x, int y, int z){ return (extent[0] <= x && extent[1] >= x && extent[2] <= y && extent[3] >= y && extent[4] <= z && extent[5] >= z); };

    m_validStroke = intersect(volume->bounds(), view->previewBounds(false), volume->spacing());

    if (m_validStroke)
    {
      auto bounds = intersection(volume->bounds(), view->previewBounds(false), volume->spacing());

      auto slice = volume->itkImage(bounds);

      itk::ImageRegionConstIteratorWithIndex<itkVolumeType> it(slice, slice->GetLargestPossibleRegion());
      it.GoToBegin();

      while(!it.IsAtEnd())
      {
        auto index = it.GetIndex();

        if(it.Value() == SEG_VOXEL_VALUE && isValid(index[0], index[1], index[2]))
        {
          auto pixel = static_cast<unsigned char*>(canvas->GetScalarPointer(index[0],index[1], index[2]));
          *pixel     = 1;
        }
        ++it;
      }
    }

    m_temporalPipeline = std::make_shared<SliceEditionPipeline>(m_colorEngine);

    m_temporalPipeline->setTemporalActor(actor, view);
    m_referenceItem->setTemporalRepresentation(m_temporalPipeline);
    m_referenceItem->invalidateRepresentations();
  }
}

//------------------------------------------------------------------------
void ManualSegmentTool::onMaskCreated(BinaryMaskSPtr<unsigned char> mask)
{
  if (isCreationMode())
  {
    createSegmentation(mask);
  }
  else
  {
    modifySegmentation(mask);
  }

  if (Mode::SINGLE_STROKE == m_mode)
  {
    setInitialStroke();
  }
  else
  {
    setMultiStroke();
  }
}

//------------------------------------------------------------------------
void ManualSegmentTool::onCategoryChange(CategoryAdapterSPtr category)
{
  getSelection()->clear();
}

//------------------------------------------------------------------------
void ManualSegmentTool::onPainterChanged(MaskPainterSPtr painter)
{
  setEventHandler(painter);
}

//------------------------------------------------------------------------
void ManualSegmentTool::onToolToggled(bool toggled)
{
  if (toggled)
  {
    setInitialStroke();
  }
}

//------------------------------------------------------------------------
void ManualSegmentTool::onStrokeModeToggled(bool toggled)
{
  auto button = static_cast<QPushButton *>(sender());

  m_mode = toggled?Mode::MULTI_STROKE:Mode::SINGLE_STROKE;

  if (toggled)
  {
    button->setIcon(QIcon(":espina/multi_stroke.svg"));
  }
  else
  {
    button->setIcon(QIcon(":espina/single_stroke.svg"));
  }

  setInitialStroke();
}

//------------------------------------------------------------------------
void ManualSegmentTool::startNextSegmentation()
{
  setInitialStroke();
}