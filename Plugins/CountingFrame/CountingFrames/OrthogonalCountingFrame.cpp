/*
    
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

// Plugin
#include "CountingFrames/OrthogonalCountingFrame.h"
#include "Extensions/CountingFrameExtension.h"
#include "CountingFrames/vtkCountingFrameSliceWidget.h"
#include "CountingFrames/vtkCountingFrame3DWidget.h"

// ESPINA
#include <Core/Analysis/Channel.h>
#include <GUI/View/View2D.h>
#include <GUI/View/View3D.h>

// VTK
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkPolyData.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>

using namespace ESPINA;
using namespace ESPINA::CF;

//-----------------------------------------------------------------------------
OrthogonalCountingFrame::OrthogonalCountingFrame(CountingFrameExtension                *channelExt,
                                                 Nm                                     inclusion[3],
                                                 Nm                                     exclusion[3])
: CountingFrame(channelExt, inclusion, exclusion)
, m_bounds(channelExt->extendedItem()->bounds())
{
  updateCountingFrame();
}


//-----------------------------------------------------------------------------
OrthogonalCountingFrame::~OrthogonalCountingFrame()
{
}

//-----------------------------------------------------------------------------
void OrthogonalCountingFrame::updateCountingFrameImplementation()
{
  Nm Left   = m_bounds[0] + m_inclusion[0];
  Nm Top    = m_bounds[2] + m_inclusion[1];
  Nm Front  = m_bounds[4] + m_inclusion[2];
  Nm Right  = m_bounds[1] - m_exclusion[0];
  Nm Bottom = m_bounds[3] - m_exclusion[1];
  Nm Back   = m_bounds[5] - m_exclusion[2];

  m_innerFrame = createRectangularRegion(Left, Top, Front, Right, Bottom, Back);

  m_countingFrame = createCountingFramePolyData();

  {
    QWriteLocker lock(&m_channelEdgesMutex);

    m_channelEdges = createRectangularRegion(m_bounds[0], m_bounds[2], m_bounds[4],
                                             m_bounds[1], m_bounds[3], m_bounds[5]);
  }

  auto channel = m_extension->extendedItem();
  auto spacing = channel->output()->spacing();

  m_totalVolume = equivalentVolume(m_bounds);

  // Extract bounds corresponding to excluded voxels
  Bounds inclusionBounds{Left, Right-spacing[0], Top, Bottom-spacing[1], Front, Back-spacing[2]};
  m_inclusionVolume = equivalentVolume(inclusionBounds);
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> OrthogonalCountingFrame::createRectangularRegion(Nm left,  Nm top,    Nm front,
                                                                              Nm right, Nm bottom, Nm back)
{
  auto region   = vtkSmartPointer<vtkPolyData>::New();
  auto vertex   = vtkSmartPointer<vtkPoints>::New();
  auto faces    = vtkSmartPointer<vtkCellArray>::New();
  auto faceData = vtkSmartPointer<vtkIntArray>::New();

  vtkIdType frontFace[4], leftFace[4] , topFace[4];
  vtkIdType backFace[4] , rightFace[4], bottomFace[4];

  // Front Inclusion Face
  frontFace[0] = vertex->InsertNextPoint(left,  bottom, front );
  frontFace[1] = vertex->InsertNextPoint(left,  top,    front );
  frontFace[2] = vertex->InsertNextPoint(right, top,    front );
  frontFace[3] = vertex->InsertNextPoint(right, bottom, front );
  faces->InsertNextCell(4, frontFace);
  faceData->InsertNextValue(INCLUSION_FACE);

  // Back Exclusion Face
  backFace[0] = vertex->InsertNextPoint(left,  bottom, back);
  backFace[1] = vertex->InsertNextPoint(left,  top,    back);
  backFace[2] = vertex->InsertNextPoint(right, top,    back);
  backFace[3] = vertex->InsertNextPoint(right, bottom, back);
  faces->InsertNextCell(4, backFace);
  faceData->InsertNextValue(EXCLUSION_FACE);

  // Left Inclusion Face
  leftFace[0] = frontFace[0];
  leftFace[1] = frontFace[1];
  leftFace[2] = backFace[1];
  leftFace[3] = backFace[0];
  faces->InsertNextCell(4, leftFace);
  faceData->InsertNextValue(INCLUSION_FACE);

  // Right Exclusion Face
  rightFace[0] = frontFace[2];
  rightFace[1] = frontFace[3];
  rightFace[2] = backFace[3];
  rightFace[3] = backFace[2];
  faces->InsertNextCell(4, rightFace);
  faceData->InsertNextValue(EXCLUSION_FACE);

  // Top Inclusion Face
  topFace[0] = frontFace[1];
  topFace[1] = frontFace[2];
  topFace[2] = backFace[2];
  topFace[3] = backFace[1];
  faces->InsertNextCell(4, topFace);
  faceData->InsertNextValue(INCLUSION_FACE);

  // Bottom Exclusion Face
  bottomFace[0] = frontFace[3];
  bottomFace[1] = frontFace[0];
  bottomFace[2] = backFace[0];
  bottomFace[3] = backFace[3];
  faces->InsertNextCell(4, bottomFace);
  faceData->InsertNextValue(EXCLUSION_FACE);

  region->SetPoints(vertex);
  region->SetPolys(faces);

  auto data = region->GetCellData();
  data->SetScalars(faceData);
  data->GetScalars()->SetName("Type");

  return region;
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> OrthogonalCountingFrame::createCountingFramePolyData()
{
  auto vertex   = vtkSmartPointer<vtkPoints>::New();
  auto faces    = vtkSmartPointer<vtkCellArray>::New();
  auto faceData = vtkSmartPointer<vtkIntArray>::New();

  Nm left   = m_bounds[0] + m_inclusion[0];
  Nm top    = m_bounds[2] + m_inclusion[1];
  Nm front  = m_bounds[4] + m_inclusion[2];
  Nm right  = m_bounds[1] - m_exclusion[0];
  Nm bottom = m_bounds[3] - m_exclusion[1];
  Nm back   = m_bounds[5] - m_exclusion[2];

  vtkIdType frontFace[4], backFace[4], leftFace[4], topFace[4], cell[4];

  frontFace[0] = vertex->InsertNextPoint(left,  bottom, front);
  frontFace[1] = vertex->InsertNextPoint(left,  top,    front);
  frontFace[2] = vertex->InsertNextPoint(right, top,    front);
  frontFace[3] = vertex->InsertNextPoint(right, bottom, front);
  backFace[0]  = vertex->InsertNextPoint(left,  bottom, back);
  backFace[1]  = vertex->InsertNextPoint(left,  top,    back);
  backFace[2]  = vertex->InsertNextPoint(right, top,    back);
  backFace[3]  = vertex->InsertNextPoint(right, bottom, back);

  // Front Inclusion Face
  faces->InsertNextCell(4, frontFace);
  faceData->InsertNextValue(INCLUSION_FACE);

  // Left Inclusion Face
  leftFace[0] = frontFace[0];
  leftFace[1] = frontFace[1];
  leftFace[2] = backFace[1];
  leftFace[3] = backFace[0];
  faces->InsertNextCell(4, leftFace);
  faceData->InsertNextValue(INCLUSION_FACE);

  // Top Inclusion Face
  topFace[0] = frontFace[1];
  topFace[1] = frontFace[2];
  topFace[2] = backFace[2];
  topFace[3] = backFace[1];
  faces->InsertNextCell(4, topFace);
  faceData->InsertNextValue(INCLUSION_FACE);

  // Back Exclusion Face
  cell[0] = vertex->InsertNextPoint(m_bounds[0], bottom,      back);
  cell[1] = vertex->InsertNextPoint(m_bounds[0], m_bounds[2], back);
  cell[2] = vertex->InsertNextPoint(right,       m_bounds[2], back);
  cell[3] = vertex->InsertNextPoint(right,       bottom,      back);
  faces->InsertNextCell(4, cell);
  faceData->InsertNextValue(EXCLUSION_FACE);

  // Right Exclusion Face
  cell[0] = vertex->InsertNextPoint(right, m_bounds[2], m_bounds[0]);
  cell[1] = vertex->InsertNextPoint(right, bottom,      m_bounds[0]);
  cell[2] = vertex->InsertNextPoint(right, bottom,      back);
  cell[3] = vertex->InsertNextPoint(right, m_bounds[2], back);
  faces->InsertNextCell(4, cell);
  faceData->InsertNextValue(EXCLUSION_FACE);

  // Bottom Exclusion Face
  cell[0] = vertex->InsertNextPoint(right,        bottom, m_bounds[4]);
  cell[1] = vertex->InsertNextPoint(m_bounds[0],  bottom, m_bounds[4]);
  cell[2] = vertex->InsertNextPoint(m_bounds[0],  bottom, back);
  cell[3] = vertex->InsertNextPoint(right,        bottom, back);
  faces->InsertNextCell(4, cell);
  faceData->InsertNextValue(EXCLUSION_FACE);

  auto countingFrame = vtkSmartPointer<vtkPolyData>::New();
  countingFrame->SetPoints(vertex);
  countingFrame->SetPolys(faces);

  auto data = countingFrame->GetCellData();
  data->SetScalars(faceData);
  data->GetScalars()->SetName("Type");

  return countingFrame;
}
