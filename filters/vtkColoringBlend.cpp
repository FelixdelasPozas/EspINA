#include "vtkColoringBlend.h"

// VTK
#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include <vtkInformation.h>
#include <vtkInformationVector.h>

#include <algorithm>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkPointData.h>
//#include "vtkMultiProcessController.h"

#include <cmath>

#include <iostream>
#include <vtkAlgorithmOutput.h>

#include <cassert>

vtkStandardNewMacro(vtkColoringBlend);


vtkColoringBlend::vtkColoringBlend()
: m_init(false)
, m_numBlendedInputs(0)
{
  //! Port 0: Blending inputs
  //! Port 1: Reference colors
  this->SetNumberOfInputPorts(2);
  this->SetNumberOfOutputPorts(1);
  invalidateRequestArea();
}

void vtkColoringBlend::AddInputConnection(int port, vtkAlgorithmOutput* input)
{
  vtkImageData *inputImage = vtkImageData::SafeDownCast(input->GetProducer()->GetOutputDataObject(0));
  
  requestArea(inputImage);
  vtkAlgorithm::AddInputConnection(port, input);
}


int vtkColoringBlend::FillInputPortInformation(int port, vtkInformation* info)
{
  if (port == 0) // Any number volume volume images
  {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkImageData");
    info->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 1);
    return 1;
  }
  else 
    if (port == 1)// Reference Colors
    {
      info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1); //WARNING: Make it mandatory?
      info->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 1);
      info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
      return 1;
    }

  vtkErrorMacro("This filter does not have more than 2 input port!");
  return 0;
}


int vtkColoringBlend::RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  
  //m_numInputs = inputVector[0]->GetNumberOfInformationObjects();
  
//   vtkInformation **inInfo = (vtkInformation**)malloc(m_numInputs*sizeof(vtkInformation*));
//   vtkImageData **input = (vtkImageData**)malloc(m_numInputs*sizeof(vtkImageData*));
//   InputPixelType **inPtr = (InputPixelType**)malloc(m_numInputs*sizeof(InputPixelType*));
//   
  for (int i  = 0; i < m_inputs.size(); i++)
  {
//     inInfo[i] = inputVector[0]->GetInformationObject(i);
//     input[i]  = vtkImageData::SafeDownCast(inInfo[i]->Get(vtkDataObject::DATA_OBJECT()));
    m_inputs[i].ptr = (InputPixelType *)(m_inputs[i].image->GetScalarPointer());
    std::cout << "Scalar type: " << m_inputs[i].image->GetScalarType() << std::endl;
  }
  
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkImageData *output = vtkImageData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkDebugMacro(<< "Creating Output Image Dimensions");
  
  output->SetDimensions(m_inputs[0].dims);
  output->SetSpacing(m_inputs[0].spacing);
  output->SetOrigin(m_inputs[0].image->GetOrigin());
  
  //output->SetScalarTypeToUnsignedChar();
  //output->SetNumberOfScalarComponents(3);
  //output->AllocateScalars();
  
  OutputPixelType *outPtr = (OutputPixelType *)(output->GetScalarPointer());
  
  
  
  if (!m_init)
  {
    vtkDebugMacro(<< "Copying first input image");
    vtkDebugMacro(<< "Dimensions " << m_inputs[0].dims[0] << " " << m_inputs[0].dims[1] << " " << m_inputs[0].dims[2]);
    
    int *extent = m_inputs[0].extent;
    
    for (int x = extent[0]; x < extent[1]; x++)
      for (int y = extent[2]; y < extent[3]; y++)
	for (int z = extent[4]; z < extent[5]; z++)
	{
	  int px = x+y*m_inputs[0].dims[0]+z*m_inputs[0].dims[0]*m_inputs[0].dims[1];
	  // Blend other images over first input
	  outPtr[3*px] = m_inputs[0].ptr[px];
	  outPtr[3*px+1] = m_inputs[0].ptr[px];
	  outPtr[3*px+2] = m_inputs[0].ptr[px];
	}
	m_numBlendedInputs = 1;
	m_init = true;
  }
  
  
  for(int i = m_numBlendedInputs; i < m_inputs.size(); i++)
  {
    int updateRegion[6];
    for (int dim = 0; dim < 3; dim++)
    {
      updateRegion[2*dim] = std::max(m_requestedArea[2*dim],m_inputs[i].requestedArea[2*dim]);
      updateRegion[2*dim+1] = std::min(m_requestedArea[2*dim+1],m_inputs[i].requestedArea[2*dim+1]);
    }
    m_inputs[i].color[0] = 255;//(255*rand())%255;
    m_inputs[i].color[1] = m_inputs[i].color[2] = 0;
    for (int x = updateRegion[0]; x < updateRegion[1]; x++)
      for (int y = updateRegion[2]; y < updateRegion[3]; y++)
	for (int z = updateRegion[4]; z < updateRegion[5]; z++)
	{
	  int px = x+y*m_inputs[0].dims[0]+z*m_inputs[0].dims[0]*m_inputs[0].dims[1];
	  int rpx = (x-m_inputs[i].requestedArea[0]) 
	    + (y-m_inputs[i].requestedArea[2])*m_inputs[i].dims[0]
	    + (z-m_inputs[i].requestedArea[4])*m_inputs[i].dims[0]*m_inputs[i].dims[1];
	  int colored = m_inputs[i].ptr[rpx];
	  if (colored)
	  {
	    for (int c = 0; c < 3; c++)
	    {
	      double r = (m_inputs[i].color[c]*0.8)/255;//Alpha
	      double f = 1.0 - r;
	      outPtr[3*px+c] = outPtr[3*px+c]*f + m_inputs[i].color[c]*0.8*r;
	    }
	  }
	}
  }
  
  output->GetPointData()->GetScalars()->SetName(m_inputs[0].image->GetPointData()->GetScalars()->GetName());
  //output->GetPointData()->SetScalars(output->GetPointData()->GetScalars());
  
  m_numBlendedInputs = m_inputs.size();
  invalidateRequestArea();
  
  return 1;
}

int vtkColoringBlend::RequestInformation(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  // get the info objects
  //   vtkInformation *inInfo = inputVector[0]->GetInformationObject(0); 
  vtkInformation* outInfo = 
    outputVector->GetInformationObject(0);  
  vtkDataObject::SetPointDataActiveScalarInfo(
    outInfo, VTK_UNSIGNED_CHAR, 3);
  
  return 1;
}



void vtkColoringBlend::PrintSelf(ostream& os, vtkIndent indent)
{
    vtkImageAlgorithm::PrintSelf(os, indent);
}


void vtkColoringBlend::invalidateRequestArea()
{
  m_requestedArea[0] = m_requestedArea[2] = m_requestedArea[4] = 0;
  m_requestedArea[1] = m_requestedArea[3] = m_requestedArea[5] = -1;
}


void vtkColoringBlend::requestArea(vtkImageData *inputImage)
{
  Input input;
  
  inputImage->Update();
  
  for (int i = 0; i < m_inputs.size();i++)
  {
    if (m_inputs[i].image == inputImage)
    {
      vtkDebugMacro(<< "Input already added");
      return;
    }
  }
  
  input.image = inputImage;
  inputImage->GetBounds(input.bounds);
  inputImage->GetSpacing(input.spacing);
  inputImage->GetExtent(input.extent);
  inputImage->GetDimensions(input.dims);
  for (int i = 0; i<6; i++)
    input.requestedArea[i] = input.bounds[i] / input.spacing[i/2];
  
  m_inputs.push_back(input);
  
  if (m_requestedArea[1] < m_requestedArea[0])// Invalid area
    memcpy(m_requestedArea,input.requestedArea,6*sizeof(int));
  else
  {
    for (int dim = 0; dim < 3; dim++)
    {
      m_requestedArea[2*dim] = std::min(m_requestedArea[2*dim],input.requestedArea[2*dim]);
      m_requestedArea[2*dim+1] = std::max(m_requestedArea[2*dim+1],input.requestedArea[2*dim+1]);
    }
  }
}

