/*
 
 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include "ContourUndoCommand.h"
#include <App/ToolGroups/Edition/ManualEditionTool.h>
#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.hxx>
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/Analysis/Output.h>

// VTK
#include <vtkPolyData.h>

namespace ESPINA
{
  //-------------------------------------------------------------------------------------
  ContourModificationUndoCommand::ContourModificationUndoCommand(ContourWidget::ContourData  previousContour,
                                                                 ContourWidget::ContourData  actualContour,
                                                                 ManualEditionTool          *tool)
  : m_previousContour{previousContour}
  , m_actualContour  {actualContour}
  , m_tool           {tool}
  {
    qDebug() << "ContourModificationUndoCommand created";
  }

  //-------------------------------------------------------------------------------------
  void ContourModificationUndoCommand::redo()
  {
    m_tool->setContour(m_actualContour);
  }

  //-------------------------------------------------------------------------------------
  void ContourModificationUndoCommand::undo()
  {
    m_tool->setContour(m_previousContour);
  }

  //-------------------------------------------------------------------------------------
  ContourRasterizeUndoCommand::ContourRasterizeUndoCommand(SegmentationAdapterPtr         segmentation,
                                                           BinaryMaskSPtr<unsigned char>  contourVolume,
                                                           ContourWidget::ContourData     contour,
                                                           ManualEditionTool             *tool)
  : m_segmentation{segmentation}
  , m_contourVolume{contourVolume}
  , m_contour{contour}
  , m_tool{tool}
  , m_createData{!hasVolumetricData(m_segmentation->output())}
  {
    qDebug() << "ContourRasterizeUndoCommand created";
  }

  //-------------------------------------------------------------------------------------
  void ContourRasterizeUndoCommand::redo()
  {
    auto output = m_segmentation->output();
    auto value = m_contour.mode == BrushSelector::BrushMode::BRUSH ? SEG_VOXEL_VALUE : SEG_BG_VALUE;

    if(m_createData)
    {
      Q_ASSERT(m_contourVolume);
      auto volume = std::make_shared<SparseVolume<itkVolumeType>>(m_contourVolume->bounds().bounds(), output->spacing(), NmVector3{});
      volume->draw(m_contourVolume);
      auto mesh = std::make_shared<MarchingCubesMesh<itkVolumeType>>(volume);

      m_segmentation->output()->setData(volume);
      m_segmentation->output()->setData(mesh);
    }
    else
    {
      if(m_contourVolume)
      {
        auto volume = volumetricData(output);
        volume->resize(boundingBox(volume->bounds(), m_contourVolume->bounds().bounds()));
        volume->draw(m_contourVolume, value);
      }
    }
  }

  //-------------------------------------------------------------------------------------
  void ContourRasterizeUndoCommand::undo()
  {
    auto output = m_segmentation->output();

    if(m_createData)
    {
      output->removeData(SparseVolume<itkVolumeType>::TYPE);
      output->removeData(MarchingCubesMesh<itkVolumeType>::TYPE);
    }
    else
    {
      if(m_contourVolume)
      {
        auto volume = volumetricData(output);
        volume->undo();
      }
    }

    m_tool->setContour(m_contour);
  }

} /* namespace ESPINA */

