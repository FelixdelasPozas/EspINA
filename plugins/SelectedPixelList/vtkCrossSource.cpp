#include "vtkCrossSource.h"

// VTK
#include "vtkObjectFactory.h"
#include "vtkImageData.h"
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkSmartPointer.h>

// DEBUG
#include <iostream>
#include <cassert>

vtkStandardNewMacro(vtkCrossSource);

void printExtent(int extent[6])
{
    std::cout << 
    extent[0] <<  " " << extent[1] <<  " " << 
    extent[2] <<  " " << extent[3] <<  " " << 
    extent[4] <<  " " << extent[5] <<  " " << 
    std::endl;
}

vtkCrossSource::vtkCrossSource()
: Radius(3)
{
  bzero(Center,3*sizeof(int));
  
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
}

int vtkCrossSource::RequestData(vtkInformation* request,
				vtkInformationVector** inputVector,
				vtkInformationVector* outputVector)
{
  vtkInformation* outInfo = outputVector->GetInformationObject(0); 

  vtkImageData *output = vtkImageData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()) );
  
  vtkSmartPointer<vtkImageData> cross =
    vtkSmartPointer<vtkImageData>::New();
    
  int xMin = Center[0] - Radius;
  int xMax = Center[0] + Radius;
  int yMin = Center[1] - Radius;
  int yMax = Center[1] + Radius;
  int zMin = Center[2];
  int zMax = zMin;
  
  cross->SetExtent(xMin,xMax, yMin, yMax, zMin, zMax);
  cross->SetScalarTypeToUnsignedChar();
  cross->SetNumberOfScalarComponents(1);
  cross->AllocateScalars();
  
  for (int x = xMin; x <= xMax; x++)
    for (int y = yMin; y <= yMax; y++)
      for (int z = zMin; z <= zMax; z++)
	  if (x == Center[0] || y == Center[1])
	    cross->SetScalarComponentFromDouble(x,y,z,0,255);
	  else
	    cross->SetScalarComponentFromDouble(x,y,z,0,0);
	
  output->ShallowCopy(cross);
  output->CopyInformation(cross);
//   output->SetScalarTypeToUnsignedChar();
//   output->SetNumberOfScalarComponents(1);
//   output->SetExtent(cross->GetExtent());
  output->SetUpdateExtent(output->GetExtent());
  output->SetWholeExtent(output->GetExtent());
  
  return 1;
}

void vtkCrossSource::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkImageAlgorithm::PrintSelf(os, indent);
}