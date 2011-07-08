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

//! Determine if two regions intersect, and if @intersection array is
//! prodived, return the intersection region
bool intersect(int *reg1, int *reg2, int *intersection = NULL)
{
  int intExt[6];
  
  for (int i = 0; i < 3; i++)
  {
    intExt[i*2] = std::max(reg1[i*2],reg2[i*2]);
    intExt[1+i*2] = std::min(reg1[1+i*2],reg2[1+i*2]);
  }
  
  if (intersection)
    memcpy(intersection,intExt,6*sizeof(int));
  
  return intExt[0] <= intExt[1] && intExt[2] <=intExt[3] && intExt[4] <= intExt[5];
}

//!NOTE: Loading segmentations from mhd and pvd files give different extent values
//! if that's the case, we have to correct the resulting extent
void correctExtent(int *area, vtkColoringBlend::Input &input, int *extent)
{
  bool differentExtent = false;
  for (int i = 0; i < 6; i++)
    differentExtent = differentExtent || input.extent[i] != input.requestedAreaExtent[i];
  
  for (int i = 0; i < 6; i++)
    extent[i] = area[i]- (differentExtent?input.requestedAreaExtent[(i/2)*2]:0);
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
  vtkInformation* outInfo = 
    outputVector->GetInformationObject(0);  
  
  // Specify output scalar data type
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
    
    int updateExtent[6];
    intersect(requestedUpdateExt,inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT()),updateExtent);
    inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), updateExtent,6);
  }
  
  return 1;
}

//! Copies input pixels to output
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

//! Blend input color into output color whenever input pixel is not 0
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
      if (*inPtr)// Non 0 pixel
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

//! Run blending algorithm in the requested extent 
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
  
  // First of all we have to regenerate the removed area using all already 
  // blended inputs in the area
  for (int r = 0; r < m_removeInputs.size(); r++)
  {
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

	// We need to update only the interesecting area
	int inputRemoveExtent[6];
	correctExtent(inputRemoveAreaExtent, m_blendedInputs[i], inputRemoveExtent);
	vtkImageData *input  = m_removeInputs[r].image;//vtkImageData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
	vtkImageIterator<InputPixelType> inIt(input, inputRemoveExtent);
	vtkImageProgressIterator<OutputPixelType> outIt(output, inputRemoveAreaExtent,this,threadId);
	
	if (i == 0) // First input has to be copied, not blended
	  copyInput(inIt,outIt);
	else
	  blendInputs(inIt,outIt);
      }
    }
  }

  // Then, after all removed areas have been regenerated, new inputs have to be blended
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
    correctExtent(inputAreaExtent,m_newInputs[i],inputExtent);
    vtkImageData *input  = m_newInputs[i].image;//vtkImageData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
    vtkImageIterator<InputPixelType> inIt(input,inputExtent);
    vtkImageIterator<OutputPixelType> outIt(output,inputAreaExtent);
    
    blendInputs(inIt, outIt, m_newInputs[i].color);
  }
}

int vtkColoringBlend::RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  // Start threaded execution
  int res = vtkThreadedImageAlgorithm::RequestData(request, inputVector, outputVector);
  // End of thredaed execution
  
  // Internal state of inputs if updated
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
}
