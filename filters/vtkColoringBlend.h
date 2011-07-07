#ifndef VTK_COLORING_BLEND_H
#define VTK_COLORING_BLEND_H

#include "vtkThreadedImageAlgorithm.h"

#include <vector>
#include <vtkImageProgressIterator.h>

//! Blend images together using alpha and coloring
//! inputs according to reference colors

//BUG: Fails when using SetInputConnection instead of AddInputConnection
  
class vtkAlgorithmOutput;
class VTK_IMAGING_EXPORT vtkColoringBlend : 
  public vtkThreadedImageAlgorithm
{
  //BTX
  typedef unsigned char InputPixelType;
  typedef unsigned char OutputPixelType;
  
public:
  struct Input
  {
    vtkImageData *image;
//     int dims[3];
    int extent[6]; //Image pixel limits
    int requestedAreaExtent[6]; //
    double bounds[6];
    double spacing[3];
    OutputPixelType color[3];
  };
  //ETX
  
public:
  static vtkColoringBlend *New();
  vtkTypeMacro(vtkColoringBlend,vtkThreadedImageAlgorithm);
    
  virtual void AddInputConnection(int port, vtkAlgorithmOutput* input);
  
  void PrintSelf(ostream& os, vtkIndent indent);
    
protected:

  vtkColoringBlend();
  virtual ~vtkColoringBlend(){};
  

  virtual int FillInputPortInformation(int port, vtkInformation* info);
//   virtual int RequestData(vtkInformation* request,
// 			  vtkInformationVector** inputVector,
// 			  vtkInformationVector* outputVector);
  
  virtual int RequestInformation(vtkInformation* request,
				 vtkInformationVector** inputVector,
				 vtkInformationVector* outputVector);
  
  virtual int RequestUpdateExtent(vtkInformation* request,
				  vtkInformationVector** inputVector,
				  vtkInformationVector* outputVector);  
  
  virtual void ThreadedRequestData(vtkInformation* request,
				   vtkInformationVector** inputVector,
				   vtkInformationVector* outputVector,
				   vtkImageData*** inData,
				   vtkImageData** outData,
				   int extent[6],
				   int threadId);
  virtual int RequestData(vtkInformation* request,
			  vtkInformationVector** inputVector,
			  vtkInformationVector* outputVector);
  
private:
  void requestArea(vtkImageData* inputImage);
  //BTX
  void copyInput(vtkImageIterator<InputPixelType> &inIt, vtkImageIterator<OutputPixelType> &outIt);
  void blendInputs(vtkImageIterator<InputPixelType> &inIt, vtkImageIterator<OutputPixelType> &outIt, OutputPixelType *color=NULL);
  //ETX
    
private:
  vtkColoringBlend(const vtkColoringBlend& );// Not implemented
  void operator=(const vtkColoringBlend&);// Not implemented
    
private:
  //BTX
  bool m_init;
  std::vector<Input> m_newInputs;
  std::vector<Input> m_blendedInputs;
  std::vector<Input> m_removeInputs;
  //ETX
};

#endif//VTK_COLORING_BLEND_H
