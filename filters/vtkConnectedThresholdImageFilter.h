#ifndef VTK_CONNECTED_THRESHOLD_IMAGE_FILTER_H
#define VTK_CONNECTED_THRESHOLD_IMAGE_FILTER_H

#include "vtkImageAlgorithm.h"

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
  double m_threshold;
  int m_seed[3];
};

#endif//VTK_CONNECTED_THRESHOLD_IMAGE_FILTER_H
