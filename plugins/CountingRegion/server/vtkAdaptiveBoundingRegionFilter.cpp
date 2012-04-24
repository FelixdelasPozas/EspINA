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


const int INCLUSION_FACE = 255;
const int EXCLUSION_FACE = 0;

vtkStandardNewMacro(vtkAdaptiveBoundingRegionFilter);

vtkAdaptiveBoundingRegionFilter::vtkAdaptiveBoundingRegionFilter()
: TotalVolume(0)
, TotalAdaptiveVolume(0)
, InclusionVolume(0)
, ExclusionVolume(0)
, ExclusionAdaptiveVolume(0)
, m_init(false)
{
  memset(InclusionOffset, 0, 3*sizeof(int));
  memset(ExclusionOffset, 0, 3*sizeof(int));
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
void vtkAdaptiveBoundingRegionFilter::computeStackMargins(vtkImageData *image)
{
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

  //NOTE: if you read this, delete next line ;)
  assert(dim[2] == extent[5] - extent[4] + 1);

  stackBoderVertex = vtkSmartPointer<vtkPoints>::New();
  const int blackThreshold = 50;
  for (int z = extent[4]; z <= extent[5]; z++)
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
      bool singlePixel = true;;
      for (int x = 0; x < dim[0]; x++)
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


    //NOTE: Espina's Counting Region Definition is used here.
    // Upper slice is the first of the stack and lower the last one
    // Left Top Corner corresponds to pixel (0,0,0), Right Top to (N,0,0)
    // and so on
    double LB[3], LT[3], RT[3], RB[3];

    // Left Bottom Corner
    face->GetPoint(1, LB);
    stackBoderVertex->InsertNextPoint(LB);

    // Left Top Corner
    face->GetPoint(0, LT);
    stackBoderVertex->InsertNextPoint(LT);

    // Right Top Corner
    face->GetPoint(2, RT);
    stackBoderVertex->InsertNextPoint(RT);

    // Right Bottom Corner
    face->GetPoint(3, RB);
    stackBoderVertex->InsertNextPoint(RB);
  }
  TotalVolume = (extent[1]-extent[0]+1)*(extent[3]-extent[2]+1)*(extent[5]-extent[4]+1);
}

//-----------------------------------------------------------------------------
void vtkAdaptiveBoundingRegionFilter::createBoundingRegion(vtkImageData* image)
{

  double spacing[3];
  image->GetSpacing(spacing);
  int extent[6];
  image->GetExtent(extent);

  int inSliceOffset = upperOffset() / spacing[2];
  int exSliceOffset = lowerOffset() / spacing[2];

  int upperSlice = extent[4] + inSliceOffset;
  upperSlice = std::max(upperSlice, extent[4]);
  upperSlice = std::min(upperSlice, extent[5]);

  int lowerSlice = extent[5] + exSliceOffset;
  lowerSlice = std::max(lowerSlice, extent[4]);
  lowerSlice = std::min(lowerSlice, extent[5]);

  // upper and lower refer to Espina's orientation
  assert(upperSlice <= lowerSlice);

  regionVertex = vtkSmartPointer<vtkPoints>::New();
  faces        = vtkSmartPointer<vtkCellArray>::New();
  faceData     = vtkSmartPointer<vtkIntArray>::New();

  for (int slice = upperSlice; slice <= lowerSlice; slice++)
  {
    vtkIdType cell[4];
    vtkIdType lastCell[4];

    double LB[3], LT[3], RT[3], RB[3];

    stackBoderVertex->GetPoint(4*slice+0, LB);
    roundToSlice(LB[0], leftOffset());
    roundToSlice(LB[1], bottomOffset());
    roundToSlice(LB[2], 0);
    cell[0] = regionVertex->InsertNextPoint(LB);

    stackBoderVertex->GetPoint(4*slice+1, LT);
    roundToSlice(LT[0], leftOffset());
    roundToSlice(LT[1], topOffset());
    roundToSlice(LT[2], 0);
    cell[1] = regionVertex->InsertNextPoint(LT);

    stackBoderVertex->GetPoint(4*slice+2, RT);
    roundToSlice(RT[0], rightOffset());
    roundToSlice(RT[1], topOffset());
    roundToSlice(RT[2], 0);
    cell[2] = regionVertex->InsertNextPoint(RT);

    stackBoderVertex->GetPoint(4*slice+3, RB);
    roundToSlice(RB[0], rightOffset());
    roundToSlice(RB[1], bottomOffset());
    roundToSlice(RB[2], 0);
    cell[3] = regionVertex->InsertNextPoint(RB);
    if (slice == upperSlice)
    {
      // Upper Inclusion Face
      faces->InsertNextCell(4, cell);
      faceData->InsertNextValue(INCLUSION_FACE);
    } else if (slice == lowerSlice)
    {
      // Lower Inclusion Face
      faces->InsertNextCell(4, cell);
      faceData->InsertNextValue(EXCLUSION_FACE);
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
      right[0] = lastCell[2];
      right[1] = lastCell[3];
      right[2] = cell[3];
      right[3] = cell[2];
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
      bottom[0] = lastCell[3];
      bottom[1] = lastCell[0];
      bottom[2] = cell[0];
      bottom[3] = cell[3];
      faces->InsertNextCell(4, bottom);
      faceData->InsertNextValue(EXCLUSION_FACE);
    }
    memcpy(lastCell,cell,4*sizeof(vtkIdType));

    // Update Volumes
    TotalAdaptiveVolume += ((RT[0] - LT[0] + 1)*(LB[1] - LT[1] + 1));
    if (slice != lowerSlice) // Don't include last exclusion face
      InclusionVolume += (((RT[0] + rightOffset())  - (LT[0] + leftOffset()))*
                          ((LB[1] + bottomOffset()) - (LT[1] + topOffset())));
  }
}

//-----------------------------------------------------------------------------
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

  if (!m_init)
  {
    computeStackMargins(image);
    m_init = true;
  }

  createBoundingRegion(image);
  region->SetPoints(regionVertex);
  region->SetPolys(faces);  
  vtkCellData *data = region->GetCellData();
  data->SetScalars(faceData);
  data->GetScalars()->SetName("Type");

  ExclusionVolume = TotalVolume - InclusionVolume;
  ExclusionAdaptiveVolume = TotalAdaptiveVolume - InclusionVolume;

  return 1;
}

