/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#include "EdgeDetector.h"

#include "AdaptiveEdges.h"

#include "Core/Model/Channel.h"

//VTK
#include <vtkCellArray.h>
#include <vtkAlgorithmOutput.h>
#include <vtkAlgorithm.h>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkOBBTree.h>

#include <QDebug>

using namespace EspINA;

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
EdgeDetector::EdgeDetector(AdaptiveEdges *extension,
                           QObject* parent)
: QThread(parent)
, m_extension(extension)
{
  m_extension->m_mutex.lock();
}

//------------------------------------------------------------------------
EdgeDetector::~EdgeDetector()
{
}

//------------------------------------------------------------------------
void EdgeDetector::run()
{
  Channel *channel       = m_extension->channel();

  vtkAlgorithm  *producer = channel->volume()->toVTK()->GetProducer();
  vtkDataObject *output   = producer->GetOutputDataObject(0);
  vtkImageData  *image    = vtkImageData::SafeDownCast(output);

  vtkSmartPointer<vtkPoints> borderVertices = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> faces       = vtkSmartPointer<vtkCellArray>::New();

  int dim[3];
  image->GetDimensions(dim);

  double spacing[3];
  image->GetSpacing(spacing);
  int extent[6];
  image->GetExtent(extent);

  AdaptiveEdges::ExtensionData &data = m_extension->s_cache[m_extension->m_channel].Data;
  data.ComputedVolume = 0;

  //   vtkDebugMacro( << "Looking for borders");

  int numComponets = image->GetNumberOfScalarComponents();
  unsigned char *imagePtr = static_cast<unsigned char *>(image->GetScalarPointer());

  assert(extent[5] == dim[2]-1);

  unsigned long zMin = extent[4];
  unsigned long zMax = extent[5];
  unsigned long yMax = dim[1];
  unsigned long xMax = dim[0];
  // defined in unsigned char values range
  const int upperThreshold = (data.BackgroundColor + data.Threshold) > 255 ? 255 : data.BackgroundColor + data.Threshold;
  const int lowerThreshold = (data.BackgroundColor - data.Threshold) < 0 ? 0 : data.BackgroundColor - data.Threshold;
  vtkIdType lastCell[4];
  for (unsigned long z = zMin; z <= zMax; z++)
  {
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
      bool singlePixel = true;;
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

    //NOTE: Espina's Counting Region Definition is used here.
    // Upper slice is the first of the stack and lower the last one
    // Left Top Corner corresponds to pixel (0,0,0), Right Top to (N,0,0)
    // and so on
    double LB[3], LT[3], RT[3], RB[3];
    vtkIdType cell[4];

    // Left Bottom Corner
    face->GetPoint(1, LB);
    cell[0] = borderVertices->InsertNextPoint(LB);

    // Left Top Corner
    face->GetPoint(0, LT);
    cell[1] = borderVertices->InsertNextPoint(LT);

    // Right Top Corner
    face->GetPoint(2, RT);
    cell[2] = borderVertices->InsertNextPoint(RT);

    // Right Bottom Corner
    face->GetPoint(3, RB);
    cell[3] = borderVertices->InsertNextPoint(RB);

    if (z == zMin)
    {
      // Upper Inclusion Face
      faces->InsertNextCell(4, cell);
    } else if (z == zMax)
    {
      // Lower Inclusion Face
      faces->InsertNextCell(4, cell);
    } else
    {
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
    memcpy(lastCell,cell,4*sizeof(vtkIdType));

    if (z != zMin)
      data.ComputedVolume += ((RT[0] - LT[0] + 1)*(LB[1] - LT[1] + 1))*spacing[2];
  }

  data.Edges->SetPoints(borderVertices);
  data.Edges->SetPolys(faces);

  m_extension->s_cache.markAsClean(m_extension->m_channel);

  m_extension->m_mutex.unlock();
}
