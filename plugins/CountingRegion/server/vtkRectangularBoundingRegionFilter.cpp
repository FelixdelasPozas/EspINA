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


#include "vtkRectangularBoundingRegionFilter.h"

#include <vtkObjectFactory.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkDataObject.h>
#include <vtkImageData.h>

#include <math.h>

#include <assert.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>

#include <vtkSmartPointer.h>
#include <vtkOBBTree.h>

const int INCLUSION_FACE = 255;
const int EXCLUSION_FACE = 0;

vtkStandardNewMacro(vtkRectangularBoundingRegionFilter);

vtkRectangularBoundingRegionFilter::vtkRectangularBoundingRegionFilter()
: TotalVolume(0)
, InclusionVolume(0)
, ExclusionVolume(0)
{
  memset(Margin, 0, 6*sizeof(double));
  memset(InclusionOffset, 0, 3*sizeof(double));
  memset(ExclusionOffset, 0, 3*sizeof(double));
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
}

vtkRectangularBoundingRegionFilter::~vtkRectangularBoundingRegionFilter()
{
}

int vtkRectangularBoundingRegionFilter::FillOutputPortInformation(int port, vtkInformation* info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
  return 1;
}

// NOTE: Lower/Upper are z-inverted. This is because of cajal specifications of what a stack is
int vtkRectangularBoundingRegionFilter::RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkInformation *regionInfo = outputVector->GetInformationObject(0);

  vtkPolyData* region = vtkPolyData::SafeDownCast(
                          regionInfo->Get(vtkDataObject::DATA_OBJECT()));
//   std::cout << "Rectangular Filter: \n";
//   std::cout << "\tInclusion[0]: " << Inclusion[0] << std::endl;
//   std::cout << "\tInclusion[1]: " << Inclusion[1] << std::endl;
//   std::cout << "\tInclusion[2]: " << Inclusion[2] << std::endl;
//   std::cout << "\tExclusion[0]: " << Exclusion[0] << std::endl;
//   std::cout << "\tExclusion[1]: " << Exclusion[1] << std::endl;
//   std::cout << "\tExclusion[2]: " << Exclusion[2] << std::endl;

  vtkSmartPointer<vtkPoints> vertex = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> faces = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkIntArray> faceData = vtkSmartPointer<vtkIntArray>::New();

  vtkIdType upperFace[4], leftFace[4], topFace[4];
  vtkIdType lowerFace[4], rightFace[4], bottomFace[4];

  double Left   = left();
  double Top    = top();
  double Upper  = upper();
  double Right  = right();
  double Bottom = bottom();
  double Lower  = lower();

  // Upper Inclusion Face
  upperFace[0] = vertex->InsertNextPoint(Left,  Bottom, Upper);
  upperFace[1] = vertex->InsertNextPoint(Left,  Top,    Upper);
  upperFace[2] = vertex->InsertNextPoint(Right, Top,    Upper);
  upperFace[3] = vertex->InsertNextPoint(Right, Bottom, Upper);
  faces->InsertNextCell(4, upperFace);
  faceData->InsertNextValue(INCLUSION_FACE);

  // Lower Exclusion Face
  lowerFace[0] = vertex->InsertNextPoint(Left,  Bottom, Lower);
  lowerFace[1] = vertex->InsertNextPoint(Left,  Top,    Lower);
  lowerFace[2] = vertex->InsertNextPoint(Right, Top,    Lower);
  lowerFace[3] = vertex->InsertNextPoint(Right, Bottom, Lower);
  faces->InsertNextCell(4, lowerFace);
  faceData->InsertNextValue(EXCLUSION_FACE);

  // Left Inclusion Face
  leftFace[0] = upperFace[0];
  leftFace[1] = upperFace[1];
  leftFace[2] = lowerFace[1];
  leftFace[3] = lowerFace[0];
  faces->InsertNextCell(4, leftFace);
  faceData->InsertNextValue(INCLUSION_FACE);

  // Right Exclusion Face
  rightFace[0] = upperFace[2];
  rightFace[1] = upperFace[3];
  rightFace[2] = lowerFace[3];
  rightFace[3] = lowerFace[2];
  faces->InsertNextCell(4, rightFace);
  faceData->InsertNextValue(EXCLUSION_FACE);

  // Top Inclusion Face
  topFace[0] = upperFace[1];
  topFace[1] = upperFace[2];
  topFace[2] = lowerFace[2];
  topFace[3] = lowerFace[1];
  faces->InsertNextCell(4, topFace);
  faceData->InsertNextValue(INCLUSION_FACE);

  // Bottom Exclusion Face
  bottomFace[0] = upperFace[3];
  bottomFace[1] = upperFace[0];
  bottomFace[2] = lowerFace[0];
  bottomFace[3] = lowerFace[3];
  faces->InsertNextCell(4, bottomFace);
  faceData->InsertNextValue(EXCLUSION_FACE);

  region->SetPoints(vertex);
  region->SetPolys(faces);
  vtkCellData *data = region->GetCellData();
  data->SetScalars(faceData);
  data->GetScalars()->SetName("Type");

  TotalVolume = (Margin[1]-Margin[0]+1)*(Margin[3]-Margin[2]+1)*(Margin[5]-Margin[4]+1);
  InclusionVolume = (Right-Left)*(Top-Bottom)*(Upper-Lower);
  ExclusionVolume = TotalVolume - InclusionVolume;

  return 1;
}

