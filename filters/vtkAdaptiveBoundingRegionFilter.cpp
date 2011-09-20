/*
 *    Copyright (c) 2011, Jorge Peña <jorge.pena.pastor@gmail.com>
 *    All rights reserved.
 * 
 *    Redistribution and use in source and binary forms, with or without
 *    modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 * Neither the name of the <organization> nor the
 *        names of its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
 * 
 *    THIS SOFTWARE IS PROVIDED BY Jorge Peña <jorge.pena.pastor@gmail.com> ''AS IS'' AND ANY
 *    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *    DISCLAIMED. IN NO EVENT SHALL Jorge Peña <jorge.pena.pastor@gmail.com> BE LIABLE FOR ANY
 *    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "vtkAdaptiveBoundingRegionFilter.h"

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
#include <vtkCellData.h>

#define Left Inclusion[0]
#define Bottom Inclusion[1]
#define Upper Inclusion[2]
#define Right Exclusion[0]
#define Top Exclusion[1]
#define Lower Exclusion[2]

const int INCLUSION_FACE = 255;
const int EXCLUSION_FACE = 0;

vtkStandardNewMacro(vtkAdaptiveBoundingRegionFilter);

vtkAdaptiveBoundingRegionFilter::vtkAdaptiveBoundingRegionFilter()
{
  bzero(Inclusion,3*sizeof(int));
  bzero(Exclusion,3*sizeof(int));
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
}

vtkAdaptiveBoundingRegionFilter::~vtkAdaptiveBoundingRegionFilter()
{
  
}

int vtkAdaptiveBoundingRegionFilter::FillInputPortInformation(int port, vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
  return 1;
}


int vtkAdaptiveBoundingRegionFilter::FillOutputPortInformation(int port, vtkInformation* info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
  return 1;
}

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
  /*NOTE: We only want a plane, not a volume
   *  // {0,0,1} <- in a cube
   *  x[0] = corner[0] + min[0];
   *  x[1] = corner[1] + min[1];
   *  x[2] = corner[2] + min[2];
   *  points->InsertNextPoint(x);
   *  // {1,0,1} <- in a cube
   *  x[0] = corner[0] + mid[0] + min[0];
   *  x[1] = corner[1] + mid[1] + min[1];
   *  x[2] = corner[2] + mid[2] + min[2];
   *  points->InsertNextPoint(x);
   *  // {0,1,1} <- in a cube
   *  x[0] = corner[0] + max[0] + min[0];
   *  x[1] = corner[1] + max[1] + min[1];
   *  x[2] = corner[2] + max[2] + min[2];
   *  points->InsertNextPoint(x);
   *  // {1,1,1} <- in a cube
   *  x[0] = corner[0] + max[0] + mid[0] + min[0];
   *  x[1] = corner[1] + max[1] + mid[1] + min[1];
   *  x[2] = corner[2] + max[2] + mid[2] + min[2];
   *  points->InsertNextPoint(x);
   */
  
  return points;
}


int vtkAdaptiveBoundingRegionFilter::RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
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
  
  vtkSmartPointer<vtkPoints> vertex = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> faces = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkIntArray> faceData = vtkSmartPointer<vtkIntArray>::New();
  
  int dim[3];
  image->GetDimensions(dim);
  
  double spacing[3];
  image->GetSpacing(spacing);
  
  vtkDebugMacro(<< "Dim: " << dim[0] << "," << dim[1] << "," << dim[2]);
  vtkDebugMacro(<< "Spacing: " << spacing[0] << "," << spacing[1] << "," << spacing[2]);
  vtkDebugMacro( << "Looking for borders");
  
  int numComponets = image->GetNumberOfScalarComponents();
  unsigned char *imagePtr = static_cast<unsigned char *>(image->GetScalarPointer());
  
  //TODO: Min values are 0 or given by extent???
  int zMin = std::max(Exclusion[2], 0);
  int zMax = std::min(Inclusion[2], dim[2]-1);
  
  const int blackThreshold = 10;
    vtkIdType lastCell[4];
  for (int z = zMin; z <= zMax; z++)
  {
    // Look for images borders in z slice:
    // We are going to take all bordering pixels (almost black) and then extract its oriented
    // bounding box.
    // We ignore pixles until we find the first non-black pixel
    // Then, we keep last non-black pixel as the other side of the line
    vtkSmartPointer<vtkPoints> nonBlackPixels = vtkSmartPointer<vtkPoints>::New();
    for (int y = 0; y < dim[1]; y++)
    {
      bool nonBlackPixelDetected = false;
      double p1[3], p2[3];
      for (int x = 0; x < dim[0]; x++)
      {
	bool nonBlackPixel = false;
	int pxId = x*numComponets + y * dim[0]*numComponets + z * dim[0] * dim[1]*numComponets; //check numComponents
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
      if (nonBlackPixelDetected)
	nonBlackPixels->InsertNextPoint(p2);
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
    // Left Bottom Corner
    face->GetPoint(0, point);
    face->GetPoint(0, leftBottom);
    point[0] += Left * spacing[0];
    point[1] += Bottom * spacing[1];
    point[0] = round(point[0]); point[1] = abs(round(point[1])); point[2] = round(point[2]);
    cell[0] = vertex->InsertNextPoint(point);
//     std::cout << "Point " << cell[0] << ": " << point[0] << " " << point[1] << " " << point[2] << std::endl;
    // Right Bottom Corner
    face->GetPoint(2, point);
    face->GetPoint(2, rightBottom);
    point[0] -= Right * spacing[0];
    point[1] += Bottom * spacing[1];
    point[0] = round(point[0]); point[1] = abs(round(point[1]));  point[2] = round(point[2]);
    cell[1] = vertex->InsertNextPoint(point);
//     std::cout << "Point " << cell[1] << ": " << point[0] << " " << point[1] << " " << point[2] << std::endl;
    // Right Top Corner
    face->GetPoint(3, point);
    face->GetPoint(3, rightTop);
    point[0] -= Right * spacing[0];
    point[1] -= Top * spacing[1];
    point[0] = round(point[0]); point[1] = abs(round(point[1]));  point[2] = round(point[2]);
    cell[2] = vertex->InsertNextPoint(point);
//     std::cout << "Point " << cell[2] << ": " << point[0] << " " << point[1] << " " << point[2] << std::endl;
    // Left Top Corner
    face->GetPoint(1, point);
    face->GetPoint(1, leftTop);
    point[0] += Left * spacing[0];
    point[1] -= Top * spacing[1];
    point[0] = round(point[0]); point[1] = abs(round(point[1]));  point[2] = round(point[2]);
    cell[3] = vertex->InsertNextPoint(point);
//     std::cout << "Point " << cell[3] << ": " << point[0] << " " << point[1] << " " << point[2] << std::endl;
    
//     std::cout << "Face: " << cell[0] << " " << cell[1] << " " << cell[2] << " " << cell[3] << std::endl;
    
    assert(leftBottom[0] < rightBottom[0]);
    assert(leftTop[0] < rightTop[0]);
    assert(leftBottom[1] < leftTop[1]);
    assert(rightBottom[1] < rightTop[1]);
    
    if (z == zMax-1)
    { 
      // Upper Inclusion Face
      faces->InsertNextCell(4, cell);
      faceData->InsertNextValue(EXCLUSION_FACE);
    } else if (z == zMin)
    {
      // Lower Inclusion Face
      faces->InsertNextCell(4, cell);
      faceData->InsertNextValue(INCLUSION_FACE);
    } else
    {
      // Create lateral faces
      
      // Left Inclusion Face
      vtkIdType left[4];
      left[0] = lastCell[0];
      left[1] = lastCell[1];
      left[2] = cell[1];
      left[3] = cell[0];
      faces->InsertNextCell(4, left);
      faceData->InsertNextValue(INCLUSION_FACE);
      
      // Right Exclusion Face
      vtkIdType right[4];
      right[0] = lastCell[3];
      right[1] = lastCell[2];
      right[2] = cell[2];
      right[3] = cell[3];
      faces->InsertNextCell(4, right);
      faceData->InsertNextValue(EXCLUSION_FACE);
      
      // Top Inclusion Face
      vtkIdType top[4];
      top[0] = lastCell[1];
      top[1] = lastCell[2];
      top[2] = cell[2];
      top[3] = cell[1];
      faces->InsertNextCell(4, top);
      faceData->InsertNextValue(INCLUSION_FACE);
      
      // Bottom Exclusion Face
      vtkIdType bottom[4];
      bottom[0] = lastCell[0];
      bottom[1] = lastCell[3];
      bottom[2] = cell[3];
      bottom[3] = cell[0];
      faces->InsertNextCell(4, bottom);
      faceData->InsertNextValue(EXCLUSION_FACE);
      
    }
    memcpy(lastCell,cell,4*sizeof(vtkIdType));
  }
  
  region->SetPoints(vertex);
  region->SetPolys(faces);  
  vtkCellData *data = region->GetCellData();
  data->SetScalars(faceData);
  data->GetScalars()->SetName("Type");
  
  return 1;
}

