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

// ESPINA
#include "AdaptiveEdgesCreator.h"
#include "ChannelEdges.h"
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>
#include <Core/Analysis/Channel.h>
#include <Core/Types.h>

//VTK
#include <vtkCellArray.h>
#include <vtkAlgorithmOutput.h>
#include <vtkAlgorithm.h>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkOBBTree.h>
#include <itkImageToVTKImageFilter.h>
#include <vtkMath.h>

using namespace ESPINA;
using namespace ESPINA::Extensions;

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPoints> plane(const double corner[3],
                                 const double max[3],
                                 const double mid[3],
                                 const double min[3])
{
  auto points = vtkSmartPointer<vtkPoints>::New();
  points->SetDataTypeToDouble();
  points->SetNumberOfPoints(4);

  double x[3];
  // {0,0,0} <- in a cube
  x[0] = corner[0];
  x[1] = corner[1];
  x[2] = corner[2];
  points->InsertPoint(0,x);
  // {1,0,0} <- in a cube
  x[0] = corner[0] + mid[0];
  x[1] = corner[1] + mid[1];
  x[2] = corner[2] + mid[2];
  points->InsertPoint(1,x);
  // {0,1,0} <- in a cube
  x[0] = corner[0] + max[0];
  x[1] = corner[1] + max[1];
  x[2] = corner[2] + max[2];
  points->InsertPoint(2,x);
  // {1,1,0} <- in a cube
  x[0] = corner[0] + max[0] + mid[0];
  x[1] = corner[1] + max[1] + mid[1];
  x[2] = corner[2] + max[2] + mid[2];
  points->InsertPoint(3,x);

  return points;
}

//------------------------------------------------------------------------
AdaptiveEdgesCreator::AdaptiveEdgesCreator(ChannelEdges *extension,
                                           SchedulerSPtr  scheduler)
: Task       {scheduler}
, m_extension{extension}
{
}

//------------------------------------------------------------------------
AdaptiveEdgesCreator::~AdaptiveEdgesCreator()
{
}

//------------------------------------------------------------------------
void AdaptiveEdgesCreator::computeEdges()
{
  auto channel = m_extension->extendedItem();
  auto volume  = readLockVolume(channel->output());
  auto bounds  = volume->bounds();
  auto image   = vtkImage<itkVolumeType>(volume, bounds);

  auto faces = vtkSmartPointer<vtkCellArray>::New();
  auto borderVertices = vtkSmartPointer<vtkPoints>::New();
  borderVertices->SetDataTypeToDouble();

  int dim[3], extent[6];
  double spacing[3];
  image->GetDimensions(dim);
  image->GetExtent(extent);
  image->GetSpacing(spacing);

  auto backgroundColor = m_extension->m_backgroundColor;
  auto threshold       = m_extension->m_threshold;

  m_extension->m_computedVolume = 0;

  auto xMin = extent[0];
  auto xMax = extent[1];
  auto yMin = extent[2];
  auto yMax = extent[3];
  auto zMin = extent[4];
  auto zMax = extent[5];

  // defined in unsigned char values range
  const int upperThreshold = (backgroundColor + threshold) > 255 ? 255 : backgroundColor + threshold;
  const int lowerThreshold = (backgroundColor - threshold) <   0 ?   0 : backgroundColor - threshold;

  auto numComponents   = image->GetNumberOfScalarComponents();

  vtkIdType lastCell[4] = {-1, -1, -1, -1};
  auto z = zMin;
  while (canExecute() && z <= zMax)
  {
    if (zMax - zMin != 0)
    {
      reportProgress((static_cast<double>(z - zMin) / static_cast<double>(zMax - zMin))*50.0);
    }
    else
    {
      reportProgress(0);
    }

    // Look for images borders in z slice:
    // We are going to take all bordering pixels (almost black) and then extract its oriented
    // bounding box.
    // We ignore pixels until we find the first non-black pixel
    // Then, we keep last non-black pixel as the other side of the line
    auto nonBlackPixels = vtkSmartPointer<vtkPoints>::New();
    nonBlackPixels->SetDataTypeToDouble();

    for (auto y = yMin; y <= yMax; y++)
    {
      auto nonBlackPixelDetected = false;
      double p1[3], p2[3];
      auto singlePixel = true;
      for (auto x = xMin; x <= xMax; x++)
      {
        auto nonBlackPixel = false;
        auto pixelPtr = reinterpret_cast<unsigned char *>(image->GetScalarPointer(x,y,z));
        for (int c = 0; c < numComponents; c++)
        {
          nonBlackPixel = nonBlackPixel || (pixelPtr[c] > upperThreshold) || (pixelPtr[c] < lowerThreshold);
        }

        if (nonBlackPixel)
        {
          if (nonBlackPixelDetected)
          {
            // First border pixel
            p2[0] = x * spacing[0];
            p2[1] = y * spacing[1];
            p2[2] = z * spacing[2];
            singlePixel = false;
          }
          else
          {
            // Last border pixel
            p1[0] = x * spacing[0];
            p1[1] = y * spacing[1];
            p1[2] = z * spacing[2];
            nonBlackPixelDetected = true;
            nonBlackPixels->InsertNextPoint(p1);
          }
        }
      }
      if (nonBlackPixelDetected && !singlePixel)
      {
        nonBlackPixels->InsertNextPoint(p2);
      }
    }

    // Now we have to simplify the slice's borders to 4
    double corner[3], max[3], mid[3], min[3], size[3];
    auto obb_tree = vtkSmartPointer<vtkOBBTree>::New();
    obb_tree->ComputeOBB(nonBlackPixels, corner, max, mid, min, size);

    auto face = plane(corner,max,mid,min);

    //NOTE: Espina's Counting Frame Definition is used here.
    // Front slice is the first of the stack and back the last one
    // Left Top Corner corresponds to pixel (0,0,0), Right Top to (N,0,0)
    // and so on
    double LB[3], LT[3], RT[3], RB[3];
    vtkIdType cell[4];

    face->GetPoint(0, LT);
    face->GetPoint(1, LB);
    face->GetPoint(2, RT);
    face->GetPoint(3, RB);

    // Correct rotation
    auto correctedLeft  = std::min(LB[0], LT[0]);
    correctedLeft = vtkMath::Round((correctedLeft - bounds[0]) / spacing[0]);
    correctedLeft = bounds[0] + (correctedLeft)*spacing[0];

    auto correctedRight = std::max(RB[0], RT[0]);
    correctedRight = vtkMath::Round((correctedRight - bounds[0]) / spacing[0]);
    correctedRight = bounds[0] + (correctedRight + 1)*spacing[0]; // the edge ends at the end of the voxel

    auto correctedTop  = std::min(LT[1], RT[1]);
    correctedTop = vtkMath::Round((correctedTop - bounds[2]) / spacing[1]);
    correctedTop = bounds[2] + (correctedTop)*spacing[1];

    auto correctedBottom  = std::max(LB[1], RB[1]);
    correctedBottom = vtkMath::Round((correctedBottom - bounds[2]) / spacing[1]);
    correctedBottom = bounds[2] + (correctedBottom + 1)*spacing[1]; // the edge ends at the end of the voxel

    // Left Bottom Corner
    LB[0]  = correctedLeft;
    LB[1]  = correctedBottom;
    LB[2] -= 0.5*spacing[2];
    cell[0] = borderVertices->InsertNextPoint(LB);

    // Left Top Corner
    LT[0]  = correctedLeft;
    LT[1]  = correctedTop;
    LT[2] -= 0.5*spacing[2];
    cell[1] = borderVertices->InsertNextPoint(LT);

    // Right Top Corner
    RT[0]  = correctedRight;
    RT[1]  = correctedTop;
    RT[2] -= 0.5*spacing[2];
    cell[2] = borderVertices->InsertNextPoint(RT);

    // Right Bottom Corner
    RB[0]  = correctedRight;
    RB[1]  = correctedBottom;
    RB[2] -= 0.5*spacing[2];
    cell[3] = borderVertices->InsertNextPoint(RB);

    if (z == zMin)
    {
      // Upper Inclusion Face
      faces->InsertNextCell(4, cell);
    }
    else
    {
      Q_ASSERT(lastCell[0] != -1);
      Q_ASSERT(lastCell[1] != -1);
      Q_ASSERT(lastCell[2] != -1);
      Q_ASSERT(lastCell[3] != -1);
      // Create lateral faces

      // Left Inclusion Face
      vtkIdType left[4];
      left[0] = lastCell[3];
      left[1] = lastCell[0];
      left[2] = cell[0];
      left[3] = cell[3];
      faces->InsertNextCell(4, left);

      // Right Exclusion Face
      vtkIdType right[4];
      right[0] = lastCell[1];
      right[1] = lastCell[2];
      right[2] = cell[2];
      right[3] = cell[1];
      faces->InsertNextCell(4, right);

      // Top Inclusion Face
      vtkIdType top[4];
      top[0] = lastCell[0];
      top[1] = lastCell[1];
      top[2] = cell[1];
      top[3] = cell[0];
      faces->InsertNextCell(4, top);

      // Bottom Exclusion Face
      vtkIdType bottom[4];
      bottom[0] = lastCell[2];
      bottom[1] = lastCell[3];
      bottom[2] = cell[3];
      bottom[3] = cell[2];
      faces->InsertNextCell(4, bottom);
    }

    if (z == zMax)
    {
      // Left Bottom Corner
      LB[2] += spacing[2];
      cell[0] = borderVertices->InsertNextPoint(LB);

      // Left Top Corner
      LT[2] += spacing[2];
      cell[1] = borderVertices->InsertNextPoint(LT);

      // Right Top Corner
      RT[2] += spacing[2];
      cell[2] = borderVertices->InsertNextPoint(RT);

      // Right Bottom Corner
      RB[2] += spacing[2];
      cell[3] = borderVertices->InsertNextPoint(RB);

      // Create lateral faces
      // Left Inclusion Face
      vtkIdType left[4];
      left[0] = lastCell[3];
      left[1] = lastCell[0];
      left[2] = cell[0];
      left[3] = cell[3];
      faces->InsertNextCell(4, left);

      // Right Exclusion Face
      vtkIdType right[4];
      right[0] = lastCell[1];
      right[1] = lastCell[2];
      right[2] = cell[2];
      right[3] = cell[1];
      faces->InsertNextCell(4, right);

      // Top Inclusion Face
      vtkIdType top[4];
      top[0] = lastCell[0];
      top[1] = lastCell[1];
      top[2] = cell[1];
      top[3] = cell[0];
      faces->InsertNextCell(4, top);

      // Bottom Exclusion Face
      vtkIdType bottom[4];
      bottom[0] = lastCell[2];
      bottom[1] = lastCell[3];
      bottom[2] = cell[3];
      bottom[3] = cell[2];
      faces->InsertNextCell(4, bottom);

      // Lower Inclusion Face
      faces->InsertNextCell(4, cell);
    }

    memcpy(lastCell,cell,4*sizeof(vtkIdType));

    if (z != zMin)
    {
      m_extension->m_computedVolume += ((RT[0] - LT[0] + 1)*(LB[1] - LT[1] + 1))*spacing[2];
    }

    ++z;
  }

  QWriteLocker lock(&m_extension->m_dataMutex);

  if (isAborted())
  {
    m_extension->m_computedVolume = 0;
  }
  else
  {
    m_extension->m_edges = vtkSmartPointer<vtkPolyData>::New();
    m_extension->m_edges->SetPoints(borderVertices);
    m_extension->m_edges->SetPolys(faces);

    reportProgress(50);
  }
}

//------------------------------------------------------------------------
void AdaptiveEdgesCreator::computeFaces()
{
  if(isAborted()) return; // no edges if user cancelled the task.

  auto borderPoints = m_extension->m_edges->GetPoints();
  auto numSlices = m_extension->m_edges->GetNumberOfPoints()/4;

  for (int face = 0; face < 6 && canExecute(); face++)
  {
    auto faceCells = vtkCellArray::New();
    auto facePoints = vtkPoints::New();
    facePoints->SetDataTypeToDouble();

    if (face < 4)
    {
      for (int i = 0; i < numSlices; i++)
      {
        double p1[3], p2[3];
        switch(face)
        {
          case 0: // LEFT
            borderPoints->GetPoint((4*i)+0, p1);
            borderPoints->GetPoint((4*i)+1, p2);

            facePoints->InsertNextPoint(p1);
            facePoints->InsertNextPoint(p2);

            break;
          case 1: // RIGHT
            borderPoints->GetPoint((4*i)+2, p1);
            borderPoints->GetPoint((4*i)+3, p2);

            facePoints->InsertNextPoint(p1);
            facePoints->InsertNextPoint(p2);

            break;
          case 2: // TOP
            borderPoints->GetPoint((4*i)+1, p1);
            borderPoints->GetPoint((4*i)+2, p2);

            facePoints->InsertNextPoint(p1);
            facePoints->InsertNextPoint(p2);

            break;
          case 3: // BOTTOM
            borderPoints->GetPoint((4*i)+3, p1);
            borderPoints->GetPoint((4*i)+0, p2);

            facePoints->InsertNextPoint(p1);
            facePoints->InsertNextPoint(p2);

            break;
          default:
            Q_ASSERT(false);
            break;
        }

        if (i == 0)
          continue;

        vtkIdType corners[4];
        corners[0] = (i*2)-2;
        corners[1] = (i*2)-1;
        corners[2] = (i*2)+1;
        corners[3] = 2*i;
        faceCells->InsertNextCell(4,corners);
      }
    }
    else
    {
      vtkIdType corners[4];
      double p[3];
      switch(face)
      {
        case 4: // Front
        {
          for (int i = 0; i < 4; ++i)
          {
            borderPoints->GetPoint(i, p);
            corners[i] = facePoints->InsertNextPoint(p);
          }
          break;
        }
        case 5: // Back
        {
          auto np = borderPoints->GetNumberOfPoints();
          for (int i = 0; i < 4; ++i)
          {
            borderPoints->GetPoint(np - (4-i), p);
            corners[i] = facePoints->InsertNextPoint(p);
          }
          break;
        }
        default:
          Q_ASSERT(false);
          break;
      }
      faceCells->InsertNextCell(4,corners);
    }

    vtkSmartPointer<vtkPolyData> poly = vtkSmartPointer<vtkPolyData>::New();
    poly->SetPoints(facePoints);
    poly->SetPolys(faceCells);

    facePoints->Delete();
    faceCells->Delete();

    {
      QWriteLocker lock(&m_extension->m_dataMutex);

      m_extension->m_faces[face] = poly;
    }

    reportProgress(50 + (static_cast<double>(face)/6.0)*50.0);
 }
}

//------------------------------------------------------------------------
void AdaptiveEdgesCreator::run()
{
  computeEdges();
  computeFaces();

  m_extension->m_hasCreatedEdges = !isAborted();
  m_extension->m_edgesTask.wakeAll();
}
