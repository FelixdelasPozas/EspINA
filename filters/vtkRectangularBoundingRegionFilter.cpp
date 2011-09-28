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

#define Left Inclusion[0]
#define Top Inclusion[1]
#define Upper Inclusion[2]
#define Right Exclusion[0]
#define Bottom Exclusion[1]
#define Lower Exclusion[2]

const int INCLUSION_FACE = 255;
const int EXCLUSION_FACE = 0;

vtkStandardNewMacro(vtkRectangularBoundingRegionFilter);

vtkRectangularBoundingRegionFilter::vtkRectangularBoundingRegionFilter()
{
  bzero(Inclusion,3*sizeof(int));
  bzero(Exclusion,3*sizeof(int));
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
}

vtkRectangularBoundingRegionFilter::~vtkRectangularBoundingRegionFilter()
{
}

// int vtkRectangularBoundingRegionFilter::FillInputPortInformation(int port, vtkInformation* info)
// {
//   return 1;
// }


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
                          regionInfo->Get(vtkDataObject::DATA_OBJECT())
                        );
  
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
  
  vtkIdType lower[4], right[4], bottom[4];
  vtkIdType upper[4], left[4], top[4];
  
  // Upper Inclusion Face
  upper[0] = vertex->InsertNextPoint(Left,Top,Upper);
  upper[1] = vertex->InsertNextPoint(Right,Top,Upper);
  upper[2] = vertex->InsertNextPoint(Right,Bottom,Upper);
  upper[3] = vertex->InsertNextPoint(Left,Bottom,Upper);
  faces->InsertNextCell(4, upper);
  faceData->InsertNextValue(INCLUSION_FACE);
  
  // Lower Exclusion Face
  lower[0] = vertex->InsertNextPoint(Left,Top,Lower);
  lower[1] = vertex->InsertNextPoint(Right,Top,Lower);
  lower[2] = vertex->InsertNextPoint(Right,Bottom,Lower);
  lower[3] = vertex->InsertNextPoint(Left,Bottom,Lower);
  faces->InsertNextCell(4, lower);
  faceData->InsertNextValue(EXCLUSION_FACE);
  
  
  // Bottom Exclusion Face
  bottom[0] = upper[2];
  bottom[1] = upper[3];
  bottom[2] = lower[3];
  bottom[3] = lower[2];
  faces->InsertNextCell(4, bottom);
  faceData->InsertNextValue(EXCLUSION_FACE);

  // Top Inclusion Face
  top[0] = upper[0];
  top[1] = upper[1];
  top[2] = lower[1];
  top[3] = lower[0];
  faces->InsertNextCell(4, top);
  faceData->InsertNextValue(INCLUSION_FACE);
  
  // Right Exclusion Face
  right[0] = upper[1];
  right[1] = upper[2];
  right[2] = lower[2];
  right[3] = lower[1];
  faces->InsertNextCell(4, right);
  faceData->InsertNextValue(EXCLUSION_FACE);

  // Left Inclusion Face
  left[0] = upper[3];
  left[1] = upper[0];
  left[2] = lower[0];
  left[3] = lower[3];
  faces->InsertNextCell(4, left);
  faceData->InsertNextValue(INCLUSION_FACE);
  
  region->SetPoints(vertex);
  region->SetPolys(faces);
  vtkCellData *data = region->GetCellData();
  data->SetScalars(faceData);
  data->GetScalars()->SetName("Type");
  return 1;
}

