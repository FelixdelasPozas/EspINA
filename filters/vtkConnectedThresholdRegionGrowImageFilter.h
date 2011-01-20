#ifndef VTK_CONNECTED_THRESHOLD_REGION_GROW_IMAGE_FILTER_H
#define VTK_CONNECTED_THRESHOLD_REGION_GROW_IMAGE_FILTER_H

#include "vtkSimpleImageToImageFilter.h"

class VTK_IMAGING_EXPORT vtkConnectedThresholdRegionGrowImageFilter : 
  public vtkSimpleImageToImageFilter
{
public:
    static vtkConnectedThresholdRegionGrowImageFilter *New();
    
    //! Gray level segmentation threshlod
    vtkSetMacro(m_threshold,double);
    vtkGetMacro(m_threshold,double);
    //! Seed coordinates
    vtkSetVector3Macro(m_seed,int);
    vtkGetVector3Macro(m_seed,int);
    
    void PrintSelf(ostream& os, vtkIndent indent);
    
protected:
    vtkConnectedThresholdRegionGrowImageFilter(){};
    ~vtkConnectedThresholdRegionGrowImageFilter(){};
    
    virtual void SimpleExecute(vtkImageData* input, vtkImageData* output);
    
private:
    vtkConnectedThresholdRegionGrowImageFilter(const vtkSimpleImageToImageFilter& );// Not implemented
    void operator=(const vtkConnectedThresholdRegionGrowImageFilter&);// Not implemented
    
private:
  double m_threshold;
  int m_seed[3];
};

#endif//VTK_CONNECTED_THRESHOLD_REGION_GROW_IMAGE_FILTER_H
