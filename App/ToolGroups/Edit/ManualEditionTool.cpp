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

#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.h>
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/IO/DataFactory/MarchingCubesFromFetchedVolumetricData.h>
#include <Core/Analysis/Filters/SourceFilter.h>
#include <Core/Utils/SignalBlocker.h>
#include <GUI/ColorEngines/MultiColorEngine.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Model/CategoryAdapter.h>
#include <GUI/Model/SegmentationAdapter.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/View/RenderView.h>
#include <GUI/Widgets/DrawingWidget.h>
#include <GUI/Widgets/ProgressAction.h>
#include <Support/Settings/Settings.h>
#include <Undo/DrawUndoCommand.h>
#include <Undo/RemoveSegmentations.h>

// Qt
#include <QAction>
#include <QUndoStack>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::GUI::ColorEngines;

//------------------------------------------------------------------------
ManualEditionTool::ManualEditionTool(Support::Context &context)
: EditTool("FreehandEdition", ":espina/manual_edition.svg", tr("Freehand Edition"), context)
, m_drawingWidget   (context.viewState(), context.model())
, m_referenceItem   {nullptr}
, m_currentHandler  {nullptr}
, m_temporalPipeline{nullptr}
{
  qRegisterMetaType<ViewItemAdapterPtr>("ViewItemAdapterPtr");

  setCheckable(true);
  setExclusive(true);

  m_drawingWidget.showCategoryControls(false);

  addSettingsWidget(&m_drawingWidget);

  connect(&m_drawingWidget, SIGNAL(painterChanged(MaskPainterSPtr)),
          this,             SLOT(onPainterChanged(MaskPainterSPtr)));

  connect(&m_drawingWidget, SIGNAL(strokeStarted(BrushPainter*,RenderView*)),
          this,             SLOT(onStrokeStarted(BrushPainter*,RenderView*)), Qt::DirectConnection);

  connect(&m_drawingWidget, SIGNAL(maskPainted(BinaryMaskSPtr<unsigned char>)),
          this,             SLOT(onMaskCreated(BinaryMaskSPtr<unsigned char>)));

  connect(context.viewState().selection().get(), SIGNAL(selectionChanged(SegmentationAdapterList)),
          this,                                  SLOT(onSelectionChanged(SegmentationAdapterList)));

  onPainterChanged(m_drawingWidget.painter());
}

//------------------------------------------------------------------------
ManualEditionTool::~ManualEditionTool()
{
}

//------------------------------------------------------------------------
void ManualEditionTool::abortOperation()
{
  m_drawingWidget.abortOperation();
}

//------------------------------------------------------------------------
void ManualEditionTool::updateReferenceItem(SegmentationAdapterPtr segmentation) const
{
  if(m_temporalPipeline)
  {
    clearTemporalPipeline();
  }

  auto currentItem = m_referenceItem;
  auto category    = segmentation->category();
  auto brushColor  = getContext().colorEngine()->color(segmentation);

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
  auto origin  = readLockVolume(output)->bounds().origin();
  auto spacing = output->spacing();

  m_drawingWidget.setMaskProperties(spacing, origin);
}

//------------------------------------------------------------------------
SegmentationAdapterSPtr ManualEditionTool::referenceSegmentation() const
{
  Q_ASSERT(m_referenceItem);

  auto segmentation = reinterpret_cast<SegmentationAdapterPtr>(m_referenceItem);
  return getModel()->smartPointer(segmentation);
}

//------------------------------------------------------------------------
bool ManualEditionTool::acceptsSelection(SegmentationAdapterList segmentations)
{
  Q_ASSERT(segmentations.size() == 1);

  updateReferenceItem(segmentations.first());

  return true;
}

//------------------------------------------------------------------------
void ManualEditionTool::modifySegmentation(BinaryMaskSPtr<unsigned char> mask)
{
  clearTemporalPipeline();

  auto undoStack = getUndoStack();

  undoStack->beginMacro(tr("Modify Segmentation"));
  undoStack->push(new DrawUndoCommand(referenceSegmentation(), mask));
  undoStack->endMacro();

  if(mask->foregroundValue() == SEG_BG_VALUE)
  {
    onVoxelDeletion(m_referenceItem);
  }
}

//------------------------------------------------------------------------
void ManualEditionTool::onStrokeStarted(BrushPainter *painter, RenderView *view)
{
  clearTemporalPipeline();

  auto volumeBounds  = readLockVolume(m_referenceItem->output())->bounds();
  auto strokePainter = painter->strokePainter();
  auto canvas        = strokePainter->strokeCanvas();

  int extent[6];
  canvas->GetExtent(extent);
  auto isValid = [&extent](int x, int y, int z){ return (extent[0] <= x && extent[1] >= x && extent[2] <= y && extent[3] >= y && extent[4] <= z && extent[5] >= z); };

  if (intersect(volumeBounds, view->previewBounds(false), volumeBounds.spacing()))
  {
    auto bounds = intersection(volumeBounds, view->previewBounds(false), volumeBounds.spacing());
    auto slice  = readLockVolume(m_referenceItem->output())->itkImage(bounds);

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

  m_temporalPipeline = std::make_shared<SliceEditionPipeline>(getContext().colorEngine());
  m_temporalPipeline->setTemporalActor(strokePainter->strokeActor(), view);

  m_referenceItem->setTemporalRepresentation(m_temporalPipeline);
  m_referenceItem->invalidateRepresentations();
}

//------------------------------------------------------------------------
void ManualEditionTool::onMaskCreated(BinaryMaskSPtr<unsigned char> mask)
{
  modifySegmentation(mask);
}

//------------------------------------------------------------------------
void ManualEditionTool::onPainterChanged(MaskPainterSPtr painter)
{
  m_currentHandler = painter;

  setEventHandler(painter);
}

//------------------------------------------------------------------------
void ManualEditionTool::restoreSettings(std::shared_ptr<QSettings> settings)
{
  m_drawingWidget.restoreSettings(settings);
}

//------------------------------------------------------------------------
void ManualEditionTool::saveSettings(std::shared_ptr<QSettings> settings)
{
  m_drawingWidget.saveSettings(settings);
}

//------------------------------------------------------------------------
void ManualEditionTool::onSelectionChanged(SegmentationAdapterList segmentations)
{
  if(!isChecked()) return;

  if(segmentations.size() == 1)
  {
    auto seg = segmentations.first();
    if(hasVolumetricData(seg->output()))
    {
      if(m_referenceItem && (seg == segmentationPtr(m_referenceItem))) return;

      updateReferenceItem(seg);
      return;
    }
  }

  if(this->isChecked()) setChecked(false);
  setEnabled(false);
}

//------------------------------------------------------------------------
void ESPINA::ManualEditionTool::setEnabled(bool value)
{
  EditTool::setEnabled(value);

  if(!value)
  {
    if(m_referenceItem)
    {
      clearTemporalPipeline();
      m_referenceItem = nullptr;
    }
  }
  else
  {
    auto selection = getSelection()->segmentations();
    if(selection.size() == 1 && hasVolumetricData(selection.first()->output()))
    {
      updateReferenceItem(selection.first());
    }
  }
}

//------------------------------------------------------------------------
void ManualEditionTool::onVoxelDeletion(ViewItemAdapterPtr item)
{
  Q_ASSERT(item && isSegmentation(item) && hasVolumetricData(item->output()));

  bool removeSegmentation = false;

  auto segmentation = segmentationPtr(item);

  {
    auto output = segmentation->output();
    SignalBlocker<OutputSPtr> blocker(output);

    if (readLockVolume(output)->isEmpty())
    {
      removeSegmentation = true;
      blocker.setLaunch(false);
    }
    else
    {
      fitToContents<itkVolumeType>(writeLockVolume(output), SEG_BG_VALUE);
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

    undoStack->beginMacro(tr("Remove Segmentation"));
    undoStack->push(new RemoveSegmentations(segmentation, getModel()));
    undoStack->endMacro();
  }
}

//------------------------------------------------------------------------
void ManualEditionTool::clearTemporalPipeline() const
{
  if(m_temporalPipeline)
  {
    Q_ASSERT(m_referenceItem != nullptr);

    m_referenceItem->clearTemporalRepresentation();
    m_temporalPipeline = nullptr;
  }
}
