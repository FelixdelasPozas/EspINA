#ifndef VTK_COLORING_BLEND_H
#define VTK_COLORING_BLEND_H

#include "vtkThreadedImageAlgorithm.h"

#include <vector>
#include <vtkImageProgressIterator.h>

//BUG: Fails when using SetInputConnection instead of AddInputConnection

//! This filter blends several label maps into a background image
//! if color are assigned to the label maps, that color is used instead of white.
//! Port 0: Background Image
//! Port 1: Label Maps
//! Label maps images interpret non-zero pixels as foreground and zero pixels as
//! background. Only foreground pixels are blended.
//TODO: Add suppor to several type of input images
class vtkAlgorithmOutput;
class VTK_IMAGING_EXPORT vtkImageLabelMapBlend : 
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
    OutputPixelType color[4];
  };
  //ETX
  
public:
  static vtkImageLabelMapBlend *New();
  vtkTypeMacro(vtkImageLabelMapBlend,vtkThreadedImageAlgorithm);
  
  virtual void AddInputConnection(int port, vtkAlgorithmOutput* input);
  virtual void RemoveInputConnection(int port, vtkAlgorithmOutput* input);
  void RemoveAllInputs();
  
  //! Blending opacity
  vtkSetMacro(Opacity,double);
  vtkGetMacro(Opacity,double);
  //! r,g,b : [0,1]
  void SetLabelMapColor(double id, double r, double g, double b, double selected=0);
  
  void PrintSelf(ostream& os, vtkIndent indent);
    
protected:
  vtkImageLabelMapBlend();
  virtual ~vtkImageLabelMapBlend(){};
  

  virtual int FillInputPortInformation(int port, vtkInformation* info);
  
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
  bool requestArea(vtkImageData* inputImage);
  //BTX
  // Old version, using iterators (couldn't get it to work with modified update_extent)
  void copyInput(vtkImageIterator<InputPixelType> &inIt, vtkImageIterator<OutputPixelType> &outIt);
  // New version using image pointer
  void copyInput(Input *input, vtkImageData *output, int updateArea[6]);
  // Old version, using iterators (couldn't get it to work with modified update_extent)
  void blendInputs(vtkImageIterator<InputPixelType> &inIt, vtkImageIterator<OutputPixelType> &outIt, OutputPixelType *color=NULL);
  // New version using image pointer
  void blendInput(Input *input, vtkImageData *output, int updateArea[6]);
  //ETX
    
private:
  vtkImageLabelMapBlend(const vtkImageLabelMapBlend& );// Not implemented
  void operator=(const vtkImageLabelMapBlend&);// Not implemented
    
private:
  //BTX
  double Opacity;
  bool m_init;
  double m_debugProcessedPixels;
  Input m_background;
  std::vector<Input *> m_newInputs;
  std::vector<Input *> m_blendedInputs;
  std::vector<Input *> m_removeInputs;
  std::vector<Input *> m_inputs;
  //ETX
};

#endif//VTK_COLORING_BLEND_H
