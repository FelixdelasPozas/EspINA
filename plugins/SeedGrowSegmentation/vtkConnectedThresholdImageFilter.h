#ifndef VTK_CONNECTED_THRESHOLD_IMAGE_FILTER_H
#define VTK_CONNECTED_THRESHOLD_IMAGE_FILTER_H

#include <vtkImageAlgorithm.h>

  //BTX
#define vtkSetVector4Macro(name,type) \
virtual void Set##name (type _arg1, type _arg2, type _arg3, type _arg4); \
virtual void Set##name (type _arg[4]);
  //ETX

class VTK_IMAGING_EXPORT vtkConnectedThresholdImageFilter : 
  public vtkImageAlgorithm
{
public:
  static vtkConnectedThresholdImageFilter *New();
  vtkTypeMacro(vtkConnectedThresholdImageFilter,vtkImageAlgorithm);
    
  //! Gray level segmentation threshlod
  vtkSetMacro(m_threshold,double);
  vtkGetMacro(m_threshold,double);
  //! Seed coordinates
  vtkSetVector3Macro(m_seed,int);
  vtkGetVector3Macro(m_seed,int);
  
  vtkSetVector4Macro(CheckPixel,int);
  
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
  //BTX
  double m_threshold;
  int m_seed[3];
  int CheckPixel[4];
  int PixelValue;
  int SegExtent[6];
  vtkImageData *m_data;
  //ETX
  
};

#endif//VTK_CONNECTED_THRESHOLD_IMAGE_FILTER_H
