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
#include <vtkStreamingDemandDrivenPipeline.h>

vtkStandardNewMacro(vtkColoringBlend);

bool validExtent(int *ext)
{
 return ext[1] > ext[0];
}

vtkColoringBlend::vtkColoringBlend()
: m_init(false)
{
  //! Port 0: Blending inputs
  //! Port 1: Reference colors
  this->SetNumberOfInputPorts(2);
  this->SetNumberOfOutputPorts(1);
//   invalidateRequestArea();
  m_newInputs.clear();
  m_removeInputs.clear();
  m_blendedInputs.clear();
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

// This method computes the input necessary to generate the output
int vtkColoringBlend::RequestUpdateExtent(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  
  int requestedUpdateExt[6];
  outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), requestedUpdateExt);
  
  for (int i = 0; i < inputVector[0]->GetNumberOfInformationObjects(); i++)
  {
    vtkInformation* inInfo = inputVector[0]->GetInformationObject(i);
    
    //WARNING: Maybe with threads/mpi we shpuld clip the extent
    inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT()),6);
    
//     if (i > 0)// Some inputs are not required to be updated //TODO
//     {
//       requestedUpdateExt[0] = requestedUpdateExt[2] = requestedUpdateExt[4] = 0;
//       requestedUpdateExt[1] = requestedUpdateExt[3] = requestedUpdateExt[5] = -1;
//       inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(),requestedUpdateExt, 6);
//     }
  }
  return 1;
}

void vtkColoringBlend::copyInput(vtkImageIterator< vtkColoringBlend::InputPixelType >& inIt, vtkImageIterator< vtkColoringBlend::OutputPixelType >& outIt)
{
  InputPixelType *inPtr;
  OutputPixelType *outPtr;
  
  while (!outIt.IsAtEnd())
  {
    // Process one row of pixles at a time
    inPtr = inIt.BeginSpan();
    outPtr = outIt.BeginSpan();
    OutputPixelType *outEndPtr = outIt.EndSpan();
    
    while (outPtr != outEndPtr)
    {
      for (int c = 0; c < 3; c++)
      {
	*outPtr = *inPtr;
	++outPtr;
      }
      ++inPtr;
    }
    inIt.NextSpan();
    outIt.NextSpan();
  }

}

void vtkColoringBlend::blendInputs(vtkImageIterator<InputPixelType> &inIt, vtkImageIterator<OutputPixelType> &outIt, OutputPixelType *color)
{
  InputPixelType *inPtr;// = (InputPixelType*)input->GetScalarPointer();
  OutputPixelType *outPtr;// = (OutputPixelType *)(output->GetScalarPointer());
  
  while (!outIt.IsAtEnd())
  {
    // Process one row of pixles at a time
    inPtr = inIt.BeginSpan();
    outPtr = outIt.BeginSpan();
    OutputPixelType *outEndPtr = outIt.EndSpan();
    
    while (outPtr != outEndPtr)
    {
      if (*inPtr)
      {
	for (int c = 0; c < 3; c++)
	{
	  double r = (color[c]*0.8)/255;//Alpha
	  double f = 1.0 - r;
	  *outPtr = *outPtr*f + color[c]*0.8*r;
	  ++outPtr;
	}
      }else
      {
	outPtr += 3;
      }
      ++inPtr;
    }
    inIt.NextSpan();
    outIt.NextSpan();
  }
}

bool intersect(int *reg1, int *reg2, int *intersection = NULL)
{
  int intExt[6];
  
  for (int i = 0; i < 3; i++)
  {
    intExt[i*2] = std::max(reg1[i*2],reg2[i*2]);
    intExt[1+i*2] = std::min(reg1[1+i*2],reg2[1+i*2]);
  }
//   intExt[0] = std::max(reg1[0],reg2[0]);
//   intExt[2] = std::max(reg1[2],reg2[2]);
//   intExt[4] = std::max(reg1[4],reg2[4]);
//   intExt[1] = std::min(reg1[1],reg1[1]);
//   intExt[3] = std::min(reg1[3],reg1[3]);
//   intExt[5] = std::min(reg1[5],reg1[5]);
  
  if (intersection)
    memcpy(intersection,intExt,6*sizeof(int));
  
  return intExt[0] <= intExt[1] && intExt[2] <=intExt[3] && intExt[4] <= intExt[5];
}

void transformAreaToExtent(int *area, int *areExtent, int *extent)
{
  extent[0] = area[0] - areExtent[0];
  extent[1] = area[1] - areExtent[0];
  extent[2] = area[2] - areExtent[2];
  extent[3] = area[3] - areExtent[2];
  extent[4] = area[4] - areExtent[4];
  extent[5] = area[5] - areExtent[4];
}


void vtkColoringBlend::ThreadedRequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector, vtkImageData*** inData, vtkImageData** outData, int extent[6], int threadId)
{
  vtkInformation *inInfo = 
    inputVector[0]->GetInformationObject(0); 
  
  
  vtkInformation* outInfo = 
    outputVector->GetInformationObject(0); 
  vtkImageData *output = vtkImageData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
  
//   char res[250];
//   sprintf(res, "Thread ID: %d: %d %d %d %d %d %d\n",threadId, extent[0], extent[1], extent[2], extent[3],extent[4],extent[5]);
//   std::cout << res;
    
  for (int r = 0; r < m_removeInputs.size(); r++)
  {
    // We have to reset the removed area
    int removeAreaExtent[6];
    if (!intersect(m_removeInputs[r].requestedAreaExtent,extent, removeAreaExtent))
      continue;
    
    for (int i = 0; i < m_blendedInputs.size(); i++)
    {
      // Check if removed area intersect current input
      int inputRemoveAreaExtent[6];
      if (intersect(removeAreaExtent, m_blendedInputs[i].requestedAreaExtent, inputRemoveAreaExtent))
      {
// 	sprintf(res, "Thread ID: %d: RemoveArea: %d %d %d %d %d %d\n",threadId, removeAreaExtent[0], removeAreaExtent[1], removeAreaExtent[2], removeAreaExtent[3],removeAreaExtent[4],removeAreaExtent[5]);
// 	std::cout << res;
// 	sprintf(res, "Thread ID: %d: InputRemoveArea %d: %d %d %d %d %d %d\n",threadId, i, inputRemoveAreaExtent[0], inputRemoveAreaExtent[1], inputRemoveAreaExtent[2], inputRemoveAreaExtent[3],inputRemoveAreaExtent[4],inputRemoveAreaExtent[5]);
// 	std::cout << res;
	int inputRemoveExtent[6];
	// We need to update only the interesecting area
	transformAreaToExtent(inputRemoveAreaExtent, m_blendedInputs[i].requestedAreaExtent, inputRemoveExtent);
	vtkImageData *input  = m_removeInputs[r].image;//vtkImageData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
	vtkImageIterator<InputPixelType> inIt(input, inputRemoveAreaExtent); //WARNING: Problems with vtkreaders...
	vtkImageProgressIterator<OutputPixelType> outIt(output, inputRemoveAreaExtent,this,threadId);
	
	if (i == 0) // First input
	  copyInput(inIt,outIt);
	else
	  blendInputs(inIt,outIt);
      }
    }
  }
  
  for (int i = 0; i < m_newInputs.size(); i++)
  {
    m_newInputs[i].color[0] = 255;//(255*rand())%255;//TODO: Get actual colors
    m_newInputs[i].color[1] = m_newInputs[i].color[2] = 0;
    
    int inputAreaExtent[6];
    if (!intersect(m_newInputs[i].requestedAreaExtent, extent, inputAreaExtent))
      continue;
    
//     sprintf(res, "Thread ID: %d: InputArea %d: %d %d %d %d %d %d\n",threadId, i, inputAreaExtent[0], inputAreaExtent[1], inputAreaExtent[2], inputAreaExtent[3],inputAreaExtent[4],inputAreaExtent[5]);
//     std::cout << res;
    
    int inputExtent[6];
    transformAreaToExtent(inputAreaExtent,m_newInputs[i].requestedAreaExtent,inputExtent);
    vtkImageData *input  = m_newInputs[i].image;//vtkImageData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
    vtkImageIterator<InputPixelType> inIt(input,inputAreaExtent); //WARNING: Problems with vtkreaders...
    vtkImageIterator<OutputPixelType> outIt(output,inputAreaExtent);
    
    blendInputs(inIt, outIt, m_newInputs[i].color);
  }
}

int vtkColoringBlend::RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  int res = vtkThreadedImageAlgorithm::RequestData(request, inputVector, outputVector);

  m_removeInputs.clear();
  m_blendedInputs.insert(m_blendedInputs.end(),m_newInputs.begin(),m_newInputs.end());
  m_newInputs.clear();
  
  return res;
}



void vtkColoringBlend::PrintSelf(ostream& os, vtkIndent indent)
{
    vtkImageAlgorithm::PrintSelf(os, indent);
}


void vtkColoringBlend::requestArea(vtkImageData *inputImage)
{
  Input input;
  
  inputImage->Update();
  
  // Check for duplicate inputs
  for (int i = 0; i < m_blendedInputs.size();i++)
    if (m_blendedInputs[i].image == inputImage)
    {
      vtkDebugMacro(<< "Input already added");
      return;
    }
    
  for (int i = 0; i < m_newInputs.size();i++)
    if (m_newInputs[i].image == inputImage)
    {
      vtkDebugMacro(<< "Input already added");
      return;
    }
  
  // Get input information
  input.image = inputImage;
  inputImage->GetBounds(input.bounds);
  inputImage->GetSpacing(input.spacing);
  inputImage->GetExtent(input.extent);
//   inputImage->GetDimensions(input.dims);
//   input.blended = false;
  for (int i = 0; i<6; i++)
    input.requestedAreaExtent[i] = input.bounds[i] / input.spacing[i/2];
  
  // To force blending of initial image
  if (m_newInputs.size() == 0 && m_blendedInputs.size() == 0 && m_removeInputs.size() == 0)
  {
    // NOTE:Usually, the same input cannot be in two vectors
    m_blendedInputs.push_back(input); 
    m_removeInputs.push_back(input);
  }
  else
  {
    m_newInputs.push_back(input);
  }
  
//   if (!validExtent(m_requestedArea))
//     memcpy(m_requestedArea,input.requestedArea,6*sizeof(int));
//   else
//   {
//     for (int dim = 0; dim < 3; dim++)
//     {
//       m_requestedArea[2*dim] = std::min(m_requestedArea[2*dim],input.requestedArea[2*dim]);
//       m_requestedArea[2*dim+1] = std::max(m_requestedArea[2*dim+1],input.requestedArea[2*dim+1]);
//     }
//   }
}


/*
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
*/
