#ifndef VTK_CONNECTED_THRESHOLD_IMAGE_FILTER_H
#define VTK_CONNECTED_THRESHOLD_IMAGE_FILTER_H

#include <vtkImageAlgorithm.h>

class VTK_IMAGING_EXPORT vtkConnectedThresholdImageFilter : 
  public vtkImageAlgorithm
{
public:
  static vtkConnectedThresholdImageFilter *New();
  vtkTypeMacro(vtkConnectedThresholdImageFilter,vtkImageAlgorithm);
    
  //! Gray level segmentation threshlod
  vtkSetMacro(Threshold,int);
  vtkGetMacro(Threshold,int);
  //! Seed coordinates
  vtkSetVector3Macro(m_seed,int);
  vtkGetVector3Macro(m_seed,int);

  void SetCheckPixel(int x, int y, int z, int value);
  void SetCheckPixel(int arg[4]);
  vtkGetVector4Macro(CheckPixel,int);
  
  vtkGetMacro(PixelValue,int);

  vtkGetVector6Macro(SegExtent,int);
  
  void PrintSelf(ostream& os, vtkIndent indent);
    
protected:

  vtkConnectedThresholdImageFilter();
  ~vtkConnectedThresholdImageFilter(){};

  virtual int RequestInformation(vtkInformation* request,
				 vtkInformationVector** inputVector,
				 vtkInformationVector* outputVector);
  virtual int RequestData(vtkInformation* request,
			  vtkInformationVector** inputVector,
			  vtkInformationVector* outputVector);
    
private:
  vtkConnectedThresholdImageFilter(const vtkConnectedThresholdImageFilter& );// Not implemented
  void operator=(const vtkConnectedThresholdImageFilter&);// Not implemented
    
private:
  int Threshold;
  int m_seed[3];
  int CheckPixel[4];
  int PixelValue;
  int SegExtent[6];
  //BTX
  vtkImageData *m_data;
  //ETX
  
};

#endif//VTK_CONNECTED_THRESHOLD_IMAGE_FILTER_H
