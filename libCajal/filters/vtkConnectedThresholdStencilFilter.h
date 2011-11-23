#ifndef VTK_CONNECTED_THRESHOLD_STENCIL_FILTER_H
#define VTK_CONNECTED_THRESHOLD_STENCIL_FILTER_H

#include "vtkImageAlgorithm.h"

class VTK_IMAGING_EXPORT vtkConnectedThresholdStencilFilter : 
  public vtkImageAlgorithm
{
public:
  static vtkConnectedThresholdStencilFilter *New();
  vtkTypeMacro(vtkConnectedThresholdStencilFilter,vtkImageAlgorithm);
    
  //! Gray level segmentation threshlod
  vtkSetMacro(m_threshold,double);
  vtkGetMacro(m_threshold,double);
  //! Seed coordinates
  vtkSetVector3Macro(m_seed,int);
  vtkGetVector3Macro(m_seed,int);
    
  void PrintSelf(ostream& os, vtkIndent indent);
    
protected:

  vtkConnectedThresholdStencilFilter();
  ~vtkConnectedThresholdStencilFilter(){};

  virtual int RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector);
//   virtual int RequestInformation(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector);
    
private:
    vtkConnectedThresholdStencilFilter(const vtkConnectedThresholdStencilFilter& );// Not implemented
  void operator=(const vtkConnectedThresholdStencilFilter&);// Not implemented
    
private:
  double m_threshold;
  int m_seed[3];
};

#endif//VTK_CONNECTED_THRESHOLD_STENCIL_FILTER_H
