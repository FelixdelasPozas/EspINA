/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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


#include "SphericalBrush.h"

#include "common/editor/FreeFormSource.h"
#include "common/model/Channel.h"
#include "common/model/EspinaModel.h"
#include "common/model/EspinaFactory.h"
#include "common/gui/ViewManager.h"
#include "common/undo/AddSegmentation.h"
#include <tools/BrushPicker.h>
#include "frontend/toolbar/editor/BrushUndoCommand.h"

#include <vtkSphere.h>

#include <QDebug>
#include <QUndoStack>

//-----------------------------------------------------------------------------
SphericalBrush::SphericalBrush(EspinaModel* model,
                               QUndoStack* undoStack,
                               ViewManager* viewManager)
: Brush(model, undoStack, viewManager)
{
}

//-----------------------------------------------------------------------------
void SphericalBrush::drawStroke(PickableItem *item,
                                IPicker::WorldRegion centers,
                                Nm radius,
                                PlaneType plane)
{
  if (centers->GetNumberOfPoints() == 0)
    return;

    DrawCommand::BrushList brushes;
    for (int i=0; i < centers->GetNumberOfPoints(); i++)
    {
      double brushCenter[3];
      centers->GetPoint(i, brushCenter);

      double brushBounds[6];//TODO 2012-10-24 Crop bounds
      brushBounds[0] = brushCenter[0] - radius;
      brushBounds[1] = brushCenter[0] + radius;
      brushBounds[2] = brushCenter[1] - radius;
      brushBounds[3] = brushCenter[1] + radius;
      brushBounds[4] = brushCenter[2] - radius;
      brushBounds[5] = brushCenter[2] + radius;

      vtkSphere *brush = vtkSphere::New();
      brush->SetCenter(brushCenter);
      brush->SetRadius(radius);
      brushes << DrawCommand::Brush(brush,BoundingBox(brushBounds));
    }

    if (!m_currentSource)
    {
      Q_ASSERT(!m_currentSeg);

      Q_ASSERT(ModelItem::CHANNEL == item->type());
      Channel *channel = dynamic_cast<Channel *>(item);

      double spacing[3];
      channel->spacing(spacing);

      Filter::NamedInputs inputs;
      Filter::Arguments args;
      FreeFormSource::Parameters params(args);
      params.setSpacing(spacing);
      m_currentSource = new FreeFormSource(inputs, args);
      m_currentSeg = m_model->factory()->createSegmentation(m_currentSource, 0);

      m_undoStack->beginMacro("Draw Segmentation");
      // We can't add empty segmentations to the model
      m_undoStack->push(new DrawCommand(m_currentSource,
                                        0,
                                        brushes,
                                        SEG_VOXEL_VALUE));
      m_undoStack->push(new AddSegmentation(channel,
                                            m_currentSource,
                                            m_currentSeg,
                                            m_viewManager->activeTaxonomy(),
                                            m_model));
      m_undoStack->endMacro();
      m_brush->setBorderColor(QColor(Qt::green));
    }else
    {
      Q_ASSERT(m_currentSource && m_currentSeg);
      EspinaVolume::PixelType value = m_erasing?SEG_BG_VALUE:SEG_VOXEL_VALUE;

      m_undoStack->push(new DrawCommand(m_currentSource,
                                        m_currentSeg->outputNumber(),
                                        brushes,
                                        value));
    }
    //   if (m_currentSeg)
    //     m_currentSeg->notifyModification(true);
}

//-----------------------------------------------------------------------------
void SphericalBrush::drawStrokeStep(PickableItem* item,
                                    double x, double y, double z,
                                    Nm radius,
                                    PlaneType plane)
{
  if (!m_erasing)
    return;
  qDebug() << "Delete Pixel";
}

