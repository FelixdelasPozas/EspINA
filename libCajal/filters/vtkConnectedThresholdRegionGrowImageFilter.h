#ifndef VTK_CONNECTED_THRESHOLD_IMAGE_FILTER_H
#define VTK_CONNECTED_THRESHOLD_IMAGE_FILTER_H

#include "vtkImageAlgorithm.h"

class VTK_IMAGING_EXPORT vtkConnectedThresholdRegionGrowImageFilter :
  public vtkImageAlgorithm
{
public:
  static vtkConnectedThresholdRegionGrowImageFilter *New();
  vtkTypeMacro(vtkConnectedThresholdRegionGrowImageFilter,vtkImageAlgorithm);
    
  //! Gray level segmentation threshlod
  vtkSetMacro(m_threshold,double);
  vtkGetMacro(m_threshold,double);
  //! Seed coordinates
  vtkSetVector3Macro(m_seed,int);
  vtkGetVector3Macro(m_seed,int);
    
  void PrintSelf(ostream& os, vtkIndent indent);
    
protected:
  vtkConnectedThresholdRegionGrowImageFilter();
  ~vtkConnectedThresholdRegionGrowImageFilter(){};

  virtual int RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector);
//   virtual int RequestInformation(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector);
    
private:
  vtkConnectedThresholdRegionGrowImageFilter(const vtkConnectedThresholdRegionGrowImageFilter& );// Not implemented
  void operator=(const vtkConnectedThresholdRegionGrowImageFilter&);// Not implemented
    
private:
  double m_threshold;
  
  int m_seed[3];
  
};

#endif//VTK_CONNECTED_THRESHOLD_IMAGE_FILTER_H
