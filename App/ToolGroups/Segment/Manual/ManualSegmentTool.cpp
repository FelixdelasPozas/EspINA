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
#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.h>
#include <Core/Analysis/Filter.h>
#include <Core/IO/DataFactory/MarchingCubesFromFetchedVolumetricData.h>
#include <Core/Utils/EspinaException.h>
#include <Core/Analysis/Filters/SourceFilter.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Model/CategoryAdapter.h>
#include <GUI/Model/Utils/ModelUtils.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/Model/SegmentationAdapter.h>
#include <GUI/Widgets/DrawingWidget.h>
#include <GUI/Widgets/ProgressAction.h>
#include <GUI/Widgets/Styles.h>
#include <GUI/Widgets/ToolButton.h>
#include <GUI/View/RenderView.h>
#include <Support/Settings/Settings.h>
#include <Undo/AddSegmentations.h>
#include <Undo/RemoveSegmentations.h>
#include <Undo/DrawUndoCommand.h>

// Qt
#include <QAction>
#include <QUndoStack>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::GUI::ColorEngines;
using namespace ESPINA::GUI::Widgets;
using namespace ESPINA::GUI::Model::Utils;

const Filter::Type ManualFilterFactory::SOURCE_FILTER    = "FreeFormSource";
const Filter::Type ManualFilterFactory::SOURCE_FILTER_V4 = "::FreeFormSource";

const QString MODE = "Stroke mode";

//-----------------------------------------------------------------------------
FilterTypeList ManualFilterFactory::providedFilters() const
{
  FilterTypeList filters;

  filters << SOURCE_FILTER;
  filters << SOURCE_FILTER_V4;

  return filters;
}

//-----------------------------------------------------------------------------
FilterSPtr ManualFilterFactory::createFilter(InputSList          inputs,
                                             const Filter::Type& filter,
                                             SchedulerSPtr       scheduler) const
{
  if (!providedFilters().contains(filter))
  {
    auto what    = QObject::tr("Unable to create filter: %1").arg(filter);
    auto details = QObject::tr("ManualFilterFactory::createFilter() -> Unknown filter: %1").arg(filter);
    throw EspinaException(what, details);
  }

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
: ProgressTool("0-FreehandSegmentationTool", ":espina/manual_segmentation.svg", tr("Freehand Segmentation"), context)
, m_model             {context.model()}
, m_factory           {context.factory()}
, m_colorEngine       {context.colorEngine()}
, m_filterFactory     {new ManualFilterFactory()}
, m_drawingWidget     {context.viewState(), context.model()}
, m_mode              {Mode::SINGLE_STROKE}
, m_createSegmentation{true}
, m_referenceItem     {nullptr}
, m_temporalPipeline  {nullptr}
{
  qRegisterMetaType<ViewItemAdapterPtr>("ViewItemAdapterPtr");

  m_factory->registerFilterFactory(m_filterFactory);

  setCheckable(true);
  setExclusive(true);

  addSettingsWidget(&m_drawingWidget);

  initMultiStrokeWidgets();

  connect(getSelection().get(), SIGNAL(selectionChanged()),
          this,                 SLOT(onSelectionChanged()));

  connect(&m_drawingWidget, SIGNAL(painterChanged(MaskPainterSPtr)),
          this,             SLOT(onPainterChanged(MaskPainterSPtr)));

  connect(&m_drawingWidget, SIGNAL(strokeStarted(BrushPainter*,RenderView*)),
          this,             SLOT(onStrokeStarted(BrushPainter*,RenderView*)));

  connect(&m_drawingWidget, SIGNAL(maskPainted(BinaryMaskSPtr<unsigned char>)),
          this,             SLOT(onMaskCreated(BinaryMaskSPtr<unsigned char>)));

  connect(&m_drawingWidget, SIGNAL(categoryChanged(CategoryAdapterSPtr)),
          this,             SLOT(onCategoryChange(CategoryAdapterSPtr)));

  onPainterChanged(m_drawingWidget.painter());
}

//------------------------------------------------------------------------
ManualSegmentTool::~ManualSegmentTool()
{
}

//------------------------------------------------------------------------
void ManualSegmentTool::abortOperation()
{
  setChecked(false);
}

//------------------------------------------------------------------------
void ManualSegmentTool::onSelectionChanged()
{
  if(isChecked())
  {
    setInitialStroke();
  }
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
  m_multiStroke         = Styles::createToolButton(":espina/single_stroke.svg", tr("Toggle single/multi stroke segmentations"));
  auto nextSegmentation = Styles::createToolButton(":espina/next_segmentation.svg", tr("Start a new multi-stroke segmentation"));

  m_multiStroke->setCheckable(true);

  connect(m_multiStroke, SIGNAL(toggled(bool)),
          this,          SLOT(onStrokeModeToggled(bool)));

  connect(m_multiStroke,    SIGNAL(toggled(bool)),
          nextSegmentation, SLOT(setVisible(bool)));

  nextSegmentation->setVisible(false);

  connect(nextSegmentation, SIGNAL(clicked(bool)),
          this,             SLOT(startNextSegmentation()));


  addSettingsWidget(m_multiStroke);
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

  m_referenceItem = getActiveChannel();

  auto brushColor = m_drawingWidget.selectedCategory()->color();

  m_drawingWidget.clearBrushImage();
  m_drawingWidget.setDrawingColor(brushColor);
  m_drawingWidget.setCanErase(false);
  m_drawingWidget.setBrushImage(QImage(":/espina/brush_new.svg"));

  auto output  = m_referenceItem->output();
  auto origin  = readLockVolume(output)->bounds().origin();
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
  clearTemporalPipeline();

  auto channel = channelPtr(m_referenceItem);
  auto output  = channel->output();

  auto spacing = output->spacing();
  auto origin  = channel->position();

  InputSList inputs;
  inputs << channel->asInput();

  auto filter = m_factory->createFilter<SourceFilter>(inputs, ManualFilterFactory::SOURCE_FILTER);

  auto volume = std::make_shared<SparseVolume<itkVolumeType>>(mask->bounds().bounds(), spacing, origin);
  volume->draw(mask);

  filter->addOutputData(0, volume);

  auto mesh = std::make_shared<MarchingCubesMesh>(filter->output(0).get());

  filter->addOutputData(0, mesh);

  auto segmentation = m_factory->createSegmentation(filter, 0);
  auto category     = m_drawingWidget.selectedCategory();
  segmentation->setCategory(category);
  segmentation->setNumber(firstUnusedSegmentationNumber(getModel()));

  SampleAdapterSList samples;
  samples << QueryAdapter::sample(channel);
  Q_ASSERT(channel && (samples.size() == 1));

  auto undoStack = getUndoStack();

  undoStack->beginMacro(tr("Add segmentation '%1'.").arg(segmentation->data().toString()));
  undoStack->push(new AddSegmentations(segmentation, samples, m_model));
  undoStack->endMacro();

  getSelection()->clear();
  getSelection()->set(toViewItemList(segmentation.get()));

  m_referenceItem = segmentation.get();
}

//------------------------------------------------------------------------
void ManualSegmentTool::modifySegmentation(BinaryMaskSPtr<unsigned char> mask)
{
  clearTemporalPipeline();

  auto undoStack    = getUndoStack();
  auto segmentation = referenceSegmentation();
  auto bounds       = mask->bounds().bounds().toString();
  auto mode         = mask->foregroundValue() == SEG_VOXEL_VALUE ? "Paint":"Erase";
  undoStack->beginMacro(tr("%1 segmentation '%2' in bounds %3.").arg(mode).arg(segmentation->data().toString()).arg(bounds));
  undoStack->push(new DrawUndoCommand(segmentation, mask));
  undoStack->endMacro();

  if(mask->foregroundValue() == SEG_BG_VALUE)
  {
    onVoxelDeletion(m_referenceItem);
  }
}

//------------------------------------------------------------------------
void ManualSegmentTool::onStrokeStarted(BrushPainter *painter, RenderView *view)
{
  if(m_temporalPipeline)
  {
    clearTemporalPipeline();
  }

  auto showStroke = isCreationMode();
  m_drawingWidget.setManageActors(showStroke);

  if (!showStroke)
  {
    auto volumeBounds  = readLockVolume(m_referenceItem->output())->bounds();
    auto strokePainter = painter->strokePainter();
    auto canvas        = strokePainter->strokeCanvas();

    int extent[6];
    canvas->GetExtent(extent);
    auto isValid = [&extent](int x, int y, int z){ return (extent[0] <= x && extent[1] >= x && extent[2] <= y && extent[3] >= y && extent[4] <= z && extent[5] >= z); };

    if (intersect(volumeBounds, view->previewBounds(false), volumeBounds.spacing()))
    {
      auto bounds = intersection(volumeBounds, view->previewBounds(false), volumeBounds.spacing());

      auto slice = readLockVolume(m_referenceItem->output())->itkImage(bounds);

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
    m_temporalPipeline->setTemporalActor(strokePainter->strokeActor(), view);

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
  if(m_currentPainter)
  {
    disconnect(m_currentPainter.get(), SIGNAL(eventHandlerInUse(bool)),
               this,                   SLOT(onEventHandlerActivated(bool)));
  }

  m_currentPainter = painter;

  setEventHandler(painter);

  if(m_currentPainter)
  {
    connect(m_currentPainter.get(), SIGNAL(eventHandlerInUse(bool)),
            this,                   SLOT(onEventHandlerActivated(bool)));
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

  if (isChecked())
  {
    setInitialStroke();
  }
}

//------------------------------------------------------------------------
void ManualSegmentTool::restoreSettings(std::shared_ptr<QSettings> settings)
{
  auto multiStroke = settings->value(MODE, false).toBool();
  m_multiStroke->setChecked(multiStroke); // signals take care of the rest of the configuration.

  settings->beginGroup("DrawingWidget");
  m_drawingWidget.restoreSettings(settings);
  settings->endGroup();
}

//------------------------------------------------------------------------
void ManualSegmentTool::saveSettings(std::shared_ptr<QSettings> settings)
{
  settings->setValue(MODE, m_multiStroke->isChecked());

  settings->beginGroup("DrawingWidget");
  m_drawingWidget.saveSettings(settings);
  settings->endGroup();
}

//------------------------------------------------------------------------
void ManualSegmentTool::startNextSegmentation()
{
  setInitialStroke();
}

//------------------------------------------------------------------------
void ManualSegmentTool::onEventHandlerActivated(bool inUse)
{
  if(inUse)
  {
    onSelectionChanged();
  }
  else
  {
    m_drawingWidget.stopDrawing();
    m_referenceItem = nullptr;
  }
}

//------------------------------------------------------------------------
void ManualSegmentTool::onVoxelDeletion(ViewItemAdapterPtr item)
{
  Q_ASSERT(item && isSegmentation(item) && hasVolumetricData(item->output()));

  bool removeSegmentation = false;

  auto segmentation = segmentationPtr(item);

  {
    if (readLockVolume(segmentation->output())->isEmpty())
    {
      removeSegmentation = true;
    }
    else
    {
      fitToContents<itkVolumeType>(writeLockVolume(segmentation->output()), SEG_BG_VALUE);
    }
  }

  if (removeSegmentation)
  {
    getViewState().setEventHandler(EventHandlerSPtr());

    auto name  = segmentation->data(Qt::DisplayRole).toString();
    auto title = tr("Deleting segmentation");
    auto msg   = tr("%1 will be deleted because all its voxels were erased.").arg(name);

    DefaultDialogs::InformationMessage(msg, title);

    auto undoStack = getUndoStack();

    undoStack->blockSignals(true);
    undoStack->undo();
    undoStack->blockSignals(false);

    undoStack->beginMacro(tr("Remove segmentation '%1'.").arg(name));
    undoStack->push(new RemoveSegmentations(segmentation, getModel()));
    undoStack->endMacro();
  }
}

//------------------------------------------------------------------------
void ManualSegmentTool::clearTemporalPipeline() const
{
  if(m_temporalPipeline)
  {
    Q_ASSERT(m_referenceItem != nullptr);

    m_referenceItem->clearTemporalRepresentation();
    m_temporalPipeline = nullptr;
  }
}
