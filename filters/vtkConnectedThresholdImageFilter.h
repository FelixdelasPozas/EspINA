#ifndef VTK_CONNECTED_THRESHOLD_IMAGE_FILTER_H
#define VTK_CONNECTED_THRESHOLD_IMAGE_FILTER_H

#include "vtkSimpleImageToImageFilter.h"

class VTK_IMAGING_EXPORT vtkConnectedThresholdImageFilter : 
  public vtkSimpleImageToImageFilter
{
public:
    static vtkConnectedThresholdImageFilter *New();
	vtkTypeMacro(vtkConnectedThresholdImageFilter,vtkSimpleImageToImageFilter); 
    
    //! Gray level segmentation threshlod
    vtkSetMacro(m_threshold,double);
    vtkGetMacro(m_threshold,double);
    //! Seed coordinates
    vtkSetVector3Macro(m_seed,int);
    vtkGetVector3Macro(m_seed,int);
    
    void PrintSelf(ostream& os, vtkIndent indent);
    
protected:
    vtkConnectedThresholdImageFilter(){};
    ~vtkConnectedThresholdImageFilter(){};
    
    virtual void SimpleExecute(vtkImageData* input, vtkImageData* output);
    
  virtual int RequestInformation(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector);
    
private:
    vtkConnectedThresholdImageFilter(const vtkSimpleImageToImageFilter& );// Not implemented
    void operator=(const vtkConnectedThresholdImageFilter&);// Not implemented
    
private:
  double m_threshold;
  int m_seed[3];
};

#endif//VTK_CONNECTED_THRESHOLD_IMAGE_FILTER_H
