/*
    Copyright (c) 2011, Jorge Peña <jorge.pena.pastor@gmail.com>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the <organization> nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY Jorge Peña <jorge.pena.pastor@gmail.com> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL Jorge Peña <jorge.pena.pastor@gmail.com> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "vtkBoundingRegionFilter.h"

#include <vtkObjectFactory.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkDataObject.h>
#include <vtkImageData.h>

#include <math.h>

#include <assert.h>
#include <vtkCellArray.h>

#include <vtkSmartPointer.h>
#include <vtkOBBTree.h>

vtkStandardNewMacro(vtkBoundingRegionFilter);

vtkBoundingRegionFilter::vtkBoundingRegionFilter()
{
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
}

vtkBoundingRegionFilter::~vtkBoundingRegionFilter()
{

}

int vtkBoundingRegionFilter::FillInputPortInformation(int port, vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
  return 1;
}


int vtkBoundingRegionFilter::FillOutputPortInformation(int port, vtkInformation* info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
  return 1;
}

vtkSmartPointer<vtkPoints> corners(const double corner[3], const double max[3],
                                   const double mid[3], const double min[3])
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
  /*
  // {0,0,1} <- in a cube
  x[0] = corner[0] + min[0];
  x[1] = corner[1] + min[1];
  x[2] = corner[2] + min[2];
  points->InsertNextPoint(x);
  // {1,0,1} <- in a cube
  x[0] = corner[0] + mid[0] + min[0];
  x[1] = corner[1] + mid[1] + min[1];
  x[2] = corner[2] + mid[2] + min[2];
  points->InsertNextPoint(x);
  // {0,1,1} <- in a cube
  x[0] = corner[0] + max[0] + min[0];
  x[1] = corner[1] + max[1] + min[1];
  x[2] = corner[2] + max[2] + min[2];
  points->InsertNextPoint(x);
  // {1,1,1} <- in a cube
  x[0] = corner[0] + max[0] + mid[0] + min[0];
  x[1] = corner[1] + max[1] + mid[1] + min[1];
  x[2] = corner[2] + max[2] + mid[2] + min[2];
  points->InsertNextPoint(x);
  */

  return points;
}


int vtkBoundingRegionFilter::RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkInformation *imageInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *regionInfo = outputVector->GetInformationObject(0);

  // Access to the input image (stack)
  vtkImageData *image = vtkImageData::SafeDownCast(
                          imageInfo->Get(vtkDataObject::DATA_OBJECT())
                        );
  vtkPolyData* region = vtkPolyData::SafeDownCast(
                          regionInfo->Get(vtkDataObject::DATA_OBJECT())
                        );

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> vertex = vtkSmartPointer<vtkCellArray>::New();

  int dim[3];
  image->GetDimensions(dim);

  double spacing[3];
  image->GetSpacing(spacing);

  std::cout << "Dim: " << dim[0] << "," << dim[1] << "," << dim[2] << std::endl;
  std::cout << "Spacing: " << spacing[0] << "," << spacing[1] << "," << spacing[2] << std::endl;

  vtkDebugMacro( << "Looking for borders");

  int numComponets = image->GetNumberOfScalarComponents();
  unsigned char *imagePtr = static_cast<unsigned char *>(image->GetScalarPointer());

  //TODO: Min values are 0 or given by extent???
  int zMin = std::max(Exclusion[2], 0);
  int zMax = std::min(Inclusion[2], dim[2]);
  for (int z = zMin; z < zMax; z++)
  {
    // Look for images borders in slice Z
    vtkSmartPointer<vtkPoints> nonBlackPoints = vtkSmartPointer<vtkPoints>::New();
    for (int y = 0; y < dim[1]; y++)
    {
      bool inside = false;
      double p1[3], p2[3];
      for (int x = 0; x < dim[0]; x++)
      {
        bool regionPixel = false;
        int pxId = x + y * dim[0] + z * dim[0] * dim[1];
        for (int c = 0; c < numComponets; c++)
          regionPixel = regionPixel || (imagePtr[pxId+c] > 10);

        if (regionPixel)
        {
          if (inside)
          {
            p2[0] = x * spacing[0];
            p2[1] = y * spacing[1];
            p2[2] = z * spacing[2];
          }
          else
          {
            p1[0] = x * spacing[0];
            p1[1] = y * spacing[1];
            p1[2] = z * spacing[2];
            inside = true;
	    //vtkIdType miId = points->InsertNextPoint(p1);
	    //vertex->InsertNextCell(1,&miId);
            nonBlackPoints->InsertNextPoint(p1);
          }
        }
      }
      if (inside)
      {
	//vtkIdType miId = points->InsertNextPoint(p2);
	//vertex->InsertNextCell(1,&miId);
	nonBlackPoints->InsertNextPoint(p2);
      }
    }
    double corner[3], max[3], mid[3], min[3], size[3];
    vtkSmartPointer<vtkOBBTree> obb_tree = vtkSmartPointer<vtkOBBTree>::New();
    obb_tree->ComputeOBB(nonBlackPoints, corner, max, mid, min, size);
     vtkSmartPointer<vtkPoints> imgCorners = corners(corner,max,mid,min);
     assert(imgCorners->GetNumberOfPoints() == 4);
     double imgCorner[3];
     vtkIdType id[4];
     // Bottom Left Corner
     imgCorners->GetPoint(0,imgCorner);
     imgCorner[0] += Left * spacing[0];
     imgCorner[1] += Bottom * spacing[1];
     id[0] = points->InsertNextPoint(imgCorner);
     //vertex->InsertNextCell(1,&id);
     // Bottom Right Corner
     imgCorners->GetPoint(2,imgCorner);
     imgCorner[0] -= Right * spacing[0];
     imgCorner[1] += Bottom * spacing[1];
     id[3] = points->InsertNextPoint(imgCorner);
     //vertex->InsertNextCell(1,&id);
     // Top Left Corner
     imgCorners->GetPoint(1,imgCorner);
     imgCorner[0] += Left * spacing[0];
     imgCorner[1] -= Top * spacing[1];
     id[1] = points->InsertNextPoint(imgCorner);
     //vertex->InsertNextCell(1,&id);
     // Top Right Corner
     imgCorners->GetPoint(3,imgCorner);
     imgCorner[0] -= Right * spacing[0];
     imgCorner[1] -= Top * spacing[1];
     id[2] = points->InsertNextPoint(imgCorner);
     vertex->InsertNextCell(4,id);
  }
  
  region->SetPoints(points);
  region->SetPolys(vertex);
  return 1;
}

