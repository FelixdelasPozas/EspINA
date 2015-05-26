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
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.hxx>
#include <Core/IO/DataFactory/MarchingCubesFromFetchedVolumetricData.h>
#include <GUI/Model/CategoryAdapter.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/Widgets/DrawingWidget.h>
#include <GUI/Widgets/ProgressAction.h>
#include <GUI/View/RenderView.h>
#include <Support/Settings/EspinaSettings.h>
#include <Filters/SourceFilter.h>
#include <GUI/Model/SegmentationAdapter.h>
#include <Undo/AddSegmentations.h>
#include <Undo/DrawUndoCommand.h>

// Qt
#include <QAction>
#include <QUndoStack>

using ESPINA::Filter;

using namespace ESPINA;

//------------------------------------------------------------------------
ManualEditionTool::ManualEditionTool(Support::Context &context)
: ProgressTool(":espina/manual_edition.svg", tr("Modify segmentations manually"), context)
, m_model        {context.model()}
, m_factory      {context.factory()}
, m_undoStack    {context.undoStack()}
, m_colorEngine  {context.colorEngine()}
, m_selection    {getSelection(context)}
, m_context      (context)
, m_drawingWidget(context)
, m_referenceItem{nullptr}
, m_validStroke  {true}
{
  qRegisterMetaType<ViewItemAdapterPtr>("ViewItemAdapterPtr");

  setCheckable(true);

  addSettingsWidget(&m_drawingWidget);

  connect(m_selection.get(), SIGNAL(selectionChanged()),
          this,              SLOT(onSelectionChanged()));

  connect(this, SIGNAL(toggled(bool)),
          this, SLOT(onToolToggled(bool)));

  connect(&m_drawingWidget, SIGNAL(painterChanged(MaskPainterSPtr)),
          this,             SLOT(onPainterChanged(MaskPainterSPtr)));

  connect(&m_drawingWidget, SIGNAL(strokeStarted(BrushPainter*,RenderView*)),
          this,             SLOT(onStrokeStarted(BrushPainter*,RenderView*)));

  connect(&m_drawingWidget, SIGNAL(maskPainted(BinaryMaskSPtr<unsigned char>)),
          this,             SLOT(onMaskCreated(BinaryMaskSPtr<unsigned char>)));

  onSelectionChanged();
}

//------------------------------------------------------------------------
ManualEditionTool::~ManualEditionTool()
{
}

//------------------------------------------------------------------------
void ManualEditionTool::abortOperation()
{
  this->m_drawingWidget.abortOperation();
}

//------------------------------------------------------------------------
void ManualEditionTool::onSelectionChanged()
{
  auto segmentations = m_selection->segmentations();

  bool validSelection = true;

  if (segmentations.size() != 1)
  {
    validSelection = false;
  }
  else if (selectedSegmentation()->isBeingModified())
  {
    validSelection = false;
  }

  if (validSelection)
  {
    updateReferenceItem();
  }

  setEnabled(validSelection);
}

//------------------------------------------------------------------------
void ManualEditionTool::updateReferenceItem() const
{
  ViewItemAdapterPtr currentItem = m_referenceItem;

  auto segmentation  = selectedSegmentation();
  auto category      = segmentation->category();
  auto brushColor    = m_colorEngine->color(segmentation);

  m_drawingWidget.setCategory(category);
  m_drawingWidget.clearBrushImage();

  m_referenceItem = segmentation;

  auto validVolume = hasVolumetricData(m_referenceItem->output());

  if (!validVolume)
  {
    m_drawingWidget.setBrushImage(QImage(":/espina/brush_new.svg"));
  }

  if (currentItem && currentItem != m_referenceItem)
  {
    m_drawingWidget.stopDrawing();
  }

  brushColor = GUI::ColorEngines::selectedColor(brushColor.hue());

  m_drawingWidget.setDrawingColor(brushColor);
  m_drawingWidget.setCanErase(validVolume);

  auto output  = m_referenceItem->output();
  auto origin  = readLockVolume(output)->origin();
  auto spacing = output->spacing();

  m_drawingWidget.setMaskProperties(spacing, origin);
}

//------------------------------------------------------------------------
void ManualEditionTool::onToolEnabled(bool enabled)
{

}

//------------------------------------------------------------------------
SegmentationAdapterSPtr ManualEditionTool::referenceSegmentation() const
{
  Q_ASSERT(m_referenceItem);

  auto segmentation = reinterpret_cast<SegmentationAdapterPtr>(m_referenceItem);
  return m_model->smartPointer(segmentation);
}

//------------------------------------------------------------------------
ChannelAdapterPtr ManualEditionTool::activeChannel() const
{
  return getActiveChannel(m_context);
}

//------------------------------------------------------------------------
SegmentationAdapterPtr ManualEditionTool::selectedSegmentation() const
{
  Q_ASSERT(!m_selection->segmentations().isEmpty());

  return m_selection->segmentations().first();
}

//------------------------------------------------------------------------
void ManualEditionTool::modifySegmentation(BinaryMaskSPtr<unsigned char> mask)
{
  m_referenceItem->clearTemporalRepresentation();

  m_undoStack->beginMacro(tr("Modify Segmentation"));
  m_undoStack->push(new DrawUndoCommand(referenceSegmentation(), mask));
  m_undoStack->endMacro();

  if(mask->foregroundValue() == SEG_BG_VALUE)
  {
    emit voxelsDeleted(m_referenceItem);
  }
}

//------------------------------------------------------------------------
void ManualEditionTool::onStrokeStarted(BrushPainter *painter, RenderView *view)
{
  painter->setStrokeVisibility(false);

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

    m_temporalPipeline = std::make_shared<SliceEditionPipeline>(m_colorEngine);

    m_temporalPipeline->setTemporalActor(actor, view);
    m_referenceItem->setTemporalRepresentation(m_temporalPipeline);
    m_referenceItem->invalidateRepresentations();
  }
}

//------------------------------------------------------------------------
void ManualEditionTool::onMaskCreated(BinaryMaskSPtr<unsigned char> mask)
{
  modifySegmentation(mask);
}

//------------------------------------------------------------------------
void ManualEditionTool::onPainterChanged(MaskPainterSPtr painter)
{
  m_context.viewState().setEventHandler(painter);
}

//------------------------------------------------------------------------
void ManualEditionTool::onToolToggled(bool toggled)
{
  auto painter = m_drawingWidget.painter();

  if (toggled)
  {
    m_context.viewState().setEventHandler(painter);
  }
  else
  {
    m_context.viewState().unsetEventHandler(painter);
  }
}
