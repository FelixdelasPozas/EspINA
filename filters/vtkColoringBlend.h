#ifndef VTK_COLORING_BLEND_H
#define VTK_COLORING_BLEND_H

#include "vtkImageAlgorithm.h"
#include <vector>

//! Blend images together using alpha and coloring
//! inputs according to reference colors

//BUG: Fails when using SetInputConnection instead of AddInputConnection
  
class vtkAlgorithmOutput;
class VTK_IMAGING_EXPORT vtkColoringBlend : 
  public vtkImageAlgorithm
{
  typedef unsigned char InputPixelType;
  typedef unsigned char OutputPixelType;
  
  struct Input
  {
    vtkImageData *image;
    InputPixelType *ptr;
    int dims[3];
    int extent[6];
    int requestedArea[6];
    double bounds[6];
    double spacing[3];
    OutputPixelType color[3];
  };
  
public:
  static vtkColoringBlend *New();
  vtkTypeMacro(vtkColoringBlend,vtkImageAlgorithm);
    
  virtual void AddInputConnection(int port, vtkAlgorithmOutput* input);
  
  void PrintSelf(ostream& os, vtkIndent indent);
    
protected:

  vtkColoringBlend();
  virtual ~vtkColoringBlend(){};
  

  virtual int FillInputPortInformation(int port, vtkInformation* info);
  virtual int RequestData(vtkInformation* request,
			  vtkInformationVector** inputVector,
			  vtkInformationVector* outputVector);
  virtual int RequestInformation(vtkInformation* request,
				 vtkInformationVector** inputVector,
				 vtkInformationVector* outputVector);
private:
  void invalidateRequestArea();
  void requestArea(vtkImageData* inputImage);
    
private:
  vtkColoringBlend(const vtkColoringBlend& );// Not implemented
  void operator=(const vtkColoringBlend&);// Not implemented
    
private:
  bool m_init;
  int m_numBlendedInputs;
  int m_requestedArea[6];
  std::vector<Input> m_inputs;
};

#endif//VTK_COLORING_BLEND_H
