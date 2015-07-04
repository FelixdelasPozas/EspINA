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
#include <Core/Analysis/Channel.h>

//VTK
#include <vtkCellArray.h>
#include <vtkAlgorithmOutput.h>
#include <vtkAlgorithm.h>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkOBBTree.h>
#include <itkImageToVTKImageFilter.h>

// Qt
#include <QDebug>

using namespace ESPINA;

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPoints> plane(const double corner[3],
                                 const double max[3],
                                 const double mid[3],
                                 const double min[3])
{
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  double x[3];
  // {0,0,0} <- in a cube
  x[0] = corner[0];
  x[1] = corner[1];
  x[2] = corner[2];
  points->InsertNextPoint(x);
  // {1,0,0} <- in a cube
  x[0] = corner[0] + mid[0];
  x[1] = corner[1] + mid[1];
  x[2] = corner[2] + mid[2];
  points->InsertNextPoint(x);
  // {0,1,0} <- in a cube
  x[0] = corner[0] + max[0];
  x[1] = corner[1] + max[1];
  x[2] = corner[2] + max[2];
  points->InsertNextPoint(x);
  // {1,1,0} <- in a cube
  x[0] = corner[0] + max[0] + mid[0];
  x[1] = corner[1] + max[1] + mid[1];
  x[2] = corner[2] + max[2] + mid[2];
  points->InsertNextPoint(x);

  return points;
}

//------------------------------------------------------------------------
AdaptiveEdgesCreator::AdaptiveEdgesCreator(ChannelEdges *extension,
                                           SchedulerSPtr  scheduler)
: Task(scheduler)
, m_extension(extension)
{
}

//------------------------------------------------------------------------
AdaptiveEdgesCreator::~AdaptiveEdgesCreator()
{
}

//------------------------------------------------------------------------
void AdaptiveEdgesCreator::run()
{
  using Itk2VtkFilter = itk::ImageToVTKImageFilter<itkVolumeType>;
  //qDebug() << "Creating Adaptive Edges" << m_extension->m_extendedItem->name();

  auto channel = m_extension->extendedItem();
  //qDebug() << "Channel Bounds" << channel->bounds().toString();

  auto volume = readLockVolume(channel->output());
  auto bounds = volume->bounds();

  itkVolumeType::Pointer itkImage = volume->itkImage();

  Itk2VtkFilter::Pointer itk2vtk = Itk2VtkFilter::New();
  itk2vtk->ReleaseDataFlagOn();
  itk2vtk->SetInput(itkImage);
  itk2vtk->Update();

  vtkImageData *vtkImage = itk2vtk->GetOutput();

  vtkSmartPointer<vtkPoints>    borderVertices = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> faces          = vtkSmartPointer<vtkCellArray>::New();

  int dim[3];
  vtkImage->GetDimensions(dim);

  double spacing[3];
  vtkImage->GetSpacing(spacing);
  int extent[6];
  vtkImage->GetExtent(extent);

  int backgroundColor = m_extension->m_backgroundColor;
  int threshold       = m_extension->m_threshold;

  m_extension->m_computedVolume = 0;

  //   vtkDebugMacro( << "Looking for borders");

  int numComponets = vtkImage->GetNumberOfScalarComponents();
  unsigned char *imagePtr = static_cast<unsigned char *>(vtkImage->GetScalarPointer());

  assert(extent[5] == dim[2]-1);

  unsigned long zMin = extent[4];
  unsigned long zMax = extent[5];
  unsigned long yMax = dim[1];
  unsigned long xMax = dim[0];

  // defined in unsigned char values range
  const int upperThreshold = (backgroundColor + threshold) > 255 ? 255 : backgroundColor + threshold;
  const int lowerThreshold = (backgroundColor - threshold) <   0 ?   0 : backgroundColor - threshold;

  vtkIdType lastCell[4] = {-1, -1, -1, -1};
  unsigned long z = zMin;
  while (canExecute() && z <= zMax)
  {
    reportProgress(double(z - zMin) / double(zMax - zMin)*100.0);
    // Look for images borders in z slice:
    // We are going to take all bordering pixels (almost black) and then extract its oriented
    // bounding box.
    // We ignore pixles until we find the first non-black pixel
    // Then, we keep last non-black pixel as the other side of the line
    vtkSmartPointer<vtkPoints> nonBlackPixels = vtkSmartPointer<vtkPoints>::New();
    for (unsigned long y = 0; y < yMax; y++)
    {
      bool nonBlackPixelDetected = false;
      double p1[3], p2[3];
      bool singlePixel = true;
      for (unsigned long x = 0; x < xMax; x++)
      {
        bool nonBlackPixel = false;
        unsigned long pxId = x*numComponets + y * dim[0]*numComponets + z * dim[0] * dim[1]*numComponets; //check numComponents
        for (int c = 0; c < numComponets; c++)
          nonBlackPixel = nonBlackPixel || (imagePtr[pxId+c] > upperThreshold) || (imagePtr[pxId+c] < lowerThreshold);

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
    vtkSmartPointer<vtkOBBTree> obb_tree = vtkSmartPointer<vtkOBBTree>::New();
    obb_tree->ComputeOBB(nonBlackPixels, corner, max, mid, min, size);

    vtkSmartPointer<vtkPoints> face = plane(corner,max,mid,min);
    assert(face->GetNumberOfPoints() == 4);

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
    double correctedLeft  = std::max(LB[0], LT[0]);
    correctedLeft = int((correctedLeft - bounds[0]) / spacing[0]);
    correctedLeft = bounds[0] + (correctedLeft)*spacing[0];

    double correctedRight = std::min(RB[0], RT[0]);
    correctedRight = int((correctedRight - bounds[0]) / spacing[0]);
    correctedRight = bounds[0] + (correctedRight + 1)*spacing[0]; // the edge ends at the end of the voxel

    double correctedTop  = std::max(LT[1], RT[1]);
    correctedTop = int((correctedTop - bounds[2]) / spacing[1]);
    correctedTop = bounds[2] + (correctedTop)*spacing[1];

    double correctedBottom  = std::min(LB[1], RB[1]);
    correctedBottom = int((correctedBottom - bounds[2]) / spacing[1]);
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
    } else
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
      //face->GetPoint(1, LB);
      LB[2] += spacing[2];
      cell[0] = borderVertices->InsertNextPoint(LB);

      // Left Top Corner
      //face->GetPoint(0, LT);
      LT[2] += spacing[2];
      cell[1] = borderVertices->InsertNextPoint(LT);

      // Right Top Corner
      //face->GetPoint(2, RT);
      RT[2] += spacing[2];
      cell[2] = borderVertices->InsertNextPoint(RT);

      // Right Bottom Corner
      //face->GetPoint(3, RB);
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

  if (isAborted())
  {
    m_extension->m_computedVolume = 0;
  }
  else
  {
    m_extension->m_edges->SetPoints(borderVertices);
    m_extension->m_edges->SetPolys(faces);

    reportProgress(100);
  }

  m_extension->m_edgesResultMutex.unlock();

  //qDebug() << "Adaptive Edges Created" << m_extension->m_extendedItem->name();
}
