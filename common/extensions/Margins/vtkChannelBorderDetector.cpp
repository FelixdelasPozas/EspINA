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


#include "vtkChannelBorderDetector.h"

#include <vtkObjectFactory.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkDataObject.h>
#include <vtkImageData.h>

#include <math.h>

#include <assert.h>

#include <vtkOBBTree.h>
#include <vtkCellData.h>

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkChannelBorderDetector);

//-----------------------------------------------------------------------------
vtkChannelBorderDetector::vtkChannelBorderDetector()
: TotalVolume(0)
, TotalAdaptiveVolume(0)
, m_init(false)
{
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
}

//-----------------------------------------------------------------------------
vtkChannelBorderDetector::~vtkChannelBorderDetector()
{
}

//-----------------------------------------------------------------------------
int vtkChannelBorderDetector::FillInputPortInformation(int port, vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
  return 1;
}

//-----------------------------------------------------------------------------
int vtkChannelBorderDetector::FillOutputPortInformation(int port, vtkInformation* info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
  return 1;
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPoints> plane(const double corner[3], const double max[3],
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

  return points;
}

//-----------------------------------------------------------------------------
void vtkChannelBorderDetector::computeChannelMargins(vtkImageData* image)
{
  borderVertices = vtkSmartPointer<vtkPoints>::New();
  faces          = vtkSmartPointer<vtkCellArray>::New();

  int dim[3];
  image->GetDimensions(dim);

  double spacing[3];
  image->GetSpacing(spacing);
  int extent[6];
  image->GetExtent(extent);

  vtkDebugMacro(<< "Dim: " << dim[0] << "," << dim[1] << "," << dim[2]);
  vtkDebugMacro(<< "Spacing: " << spacing[0] << "," << spacing[1] << "," << spacing[2]);
  vtkDebugMacro( << "Looking for borders");

  int numComponets = image->GetNumberOfScalarComponents();
  unsigned char *imagePtr = static_cast<unsigned char *>(image->GetScalarPointer());

  assert(extent[5] == dim[2]-1);

  unsigned long zMin = extent[4];
  unsigned long zMax = extent[5];
  unsigned long yMax = dim[1];
  unsigned long xMax = dim[0];
  const int blackThreshold = 50;
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
	  nonBlackPixel = nonBlackPixel || (imagePtr[pxId+c] > blackThreshold);

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

    double leftBottom[3], rightBottom[3], rightTop[3], leftTop[3];

    double point[3];
    vtkIdType cell[4];
    // Left Top Corner
    face->GetPoint(0, point);
    face->GetPoint(0, leftTop);
    int leftMargin = point[0];
    int topMargin = point[1];
    cell[0] = borderVertices->InsertNextPoint(point);
    //     std::cout << "Point " << cell[0] << ": " << point[0] << " " << point[1] << " " << point[2] << std::endl;

    // Right Top Corner
    face->GetPoint(2, point);
    face->GetPoint(2, rightTop);
    cell[1] = borderVertices->InsertNextPoint(point);
    //     std::cout << "Point " << cell[1] << ": " << point[0] << " " << point[1] << " " << point[2] << std::endl;

    // Right Bottom Corner
    face->GetPoint(3, point);//WARNING: I use clockwise order from 0,0,0 according to espina's view
    face->GetPoint(3, rightBottom);
    int rightMargin = point[0];
    int bottomMargin = point[1];
    cell[2] = borderVertices->InsertNextPoint(point);
    //     std::cout << "Point " << cell[2] << ": " << point[0] << " " << point[1] << " " << point[2] << std::endl;

    // Left Bottom Corner
    face->GetPoint(1, point);//WARNING: I use clockwise order from 0,0,0 according to espina's view
    face->GetPoint(1, leftBottom);
    cell[3] = borderVertices->InsertNextPoint(point);
    //     std::cout << "Point " << cell[3] << ": " << point[0] << " " << point[1] << " " << point[2] << std::endl;
    //     std::cout << "Face: " << cell[0] << " " << cell[1] << " " << cell[2] << " " << cell[3] << std::endl;

    TotalAdaptiveVolume += ((rightMargin - leftMargin + 1)*(bottomMargin - topMargin+1));

//     assert(leftBottom[0] < rightBottom[0]);
//     assert(leftTop[0] < rightTop[0]);
//     assert(leftBottom[1] > leftTop[1]);
//     assert(rightBottom[1] > rightTop[1]);

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
  }

  TotalVolume = (extent[1]-extent[0]+1)*(extent[3]-extent[2]+1)*(extent[5]-extent[4]+1);
}

//-----------------------------------------------------------------------------
int vtkChannelBorderDetector::RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
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

  if (!m_init)
  {
    computeChannelMargins(image);
    m_init = true;
  }

  region->SetPoints(borderVertices);
  region->SetPolys(faces);

  return 1;
}
