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

#define Left InclusionOffset[0]
#define Top InclusionOffset[1]
#define Upper InclusionOffset[2]
#define Right ExclusionOffset[0]
#define Bottom ExclusionOffset[1]
#define Lower ExclusionOffset[2]

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
  borderVertices = vtkSmartPointer<vtkPoints>::New();
  faces          = vtkSmartPointer<vtkCellArray>::New();
  faceData       = vtkSmartPointer<vtkIntArray>::New();

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
  //TODO: Min values are 0 or given by extent???
  //int zMin = std::max(Inclusion[2], 0);
  //int zMax = std::min(Inclusion[2], dim[2]-1);
  assert(InclusionOffset[2] >= 0 && ExclusionOffset[2] >= 0);


  int zMin = std::min(extent[4] + int(InclusionOffset[2]), extent[5]);//TODO: change casting
  int zMax = std::max(extent[5] - int(ExclusionOffset[2]), extent[4]);
  const int blackThreshold = 50;
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
    if (z != zMax) // Don't include last exclusion face
      InclusionVolume += (((rightMargin - Right) - (leftMargin + Left))*((bottomMargin - Bottom) - (topMargin + Top)));

//     assert(leftBottom[0] < rightBottom[0]);
//     assert(leftTop[0] < rightTop[0]);
//     assert(leftBottom[1] > leftTop[1]);
//     assert(rightBottom[1] > rightTop[1]);

    if (z == zMin)
    {
      // Upper Inclusion Face
      faces->InsertNextCell(4, cell);
      faceData->InsertNextValue(INCLUSION_FACE);
    } else if (z == zMax)
    {
      // Lower Inclusion Face
      faces->InsertNextCell(4, cell);
      faceData->InsertNextValue(EXCLUSION_FACE);
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
      faceData->InsertNextValue(INCLUSION_FACE);

      // Right Exclusion Face
      vtkIdType right[4];
      right[0] = lastCell[1];
      right[1] = lastCell[2];
      right[2] = cell[2];
      right[3] = cell[1];
      faces->InsertNextCell(4, right);
      faceData->InsertNextValue(EXCLUSION_FACE);

      // Top Inclusion Face
      vtkIdType top[4];
      top[0] = lastCell[0];
      top[1] = lastCell[1];
      top[2] = cell[1];
      top[3] = cell[0];
      faces->InsertNextCell(4, top);
      faceData->InsertNextValue(INCLUSION_FACE);

      // Bottom Exclusion Face
      vtkIdType bottom[4];
      bottom[0] = lastCell[2];
      bottom[1] = lastCell[3];
      bottom[2] = cell[3];
      bottom[3] = cell[2];
      faces->InsertNextCell(4, bottom);
      faceData->InsertNextValue(EXCLUSION_FACE);
    }
    memcpy(lastCell,cell,4*sizeof(vtkIdType));
  }

  TotalVolume = (extent[1]-extent[0]+1)*(extent[3]-extent[2]+1)*(extent[5]-extent[4]+1);
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPoints> vtkAdaptiveBoundingRegionFilter::applyOffsets()
{
  vtkSmartPointer<vtkPoints> vertices = vtkSmartPointer<vtkPoints>::New();

  for(int v=0; v < borderVertices->GetNumberOfPoints(); v+=4)
  {
    double p0[3];
    borderVertices->GetPoint(v, p0);
    p0[0] = floor((p0[0] + Left)+0.5);
    p0[1] = floor((p0[1] + Top)+0.5);
    p0[2] = floor(p0[2]+0.5);
    vertices->InsertNextPoint(p0);

    double p1[3];
    borderVertices->GetPoint(v+1, p1);
    p1[0] = floor((p1[0] - Right)+0.5);
    p1[1] = floor((p1[1] + Top)+0.5);
    p1[2] = floor(p1[2]+0.5);
    vertices->InsertNextPoint(p1);

    double p2[3];
    borderVertices->GetPoint(v+2, p2);
    p2[0] = floor((p2[0] - Right)+0.5);
    p2[1] = floor((p2[1] - Bottom)+0.5);
    p2[2] = floor(p2[2]+0.5);
    vertices->InsertNextPoint(p2);

    double p3[3];
    borderVertices->GetPoint(v+3, p3);
    p3[0] = floor((p3[0] + Left)+0.5);
    p3[1] = floor((p3[1] - Bottom)+0.5);
    p3[2] = floor(p3[2]+0.5);
    vertices->InsertNextPoint(p3);
  }
  return vertices;
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

  if (!m_init
    || LastComputedUpper != InclusionOffset[2]
    || LastComputedLower != ExclusionOffset[2])
  {
    computeStackMargins(image);
    m_init = true;
    LastComputedUpper = InclusionOffset[2];
    LastComputedLower = ExclusionOffset[2];
  }

  region->SetPoints(applyOffsets());
  region->SetPolys(faces);  
  vtkCellData *data = region->GetCellData();
  data->SetScalars(faceData);
  data->GetScalars()->SetName("Type");

  ExclusionVolume = TotalVolume - InclusionVolume;
  ExclusionAdaptiveVolume = TotalAdaptiveVolume - InclusionVolume;

  return 1;
}

