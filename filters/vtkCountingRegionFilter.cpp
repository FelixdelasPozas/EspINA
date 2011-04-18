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

vtkStandardNewMacro(vtkCountingRegionFilter);

vtkCountingRegionFilter::vtkCountingRegionFilter()
{
  this->SetNumberOfInputPorts(2);
  this->SetNumberOfOutputPorts(1);
}

vtkCountingRegionFilter::~vtkCountingRegionFilter()
{

}

int vtkCountingRegionFilter::FillInputPortInformation(int port, vtkInformation* info)
{
  if (port == 0) // Any number volume volume images
  {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkImageData");
    return 1;
  }
  else 
    if (port == 1)// The bounding regions which defines valid/invalid inputs
    {
      info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
      info->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 1);
      info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
      return 1;
    }

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


int vtkCountingRegionFilter::RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkInformation *inInfo0 = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  
  //int numConnections = outputVector->GetNumberOfInformationObjects();
  //vtkInformation *outInfo1 = outputVector->GetInformationObject(0);
  int numSeg = inputVector[0]->GetNumberOfInformationObjects();
  int numRegions = inputVector[1]->GetNumberOfInformationObjects();

  vtkImageData *input0 = NULL;
  if (numSeg)
    input0 = vtkImageData::SafeDownCast(inInfo0->Get(vtkDataObject::DATA_OBJECT()));
  
  Discarted = 0;
  vtkPolyData *regions = NULL;
  vtkInformation *regionInfo = NULL;
  for (int r = 0; r < numRegions; r++)
  {
    regionInfo = inputVector[1]->GetInformationObject(r);
    regions = vtkPolyData::SafeDownCast(regionInfo->Get(vtkDataObject::DATA_OBJECT()));
    double bounds[6];
    regions->GetPoints()->GetBounds(bounds);
    if (numRegions > 1)
      Discarted = 1;
  }
/*
  vtkArrayData *result = vtkArrayData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT())
  );
  //vtkIntArray *output1 = vtkIntArray::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
  
  //vtkIntArray *output = vtkIntArray::GetData(outputVector);
  assert(!result->GetNumberOfArrays());
  vtkDenseArray<int> *array = vtkDenseArray<int>::New();
  array->Resize(1);
  
  array->SetValue(0,1);
    
  result->AddArray(array);*/
  return 1;
}