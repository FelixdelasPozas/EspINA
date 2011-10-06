#include "vtkImageLabelMapBlend.h"

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
#include <algorithm>

#include <iostream>
#include <vtkAlgorithmOutput.h>

#include <cassert>
#include <vtkStreamingDemandDrivenPipeline.h>

typedef unsigned long long ImageOffset;

vtkStandardNewMacro(vtkImageLabelMapBlend);

// bool validExtent(int *ext)
// {
//  return ext[1] > ext[0];
// }

void printExtent(int extent[6])
{
    std::cout << 
    extent[0] <<  " " << extent[1] <<  " " << 
    extent[2] <<  " " << extent[3] <<  " " << 
    extent[4] <<  " " << extent[5] <<  " " << 
    std::endl;
}

vtkImageLabelMapBlend::vtkImageLabelMapBlend()
: Opacity(0.6)
, m_init(false)
, m_debugProcessedPixels(0)
{
  //! Port 0: Blending inputs
  //! Port 1: Reference colors
  this->SetNumberOfInputPorts(2);
  this->SetNumberOfOutputPorts(1);
  m_newInputs.clear();
  m_removeInputs.clear();
  m_blendedInputs.clear();
  m_inputs.clear();
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
void correctExtent(int *area, vtkImageLabelMapBlend::Input &input, int *extent)
{
  bool differentExtent = false;
  for (int i = 0; i < 6; i++)
    differentExtent = differentExtent || input.extent[i] != input.requestedAreaExtent[i];
  
  for (int i = 0; i < 6; i++)
    extent[i] = area[i]- (differentExtent?input.requestedAreaExtent[(i/2)*2]:0);
}


void vtkImageLabelMapBlend::AddInputConnection(int port, vtkAlgorithmOutput* input)
{
  if (port == 0)
  {
    
    vtkImageData *inputImage = vtkImageData::SafeDownCast(
      input->GetProducer()->GetOutputDataObject(input->GetIndex()));
    
    if (requestArea(inputImage))
      vtkAlgorithm::AddInputConnection(port, input);
  }
}

void vtkImageLabelMapBlend::RemoveInputConnection(int port, vtkAlgorithmOutput* input)
{
  if (port == 0)
  {
    vtkImageData *inputImage = vtkImageData::SafeDownCast(
      input->GetProducer()->GetOutputDataObject(0));
    
    // Find inputs and move to removeInputs vector
    int numDeletions = 0;
    
    std::vector<Input *>::iterator it;
    it = m_inputs.begin();
    while (it != m_inputs.end())
    { 
      if ((*it)->image == inputImage)
      {
	it = m_inputs.erase(it);
	numDeletions++;
      }
      else
	it++;
    }
    
    it = m_blendedInputs.begin();
    while (it != m_blendedInputs.end())
    { 
      if ((*it)->image == inputImage)
      {
	m_removeInputs.push_back(*it);
	it = m_blendedInputs.erase(it);
	numDeletions++;
      }
      else
	it++;
    }

    it = m_newInputs.begin();
    while (it != m_newInputs.end())
    { 
      if ((*it)->image == inputImage)
      {
	m_removeInputs.push_back(*it);
	it = m_newInputs.erase(it);
	numDeletions++;
      }
      else
	it++;
    }
    if (numDeletions != 2)
      vtkDebugMacro(<< "WARNING: Input has been removed " << numDeletions << " times.");
  }
  
  vtkAlgorithm::RemoveInputConnection(port, input);
}

void vtkImageLabelMapBlend::RemoveAllInputs()
{
  // If there were already blended areas, those should be marked to be removed
  //TODO: Memory leak...
  m_removeInputs.insert(m_removeInputs.end(),m_blendedInputs.begin(),m_blendedInputs.end());
  m_blendedInputs.clear();
  // New inputs haven't already been blended so we can discard them safely
  m_newInputs.clear();
  m_inputs.clear();
  vtkThreadedImageAlgorithm::RemoveAllInputs();
}

void vtkImageLabelMapBlend::SetLabelMapColor(double id, double r, double g, double b, double selected)
{
  assert(id < m_inputs.size());
  double rComponent = r*255;
  double gComponent = g*255;
  double bComponent = b*255;
  bool isSelected = selected;
  if ((m_inputs[id]->color[0] != rComponent 
    || m_inputs[id]->color[1] != gComponent 
    || m_inputs[id]->color[2] != bComponent
    || m_inputs[id]->color[3] != isSelected
  )
    && rComponent >= 0)
  {
    m_inputs[id]->color[0] = rComponent;
    m_inputs[id]->color[1] = gComponent;
    m_inputs[id]->color[2] = bComponent;
    m_inputs[id]->color[3] = isSelected;
    
    // If it was already blended we have to reset its area
    std::vector<Input *>::iterator it = m_blendedInputs.begin();
    while (it != m_blendedInputs.end())
    { 
      if ((*it)->image ==  m_inputs[id]->image)
      {
	// Regenerate all its region
	m_removeInputs.push_back(*it);
	// and we want to paint it as well
	m_newInputs.push_back(*it);
	m_blendedInputs.erase(it);
	break;
      }
      it++;
    }
  }
}





int vtkImageLabelMapBlend::FillInputPortInformation(int port, vtkInformation* info)
{
  if (port == 0) // Any number volume volume images
  {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkImageData");
    info->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 1);
    return 1;
  }
  else if (port == 1)// Reference Colors
  {
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1); //WARNING: Make it mandatory?
    info->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 1);
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
    return 1;
  }
  else
  {
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1); //WARNING: Make it mandatory?
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkImageData");
    info->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 1);
    return 1;
  }

  vtkErrorMacro("This filter does not have more than 3 input port!");
  return 0;
}

int vtkImageLabelMapBlend::RequestInformation(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
//     std::cout << "Request Info" << std::endl;
  // get the info objects
  vtkInformation* outInfo = 
    outputVector->GetInformationObject(0);  
  
  // Specify output scalar data type
  vtkDataObject::SetPointDataActiveScalarInfo(
    outInfo, VTK_UNSIGNED_CHAR, 3);
  
  return 1;
}

// This method computes the input necessary to generate the output
int vtkImageLabelMapBlend::RequestUpdateExtent(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
//     std::cout << "Request Update Extent" << std::endl;
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
  
  int requiredUpdateArea[6];
  bool validArea = false;
  
  if (m_newInputs.size() > 0)
  {
    Input *input = m_newInputs[0];
    memcpy(requiredUpdateArea,input->requestedAreaExtent,6*sizeof(int));
    for (std::size_t i=1; i < m_newInputs.size(); i++)
    {
      for (int d=0; d<3; d++)
      {
	requiredUpdateArea[2*d] = std::min(requiredUpdateArea[2*d],m_newInputs[i]->requestedAreaExtent[2*d]);
	requiredUpdateArea[2*d+1] = std::max(requiredUpdateArea[2*d+1],m_newInputs[i]->requestedAreaExtent[2*d+1]);
      }
    }
    validArea = true;
  } else if (m_removeInputs.size() > 0)
  {
    if (!validArea)
    {
      Input *input = m_removeInputs[0];
      memcpy(requiredUpdateArea,input->requestedAreaExtent,6*sizeof(int));
    }
    for (int i=1; i < m_removeInputs.size(); i++)
    {
      for (int d=0; d<3; d++)
      {
	requiredUpdateArea[2*d] = std::min(requiredUpdateArea[2*d],m_removeInputs[i]->requestedAreaExtent[2*d]);
	requiredUpdateArea[2*d+1] = std::max(requiredUpdateArea[2*d+1],m_removeInputs[i]->requestedAreaExtent[2*d+1]);
      }
    }
    validArea = true;
  } else if (m_blendedInputs.size() > 0 && !validArea)
  {
    Input *input = m_blendedInputs[0];
    memcpy(requiredUpdateArea,input->requestedAreaExtent,6*sizeof(int));
  }
  outInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(),
	       requiredUpdateArea,
	       6);
  
  return 1;
}

//! Copies input pixels to output
//! DEPRECATED: Problems while changing update extent
void vtkImageLabelMapBlend::copyInput(vtkImageIterator< vtkImageLabelMapBlend::InputPixelType >& inIt, vtkImageIterator< vtkImageLabelMapBlend::OutputPixelType >& outIt)
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
      m_debugProcessedPixels++;
      ++inPtr;
    }
    inIt.NextSpan();
    outIt.NextSpan();
  }

}

//! Copies input pixels to output
void vtkImageLabelMapBlend::copyInput(vtkImageLabelMapBlend::Input* input, vtkImageData* output, int updateArea[6])
{
  int outDim[3], inDim[3];
  //output->GetDimensions(dim); //dimensions of defined update_extent
  Input *mainInput = m_blendedInputs[0];
  for (int d = 0; d < 3; d++)
  {
    outDim[d] = mainInput->extent[2*d+1] - mainInput->extent[2*d] + 1;
    inDim[d] = input->extent[2*d+1] - input->extent[2*d] + 1;
  }
  
  // Input must be monocrome, thus we don't need numComponents
  int numComponets = output->GetNumberOfScalarComponents();
  
  unsigned char *outputPtr = static_cast<unsigned char *>(output->GetScalarPointer());
  unsigned char *inputPtr = static_cast<unsigned char *>(input->image->GetScalarPointer());
  
//   std::cout << "Copying Image with Lower Extent: " << input->extent[0] << " " << input->extent[1] << " " << input->extent[2] << " " << std::endl;
//   std::cout << "Copying Image with Update Area: " << updateArea[0] << " " << updateArea[1] << " " << updateArea[2] << " " << std::endl;
  
  
  for (unsigned int z = updateArea[4]; z <= updateArea[5]; z++)
  {
    ImageOffset zInOffset = (z-input->extent[4])* inDim[0]* inDim[1]; 
    ImageOffset zOutOffset = numComponets * z * outDim[0] * outDim[1];
    for (unsigned int y = updateArea[2]; y <= updateArea[3]; y++)
    {
      ImageOffset yInOffset = (y-input->extent[2])* inDim[0];
      ImageOffset yOutOffset = numComponets * y * outDim[0];
      for (unsigned int x = updateArea[0]; x <= updateArea[1]; x++)
      {
	ImageOffset inPix = (x-input->extent[0]) + yInOffset + zInOffset;
	ImageOffset outPix = numComponets*x + yOutOffset + zOutOffset;
	
	for (int c = 0; c < numComponets; c++)
	  outputPtr[outPix+c] = inputPtr[inPix];
      }
    }
  }
}


//! Blend input color into output color whenever input pixel is not 0
//! DEPRECATED: Problems while changing update extent
void vtkImageLabelMapBlend::blendInputs(vtkImageIterator<InputPixelType> &inIt, vtkImageIterator<OutputPixelType> &outIt, OutputPixelType *color)
{
  InputPixelType *inPtr;// = (InputPixelType*)input->GetScalarPointer();
  OutputPixelType *outPtr;// = (OutputPixelType *)(output->GetScalarPointer());
  
  while (!inIt.IsAtEnd() && !outIt.IsAtEnd())
  {
    // Process one row of pixles at a time
    inPtr = inIt.BeginSpan();
    InputPixelType *inEndPtr = inIt.EndSpan();
    outPtr = outIt.BeginSpan();
    OutputPixelType *outEndPtr = outIt.EndSpan();
    
    int spacing = 0;
    while (inPtr && inPtr != inEndPtr && outPtr != outEndPtr)
    {
      if (*inPtr)// Non 0 pixel
      {
	spacing = (spacing+1)%2;
	for (int c = 0; c < 3; c++)
	{
	  int shiftComponent = (spacing == 0 && color[3])?(c+1)%4:c;
	  double r = Opacity;//constant opacity
	  double f = 1.0 - r;
	  *outPtr = *outPtr*f + color[shiftComponent]*r;
	  ++outPtr;
	}
      }else
      {
	outPtr += 3;
      }
      m_debugProcessedPixels++;
      ++inPtr;
    }
    inIt.NextSpan();
    outIt.NextSpan();
  }
}

// bool isBorderPixel(int x, int y, int z, int dim[3], int extent[6])
// {
// }

//! Blend input color into output color whenever input pixel is not 0
void vtkImageLabelMapBlend::blendInput(vtkImageLabelMapBlend::Input* input, vtkImageData* output, int updateArea[6])
{
  int outDim[3], inDim[3];
  //output->GetDimensions(dim); //dimensions of defined update_extent
  Input *mainInput = m_blendedInputs[0];
  for (int d = 0; d < 3; d++)
  {
    outDim[d] = mainInput->extent[2*d+1] - mainInput->extent[2*d] + 1;
    inDim[d] = input->extent[2*d+1] - input->extent[2*d] + 1;
  }
  
  int numComponets = output->GetNumberOfScalarComponents();
  
  unsigned char *outputPtr = static_cast<unsigned char *>(output->GetScalarPointer());
  unsigned char *inputPtr = static_cast<unsigned char *>(input->image->GetScalarPointer());
  
  bool isSelected = input->color[3];
  
//   std::cout << "Blending Image with Lower Extent: " << input->extent[0] << " " << input->extent[1] << " " << input->extent[2] << " " << std::endl;
//   std::cout << "Blending Image with Update Area: " << updateArea[0] << " " << updateArea[1] << " " << updateArea[2] << " " << std::endl;
  
  for (unsigned int z = updateArea[4]; z <= updateArea[5]; z++)
  {
    ImageOffset zInOffset = (z-input->extent[4])* inDim[0]* inDim[1]; 
    ImageOffset zOutOffset = numComponets * z * outDim[0] * outDim[1];
    for (unsigned int y = updateArea[2]; y <= updateArea[3]; y++)
    {
      ImageOffset yInOffset = (y-input->extent[2])* inDim[0];
      ImageOffset yOutOffset = numComponets * y * outDim[0];
      for (unsigned int x = updateArea[0]; x <= updateArea[1]; x++)
      {
	ImageOffset inPix = (x-input->extent[0]) + yInOffset + zInOffset;
	ImageOffset outPix = numComponets*x + yOutOffset + zOutOffset;
	
	if (inputPtr[inPix]) //Non 0 pixel
	{
	  for (int c = 0; c < numComponets; c++)
	  {
	    double r = Opacity + (isSelected?0.4:0);//constant opacity
	    double f = 1.0 - r;
	    outputPtr[outPix+c] = outputPtr[outPix+c]*f + input->color[c]*r;
	  }
	}
      }// X loop
    }// Y loop
  }// Z loop
}


//! Run blending algorithm in the requested extent 
void vtkImageLabelMapBlend::ThreadedRequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector, vtkImageData*** inData, vtkImageData** outData, int extent[6], int threadId)
{
//   std::cout << "Request Threaded Data" << std::endl;
  vtkInformation *inInfo = 
    inputVector[0]->GetInformationObject(0); 
  
  vtkInformation* outInfo = 
    outputVector->GetInformationObject(0); 
  vtkImageData *output = vtkImageData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
  
  m_debugProcessedPixels = 0;
//   char res[250];
//   sprintf(res, "Thread ID: %d: %d %d %d %d %d %d\n",threadId, extent[0], extent[1], extent[2], extent[3],extent[4],extent[5]);
//   std::cout << res;
  
  // First of all we have to regenerate the removed area using all already 
  // blended inputs in the area
  for (unsigned int r = 0; r < m_removeInputs.size(); r++)
  {
//     std::cout << "Regenerate Blender" << std::endl;
    int removeAreaExtent[6];
    if (!intersect(m_removeInputs[r]->requestedAreaExtent,extent, removeAreaExtent))
      continue;
    
    for (unsigned int i = 0; i < m_blendedInputs.size(); i++)
    {
      // Check if removed area intersect current input
      int inputRemoveAreaExtent[6];
      if (intersect(removeAreaExtent, m_blendedInputs[i]->requestedAreaExtent, inputRemoveAreaExtent))
      {
// 	sprintf(res, "Thread ID: %d: RemoveArea: %d %d %d %d %d %d\n",threadId, removeAreaExtent[0], removeAreaExtent[1], removeAreaExtent[2], removeAreaExtent[3],removeAreaExtent[4],removeAreaExtent[5]);
// 	std::cout << res;
// 	sprintf(res, "Thread ID: %d: InputRemoveArea %d: %d %d %d %d %d %d\n",threadId, i, inputRemoveAreaExtent[0], inputRemoveAreaExtent[1], inputRemoveAreaExtent[2], inputRemoveAreaExtent[3],inputRemoveAreaExtent[4],inputRemoveAreaExtent[5]);
// 	std::cout << res;

	// We need to update only the interesecting area
	int inputRemoveExtent[6];
	correctExtent(inputRemoveAreaExtent, *m_blendedInputs[i], inputRemoveExtent);
	vtkImageData *input  = m_blendedInputs[i]->image;//vtkImageData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
	vtkImageIterator<InputPixelType> inIt(input, inputRemoveExtent);
	vtkImageProgressIterator<OutputPixelType> outIt(output, inputRemoveAreaExtent,this,threadId);
	
// 	std::cout << "Regenerating area ";
// 	printExtent(inputRemoveAreaExtent);
	
	if (i == 0) // First input has to be copied, not blended
	  copyInput(m_blendedInputs[0],output,inputRemoveAreaExtent);
	else
	  blendInput(m_blendedInputs[i],output,inputRemoveAreaExtent);
// 	  blendInputs(inIt,outIt,m_blendedInputs[i]->color);
      }
    }
  }

  // Then, after all removed areas have been regenerated, new inputs have to be blended
  for (unsigned int i = 0; i < m_newInputs.size(); i++)
  {
//     std::cout << "Update Blender" << std::endl;
    int inputAreaExtent[6];
//     std::cout << "New Input area ";
//     printExtent(m_newInputs[i]->requestedAreaExtent);
//     std::cout << "New Input Extent ";
//     printExtent(m_newInputs[i]->extent);
//     std::cout << "Request Data Extent ";
//     printExtent(extent);
    if (!intersect(m_newInputs[i]->requestedAreaExtent, extent, inputAreaExtent))
      continue;
//     std::cout << "Updating area ";
//     printExtent(inputAreaExtent);
    
//     sprintf(res, "Thread ID: %d: InputArea %d: %d %d %d %d %d %d\n",threadId, i, inputAreaExtent[0], inputAreaExtent[1], inputAreaExtent[2], inputAreaExtent[3],inputAreaExtent[4],inputAreaExtent[5]);
//     std::cout << res;
    int inputExtent[6];
    correctExtent(inputAreaExtent,*m_newInputs[i],inputExtent);
    vtkImageData *input  = m_newInputs[i]->image;//vtkImageData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
    vtkImageIterator<InputPixelType> inIt(input,inputExtent);
    //vtkImageIterator<OutputPixelType> outIt(output,inputAreaExtent);//Doesn't work if we change the update extent...
	
//     blendInputs(inIt, outIt, m_newInputs[i]->color);
    blendInput(m_newInputs[i], output,inputAreaExtent);
    
  }
  
  
  
//   std::cout << "\n\t\tCOLORING BLENDER: " << m_debugProcessedPixels << " pixel processed\n\n" << std::endl;
}

int vtkImageLabelMapBlend::RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
//   std::cout << "Request Data" << std::endl;
/*  
  vtkInformation* outInfo = outputVector->GetInformationObject(0); 
  if (m_newInputs.size() > 0)
  {
    std::cout << "Updating new Extent ";
    Input *input = m_newInputs[0];
    outInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(),
		 input->requestedAreaExtent,
		 6);
  } else if (m_blendedInputs.size() > 0)
  {
    std::cout << "Updating zero Extent ";
    outInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(),
		 m_blendedInputs[0]->requestedAreaExtent,
		 6);
  }*/
  // Start threaded execution
  int res = vtkThreadedImageAlgorithm::RequestData(request, inputVector, outputVector);
  // End of thredaed execution
  
//   std::cout << "\t\t\t\tUpdating\n";
  
  // Internal state of inputs if updated
  m_removeInputs.clear();
  m_blendedInputs.insert(m_blendedInputs.end(),m_newInputs.begin(),m_newInputs.end());
  m_newInputs.clear();
  
  return res;
}

void vtkImageLabelMapBlend::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkImageAlgorithm::PrintSelf(os, indent);
}

bool vtkImageLabelMapBlend::requestArea(vtkImageData *inputImage)
{
  Input *input = new Input();;
  
  inputImage->Update();
  
  // Check for duplicate inputs
  for (std::size_t i = 0; i < m_blendedInputs.size();i++)
    if (m_blendedInputs[i]->image == inputImage)
    {
      vtkDebugMacro(<< "Input already added");
      return false;
    }
  
  for (unsigned int i = 0; i < m_newInputs.size();i++)
    if (m_newInputs[i]->image == inputImage)
    {
      vtkDebugMacro(<< "Input already added");
      return false;
    }
  
  for (std::vector<Input *>::iterator it = m_removeInputs.begin(); it != m_removeInputs.end(); it++)
    if ((*it)->image == inputImage)
    {
      vtkDebugMacro(<< "Input was previously blended");
      m_blendedInputs.push_back(*it);
      m_inputs.push_back(*it);
      m_removeInputs.erase(it);
      return true;
    }
  
  // Get input information
  input->image = inputImage;
  inputImage->GetBounds(input->bounds);
  inputImage->GetSpacing(input->spacing);
  inputImage->GetExtent(input->extent);
  //   inputImage->GetDimensions(input.dims);
  //   input.blended = false;
  for (int i = 0; i<6; i++)
    input->requestedAreaExtent[i] = input->extent[i];// NOTE: before: input->bounds[i] / input->spacing[i/2]; but it crashed due to rounding
   
//   for (int i = 0; i<3; i++)
//   {
//     input->requestedAreaExtent[2*i] = std::max(int(input->bounds[2*i] / input->spacing[i]), input->extent[2*i]);
//     input->requestedAreaExtent[2*i+1] = std::min(int(input->bounds[2*i+1] / input->spacing[i]), input->extent[2*i+1]);
//   }
  
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
 m_inputs.push_back(input);
 return true;
}
