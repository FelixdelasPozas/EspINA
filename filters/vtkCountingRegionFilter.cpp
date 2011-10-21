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


#include "vtkCountingRegionFilter.h"

#include <vtkObjectFactory.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkImageData.h>
#include <vtkArrayData.h>
#include <vtkSmartPointer.h>
#include <vtkIntArray.h>

#include <assert.h>
#include <vtkPolyData.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>

#include <math.h>

class BoundingBox
{
public:
  double xMin, xMax;
  double yMin, yMax;
  double zMin, zMax;
  
  BoundingBox(vtkPoints *points);
  BoundingBox(vtkImageData *image);
  bool intersect(BoundingBox &bb);
  BoundingBox intersection(BoundingBox &bb);
  
private:
  BoundingBox(){}
};

BoundingBox::BoundingBox(vtkPoints* points)
{
  assert(points->GetNumberOfPoints());
  // Init Bounding Box
  xMax = xMin = points->GetPoint(0)[0];
  yMax = yMin = points->GetPoint(0)[1];
  zMax = zMin = points->GetPoint(0)[2];
  
  for (int p=1; p < points->GetNumberOfPoints(); p++)
  {
    xMin = std::min(xMin, points->GetPoint(p)[0]);
    xMax = std::max(xMax, points->GetPoint(p)[0]);
    yMin = std::min(yMin, points->GetPoint(p)[1]);
    yMax = std::max(yMax, points->GetPoint(p)[1]);
    zMin = std::min(zMin, points->GetPoint(p)[2]);
    zMax = std::max(zMax, points->GetPoint(p)[2]);
  }
}

BoundingBox::BoundingBox(vtkImageData* image)
{
  double bounds[6];
  image->GetBounds(bounds);
  xMin = bounds[0];
  xMax = bounds[1];
  yMin = bounds[2];
  yMax = bounds[3];
  zMin = bounds[4];
  zMax = bounds[5];
}


bool BoundingBox::intersect(BoundingBox& bb)
{
  bool xOverlap = xMin <= bb.xMax && xMax >= bb.xMin;
  bool yOverlap = yMin <= bb.yMax && yMax >= bb.yMin;
  bool zOverlap = zMin <= bb.zMax && zMax >= bb.zMin;
  
  return xOverlap && yOverlap && zOverlap;
}

BoundingBox BoundingBox::intersection(BoundingBox& bb)
{
  BoundingBox res;
  res.xMin = std::max(xMin, bb.xMin);
  res.xMax = std::min(xMax, bb.xMax);
  res.yMin = std::max(yMin, bb.yMin);
  res.yMax = std::min(yMax, bb.yMax);
  res.zMin = std::max(zMin, bb.zMin);
  res.zMax = std::min(zMax, bb.zMax);
  return res;
}



vtkStandardNewMacro(vtkCountingRegionFilter);

vtkCountingRegionFilter::vtkCountingRegionFilter()
{
  this->SetNumberOfInputPorts(1);
}

vtkCountingRegionFilter::~vtkCountingRegionFilter()
{

}

// int vtkCountingRegionFilter::RequestInformation(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
// {
//   // get the info objects
//   vtkInformation* outInfo = outputVector->GetInformationObject(0);
//   vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
// //   vtkInformation *inInfo2 = inputVector[1]->GetInformationObject(0);
// 
//   int ext[6], ext2[6], idx;
// 
//   inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),ext);
//   outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),ext,6);
// 
//   return 1;
// 
// }


int vtkCountingRegionFilter::FillInputPortInformation(int port, vtkInformation* info)
{
  if (port == 0) // Any number volume volume images
  {
//     info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
      info->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 1);
    return 1;
  }
  else 
//     if (port == 1)// The bounding regions which defines valid/invalid inputs
//     {
//       info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
//       info->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 1);
//       info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
//       return 1;
//     }

  vtkErrorMacro("This filter does not have more than 2 input port!");
  return 0;
}

/*
int vtkCountingRegionFilter::FillOutputPortInformation(int port, vtkInformation* info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkArrayData");
  return 1;
  if (port == 0)// Valid inputs
  {
    return 1;
  }
  else
    if (port == 1)// Discarted outputs
    {
      return 1;
    }
  vtkErrorMacro("This filter does not have more than 2 output port!");
  return 0;
}
*/

bool realCollision(vtkImageData *input, BoundingBox interscetion)
{
  double spacing[3];
  input->GetSpacing(spacing);
  for (int z = interscetion.zMin; z <= interscetion.zMax; z++)
    for (int y = interscetion.yMin; y <= interscetion.yMax; y++)
      for (int x = interscetion.xMin; x <= interscetion.xMax; x++)
      {
	int px = round(x/spacing[0]);
	int py = round(y/spacing[1]);
	int pz = round(z/spacing[2]);
	if (input->GetScalarComponentAsDouble(px,py,pz,0))
	  return true;
      }
  
  return false;
}

bool discartedByRegion(vtkImageData *input, BoundingBox &inputBB, vtkPolyData *region)
{
  vtkPoints *regionPoints = region->GetPoints();
  vtkCellArray *regionFaces = region->GetPolys();
  vtkCellData *faceData = region->GetCellData();
  
  BoundingBox regionBB(regionPoints);
  
  // If there is no intersection (nor is inside), then it is discarted
  if (!inputBB.intersect(regionBB))
    return true;

  bool collisionDected = false;
  // Otherwise, we have to test all faces collisions
  int numOfCells = regionFaces->GetNumberOfCells();
  regionFaces->InitTraversal();
  for(int f=0; f < numOfCells; f++)
  {
    vtkIdType npts, *pts;
    regionFaces->GetNextCell(npts, pts);
    
    vtkSmartPointer<vtkPoints> facePoints = vtkSmartPointer<vtkPoints>::New();
    for (int i=0; i < npts; i++)
      facePoints->InsertNextPoint(regionPoints->GetPoint(pts[i]));
    
    BoundingBox faceBB(facePoints);
    if (inputBB.intersect(faceBB) && realCollision(input, inputBB.intersection(faceBB)))
    {
      if (faceData->GetScalars()->GetComponent(f,0) == 0)
	return true;
      collisionDected = true;
    }
  }

  if (collisionDected)
    return false;
  
  // If no collision was detected we have to check for inclusion
  for (int p=0; p < regionPoints->GetNumberOfPoints(); p +=8)
  {
    vtkSmartPointer<vtkPoints> slicePoints = vtkSmartPointer<vtkPoints>::New();
    for (int i=0; i < 8; i++)
	slicePoints->InsertNextPoint(regionPoints->GetPoint(p+i));
    
    BoundingBox sliceBB(slicePoints);
    if (inputBB.intersect(sliceBB) &&  realCollision(input, inputBB.intersection(sliceBB)))
      return false;//;
  }
  
  // If no internal collision was detected, then the input was indeed outside our 
  // bounding region
  return true;
}

int vtkCountingRegionFilter::RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkInformation *inputInfo = inputVector[0]->GetInformationObject(0);
  int numSeg = inputVector[0]->GetNumberOfInformationObjects();

  vtkImageData *input = NULL;
  if (numSeg)
    input = vtkImageData::SafeDownCast(inputInfo->Get(vtkDataObject::DATA_OBJECT()));

  BoundingBox inputBB(input);
  
  Discarted = 0;
  for (int r = 1; r < numSeg; r++)
  { 
    vtkInformation *regionInfo =  inputVector[0]->GetInformationObject(r);
    vtkPolyData *region= vtkPolyData::SafeDownCast(regionInfo->Get(vtkDataObject::DATA_OBJECT()));
    
    if (Discarted = discartedByRegion(input, inputBB, region))
      break;
  }
  
  return 1;
}